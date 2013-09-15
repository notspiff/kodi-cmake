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

#include "SeasonImportHandler.h"
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
 * Checks whether two seasons are the same by comparing them by title and year
 */
static bool IsSameSeason(CVideoInfoTag& left, CVideoInfoTag& right)
{
  return left.m_strShowTitle == right.m_strShowTitle
      && left.m_iYear        == right.m_iYear
      && left.m_iSeason      == right.m_iSeason;
}

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

std::set<MediaType> CSeasonImportHandler::GetDependencies() const
{
  std::set<MediaType> types;
  types.insert(MediaTypeTvShow);
  return types;
}

std::set<MediaType> CSeasonImportHandler::GetRequiredMediaTypes() const
{
  std::set<MediaType> types;
  types.insert(MediaTypeEpisode);
  return types;
}

std::vector<MediaType> CSeasonImportHandler::GetGroupedMediaTypes() const
{
  std::vector<MediaType> types;
  types.push_back(MediaTypeTvShow);
  types.push_back(MediaTypeSeason);
  types.push_back(MediaTypeEpisode);
  return types;
}

bool CSeasonImportHandler::HandleImportedItems(CVideoDatabase &videodb, const CMediaImport &import, const CFileItemList &items, IMediaImportTask *task)
{
  bool checkCancelled = task != NULL;
  if (checkCancelled && task->ShouldCancel(0, items.Size()))
    return false;
  
  task->SetProgressTitle(StringUtils::Format(g_localizeStrings.Get(37026), MediaTypes::GetPluralLocalization(MediaTypeSeason).c_str(), import.GetSource().GetFriendlyName().c_str()));
  task->SetProgressText("");

  CFileItemList storedItems;
  videodb.GetSeasonsByWhere("videodb://tvshows/titles/", GetFilter(import), storedItems, true);
  
  int total = storedItems.Size() + items.Size();
  if (checkCancelled && task->ShouldCancel(0, total))
    return false;

  CVideoThumbLoader thumbLoader;
  thumbLoader.OnLoaderStart();

  const CMediaImportSettings &importSettings = import.GetSettings();

  int progress = 0;
  CFileItemList newItems; newItems.Copy(items);
  for (int i = 0; i < storedItems.Size(); i++)
  {
    if (checkCancelled && task->ShouldCancel(progress, items.Size()))
      return false;

    CFileItemPtr oldItem = storedItems[i];
    bool found = false;
    for (int j = 0; j < newItems.Size() ; j++)
    {
      if (checkCancelled && task->ShouldCancel(progress, items.Size()))
        return false;

      CFileItemPtr newItem = newItems[j];
      task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37031), 
                                                newItem->GetVideoInfoTag()->m_strShowTitle.c_str(),
                                                newItem->GetVideoInfoTag()->m_strTitle.c_str()));

      if (IsSameSeason(*oldItem->GetVideoInfoTag(), *newItem->GetVideoInfoTag()))
      {
        // get rid of items we already have from the new items list
        newItems.Remove(j);
        total--;
        found = true;

        thumbLoader.LoadItem(oldItem.get());
        
        // check if we need to update (db writing is expensive)
        // but only if synchronisation is enabled
        if (importSettings.UpdateImportedMediaItems() &&
            !Compare(oldItem.get(), newItem.get(), true, false))
        {
          task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37033), 
                                                    newItem->GetVideoInfoTag()->m_strShowTitle.c_str(),
                                                    newItem->GetVideoInfoTag()->m_strTitle.c_str()));

          PrepareExistingItem(newItem.get(), oldItem.get());
          videodb.SetDetailsForSeason(*(newItem->GetVideoInfoTag()), newItem->GetArt(), newItem->GetVideoInfoTag()->m_iIdShow, newItem->GetVideoInfoTag()->m_iDbId);
        }
        break;
      }
    }

    // delete items that are not in newItems
    if (!found)
    {
      task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37032), 
                                                oldItem->GetVideoInfoTag()->m_strShowTitle.c_str(),
                                                oldItem->GetVideoInfoTag()->m_strTitle.c_str()));
      videodb.DeleteSeason(oldItem->GetVideoInfoTag()->m_iDbId);
    }

    task->SetProgress(progress++, total);
  }

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
    CVideoInfoTag *season = pItem->GetVideoInfoTag();
    PrepareItem(import, pItem.get(), videodb);

    task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37034), season->m_strShowTitle.c_str(), season->m_strTitle.c_str()));
    
    season->m_iIdShow = FindTvShow(tvshowsMap, pItem);
    // if the tvshow doesn't exist, create a very basic version of it with the info we got from the season
    if (season->m_iIdShow <= 0)
    {
      CVideoInfoTag tvshow;
      tvshow.m_basePath = season->m_basePath;
      tvshow.m_cast = season->m_cast;
      tvshow.m_country = season->m_country;
      tvshow.m_director = season->m_director;
      tvshow.m_genre = season->m_genre;
      tvshow.m_iYear = season->m_iYear;
      tvshow.m_parentPathID = season->m_parentPathID;
      tvshow.m_premiered = season->m_premiered;
      tvshow.m_strMPAARating = season->m_strMPAARating;
      tvshow.m_strPlot = season->m_strPlot;
      tvshow.m_strSource = season->m_strSource;
      tvshow.m_strImportPath = season->m_strImportPath;
      tvshow.m_strTitle = tvshow.m_strShowTitle = season->m_strShowTitle;
      tvshow.m_studio = season->m_studio;
      tvshow.m_type = MediaTypeTvShow;
      tvshow.m_writingCredits = season->m_writingCredits;

      // try to find a proper path by going up in the path hierarchy once
      tvshow.m_strPath = tvshow.m_strShowPath = URIUtils::GetParentPath(season->GetPath());

      // add the basic tvshow to the database
      tvshow.m_iDbId = tvshow.m_iIdShow = videodb.SetDetailsForTvShow(tvshow.m_strPath, tvshow, CGUIListItem::ArtMap(), std::map<int, std::map<std::string, std::string> >());
      // store the tvshow's database ID in the season
      season->m_iIdShow = tvshow.m_iDbId;

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
    videodb.SetDetailsForSeason(*season, pItem->GetArt(), season->m_iIdShow);

    task->SetProgress(progress++, total);
  }

  return true;
}
