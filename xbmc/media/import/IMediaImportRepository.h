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

#include "media/MediaType.h"
#include "media/import/MediaImport.h"
#include "media/import/MediaImportSource.h"

class CGUIDialogProgress;

/*!
 * \brief TODO
 */
class IMediaImportRepository
{
public:
  virtual ~IMediaImportRepository() { }

  virtual bool Initialize() = 0;
  
  virtual std::vector<CMediaImport> GetImports() const = 0;
  virtual std::vector<CMediaImport> GetImportsBySource(const std::string &sourceIdentifier) const = 0;
  virtual std::vector<CMediaImport> GetImportsByMediaType(const MediaType &mediaType) const = 0;
  virtual bool GetImport(const std::string &path, const MediaType &mediaType, CMediaImport &import) const = 0;

  virtual bool AddImport(const CMediaImport &import) = 0;
  virtual bool UpdateImport(const CMediaImport &import) = 0;
  virtual bool RemoveImport(const CMediaImport &import, CGUIDialogProgress *progress = NULL) = 0;
  
  virtual bool UpdateLastSync(CMediaImport &import) = 0;

  virtual std::vector<CMediaImportSource> GetSources(const MediaType &mediaType = MediaTypeNone) const = 0;
  virtual bool GetSource(const std::string &identifier, CMediaImportSource &source) const = 0;

  virtual bool AddSource(const CMediaImportSource &source) = 0;
  virtual bool UpdateSource(const CMediaImportSource &source) = 0;
  virtual bool RemoveSource(const std::string &identifier, CGUIDialogProgress *progress = NULL) = 0;
};
