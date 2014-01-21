#pragma once
/*
 *      Copyright (C) 2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <set>
#include <string>

#include "media/MediaType.h"
#include "media/import/MediaImport.h"
#include "media/import/MediaImportSource.h"
#include "media/import/IMediaImportTask.h"

class CFileItemList;

// job is deleted after work is done, so we need proxy job to trigger IMediaImport::DoImport
class CMediaImportSourceRegistrationTask : public IMediaImportTask
{
public:
  CMediaImportSourceRegistrationTask(const CMediaImportSource &source);
  virtual ~CMediaImportSourceRegistrationTask() { }

  /*!
   * \brief Returns the source to be registered/added.
   */
  const CMediaImportSource& GetImportSource() const { return m_source; }

  /*!
   * \brief Returns the list of imports from the registered source that need
   * to be added.
   */
  const std::vector<CMediaImport>& GetImports() const { return m_imports; }

  static std::set<MediaType> ShowAndGetMediaTypesToImport(const std::string &sourceId, bool allowMultipleImports = true);
  static std::set<MediaType> ShowAndGetMediaTypesToImport(const CMediaImportSource &source, bool allowMultipleImports = true);

  static bool GetMediaTypesToImport(const std::set<MediaType> &availableMediaTypes, CFileItemList &items);

  // implementation of CJob
  virtual bool DoWork();
  virtual const char *GetType() const { return "MediaImportSourceRegistrationTask"; };

protected:
  CMediaImportSource m_source;
  std::vector<CMediaImport> m_imports;
};
