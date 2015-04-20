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

#include <algorithm>
#include <string.h>

#include "MediaImportManager.h"
#include "FileItem.h"
#include "GUIUserMessages.h"
#include "LibraryQueue.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "dialogs/GUIDialogProgress.h"
#include "guilib/GUIWindowManager.h"
#include "interfaces/AnnouncementManager.h"
#include "media/import/IMediaImporter.h"
#include "media/import/IMediaImportHandler.h"
#include "media/import/IMediaImportRepository.h"
#include "media/import/MediaImport.h"
#include "media/import/MediaImportSource.h"
#include "media/import/MediaImportTaskProcessorJob.h"
#include "media/import/task/MediaImportChangesetTask.h"
#include "media/import/task/MediaImportRemovalTask.h"
#include "media/import/task/MediaImportRetrievalTask.h"
#include "media/import/task/MediaImportScrapingTask.h"
#include "media/import/task/MediaImportSourceRegistrationTask.h"
#include "media/import/task/MediaImportSynchronisationTask.h"
#include "media/import/task/MediaImportUpdateTask.h"
#include "threads/SingleLock.h"
#include "utils/log.h"
#include "utils/SpecialSort.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "utils/Variant.h"

// TODO
#include "media/import/handler/EpisodeImportHandler.h"
#include "media/import/handler/MovieImportHandler.h"
#include "media/import/handler/MusicVideoImportHandler.h"
#include "media/import/handler/SeasonImportHandler.h"
#include "media/import/handler/TvShowImportHandler.h"

CMediaImportManager::CMediaImportManager()
{
  // TODO
  RegisterMediaImportHandler(MediaImportHandlerPtr(new CMovieImportHandler()));
  RegisterMediaImportHandler(MediaImportHandlerPtr(new CTvShowImportHandler()));
  RegisterMediaImportHandler(MediaImportHandlerPtr(new CSeasonImportHandler()));
  RegisterMediaImportHandler(MediaImportHandlerPtr(new CEpisodeImportHandler()));
  RegisterMediaImportHandler(MediaImportHandlerPtr(new CMusicVideoImportHandler()));
}

CMediaImportManager::~CMediaImportManager()
{
  Uninitialize();
}

CMediaImportManager& CMediaImportManager::Get()
{
  static CMediaImportManager instance;
  return instance;
}

void CMediaImportManager::Uninitialize()
{
  // cancel all MediaImportTaskProcessorJobs
  CancelAllTaskProcessorJobs();

  {
    CSingleLock lock(m_importersLock);
    m_importers.clear();
  }

  {
    CSingleLock lock(m_importHandlersLock);
    m_importHandlers.clear();
    m_importHandlersMap.clear();
  }

  {
    CSingleLock lock(m_sourcesLock);
    for (const auto& it : m_sources)
      UnregisterSource(it.first);
    m_sources.clear();
  }

  {
    CSingleLock lock(m_importRepositoriesLock);
    m_importRepositories.clear();
  }
}

void CMediaImportManager::RegisterImportRepository(MediaImportRepositoryPtr importRepository)
{
  if (importRepository == nullptr || !importRepository->Initialize())
  {
    CLog::Log(LOGWARNING, "CMediaImportManager: failed to register and initialize given import repository");
    return;
  }

  std::vector<CMediaImportSource> sources = importRepository->GetSources();

  CSingleLock repositoriesLock(m_importRepositoriesLock);
  m_importRepositories.insert(importRepository);
  
  // add the sources from the repository. if the same source is in multiple
  // repositories, the std::map::insert will not overwrite the existing entry
  CSingleLock sourcesLock(m_sourcesLock);
  for (const auto& itSource : sources)
  {
    MediaImportSource importSource = { false, false };
    m_sources.insert(std::make_pair(itSource.GetIdentifier(), importSource));
  }

  CLog::Log(LOGDEBUG, "CMediaImportManager: new import repository with %zu sources added", sources.size());
}

bool CMediaImportManager::UnregisterImportRepository(const MediaImportRepositoryPtr importRepository)
{
  if (importRepository == nullptr)
    return false;

  CSingleLock repositoriesLock(m_importRepositoriesLock);
  const auto& it = m_importRepositories.find(importRepository);
  if (it == m_importRepositories.end())
    return false;

  // remove all sources from that repository
  std::vector<CMediaImportSource> sources = importRepository->GetSources();
  m_importRepositories.erase(it);

  CSingleLock sourceslock(m_sourcesLock);
  for (const auto& itSource : sources)
  {
    // only remove the source if it isn't part of another repository as well
    CMediaImportSource source(itSource.GetIdentifier());
    if (!FindSource(source.GetIdentifier(), source))
      m_sources.erase(itSource.GetIdentifier());
  }

  CLog::Log(LOGDEBUG, "CMediaImportManager: import repository with %zu sources removed", sources.size());
  return true;
}

void CMediaImportManager::RegisterImporter(MediaImporterPtr importer)
{
  if (importer == nullptr)
    return;

  CSingleLock lock(m_importersLock);
  if (m_importers.find(importer->GetIdentification()) == m_importers.end())
  {
    m_importers.insert(std::make_pair(importer->GetIdentification(), importer));
    CLog::Log(LOGDEBUG, "CMediaImportManager: new importer %s added", importer->GetIdentification());
  }
}

bool CMediaImportManager::UnregisterImporter(MediaImporterPtr importer)
{
  CSingleLock lock(m_importersLock);
  auto&& it = m_importers.find(importer->GetIdentification());
  if (it == m_importers.end())
    return false;

  m_importers.erase(it);
  CLog::Log(LOGDEBUG, "CMediaImportManager: importer %s removed", importer->GetFriendlySourceName().c_str());
  lock.Leave();

  return true;
}

std::vector<MediaImporterConstPtr> CMediaImportManager::GetImporters() const
{
  std::vector<MediaImporterConstPtr> importer;

  CSingleLock lock(m_importersLock);
  for (const auto& it : m_importers)
    importer.push_back(it.second);

  return importer;
}

const MediaImporterConstPtr CMediaImportManager::GetImporter(const std::string &path) const
{
  CSingleLock lock(m_importersLock);
  const auto& importer = std::find_if(m_importers.begin(), m_importers.end(),
    [&path](const std::pair<const char*, MediaImporterConstPtr>& importer) -> bool { return importer.second->CanImport(path); });
  if (importer != m_importers.end())
    return importer->second;

  return MediaImporterPtr();
}

void CMediaImportManager::RegisterMediaImportHandler(MediaImportHandlerPtr importHandler)
{
  if (importHandler == nullptr)
    return;

  CSingleLock lock(m_importHandlersLock);
  if (m_importHandlersMap.find(importHandler->GetMediaType()) == m_importHandlersMap.end())
  {
    m_importHandlersMap.insert(make_pair(importHandler->GetMediaType(), importHandler));

    // build a dependency list
    std::vector< std::pair<MediaType, MediaType > > dependencies;
    for (const auto& itHandler : m_importHandlersMap)
    {
      std::set<MediaType> mediaTypes = itHandler.second->GetDependencies();
      for (const auto& itMediaType : mediaTypes)
        dependencies.push_back(make_pair(itHandler.first, itMediaType));
    }

    // re-sort the import handlers and their dependencies
    std::vector<MediaType> result = SpecialSort::SortTopologically(dependencies);
    std::map<MediaType, MediaImportHandlerConstPtr> handlersCopy(m_importHandlersMap.begin(), m_importHandlersMap.end());
    m_importHandlers.clear();
    for (const auto& it : result)
    {
      m_importHandlers.push_back(handlersCopy.find(it)->second);
      handlersCopy.erase(it);
    }
    for (const auto& itHandler : handlersCopy)
      m_importHandlers.push_back(itHandler.second);

    CLog::Log(LOGDEBUG, "CMediaImportManager: new import handler for %s added", importHandler->GetMediaType().c_str());
  }
}

void CMediaImportManager::UnregisterMediaImportHandler(MediaImportHandlerPtr importHandler)
{
  if (importHandler == nullptr)
    return;

  CSingleLock lock(m_importHandlersLock);
  auto&& it = m_importHandlersMap.find(importHandler->GetMediaType());
  if (it == m_importHandlersMap.end() || it->second != importHandler)
    return;

  // remove the import handler from the map
  m_importHandlersMap.erase(it);
  // and from the sorted vector
  std::remove_if(m_importHandlers.begin(), m_importHandlers.end(),
    [&importHandler](const MediaImportHandlerConstPtr& vecIt) {
    if (vecIt == importHandler)
    {
      CLog::Log(LOGDEBUG, "CMediaImportManager: import handler for %s removed", importHandler->GetMediaType().c_str());
      return true;
    }

    return false;
  });
}

const MediaImportHandlerConstPtr CMediaImportManager::GetImportHandler(const MediaType& mediaType) const
{
  const auto& itMediaImportHandler = m_importHandlersMap.find(mediaType);
  if (itMediaImportHandler == m_importHandlersMap.end())
    return MediaImportHandlerPtr();

  return itMediaImportHandler->second;
}

std::vector<MediaImportHandlerConstPtr> CMediaImportManager::GetImportHandlers() const
{
  return m_importHandlers;
}

std::vector<MediaImportHandlerConstPtr> CMediaImportManager::GetImportHandlers(const std::set<MediaType>& mediaTypes) const
{
  if (mediaTypes.empty())
    return GetImportHandlers();

  std::vector<MediaImportHandlerConstPtr> importHandlers;
  CSingleLock lock(m_importHandlersLock);
  std::copy_if(m_importHandlers.begin(), m_importHandlers.end(), importHandlers.begin(),
    [&mediaTypes](const MediaImportHandlerConstPtr& importHandler) { return mediaTypes.find(importHandler->GetMediaType()) != mediaTypes.end(); });

  return importHandlers;
}

std::set<MediaType> CMediaImportManager::GetSupportedMediaTypes() const
{
  std::set<MediaType> mediaTypes;

  CSingleLock lock(m_importHandlersLock);
  // get all media types for which there are handlers
  for (const auto& it : m_importHandlers)
    mediaTypes.insert(it->GetMediaType());

  return mediaTypes;
}

std::vector<MediaType> CMediaImportManager::GetSupportedMediaTypesOrdered(bool reversed /* = false */) const
{
  std::vector<MediaType> mediaTypesOrdered;

  CSingleLock lock(m_importHandlersLock);
  // get all media types for which there are handlers
  for (const auto& it : m_importHandlers)
    mediaTypesOrdered.push_back(it->GetMediaType());
  lock.unlock();

  if (reversed)
    std::reverse(mediaTypesOrdered.begin(), mediaTypesOrdered.end());

  return mediaTypesOrdered;
}

std::vector<MediaType> CMediaImportManager::GetSupportedMediaTypesOrdered(const std::set<MediaType>& mediaTypes, bool reversed /* = false */) const
{
  std::vector<MediaType> mediaTypesOrdered;
  if (mediaTypes.empty())
    return GetSupportedMediaTypesOrdered();

  CSingleLock lock(m_importHandlersLock);
  for (const auto& it : m_importHandlers)
  {
    if (mediaTypes.find(it->GetMediaType()) != mediaTypes.end())
      mediaTypesOrdered.push_back(it->GetMediaType());
  }

  if (reversed)
    std::reverse(mediaTypesOrdered.begin(), mediaTypesOrdered.end());

  return mediaTypesOrdered;
}

std::vector< std::vector<MediaType> > CMediaImportManager::GetSupportedMediaTypesGrouped(const std::set<MediaType> &mediaTypes) const
{
  std::vector< std::vector<MediaType> > supportedMediaTypes;
  std::set<MediaType> handledMediaTypes;

  CSingleLock lock(m_importHandlersLock);
  // get all media types for which there are handlers
  for (const auto& handler : m_importHandlers)
  {
    if (mediaTypes.find(handler->GetMediaType()) == mediaTypes.end())
      continue;

    // make sure all required media types are available
    std::set<MediaType> requiredMediaTypes = handler->GetRequiredMediaTypes();
    if (std::any_of(requiredMediaTypes.begin(), requiredMediaTypes.end(), [&mediaTypes](const MediaType& mediaType) { return mediaTypes.find(mediaType) == mediaTypes.end(); }))
      continue;

    handledMediaTypes.insert(handler->GetMediaType());
  }

  for (const auto& handler : m_importHandlers)
  {
    const auto& itHandledMediaType = handledMediaTypes.find(handler->GetMediaType());
    if (itHandledMediaType == handledMediaTypes.end())
      continue;

    std::vector<MediaType> group;
    std::vector<MediaType> groupedMediaTypes = handler->GetGroupedMediaTypes();
    for (const auto& itGroupedMediaType : groupedMediaTypes)
    {
      const auto& itMediaType = handledMediaTypes.find(itGroupedMediaType);
      if (itMediaType != handledMediaTypes.end())
      {
        group.push_back(*itMediaType);
        handledMediaTypes.erase(itMediaType);
      }
    }

    if (group.empty())
    {
      group.push_back(*itHandledMediaType);
      handledMediaTypes.erase(itHandledMediaType);
    }

    supportedMediaTypes.push_back(group);
  }

  return supportedMediaTypes;
}

std::set<MediaType> CMediaImportManager::GetDependingMediaTypes(const MediaType &mediaType) const
{
  std::set<MediaType> dependingMediaTypes;
  if (mediaType.empty())
    return dependingMediaTypes;

  CSingleLock lock(m_importHandlersLock);
  for (const auto& itHandler : m_importHandlers)
  {
    std::set<MediaType> dependencies = itHandler->GetDependencies();
    if (dependencies.find(mediaType) != dependencies.end())
      dependingMediaTypes.insert(itHandler->GetMediaType());
  }

  return dependingMediaTypes;
}
  
std::set<MediaType> CMediaImportManager::GetGroupedMediaTypes(const MediaType &mediaType) const
{
  std::set<MediaType> groupedMediaTypes;
  if (mediaType.empty())
    return groupedMediaTypes;

  CSingleLock lock(m_importHandlersLock);
  const auto& itHandler = m_importHandlersMap.find(mediaType);
  if (itHandler == m_importHandlersMap.end())
    return groupedMediaTypes;

  std::vector<MediaType> vecGroupedMediaTypes = itHandler->second->GetGroupedMediaTypes();
  lock.Leave();

  groupedMediaTypes.insert(vecGroupedMediaTypes.begin(), vecGroupedMediaTypes.end());
  return groupedMediaTypes;
}
  
std::set<MediaType> CMediaImportManager::GetConnectedMediaTypes(const MediaType &mediaType) const
{
  std::set<MediaType> connectedMediaTypes;
  if (mediaType.empty())
    return connectedMediaTypes;

  CSingleLock lock(m_importHandlersLock);
  const auto& itHandler = m_importHandlersMap.find(mediaType);
  if (itHandler == m_importHandlersMap.end())
    return connectedMediaTypes;
  
  std::vector<MediaType> groupedMediaTypes = itHandler->second->GetGroupedMediaTypes();
  std::set<MediaType> requiredMediaTypes = itHandler->second->GetRequiredMediaTypes();
  connectedMediaTypes = CMediaImportManager::Get().GetDependingMediaTypes(mediaType);

  lock.Leave();

  connectedMediaTypes.insert(groupedMediaTypes.begin(), groupedMediaTypes.end());
  connectedMediaTypes.insert(requiredMediaTypes.begin(), requiredMediaTypes.end());

  // remove the passed in media type from the list
  connectedMediaTypes.erase(mediaType);

  return connectedMediaTypes;
}

void CMediaImportManager::RegisterSource(const std::string& sourceID, const std::string& friendlyName, const std::string& iconUrl /* = "" */, const std::set<MediaType>& mediaTypes /* = std::set<MediaType>() */)
{
  if (sourceID.empty() || friendlyName.empty())
  {
    CLog::Log(LOGWARNING, "CMediaImportManager: unable to register source with invalid identifier \"%s\" or friendly name \"%s\"",
      sourceID.c_str(), friendlyName.c_str());
    return;
  }

  CMediaImportSource source(sourceID, friendlyName, iconUrl, mediaTypes);

  CSingleLock sourcesLock(m_sourcesLock);
  auto&& itSource = m_sources.find(sourceID);
  if (itSource == m_sources.end() || !FindSource(sourceID, source))
  {
    CMediaImportTaskProcessorJob *processorJob = CMediaImportTaskProcessorJob::RegisterSource(source, this);
    AddTaskProcessorJob(source.GetIdentifier(), processorJob);

    CLog::Log(LOGINFO, "CMediaImportManager: source registration task for %s (%s) started",
      source.GetFriendlyName().c_str(), source.GetIdentifier().c_str());
    return;
  }

  // update any possibly changed values
  CMediaImportSource updatedSource = source;
  updatedSource.SetIconUrl(iconUrl);
  updatedSource.SetFriendlyName(friendlyName);
  updatedSource.SetAvailableMediaTypes(mediaTypes);

  bool added = false, updated = false;
  CSingleLock repositoriesLock(m_importRepositoriesLock);
  // try to update the source in at least one of the repositories
  for (auto& repository : m_importRepositories)
  {
    // if the source doesn't exist in a repository, add it now
    if (!repository->GetSource(updatedSource.GetIdentifier(), source))
    {
      if (repository->AddSource(updatedSource))
        added = true;
    }
    else if (repository->UpdateSource(updatedSource))
      updated = true;
  }
  repositoriesLock.Leave();
  sourcesLock.Leave();

  if (added)
    OnSourceAdded(updatedSource);
  if (updated)
  {
    CLog::Log(LOGDEBUG, "CMediaImportManager: source %s (%s) updated",
      source.GetFriendlyName().c_str(), sourceID.c_str());
    OnSourceUpdated(updatedSource);
  }

  // start processing all imports of the source
  Import(updatedSource, true);
}

void CMediaImportManager::UnregisterSource(const std::string& sourceID)
{
  if (sourceID.empty())
    return;

  CSingleLock sourcesLock(m_sourcesLock);
  auto&& itSource = m_sources.find(sourceID);
  if (itSource == m_sources.end() || itSource->second.removing)
    return;

  itSource->second.active = false;
  CLog::Log(LOGDEBUG, "CMediaImportManager: source %s disabled", sourceID.c_str());
  sourcesLock.Leave();

  // if there are tasks for the source which haven't started yet or are still running try to cancel them
  CancelTaskProcessorJobs(sourceID);

  // disable all items imported from the unregistered source
  std::vector<CMediaImport> imports = GetImportsBySource(sourceID);
  CSingleLock handlersLock(m_importHandlersLock);
  for (const auto& import : imports)
  {
    auto&& itHandler = m_importHandlersMap.find(import.GetMediaType());
    if (itHandler != m_importHandlersMap.end())
    {
      IMediaImportHandler* importHandler = itHandler->second->Create();
      importHandler->SetImportedItemsEnabled(import, false);
      delete importHandler;
    }
  }
  handlersLock.Leave();

  CMediaImportSource source(sourceID);
  if (!FindSource(sourceID, source))
    return;

  OnSourceDeactivated(source);
}

void CMediaImportManager::RemoveSource(const std::string& sourceID)
{
  if (sourceID.empty())
    return;

  CSingleLock sourcesLock(m_sourcesLock);
  auto&& itSource = m_sources.find(sourceID);
  if (itSource == m_sources.end() || itSource->second.removing)
    return;

  CMediaImportSource source(sourceID);
  if (!FindSource(sourceID, source))
    return;

  // mark the source as being removed
  itSource->second.removing = true;

  // if there are tasks for the source which haven't started yet or are still running try to cancel them
  CancelTaskProcessorJobs(sourceID);

  std::vector<CMediaImport> imports;
  CSingleLock importRepositoriesLock(m_importRepositoriesLock);
  for (const auto& repository : m_importRepositories)
  {
    std::vector<CMediaImport> repoImports = repository->GetImportsBySource(sourceID);
    for (auto& import : repoImports)
      imports.push_back(import);
  }
  importRepositoriesLock.Leave();

  CMediaImportTaskProcessorJob *processorJob = CMediaImportTaskProcessorJob::Remove(source, imports, this);
  AddTaskProcessorJob(source.GetIdentifier(), processorJob);

  CLog::Log(LOGINFO, "CMediaImportManager: source removal task for %s (%s) started",
    source.GetFriendlyName().c_str(), source.GetIdentifier().c_str());
}

bool CMediaImportManager::HasSources() const
{
  return !m_sources.empty();
}

bool CMediaImportManager::HasSources(bool active) const
{
  if (!HasSources())
    return false;

  CSingleLock sourcesLock(m_sourcesLock);
  return std::any_of(m_sources.begin(), m_sources.end(),
    [active](const std::pair<std::string, MediaImportSource>& source) { return source.second.active == active; });
}

std::vector<CMediaImportSource> CMediaImportManager::GetSources() const
{
  std::map<std::string, CMediaImportSource> mapSources;

  CSingleLock lock(m_importRepositoriesLock);
  for (const auto& repository : m_importRepositories)
  {
    std::vector<CMediaImportSource> tmpSources = repository->GetSources();
    for (auto& source : tmpSources)
    {
      const auto& itSourceLocal = m_sources.find(source.GetIdentifier());
      if (itSourceLocal == m_sources.end() || itSourceLocal->second.removing)
        continue;
      
      auto&& itSourceMap = mapSources.find(source.GetIdentifier());
      if (itSourceMap == mapSources.end())
      {
        // add the source to the list
        source.SetActive(itSourceLocal->second.active);
        mapSources.insert(std::make_pair(source.GetIdentifier(), source));
      }
      else
      {
        // check if we need to update the last synced timestamp
        if (itSourceMap->second.GetLastSynced() < source.GetLastSynced())
          itSourceMap->second.SetLastSynced(source.GetLastSynced());

        // update the list of media types
        std::set<MediaType> mediaTypes = itSourceMap->second.GetAvailableMediaTypes();
        mediaTypes.insert(source.GetAvailableMediaTypes().begin(), source.GetAvailableMediaTypes().end());
        itSourceMap->second.SetAvailableMediaTypes(mediaTypes);
      }
    }
  }
  lock.Leave();

  std::vector<CMediaImportSource> sources;
  for (const auto& itSourceMap : mapSources)
    sources.push_back(itSourceMap.second);

  return sources;
}

std::vector<CMediaImportSource> CMediaImportManager::GetSources(bool active) const
{
  std::map<std::string, CMediaImportSource> mapSources;

  CSingleLock lock(m_importRepositoriesLock);
  for (const auto& repository : m_importRepositories)
  {
    std::vector<CMediaImportSource> tmpSources = repository->GetSources();
    for (auto& itSource : tmpSources)
    {
      const auto& itSourceLocal = m_sources.find(itSource.GetIdentifier());
      if (itSourceLocal == m_sources.end() ||
          itSourceLocal->second.active != active ||
          itSourceLocal->second.removing)
        continue;

      auto&& itSourceMap = mapSources.find(itSource.GetIdentifier());
      if (itSourceMap == mapSources.end())
      {
        // add the source to the list
        itSource.SetActive(itSourceLocal->second.active);
        mapSources.insert(std::make_pair(itSource.GetIdentifier(), itSource));
      }
      else
      {
        // check if we need to update the last synced timestamp
        if (itSourceMap->second.GetLastSynced() < itSource.GetLastSynced())
          itSourceMap->second.SetLastSynced(itSource.GetLastSynced());

        // update the list of media types
        std::set<MediaType> mediaTypes = itSourceMap->second.GetAvailableMediaTypes();
        mediaTypes.insert(itSource.GetAvailableMediaTypes().begin(), itSource.GetAvailableMediaTypes().end());
        itSourceMap->second.SetAvailableMediaTypes(mediaTypes);
      }
    }
  }
  lock.Leave();

  std::vector<CMediaImportSource> sources;
  for (const auto& itSourceMap : mapSources)
    sources.push_back(itSourceMap.second);

  return sources;
}

bool CMediaImportManager::GetSource(const std::string& sourceID, CMediaImportSource& source) const
{
  if (sourceID.empty())
    return false;

  const auto& itSource = m_sources.find(sourceID);
  if (itSource == m_sources.end() || itSource->second.removing)
    return false;

  if (!FindSource(sourceID, source))
    return false;

  source.SetActive(itSource->second.active);
  return true;
}

bool CMediaImportManager::IsSourceActive(const std::string& sourceID) const
{
  if (sourceID.empty())
    return false;

  const auto& itSource = m_sources.find(sourceID);
  if (itSource == m_sources.end() || itSource->second.removing)
    return false;

  return itSource->second.active;
}

bool CMediaImportManager::IsSourceActive(const CMediaImportSource& source) const
{
  return IsSourceActive(source.GetIdentifier());
}

bool CMediaImportManager::HasImports() const
{
  if (m_sources.empty())
    return false;

  CSingleLock importRepositoriesLock(m_importRepositoriesLock);
  return std::any_of(m_importRepositories.begin(), m_importRepositories.end(),
    [](const MediaImportRepositoryPtr& repository) { return !repository->GetImports().empty(); });
}

bool CMediaImportManager::HasImports(const std::string& sourceID) const
{
  if (sourceID.empty())
    return false;

  const auto& itSource = m_sources.find(sourceID);
  if (itSource == m_sources.end() || itSource->second.removing)
    return false;

  CSingleLock importRepositoriesLock(m_importRepositoriesLock);
  for (const auto& repository : m_importRepositories)
  {
    std::vector<CMediaImport> repoImports = repository->GetImportsBySource(sourceID);
    if (!repoImports.empty())
      return true;
  }

  return false;
}

bool CMediaImportManager::HasImports(const CMediaImportSource &source) const
{
  return HasImports(source.GetIdentifier());
}

bool CMediaImportManager::AddImport(const std::string &sourceID, const std::string &path, const MediaType &mediaType)
{
  if (sourceID.empty() || path.empty() || mediaType == MediaTypeNone)
  {
    CLog::Log(LOGERROR, "CMediaImportManager: unable to add new import from source \"%s\" with path \"%s\" and media type \"%s\"",
              sourceID.c_str(), path.c_str(), mediaType.c_str());
    return false;
  }

  // check if the import already exists
  CMediaImport import(path, mediaType, sourceID);
  if (FindImport(path, mediaType, import))
  {
    CLog::Log(LOGERROR, "CMediaImportManager: unable to add already existing import from source \"%s\" with path \"%s\" and media type \"%s\"",
              sourceID.c_str(), path.c_str(), mediaType.c_str());
    return false;
  }

  // check if the given source exists
  CMediaImportSource source(sourceID);
  CSingleLock sourcesLock(m_sourcesLock);
  auto&& itSource = m_sources.find(sourceID);
  if (itSource == m_sources.end() ||  // source doesn't exist
      !FindSource(sourceID, source))
  {
    CLog::Log(LOGWARNING, "CMediaImportManager: trying to add new import from unknown source \"%s\"", sourceID.c_str());
    return false;
  }

  // check if there's an import handler that can handle the given media type
  if (m_importHandlersMap.find(mediaType) == m_importHandlersMap.end())
  {
    CLog::Log(LOGERROR, "CMediaImportManager: unable to add new import from source \"%s\" with path \"%s\" and media type \"%s\" because there is no matching import handler available",
              sourceID.c_str(), path.c_str(), mediaType.c_str());
    return false;
  }

  CMediaImport newImport(path, mediaType, source);
  if (!AddImport(newImport))
  {
    CLog::Log(LOGERROR, "CMediaImportManager: failed to add new import for source \"%s\" with path \"%s\" and media type \"%s\" to any import repository",
              sourceID.c_str(), path.c_str(), mediaType.c_str());
    return false;
  }
  sourcesLock.Leave();

  OnImportAdded(newImport);

  return true;
}

bool CMediaImportManager::AddImports(const std::string &sourceID, const std::string &path, const std::set<MediaType> &mediaTypes)
{
  if (sourceID.empty() || path.empty() || mediaTypes.empty())
  {
    CLog::Log(LOGERROR, "CMediaImportManager: unable to add new import from source \"%s\" with path \"%s\" and media types (%zu)",
              sourceID.c_str(), path.c_str(), mediaTypes.size());
    return false;
  }

  // check if the given source exists
  CMediaImportSource source(sourceID);
  CSingleLock sourcesLock(m_sourcesLock);
  auto&& itSource = m_sources.find(sourceID);
  if (itSource == m_sources.end() ||  // source doesn't exist
      itSource->second.removing ||
      !FindSource(sourceID, source))
  {
    CLog::Log(LOGWARNING, "CMediaImportManager: trying to add new imports from unknown source \"%s\"", sourceID.c_str());
    return false;
  }

  std::vector<CMediaImport> addedImports;
  for (const auto& itMediaType : mediaTypes)
  {
    // check if the import already exists
    CMediaImport import(path, itMediaType, sourceID);
    if (FindImport(path, itMediaType, import))
    {
      CLog::Log(LOGERROR, "CMediaImportManager: unable to add already existing import from source \"%s\" with path \"%s\" and media type \"%s\"",
                sourceID.c_str(), path.c_str(), itMediaType.c_str());
      continue;
    }

    // check if there's an import handler that can handle the given media type
    if (m_importHandlersMap.find(itMediaType) == m_importHandlersMap.end())
    {
      CLog::Log(LOGERROR, "CMediaImportManager: unable to add new import from source \"%s\" with path \"%s\" and media type \"%s\" because there is no matching import handler available",
                sourceID.c_str(), path.c_str(), itMediaType.c_str());
      continue;
    }

    CMediaImport newImport(path, itMediaType, source);
    if (!AddImport(newImport))
    {
      CLog::Log(LOGERROR, "CMediaImportManager: failed to add new import for source \"%s\" with path \"%s\" and media type \"%s\" to any import repository",
                sourceID.c_str(), path.c_str(), itMediaType.c_str());
      continue;
    }

    addedImports.push_back(newImport);
  }
  sourcesLock.Leave();

  for (const auto& import : addedImports)
    OnImportAdded(import);

  return true;
}

bool CMediaImportManager::UpdateImport(const CMediaImport &import)
{
  if (import.GetPath().empty() || import.GetMediaType().empty())
    return false;

  bool updated = false;
  CSingleLock importRepositoriesLock(m_importRepositoriesLock);
  for (const auto& repository : m_importRepositories)
    updated |= repository->UpdateImport(import);
  importRepositoriesLock.Leave();

  OnImportUpdated(import);

  return updated;
}

void CMediaImportManager::RemoveImport(const std::string& path, const MediaType &mediaType)
{
  if (path.empty() || mediaType == MediaTypeNone)
    return;

  CMediaImport import(path, mediaType, "");
  if (!FindImport(path, mediaType, import))
    return;

  CMediaImportTaskProcessorJob *processorJob = CMediaImportTaskProcessorJob::Remove(import, this);
  AddTaskProcessorJob(import.GetSource().GetIdentifier(), processorJob);

  CLog::Log(LOGINFO, "CMediaImportManager: import removal task for %s items of %s started", import.GetMediaType().c_str(), import.GetPath().c_str());
}

std::vector<CMediaImport> CMediaImportManager::GetImports() const
{
  std::vector<CMediaImport> imports;
  CSingleLock importRepositoriesLock(m_importRepositoriesLock);
  CSingleLock sourcesLock(m_sourcesLock);
  for (const auto& repository : m_importRepositories)
  {
    std::vector<CMediaImport> repoImports = repository->GetImports();
    for (auto& import : repoImports)
    {
      const auto& itSource = m_sources.find(import.GetSource().GetIdentifier());
      if (itSource == m_sources.end() || itSource->second.removing)
        continue;

      import.SetActive(itSource->second.active);
      imports.push_back(import);
    }
  }

  return imports;
}

std::vector<CMediaImport> CMediaImportManager::GetImportsByMediaType(const MediaType &mediaType) const
{
  std::vector<CMediaImport> imports;
  if (mediaType.empty())
    return imports;

  CSingleLock importRepositoriesLock(m_importRepositoriesLock);
  CSingleLock sourcesLock(m_sourcesLock);
  for (const auto& repository : m_importRepositories)
  {
    std::vector<CMediaImport> repoImports = repository->GetImportsByMediaType(mediaType);
    for (auto& import : repoImports)
    {
      const auto& itSource = m_sources.find(import.GetSource().GetIdentifier());
      if (itSource == m_sources.end() || itSource->second.removing)
        continue;

      import.SetActive(itSource->second.active);
      imports.push_back(import);
    }
  }

  return imports;
}

std::vector<CMediaImport> CMediaImportManager::GetImportsBySource(const std::string &sourceID) const
{
  std::vector<CMediaImport> imports;
  if (sourceID.empty())
    return imports;
  
  CSingleLock sourcesLock(m_sourcesLock);
  const auto& itSource = m_sources.find(sourceID);
  if (itSource == m_sources.end() || itSource->second.removing)
    return imports;

  CSingleLock importRepositoriesLock(m_importRepositoriesLock);
  for (const auto& repository : m_importRepositories)
  {
    std::vector<CMediaImport> repoImports = repository->GetImportsBySource(sourceID);
    for (auto& import : repoImports)
    {
      import.SetActive(itSource->second.active);
      imports.push_back(import);
    }
  }

  return imports;
}

bool CMediaImportManager::GetImport(const std::string &path, const MediaType &mediaType, CMediaImport &import) const
{
  if (path.empty())
    return false;

  return FindImport(path, mediaType, import);
}

bool CMediaImportManager::Import()
{
  bool result = false;
  CSingleLock sourcesLock(m_sourcesLock);
  for (const auto& source : m_sources)
  {
    if (!source.second.active || source.second.removing)
      continue;

    result |= Import(source.first);
  }

  return result;
}

bool CMediaImportManager::Import(const std::string& sourceID)
{
  if (sourceID.empty())
  {
    CLog::Log(LOGWARNING, "CMediaImportManager: unable to import from an empty source identifier");
    return false;
  }

  CMediaImportSource source(sourceID);
  if (!FindSource(sourceID, source))
    return false;

  Import(source, false);
  return true;
}

bool CMediaImportManager::Import(const std::string& path, const MediaType& mediaType)
{
  if (path.empty() || mediaType.empty())
  {
    CLog::Log(LOGWARNING, "CMediaImportManager: unable to import from invalid path \"%s\" or without media type (%s)",
              path.c_str(), mediaType.c_str());
    return false;
  }

  CMediaImport import(path, mediaType, "");
  if (!FindImport(path, mediaType, import))
    return false;

  // check if the source exists and is active
  CSingleLock sourcesLock(m_sourcesLock);
  auto&& itSource = m_sources.find(import.GetSource().GetIdentifier());
  if (itSource == m_sources.end() ||  // source doesn't exist
      itSource->second.removing ||
      !itSource->second.active)
  {
    CLog::Log(LOGWARNING, "CMediaImportManager: unable to import from unregistered source \"%s\"",
              import.GetSource().GetIdentifier().c_str());
    return false;
  }

  // import media items from the given import (and all other imports whose media types depend upon this import's one or are grouped together with it)
  Import(import);

  return true;
}

void CMediaImportManager::Import(const CMediaImportSource &source, bool automatically /* = false */)
{
  if (!m_sources[source.GetIdentifier()].active)
  {
    m_sources[source.GetIdentifier()].active = true;

    OnSourceActivated(source);
  }

  CMediaImportTaskProcessorJob *processorJob = CMediaImportTaskProcessorJob::Import(source, automatically, this);
  AddTaskProcessorJob(source.GetIdentifier(), processorJob);

  CLog::Log(LOGINFO, "CMediaImportManager: import task for source %s started", source.GetIdentifier().c_str());
}

void CMediaImportManager::Import(const CMediaImport &import, bool automatically /* = false */)
{
  CMediaImportTaskProcessorJob *processorJob = CMediaImportTaskProcessorJob::Import(import, automatically, this);
  AddTaskProcessorJob(import.GetSource().GetIdentifier(), processorJob);

  CLog::Log(LOGINFO, "CMediaImportManager: import task for %s items from %s started", import.GetMediaType().c_str(), import.GetPath().c_str());
}

bool CMediaImportManager::AddSource(const CMediaImportSource &source)
{
  bool added = false;
  CSingleLock repositoriesLock(m_importRepositoriesLock);
  // try to add the source to at least one of the repositories
  for (auto& repository : m_importRepositories)
  {
    if (repository->AddSource(source))
      added = true;
  }

  if (!added)
    return false;

  MediaImportSource importSource = { true, false };
  m_sources.insert(std::make_pair(source.GetIdentifier(), importSource));

  return true;
}

bool CMediaImportManager::FindSource(const std::string &sourceID, CMediaImportSource &source) const
{
  CSingleLock repositoriesLock(m_importRepositoriesLock);
  // try to find the source in at least one of the repositories
  return std::any_of(m_importRepositories.begin(), m_importRepositories.end(),
    [&sourceID, &source](const MediaImportRepositoryPtr& repository) { return repository->GetSource(sourceID, source); });
}

bool CMediaImportManager::AddImport(const CMediaImport &import)
{
  bool added = false;
  CSingleLock repositoriesLock(m_importRepositoriesLock);
  // try to add the import to at least one of the repositories
  for (auto& repository : m_importRepositories)
  {
    if (repository->AddImport(import))
      added = true;
  }

  return added;
}

bool CMediaImportManager::FindImport(const std::string &path, const MediaType &mediaType, CMediaImport &import) const
{
  CSingleLock repositoriesLock(m_importRepositoriesLock);
  // try to find the import in at least one of the repositories
  return std::any_of(m_importRepositories.begin(), m_importRepositories.end(),
    [&path, &mediaType, &import](const MediaImportRepositoryPtr& repository) { return repository->GetImport(path, mediaType, import); });
}

bool CMediaImportManager::UpdateImportedItem(const CFileItem &item)
{
  if (!item.IsImported())
    return false;
  
  std::string sourceID = item.GetSource();
  std::string importPath = item.GetImportPath();
  MediaType mediaType;

  CSingleLock repositoriesLock(m_importRepositoriesLock);
  // try to add the source to at least one of the repositories
  for (const auto& repository : m_importRepositories)
  {
    if (repository->GetMediaType(item, mediaType))
      break;
  }
  repositoriesLock.Leave();

  if (mediaType.empty())
  {
    CLog::Log(LOGWARNING, "CMediaImportManager: unable to update %s due to unknown media type", item.GetPath().c_str());
    return false;
  }

  CMediaImport import(importPath, mediaType, sourceID);
  if (!FindImport(importPath, mediaType, import))
  {
    CLog::Log(LOGWARNING, "CMediaImportManager: unable to update %s due to no import found for %s of %s", item.GetPath().c_str(), importPath.c_str(), mediaType.c_str());
    return false;
  }

  if (!import.GetSettings().UpdatePlaybackMetadataOnSource())
    return false;

  if (!m_sources[import.GetSource().GetIdentifier()].active)
  {
    CLog::Log(LOGWARNING, "CMediaImportManager: unable to update item %s on inactive source %s", item.GetPath().c_str(), import.GetSource().GetIdentifier().c_str());
    return false;
  }

  const MediaImporterConstPtr importer = GetImporter(importPath);
  if (importer == nullptr)
    return false;

  CMediaImportTaskProcessorJob *processorJob = CMediaImportTaskProcessorJob::UpdateImportedItem(import, item, this);
  AddTaskProcessorJob(import.GetSource().GetIdentifier(), processorJob);

  CLog::Log(LOGINFO, "CMediaImportManager: import update task for %s on %s started", item.GetPath().c_str(), import.GetPath().c_str());

  return true;
}

void CMediaImportManager::AddTaskProcessorJob(const std::string &path, CMediaImportTaskProcessorJob *job)
{
  if (job == nullptr)
    return;

  auto&& itJobs = m_jobMap.find(path);
  if (itJobs == m_jobMap.end())
  {
    MediaImportTaskProcessorJobs jobs;
    jobs.insert(job);
    m_jobMap.insert(std::make_pair(path, jobs));
  }
  else
    itJobs->second.insert(job);

  CLibraryQueue::Get().AddJob(job, this);
}

void CMediaImportManager::RemoveTaskProcessorJob(const std::string &path, const CMediaImportTaskProcessorJob *job)
{
  auto&& itJobs = m_jobMap.find(path);
  if (itJobs != m_jobMap.end())
  {
    itJobs->second.erase(const_cast<CMediaImportTaskProcessorJob*>(job));
    if (itJobs->second.empty())
      m_jobMap.erase(itJobs);
  }
}

void CMediaImportManager::CancelTaskProcessorJobs(const std::string &sourceID)
{
  const auto& itJobs = m_jobMap.find(sourceID);
  if (itJobs != m_jobMap.end())
  {
    for (const auto& itJob : itJobs->second)
    {
      if (itJob != nullptr)
        CLibraryQueue::Get().CancelJob(itJob);
    }

    m_jobMap.erase(sourceID);
    CLog::Log(LOGDEBUG, "CMediaImportManager: tasks for %s cancelled", sourceID.c_str());
  }
}

void CMediaImportManager::CancelAllTaskProcessorJobs()
{
  std::vector<std::string> sources;
  for (const auto& itJobs : m_jobMap)
    sources.push_back(itJobs.first);

  for (const auto& source : sources)
    CancelTaskProcessorJobs(source);
}

void CMediaImportManager::OnJobComplete(unsigned int jobID, bool success, CJob *job)
{
  CSingleLock jobsLock(m_jobsLock);
  if (job != nullptr)
  {
    CMediaImportTaskProcessorJob *processorJob = static_cast<CMediaImportTaskProcessorJob*>(job);
    if (processorJob != nullptr)
      RemoveTaskProcessorJob(processorJob->GetPath(), processorJob);

    const std::string& sourceID = processorJob->GetPath();
    if (!sourceID.empty())
    {
      CSingleLock sourcesLock(m_sourcesLock);
      // check if the source belonging to the completed process is being removed
      auto&& itSource = m_sources.find(sourceID);
      if (itSource != m_sources.end() && itSource->second.removing)
      {
        CMediaImportSource source(sourceID);
        if (FindSource(sourceID, source))
        {
          CSingleLock repositoriesLock(m_importRepositoriesLock);
          for (auto& repository : m_importRepositories)
            repository->RemoveSource(sourceID);
          repositoriesLock.Leave();

          m_sources.erase(itSource);
          sourcesLock.Leave();

          OnSourceRemoved(source);
          CLog::Log(LOGDEBUG, "CMediaImportManager: source %s (%s) removed", source.GetFriendlyName().c_str(), sourceID.c_str());
        }
      }
    }
  }
}

void CMediaImportManager::OnJobProgress(unsigned int jobID, unsigned int progress, unsigned int total, const CJob *job)
{
  if (strcmp(job->GetType(), "MediaImportTaskProcessorJob") == 0)
  {
    const CMediaImportTaskProcessorJob* processorJob = static_cast<const CMediaImportTaskProcessorJob*>(job);
    if (processorJob->GetCurrentTask() != nullptr &&
        processorJob->GetCurrentTask()->GetProgressBarHandle() != nullptr)
      processorJob->GetCurrentTask()->GetProgressBarHandle()->SetProgress(progress, total);
  }
}

bool CMediaImportManager::OnTaskComplete(bool success, IMediaImportTask *task)
{
  if (task == nullptr)
    return false;

  MediaImportTaskType taskType = task->GetType();
  if (taskType == MediaImportTaskType::SourceRegistration)
  {
    CMediaImportSourceRegistrationTask* sourceRegistrationTask = dynamic_cast<CMediaImportSourceRegistrationTask*>(task);
    if (sourceRegistrationTask == nullptr)
      return false;

    const CMediaImportSource &source = sourceRegistrationTask->GetImport().GetSource();

    // nothing to do if the import job failed
    if (!success)
    {
      CLog::Log(LOGWARNING, "CMediaImportManager: source registration task for (%s) %s failed",
        source.GetFriendlyName().c_str(), source.GetIdentifier().c_str());
      return false;
    }

    if (!AddSource(source))
    {
      CLog::Log(LOGWARNING, "CMediaImportManager: source (%s) %s couldn't be added to any import repository",
        source.GetFriendlyName().c_str(), source.GetIdentifier().c_str());
      return false;
    }
    OnSourceAdded(source);

    CLog::Log(LOGDEBUG, "CMediaImportManager: new source %s (%s) registered",
      source.GetFriendlyName().c_str(), source.GetIdentifier().c_str());
    CGUIDialogKaiToast::QueueNotification(source.GetIconUrl(), g_localizeStrings.Get(39000), source.GetFriendlyName());
  }
  else if (taskType == MediaImportTaskType::Synchronisation)
  {
    CMediaImportSynchronisationTask* synchronisationTask = dynamic_cast<CMediaImportSynchronisationTask*>(task);
    if (synchronisationTask == nullptr)
      return false;

    if (!success)
      return false;

    CMediaImport import = synchronisationTask->GetImport();
    CSingleLock repositoriesLock(m_importRepositoriesLock);
    for (auto& repository : m_importRepositories)
    {
      if (repository->UpdateLastSync(import))
      {
        OnSourceUpdated(import.GetSource());
        OnImportUpdated(import);
        break;
      }
    }
  }
  else if (taskType == MediaImportTaskType::Removal)
  {
    CMediaImportRemovalTask* removalTask = dynamic_cast<CMediaImportRemovalTask*>(task);
    if (removalTask == nullptr)
      return false;

    if (!success)
      return false;

    const CMediaImport& import = removalTask->GetImport();

    // remove the import from the import repositories
    CSingleLock repositoriesLock(m_importRepositoriesLock);
    for (auto& repository : m_importRepositories)
      repository->RemoveImport(import);
    repositoriesLock.Leave();

    // let everyone know that the import has been removed
    OnImportRemoved(import);
  }

  return true;
}

void CMediaImportManager::OnSourceAdded(const CMediaImportSource &source)
{
  SendSourceMessage(source, GUI_MSG_SOURCE_ADDED);
}

void CMediaImportManager::OnSourceUpdated(const CMediaImportSource &source)
{
  SendSourceMessage(source, GUI_MSG_SOURCE_UPDATED);
}

void CMediaImportManager::OnSourceRemoved(const CMediaImportSource &source)
{
  SendSourceMessage(source, GUI_MSG_SOURCE_REMOVED);
}

void CMediaImportManager::OnSourceActivated(const CMediaImportSource &source)
{
  SendSourceMessage(source, GUI_MSG_SOURCE_ACTIVE_CHANGED, 1);
}

void CMediaImportManager::OnSourceDeactivated(const CMediaImportSource &source)
{
  SendSourceMessage(source, GUI_MSG_SOURCE_ACTIVE_CHANGED, 0);
}

void CMediaImportManager::OnImportAdded(const CMediaImport &import)
{
  SendImportMessage(import, GUI_MSG_IMPORT_ADDED);
}

void CMediaImportManager::OnImportUpdated(const CMediaImport &import)
{
  SendImportMessage(import, GUI_MSG_IMPORT_UPDATED);
}

void CMediaImportManager::OnImportRemoved(const CMediaImport &import)
{
  SendImportMessage(import, GUI_MSG_IMPORT_REMOVED);
}

void CMediaImportManager::SendSourceMessage(const CMediaImportSource &source, int message, int param /* = 0 */)
{
  CFileItemPtr sourceItem(new CFileItem(source.GetFriendlyName()));
  sourceItem->SetProperty("Source.ID", source.GetIdentifier());

  CGUIMessage msg(GUI_MSG_NOTIFY_ALL, 0, 0, message, param, sourceItem);
  g_windowManager.SendThreadMessage(msg);
}

void CMediaImportManager::SendImportMessage(const CMediaImport &import, int message)
{
  CFileItemPtr importItem(new CFileItem());
  importItem->SetProperty("Source.ID", import.GetSource().GetIdentifier());
  importItem->SetProperty("Import.Path", import.GetPath());
  importItem->SetProperty("Import.MediaType", import.GetMediaType());

  CGUIMessage msg(GUI_MSG_NOTIFY_ALL, 0, 0, message, 0, importItem);
  g_windowManager.SendThreadMessage(msg);
}
