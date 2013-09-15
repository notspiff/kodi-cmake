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

#include "media/import/IMediaImportHandler.h"
#include "dbwrappers/Database.h"

class CFileItem;
class CVideoDatabase;

class CVideoImportHandler : public IMediaImportHandler
{
public:
  virtual ~CVideoImportHandler() { }

  virtual void HandleImportedItems(const CMediaImport &import, const CFileItemList &items, IMediaImportTask *task);

  virtual void SetImportedItemsEnabled(const CMediaImport &import, bool enable);

protected:
  CVideoImportHandler() { }

  virtual bool HandleImportedItems(CVideoDatabase &videodb, const CMediaImport &import, const CFileItemList &items, IMediaImportTask *task) = 0;

  void PrepareItem(const CMediaImport &import, CFileItem* pItem, CVideoDatabase &videodb);
  void PrepareExistingItem(CFileItem *updatedItem, const CFileItem *originalItem);
  void SetDetailsForFile(const CFileItem *pItem, bool reset, CVideoDatabase &videodb);
  bool SetImportForItem(const CFileItem *pItem, const CMediaImport &import, CVideoDatabase &videodb);

  static CDatabase::Filter GetFilter(const CMediaImport &import, bool enabledItems = false);
  static bool Compare(const CFileItem *originalItem, const CFileItem *newItem, bool allMetadata = true, bool playbackMetadata = true);
};
