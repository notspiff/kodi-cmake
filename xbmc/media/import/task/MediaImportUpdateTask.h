#pragma once
/*
 *      Copyright (C) 2014 Team XBMC
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

#include "FileItem.h"
#include "media/import/IMediaImportTask.h"

class IMediaImporter;

class CMediaImportUpdateTask : public IMediaImportTask
{
public:
  CMediaImportUpdateTask(const CMediaImport &import, const CFileItem &item);
  virtual ~CMediaImportUpdateTask();

  /*!
   * \brief Get the IMediaImporter instance used by the import job
   */
  const MediaImporterPtr GetImporter() const { return m_importer; }

  const CFileItem& GetItem() const { return m_item; }

  // implementation of IMediaImportTask
  virtual MediaImportTaskType GetType() { return MediaImportTaskType::Update; }
  virtual bool DoWork();

protected:
  MediaImporterPtr m_importer;
  CFileItem m_item;
};
