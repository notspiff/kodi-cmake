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

#include "TvShowImportHandler.h"
#include "FileItem.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "video/VideoDatabase.h"

/*!
 * Checks whether two tvshows are the same by comparing them by title and year
 */
static bool IsSameTVShow(const CVideoInfoTag& left, const CVideoInfoTag& right)
{
  return left.m_strShowTitle == right.m_strShowTitle
      && left.m_iYear        == right.m_iYear;
}

std::set<MediaType> CTvShowImportHandler::GetRequiredMediaTypes() const
{
  std::set<MediaType> types;
  types.insert(MediaTypeEpisode);
  return types;
}

std::vector<MediaType> CTvShowImportHandler::GetGroupedMediaTypes() const
{
  std::vector<MediaType> types;
  types.push_back(MediaTypeTvShow);
  types.push_back(MediaTypeSeason);
  types.push_back(MediaTypeEpisode);
  return types;
}

bool CTvShowImportHandler::AddImportedItem(const CMediaImport &import, CFileItem* item)
{
  if (item == nullptr)
    return false;

  // make sure that the source and import path are set
  PrepareItem(import, item);

  // and prepare the tvshow paths
  std::vector< std::pair<std::string, std::string> > tvshowPaths;
  tvshowPaths.push_back(std::make_pair(item->GetPath(), item->GetVideoInfoTag()->m_basePath));
  // we don't know the season art yet
  std::map<int, std::map<std::string, std::string> > seasonArt;

  const CVideoInfoTag* info = item->GetVideoInfoTag();

  // check if there already is a local tvshow with the same name
  CFileItemList tvshows;
  m_db.GetTvShowsByName(info->m_strTitle, tvshows);
  bool exists = false;
  if (!tvshows.IsEmpty())
  {
    CFileItemPtr tvshow;
    for (int i = 0; i < tvshows.Size();)
    {
      tvshow = tvshows.Get(i);
      // remove tvshows without a CVideoInfoTag
      if (!tvshow->HasVideoInfoTag())
      {
        tvshows.Remove(i);
        continue;
      }

      CVideoInfoTag* tvshowInfo = tvshow->GetVideoInfoTag();
      if (!m_db.GetTvShowInfo(tvshowInfo->GetPath(), *tvshowInfo, tvshowInfo->m_iDbId, tvshow.get()))
      {
        tvshows.Remove(i);
        continue;
      }

      // check if the scraper identifier or the title and year match
      if ((!tvshowInfo->m_strIMDBNumber.empty() && tvshowInfo->m_strIMDBNumber == info->m_strIMDBNumber) ||
          (tvshowInfo->m_iYear > 0 && tvshowInfo->m_iYear == info->m_iYear && tvshowInfo->m_strTitle == info->m_strTitle))
      {
        exists = true;
        break;
      }
      // remove tvshows that don't even match in title
      else if (tvshowInfo->m_strTitle != info->m_strTitle)
      {
        tvshows.Remove(i);
        continue;
      }

      ++i;
    }

    // if there was no exact match and there are still tvshows left that match in title
    // and the new item doesn't have a scraper identifier and no year
    // we take the first match
    if (!exists && !tvshows.IsEmpty() &&
        info->m_strIMDBNumber.empty() && info->m_iYear <= 0)
    {
      tvshow = tvshows.Get(0);
      exists = true;
    }

    // simply add the path of the imported tvshow to the tvshow's paths
    if (exists && tvshow != nullptr)
      item->GetVideoInfoTag()->m_iDbId = m_db.SetDetailsForTvShow(tvshowPaths, *(tvshow->GetVideoInfoTag()), tvshow->GetArt(), seasonArt, tvshow->GetVideoInfoTag()->m_iDbId);
  }

  // couldn't find a matching local tvshow so add the newly imported one
  if (!exists)
    item->GetVideoInfoTag()->m_iDbId = m_db.SetDetailsForTvShow(tvshowPaths, *(item->GetVideoInfoTag()), item->GetArt(), seasonArt);

  // make sure that the tvshow was properly added
  if (item->GetVideoInfoTag()->m_iDbId <= 0)
    return false;

  return SetImportForItem(item, import);
}

bool CTvShowImportHandler::UpdateImportedItem(const CMediaImport &import, CFileItem* item)
{
  if (item == nullptr || !item->HasVideoInfoTag() || item->GetVideoInfoTag()->m_iDbId <= 0)
    return false;

  std::vector< std::pair<std::string, std::string> > tvshowPaths;
  tvshowPaths.push_back(std::make_pair(item->GetPath(), item->GetVideoInfoTag()->m_basePath));
  std::map<int, std::map<std::string, std::string> > seasonArt;
  return m_db.SetDetailsForTvShow(tvshowPaths, *(item->GetVideoInfoTag()), item->GetArt(), seasonArt, item->GetVideoInfoTag()->m_iDbId) > 0;
}

bool CTvShowImportHandler::RemoveImportedItem(const CMediaImport &import, const CFileItem* item)
{
  if (item == nullptr || !item->HasVideoInfoTag() || item->GetVideoInfoTag()->m_iDbId <= 0)
    return false;

  // get all paths belonging to the tvshow
  std::map<int, std::string> tvshowPaths;
  if (!m_db.GetPathsForTvShow(item->GetVideoInfoTag()->m_iDbId, tvshowPaths))
    return false;

  // something is wrong as the tvshow doesn't have any paths
  if (tvshowPaths.empty())
    return false;

  // we only handle the case where more than one path belongs to the tvshow because
  // we can't delete the tvshow completely before not having synced the episodes
  if (tvshowPaths.size() == 1)
    return true;

  for (const auto& tvshowPath : tvshowPaths)
  {
    // check if the tvshow path is a sub-path of the media import
    if (URIUtils::PathStarts(tvshowPath.second, import.GetPath().c_str()))
    {
      // remove the path from the tvshow
      m_db.RemovePathFromTvShow(item->GetVideoInfoTag()->m_iDbId, tvshowPath.first);
      m_db.RemoveImportFromItem(item->GetVideoInfoTag()->m_iDbId, import);
      break;
    }
  }

  return true;
}

bool CTvShowImportHandler::CleanupImportedItems(const CMediaImport &import)
{
  if (!m_db.Open())
    return false;

  CFileItemList importedTvShows;
  if (!GetLocalItems(m_db, import, importedTvShows))
    return false;

  m_db.BeginTransaction();

  for (int i = 0; i < importedTvShows.Size(); ++i)
  {
    CFileItemPtr importedTvShow = importedTvShows.Get(i);
    if (!importedTvShow->HasVideoInfoTag() || importedTvShow->GetVideoInfoTag()->m_iDbId <= 0)
      continue;

    // get all episodes of the tvshow
    CVideoDbUrl videoUrl;
    if (!videoUrl.FromString(StringUtils::Format("videodb://tvshows/titles/%d/-1/", importedTvShow->GetVideoInfoTag()->m_iDbId)))
      continue;
    videoUrl.AddOption("tvshowid", importedTvShow->GetVideoInfoTag()->m_iDbId);

    CFileItemList episodes;
    if (!m_db.GetEpisodesByWhere(videoUrl.ToString(), CDatabase::Filter(), episodes, true, SortDescription(), false))
      continue;

    // loop through all episodes and count the imported ones
    bool hasImportedEpisodes = false;
    for (int i = 0; i < episodes.Size(); ++i)
    {
      if (episodes.Get(i)->IsImported())
      {
        hasImportedEpisodes = true;
        break;
      }
    }

    // if there no imported episodes we can remove the tvshow
    if (!hasImportedEpisodes)
      RemoveImportedItem(m_db, import, importedTvShow.get());
  }

  m_db.CommitTransaction();

  return true;
}

bool CTvShowImportHandler::GetLocalItems(CVideoDatabase &videodb, const CMediaImport &import, CFileItemList& items) const
{
  return videodb.GetTvShowsByWhere("videodb://tvshows/titles/", GetFilter(import), items, SortDescription(), import.GetSettings().UpdateImportedMediaItems());
}

CFileItemPtr CTvShowImportHandler::FindMatchingLocalItem(const CFileItem* item, CFileItemList& localItems) const
{
  for (int i = 0; i < localItems.Size(); ++i)
  {
    CFileItemPtr localItem = localItems.Get(i);
    if (IsSameTVShow(*item->GetVideoInfoTag(), *localItem->GetVideoInfoTag()))
      return localItem;
  }

  return CFileItemPtr();
}

MediaImportChangesetType CTvShowImportHandler::DetermineChangeset(const CMediaImport &import, CFileItem* item, CFileItemPtr localItem, CFileItemList& localItems, bool updatePlaybackMetadata)
{
  return CVideoImportHandler::DetermineChangeset(import, item, localItem, localItems, false);
}

bool CTvShowImportHandler::RemoveImportedItems(CVideoDatabase &videodb, const CMediaImport &import) const
{
  CFileItemList items;
  if (!GetLocalItems(videodb, import, items))
    return false;

  for (int index = 0; index < items.Size(); ++index)
    RemoveImportedItem(videodb, import, items.Get(index).get());

  return CVideoImportHandler::RemoveImportedItems(videodb, import);
}

void CTvShowImportHandler::RemoveImportedItem(CVideoDatabase &videodb, const CMediaImport &import, const CFileItem* item) const
{
  // check if the tvshow still has episodes or not
  if (item == nullptr || !item->HasVideoInfoTag())
    return;

  // if there are other episodes only remove the path and the import link to the tvshow and not the whole tvshow
  if (item->GetVideoInfoTag()->m_iEpisode > 0)
  {
    videodb.RemovePathFromTvShow(item->GetVideoInfoTag()->m_iDbId, item->GetVideoInfoTag()->GetPath());
    videodb.RemoveImportFromItem(item->GetVideoInfoTag()->m_iDbId, import);
  }
  else
    videodb.DeleteTvShow(item->GetVideoInfoTag()->m_iDbId, false, false);

  // either way remove the path
  videodb.DeletePath(-1, item->GetVideoInfoTag()->m_strPath);
}
