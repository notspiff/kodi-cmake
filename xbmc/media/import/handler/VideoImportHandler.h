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

#include "dbwrappers/Database.h"
#include "media/import/IMediaImportHandler.h"
#include "video/VideoDatabase.h"
#include "video/VideoThumbLoader.h"

class CFileItem;

class CVideoImportHandler : public IMediaImportHandler
{
public:
  virtual ~CVideoImportHandler() { }

  virtual std::string GetItemLabel(const CFileItem* item) const;

  virtual bool GetLocalItems(const CMediaImport &import, CFileItemList& items);

  virtual bool StartChangeset(const CMediaImport &import);
  virtual bool FinishChangeset(const CMediaImport &import);
  virtual MediaImportChangesetType DetermineChangeset(const CMediaImport &import, CFileItem* item, CFileItemList& localItems);

  virtual bool StartSynchronisation(const CMediaImport &import);
  virtual bool FinishSynchronisation(const CMediaImport &import);

  virtual bool RemoveImportedItems(const CMediaImport &import);

  virtual void SetImportedItemsEnabled(const CMediaImport &import, bool enable);

protected:
  CVideoImportHandler() { }

  virtual bool GetLocalItems(CVideoDatabase &videodb, const CMediaImport &import, CFileItemList& items) const = 0;

  virtual CFileItemPtr FindMatchingLocalItem(const CFileItem* item, CFileItemList& localItems) const;
  virtual MediaImportChangesetType DetermineChangeset(const CMediaImport &import, CFileItem* item, CFileItemPtr localItem, CFileItemList& localItems, bool updatePlaybackMetadata);

  virtual bool RemoveImportedItems(CVideoDatabase &videodb, const CMediaImport &import) const;
  virtual void RemoveImportedItem(CVideoDatabase &videodb, const CMediaImport &import, const CFileItem* item) const = 0;

  void PrepareItem(const CMediaImport &import, CFileItem* pItem);
  void PrepareExistingItem(CFileItem *updatedItem, const CFileItem *originalItem);
  void SetDetailsForFile(const CFileItem *pItem, bool reset);
  bool SetImportForItem(const CFileItem *pItem, const CMediaImport &import);
  void RemoveFile(CVideoDatabase &videodb, const CFileItem *item) const;

  static CDatabase::Filter GetFilter(const CMediaImport &import, bool enabledItems = false);
  static bool Compare(const CFileItem *originalItem, const CFileItem *newItem, bool allMetadata = true, bool playbackMetadata = true);

  CVideoDatabase m_db;
  CVideoThumbLoader m_thumbLoader;
};
