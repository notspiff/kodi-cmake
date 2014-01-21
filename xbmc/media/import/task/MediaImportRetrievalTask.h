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

#include <map>

#include "FileItem.h"
#include "media/MediaType.h"
#include "media/import/IMediaImportTask.h"

class IMediaImporter;

class CMediaImportRetrievalTask : public IMediaImportTask
{
public:
  CMediaImportRetrievalTask(const CMediaImport &import);
  virtual ~CMediaImportRetrievalTask();

  /*!
   * \brief Get the IMediaImporter instance used by the import job
   */
  IMediaImporter* GetImporter() const { return m_importer; }

  /*!
   * \brief TODO
   */
  const MediaType& GetMediaType() const { return GetImport().GetMediaType(); }

  /*!
   * \brief Get a list of imported items of the given media type
   *
   * \param mediaType media type of the imported items
   * \param list of imported items
   */
  bool GetImportedMedia(CFileItemList& items) const;

  /*!
   * \brief Add an imported item of a specific media type
   *
   * \param media type of the imported item
   * \param item imported item
   */
  void AddItem(const CFileItemPtr& item);
  /*!
   * \brief Add a list of imported items of a specific media type
   *
   * \param media type of the imported item
   * \param items imported items
   */
  void SetItems(const std::vector<CFileItemPtr>& items);

  // implementation of IMediaImportTask
  virtual bool DoWork();
  virtual const char *GetType() const { return "MediaImportRetrievalTask"; }

protected:
  IMediaImporter* m_importer;
  std::vector<CFileItemPtr> m_importedMedia;
};
