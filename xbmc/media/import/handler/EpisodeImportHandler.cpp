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

#include "EpisodeImportHandler.h"
#include "FileItem.h"
#include "dialogs/GUIDialogExtendedProgressBar.h"
#include "media/import/IMediaImportTask.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "video/VideoDatabase.h"
#include "video/VideoThumbLoader.h"

typedef std::set<CFileItemPtr> TvShowsSet;
typedef std::map<std::string, TvShowsSet> TvShowsMap;

/*!
 * Tries to find the tvshow from the given map to which the given episode belongs
 */
static int FindTvShow(const TvShowsMap &tvshowsMap, CFileItemPtr episodeItem)
{
  if (episodeItem == NULL)
    return -1;

  // no comparison possible without a title
  if (episodeItem->GetVideoInfoTag()->m_strShowTitle.empty())
    return -1;

  // check if there is a tvshow with a matching title
  TvShowsMap::const_iterator tvshowsIter = tvshowsMap.find(episodeItem->GetVideoInfoTag()->m_strShowTitle);
  if (tvshowsIter == tvshowsMap.end() ||
      tvshowsIter->second.size() <= 0)
    return -1;

  // if there is only one matching tvshow, we can go with that one
  if (tvshowsIter->second.size() == 1)
    return tvshowsIter->second.begin()->get()->GetVideoInfoTag()->m_iDbId;

  // use the path of the episode and tvshow to find the right tvshow
  for (TvShowsSet::const_iterator it = tvshowsIter->second.begin(); it != tvshowsIter->second.end(); ++it)
  {
    if (URIUtils::IsInPath(episodeItem->GetVideoInfoTag()->GetPath(), (*it)->GetVideoInfoTag()->GetPath()))
      return (*it)->GetVideoInfoTag()->m_iDbId;
  }

  return -1;
}

std::set<MediaType> CEpisodeImportHandler::GetDependencies() const
{
  std::set<MediaType> types;
  types.insert(MediaTypeTvShow);
  types.insert(MediaTypeSeason);
  return types;
}

std::vector<MediaType> CEpisodeImportHandler::GetGroupedMediaTypes() const
{
  std::vector<MediaType> types;
  types.push_back(MediaTypeTvShow);
  types.push_back(MediaTypeSeason);
  types.push_back(MediaTypeEpisode);
  return types;
}

bool CEpisodeImportHandler::HandleImportedItems(CVideoDatabase &videodb, const CMediaImport &import, const CFileItemList &items, IMediaImportTask *task)
{
  bool checkCancelled = task != NULL;
  if (checkCancelled && task->ShouldCancel(0, items.Size()))
    return false;
  
  task->SetProgressTitle(StringUtils::Format(g_localizeStrings.Get(37026), MediaTypes::GetPluralLocalization(MediaTypeEpisode).c_str(), import.GetSource().GetFriendlyName().c_str()));
  task->SetProgressText("");

  const CMediaImportSettings &importSettings = import.GetSettings();
  CFileItemList storedItems;
  videodb.GetEpisodesByWhere("videodb://tvshows/titles/", GetFilter(import), storedItems, true, SortDescription(), importSettings.UpdateImportedMediaItems());
  
  int total = storedItems.Size() + items.Size();
  if (checkCancelled && task->ShouldCancel(0, total))
    return false;

  CVideoThumbLoader thumbLoader;
  if (importSettings.UpdateImportedMediaItems())
    thumbLoader.OnLoaderStart();

  int progress = 0;
  CFileItemList newItems; newItems.Copy(items);
  for (int i = 0; i < storedItems.Size(); i++)
  {
    if (checkCancelled && task->ShouldCancel(progress, items.Size()))
      return false;

    CFileItemPtr oldItem = storedItems[i];
    CFileItemPtr pItem = newItems.Get(oldItem->GetVideoInfoTag()->GetPath());

    task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37031),
                                              oldItem->GetVideoInfoTag()->m_strShowTitle.c_str(),
                                              oldItem->GetVideoInfoTag()->m_strTitle.c_str()));

    // delete items that are not in newItems
    if (pItem == NULL)
    {
      task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37032), 
                                                oldItem->GetVideoInfoTag()->m_strShowTitle.c_str(),
                                                oldItem->GetVideoInfoTag()->m_strTitle.c_str()));
      videodb.DeleteEpisode(oldItem->GetVideoInfoTag()->m_iDbId);
    }
    // item is in both lists
    else
    {
      // get rid of items we already have from the new items list
      newItems.Remove(pItem.get());
      total--;

      // only process the item with the thumb loader if updates to artwork etc. are allowed
      if (importSettings.UpdateImportedMediaItems())
        thumbLoader.LoadItem(oldItem.get());

      // check if we need to update (db writing is expensive)
      // but only if synchronisation is enabled
      if (importSettings.UpdateImportedMediaItems() &&
          !Compare(oldItem.get(), pItem.get(), importSettings.UpdateImportedMediaItems(), importSettings.UpdatePlaybackMetadataFromSource()))
      {
        task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37033), 
                                                  pItem->GetVideoInfoTag()->m_strShowTitle.c_str(),
                                                  pItem->GetVideoInfoTag()->m_strTitle.c_str()));

        PrepareExistingItem(pItem.get(), oldItem.get());

        if (importSettings.UpdateImportedMediaItems())
        {
          videodb.SetDetailsForEpisode(pItem->GetPath(), *(pItem->GetVideoInfoTag()), pItem->GetArt(), pItem->GetVideoInfoTag()->m_iIdShow, pItem->GetVideoInfoTag()->m_iDbId);
          if (importSettings.UpdatePlaybackMetadataFromSource())
            SetDetailsForFile(pItem.get(), true, videodb);
        }
      }
    }

    task->SetProgress(progress++, total);
  }

  if (importSettings.UpdateImportedMediaItems())
    thumbLoader.OnLoaderFinish();

  if (newItems.Size() <= 0)
    return true;

  // create a map of tvshows imported from the same source
  CFileItemList tvshows;
  videodb.GetTvShowsByWhere("videodb://tvshows/titles/", GetFilter(import), tvshows);

  TvShowsMap tvshowsMap;
  TvShowsMap::iterator tvshowsIter;
  for (int tvshowsIndex = 0; tvshowsIndex < tvshows.Size(); tvshowsIndex++)
  {
    CFileItemPtr tvshow = tvshows[tvshowsIndex];

    if (!tvshow->HasVideoInfoTag() || tvshow->GetVideoInfoTag()->m_strTitle.empty())
      continue;

    tvshowsIter = tvshowsMap.find(tvshow->GetVideoInfoTag()->m_strTitle);
    if (tvshowsIter == tvshowsMap.end())
    {
      TvShowsSet tvshowsSet; tvshowsSet.insert(tvshow);
      tvshowsMap.insert(make_pair(tvshow->GetVideoInfoTag()->m_strTitle, tvshowsSet));
    }
    else
      tvshowsIter->second.insert(tvshow);
  }

  // add any (remaining) new items
  for (int i = 0; i < newItems.Size(); i++)
  {
    if (checkCancelled && task->ShouldCancel(progress, items.Size()))
      return false;

    CFileItemPtr pItem = newItems[i];
    CVideoInfoTag *episode = pItem->GetVideoInfoTag();
    PrepareItem(import, pItem.get(), videodb);

    task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37034), episode->m_strShowTitle.c_str(), episode->m_strTitle.c_str()));
    
    episode->m_iIdShow = FindTvShow(tvshowsMap, pItem);
    // if the tvshow doesn't exist, create a very basic version of it with the info we got from the episode
    if (episode->m_iIdShow <= 0)
    {
      CVideoInfoTag tvshow;
      tvshow.m_basePath = episode->m_basePath;
      tvshow.m_cast = episode->m_cast;
      tvshow.m_country = episode->m_country;
      tvshow.m_director = episode->m_director;
      tvshow.m_genre = episode->m_genre;
      tvshow.m_iYear = episode->m_iYear;
      tvshow.m_parentPathID = episode->m_parentPathID;
      tvshow.m_premiered = episode->m_premiered;
      tvshow.m_strMPAARating = episode->m_strMPAARating;
      tvshow.m_strSource = episode->m_strSource;
      tvshow.m_strImportPath = episode->m_strImportPath;
      tvshow.m_strTitle = tvshow.m_strShowTitle = episode->m_strShowTitle;
      tvshow.m_studio = episode->m_studio;
      tvshow.m_type = MediaTypeTvShow;
      tvshow.m_writingCredits = episode->m_writingCredits;

      // try to find a proper path by going up in the path hierarchy twice
      // (once for season and once for tvshow)
      std::string showPath, testPath;
      showPath = tvshow.m_basePath;
      testPath = URIUtils::GetParentPath(episode->GetPath());
      if (testPath != tvshow.m_basePath)
      {
        showPath = testPath;
        testPath = URIUtils::GetParentPath(showPath);
        if (testPath != tvshow.m_basePath)
          showPath = testPath;
      }
      tvshow.m_strPath = tvshow.m_strShowPath = showPath;

      // add the basic tvshow to the database
      tvshow.m_iDbId = tvshow.m_iIdShow = videodb.SetDetailsForTvShow(tvshow.m_strPath, tvshow, CGUIListItem::ArtMap(), std::map<int, std::map<std::string, std::string> >());
      // store the tvshow's database ID in the episode
      episode->m_iIdShow = tvshow.m_iDbId;

      // add the tvshow to the tvshow map
      CFileItemPtr tvshowItem(new CFileItem(tvshow));
      tvshowsIter = tvshowsMap.find(tvshow.m_strTitle);
      if (tvshowsIter == tvshowsMap.end())
      {
        TvShowsSet tvshowsSet; tvshowsSet.insert(tvshowItem);
        tvshowsMap.insert(make_pair(tvshow.m_strTitle, tvshowsSet));
      }
      else
        tvshowsIter->second.insert(tvshowItem);
    }
    videodb.SetDetailsForEpisode(pItem->GetPath(), *episode, pItem->GetArt(), episode->m_iIdShow);
    SetDetailsForFile(pItem.get(), false, videodb);
    SetImportForItem(pItem.get(), import, videodb);

    task->SetProgress(progress++, total);
  }

  return true;
}
