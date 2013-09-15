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

#include "VideoImportHandler.h"
#include "FileItem.h"
#include "media/import/IMediaImportTask.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "video/VideoDatabase.h"

std::string CVideoImportHandler::GetItemLabel(const CFileItem* item) const
{
  if (item == nullptr)
    return "";

  if (item->HasVideoInfoTag() && !item->GetVideoInfoTag()->m_strTitle.empty())
    return item->GetVideoInfoTag()->m_strTitle;

  return item->GetLabel();
}

bool CVideoImportHandler::GetLocalItems(const CMediaImport &import, CFileItemList& items)
{
  if (!m_db.Open())
    return false;

  bool result = GetLocalItems(m_db, import, items);

  m_db.Close();
  return result;
}

bool CVideoImportHandler::StartChangeset(const CMediaImport &import)
{
  // start the background loader if necessary
  if (import.GetSettings().UpdateImportedMediaItems())
    m_thumbLoader.OnLoaderStart();

  return true;
}

bool CVideoImportHandler::FinishChangeset(const CMediaImport &import)
{
  // stop the background loader if necessary
  if (import.GetSettings().UpdateImportedMediaItems())
    m_thumbLoader.OnLoaderFinish();

  return true;
}

MediaImportChangesetType CVideoImportHandler::DetermineChangeset(const CMediaImport &import, CFileItem* item, CFileItemList& localItems)
{
  if (item == nullptr || !item->HasVideoInfoTag())
    return MediaImportChangesetTypeNone;

  return DetermineChangeset(import, item, FindMatchingLocalItem(item, localItems), localItems, import.GetSettings().UpdatePlaybackMetadataFromSource());
}

bool CVideoImportHandler::StartSynchronisation(const CMediaImport &import)
{
  if (!m_db.Open())
    return false;

  m_db.BeginTransaction();
  return true;
}

bool CVideoImportHandler::FinishSynchronisation(const CMediaImport &import)
{
  if (!m_db.IsOpen())
    return false;

  // now make sure the items are enabled
  SetImportedItemsEnabled(import, true);

  m_db.CommitTransaction();
  m_db.Close();
  return true;
}

bool CVideoImportHandler::RemoveImportedItems(const CMediaImport &import)
{
  if (!m_db.Open())
    return false;

  m_db.BeginTransaction();

  bool success = RemoveImportedItems(m_db, import);

  if (success)
    m_db.CommitTransaction();
  else
    m_db.RollbackTransaction();

  m_db.Close();
  return success;
}

void CVideoImportHandler::SetImportedItemsEnabled(const CMediaImport &import, bool enable)
{
  if (!m_db.Open())
    return;

  m_db.SetImportItemsEnabled(enable, import);
  m_db.Close();
}

CFileItemPtr CVideoImportHandler::FindMatchingLocalItem(const CFileItem* item, CFileItemList& localItems) const
{
  for (int i = 0; i < localItems.Size(); ++i)
  {
    CFileItemPtr localItem = localItems.Get(i);
    if (!localItem->HasVideoInfoTag())
      continue;

    if (localItem->GetVideoInfoTag()->GetPath() == item->GetVideoInfoTag()->GetPath())
      return localItem;
  }

  return CFileItemPtr();
}

MediaImportChangesetType CVideoImportHandler::DetermineChangeset(const CMediaImport &import, CFileItem* item, CFileItemPtr localItem, CFileItemList& localItems, bool updatePlaybackMetadata)
{
  if (localItem == nullptr)
    return MediaImportChangesetTypeAdded;

  // remove the matching item from the local list so that the imported item is not considered non-existant
  localItems.Remove(localItem.get());

  const CMediaImportSettings& settings = import.GetSettings();

  // nothing to do if we don't need to update imported media items
  if (!settings.UpdateImportedMediaItems())
    return MediaImportChangesetTypeNone;

  // retrieve all details for the previously imported item
  if (!m_thumbLoader.LoadItem(localItem.get()))
    CLog::Log(LOGWARNING, "Failed to retrieve details for local item %s during media importing", localItem->GetVideoInfoTag()->GetPath().c_str());

  // compare the previously imported item with the newly imported item
  if (Compare(localItem.get(), item, settings.UpdateImportedMediaItems(), settings.UpdatePlaybackMetadataFromSource()))
    return MediaImportChangesetTypeNone;

  // the newly imported item has changed from the previously imported one so get some information from the local item as preparation
  PrepareExistingItem(item, localItem.get());

  return MediaImportChangesetTypeChanged;
}

bool CVideoImportHandler::RemoveImportedItems(CVideoDatabase &videodb, const CMediaImport &import) const
{
  return videodb.DeleteItemsFromImport(import);
}

void CVideoImportHandler::PrepareItem(const CMediaImport &import, CFileItem* pItem)
{
  if (pItem == nullptr || !pItem->HasVideoInfoTag() ||
      import.GetPath().empty() || import.GetSource().GetIdentifier().empty())
    return;

  const std::string &sourceID = import.GetSource().GetIdentifier();
  m_db.AddPath(sourceID);
  int idPath = m_db.AddPath(import.GetPath(), sourceID);

  // set the proper source
  pItem->SetSource(sourceID);
  pItem->SetImportPath(import.GetPath());

  // set the proper base and parent path
  pItem->GetVideoInfoTag()->m_basePath = sourceID;
  pItem->GetVideoInfoTag()->m_parentPathID = idPath;

  if (!pItem->m_bIsFolder)
    pItem->GetVideoInfoTag()->m_iFileId = m_db.AddFile(pItem->GetPath(), import.GetPath());
}

void CVideoImportHandler::PrepareExistingItem(CFileItem *updatedItem, const CFileItem *originalItem)
{
  if (updatedItem == nullptr || originalItem == nullptr ||
      !updatedItem->HasVideoInfoTag() || !originalItem->HasVideoInfoTag())
    return;

  updatedItem->GetVideoInfoTag()->m_iDbId = originalItem->GetVideoInfoTag()->m_iDbId;
  updatedItem->GetVideoInfoTag()->m_iFileId = originalItem->GetVideoInfoTag()->m_iFileId;
  updatedItem->GetVideoInfoTag()->m_iIdShow = originalItem->GetVideoInfoTag()->m_iIdShow;
  updatedItem->GetVideoInfoTag()->m_iIdSeason = originalItem->GetVideoInfoTag()->m_iIdSeason;

  updatedItem->SetSource(originalItem->GetSource());
  updatedItem->SetImportPath(originalItem->GetImportPath());
  updatedItem->GetVideoInfoTag()->m_basePath = originalItem->GetVideoInfoTag()->m_basePath;
  updatedItem->GetVideoInfoTag()->m_parentPathID = originalItem->GetVideoInfoTag()->m_parentPathID;
}

void CVideoImportHandler::SetDetailsForFile(const CFileItem *pItem, bool reset)
{
  // clean resume bookmark
  if (reset)
    m_db.DeleteResumeBookMark(pItem->GetPath());

  if (pItem->GetVideoInfoTag()->m_resumePoint.IsPartWay())
    m_db.AddBookMarkToFile(pItem->GetPath(), pItem->GetVideoInfoTag()->m_resumePoint, CBookmark::RESUME);

  m_db.SetPlayCount(*pItem, pItem->GetVideoInfoTag()->m_playCount, pItem->GetVideoInfoTag()->m_lastPlayed);
}

bool CVideoImportHandler::SetImportForItem(const CFileItem *pItem, const CMediaImport &import)
{
  return m_db.SetImportForItem(pItem->GetVideoInfoTag()->m_iDbId, import);
}

void CVideoImportHandler::RemoveFile(CVideoDatabase &videodb, const CFileItem *item) const
{
  if (!videodb.IsOpen() || item == nullptr || !item->HasVideoInfoTag())
    return;

  videodb.DeleteFile(item->GetVideoInfoTag()->m_iFileId, item->GetVideoInfoTag()->GetPath());
}

CDatabase::Filter CVideoImportHandler::GetFilter(const CMediaImport &import, bool enabledItems /* = false */)
{
  std::string strWhere;
  if (!import.GetPath().empty())
    strWhere += StringUtils::Format("importPath = '%s'", import.GetPath().c_str());

  return CDatabase::Filter(strWhere);
}

bool CVideoImportHandler::Compare(const CFileItem *originalItem, const CFileItem *newItem, bool allMetadata /* = true */, bool playbackMetadata /* = true */)
{
  if (originalItem == nullptr || !originalItem->HasVideoInfoTag() ||
      newItem == nullptr || !newItem->HasVideoInfoTag())
    return false;

  if (allMetadata)
  {
    if (originalItem->GetArt() != newItem->GetArt())
      return false;

    if (originalItem->GetVideoInfoTag()->Equals(*newItem->GetVideoInfoTag(), true))
      return true;

    std::set<Field> differences;
    if (!originalItem->GetVideoInfoTag()->GetDifferences(*newItem->GetVideoInfoTag(), differences, true))
      return true;

    // if playback metadata shouldn't be compared simply remove them from the list of differences
    if (!playbackMetadata)
    {
      differences.erase(FieldPlaycount);  // playcount
      differences.erase(FieldLastPlayed); // lastplayed
      differences.erase(FieldInProgress); // resume point
    }

    // special handling for actors without artwork
    const auto& it = differences.find(FieldActor);
    if (it != differences.end())
    {
      const std::vector<SActorInfo> &originalCast = originalItem->GetVideoInfoTag()->m_cast;
      const std::vector<SActorInfo> &newCast = newItem->GetVideoInfoTag()->m_cast;
      if (originalCast.size() == newCast.size())
      {
        bool equal = true;
        for(size_t index = 0; index < originalCast.size(); ++index)
        {
          const SActorInfo &originalActor = originalCast.at(index);
          const SActorInfo &newActor = newCast.at(index);

          if (originalActor.strName != newActor.strName ||
              originalActor.strRole != newActor.strRole ||
              (!newActor.thumb.empty() && originalActor.thumb != newActor.thumb))
          {
            equal = false;
            break;
          }
        }

        if (equal)
          differences.erase(it);
      }
    }

    return differences.empty();
  }

  const CVideoInfoTag *originalDetails = originalItem->GetVideoInfoTag();
  const CVideoInfoTag *newDetails = newItem->GetVideoInfoTag();
  return originalDetails->m_playCount == newDetails->m_playCount &&
         originalDetails->m_lastPlayed == newDetails->m_lastPlayed &&
         originalDetails->m_resumePoint.timeInSeconds == newDetails->m_resumePoint.timeInSeconds;
}
