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

#include "media/import/handler/VideoImportHandler.h"

class CMusicVideoImportHandler : public CVideoImportHandler
{
public:
  CMusicVideoImportHandler() { }
  virtual ~CMusicVideoImportHandler() { }

  virtual IMediaImportHandler* Create() const { return new CMusicVideoImportHandler(); }

  virtual MediaType GetMediaType() const { return MediaTypeMusicVideo; }

  virtual bool AddImportedItem(const CMediaImport &import, CFileItem* item);
  virtual bool UpdateImportedItem(const CMediaImport &import, CFileItem* item);
  virtual bool RemoveImportedItem(const CMediaImport &import, const CFileItem* item);
  virtual bool CleanupImportedItems(const CMediaImport &import) { return true; }

protected:
  virtual bool GetLocalItems(CVideoDatabase &videodb, const CMediaImport &import, CFileItemList& items) const;

  virtual void RemoveImportedItem(CVideoDatabase &videodb, const CMediaImport &import, const CFileItem* item) const;
};
