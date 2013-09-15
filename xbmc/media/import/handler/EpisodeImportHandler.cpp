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
#include "media/import/IMediaImportTask.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"

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

std::string CEpisodeImportHandler::GetItemLabel(const CFileItem* item) const
{
  if (item != nullptr && item->HasVideoInfoTag() && !item->GetVideoInfoTag()->m_strShowTitle.empty())
  {
    return StringUtils::Format(g_localizeStrings.Get(39015).c_str(),
      item->GetVideoInfoTag()->m_strShowTitle.c_str(),
      item->GetVideoInfoTag()->m_strTitle.c_str());
  }

  return CVideoImportHandler::GetItemLabel(item);
}

bool CEpisodeImportHandler::StartSynchronisation(const CMediaImport &import)
{
  if (!CVideoImportHandler::StartSynchronisation(import))
    return false;

  // create a map of tvshows imported from the same source
  CFileItemList tvshows;
  if (!m_db.GetTvShowsByWhere("videodb://tvshows/titles/", GetFilter(import), tvshows))
    return false;

  m_tvshows.clear();

  TvShowsMap::iterator tvshowsIter;
  for (int tvshowsIndex = 0; tvshowsIndex < tvshows.Size(); tvshowsIndex++)
  {
    CFileItemPtr tvshow = tvshows.Get(tvshowsIndex);
    if (!tvshow->HasVideoInfoTag() || tvshow->GetVideoInfoTag()->m_strTitle.empty())
      continue;

    tvshowsIter = m_tvshows.find(tvshow->GetVideoInfoTag()->m_strTitle);
    if (tvshowsIter == m_tvshows.end())
    {
      TvShowsSet tvshowsSet; tvshowsSet.insert(tvshow);
      m_tvshows.insert(make_pair(tvshow->GetVideoInfoTag()->m_strTitle, tvshowsSet));
    }
    else
      tvshowsIter->second.insert(tvshow);
  }

  return true;
}

bool CEpisodeImportHandler::AddImportedItem(const CMediaImport &import, CFileItem* item)
{
  if (item == nullptr)
    return false;

  PrepareItem(import, item);

  CVideoInfoTag *episode = item->GetVideoInfoTag();

  // try to find an existing tvshow that the episode belongs to
  episode->m_iIdShow = FindTvShowId(item);

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
    tvshow.m_strPath = showPath;

    // add the basic tvshow to the database
    std::vector< std::pair<std::string, std::string> > tvshowPaths; tvshowPaths.push_back(std::make_pair(tvshow.m_strPath, tvshow.m_basePath));
    tvshow.m_iDbId = tvshow.m_iIdShow = m_db.SetDetailsForTvShow(tvshowPaths, tvshow, CGUIListItem::ArtMap(), std::map<int, std::map<std::string, std::string> >());

    // store the tvshow's database ID in the episode
    episode->m_iIdShow = tvshow.m_iDbId;

    // add the tvshow to the tvshow map
    CFileItemPtr tvshowItem(new CFileItem(tvshow));
    tvshowItem->SetSource(item->GetSource());
    tvshowItem->SetImportPath(item->GetImportPath());

    auto&& tvshowsIter = m_tvshows.find(tvshow.m_strTitle);
    if (tvshowsIter == m_tvshows.end())
    {
      TvShowsSet tvshowsSet; tvshowsSet.insert(tvshowItem);
      m_tvshows.insert(make_pair(tvshow.m_strTitle, tvshowsSet));
    }
    else
      tvshowsIter->second.insert(tvshowItem);
  }

  episode->m_iDbId = m_db.SetDetailsForEpisode(item->GetPath(), *episode, item->GetArt(), episode->m_iIdShow);
  if (episode->m_iDbId <= 0)
    return false;

  SetDetailsForFile(item, false);
  return SetImportForItem(item, import);
}

bool CEpisodeImportHandler::UpdateImportedItem(const CMediaImport &import, CFileItem* item)
{
  if (item == nullptr || !item->HasVideoInfoTag() || item->GetVideoInfoTag()->m_iDbId <= 0)
    return false;

  if (m_db.SetDetailsForEpisode(item->GetPath(), *(item->GetVideoInfoTag()), item->GetArt(), item->GetVideoInfoTag()->m_iIdShow, item->GetVideoInfoTag()->m_iDbId) <= 0)
    return false;

  if (import.GetSettings().UpdatePlaybackMetadataFromSource())
    SetDetailsForFile(item, true);

  return true;
}

bool CEpisodeImportHandler::RemoveImportedItem(const CMediaImport &import, const CFileItem* item)
{
  if (item == nullptr || !item->HasVideoInfoTag())
    return false;

  RemoveImportedItem(m_db, import, item);
  return true;
}

bool CEpisodeImportHandler::GetLocalItems(CVideoDatabase &videodb, const CMediaImport &import, CFileItemList& items) const
{
  return videodb.GetEpisodesByWhere("videodb://tvshows/titles/", GetFilter(import), items, true, SortDescription(), import.GetSettings().UpdateImportedMediaItems());
}

void CEpisodeImportHandler::RemoveImportedItem(CVideoDatabase &videodb, const CMediaImport &import, const CFileItem* item) const
{
  if (item == nullptr || !item->HasVideoInfoTag())
    return;

  videodb.DeleteEpisode(item->GetVideoInfoTag()->m_iDbId);
  RemoveFile(videodb, item);
}

int CEpisodeImportHandler::FindTvShowId(const CFileItem* episodeItem)
{
  if (episodeItem == nullptr || !episodeItem->HasVideoInfoTag())
    return -1;

  // no comparison possible without a title
  if (episodeItem->GetVideoInfoTag()->m_strShowTitle.empty())
    return -1;

  // check if there is a tvshow with a matching title
  const auto& tvshowsIter = m_tvshows.find(episodeItem->GetVideoInfoTag()->m_strShowTitle);
  if (tvshowsIter == m_tvshows.end() ||
    tvshowsIter->second.size() <= 0)
    return -1;

  // if there is only one matching tvshow, we can go with that one
  if (tvshowsIter->second.size() == 1)
    return tvshowsIter->second.begin()->get()->GetVideoInfoTag()->m_iDbId;

  // use the path of the episode and tvshow to find the right tvshow
  for (const auto& it : tvshowsIter->second)
  {
    if (URIUtils::IsInPath(episodeItem->GetVideoInfoTag()->GetPath(), it->GetVideoInfoTag()->GetPath()))
      return it->GetVideoInfoTag()->m_iDbId;
  }

  return -1;
}
