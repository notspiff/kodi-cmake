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

#include <string.h>

#include "MediaImportManager.h"
#include "FileItem.h"
#include "GUIUserMessages.h"
#include "guilib/GUIWindowManager.h"
#include "interfaces/AnnouncementManager.h"
#include "media/import/IMediaImporter.h"
#include "media/import/IMediaImportHandler.h"
#include "media/import/IMediaImportRepository.h"
#include "media/import/MediaImport.h"
#include "media/import/MediaImportSource.h"
#include "media/import/MediaImportTaskProcessorJob.h"
#include "media/import/task/MediaImportRetrievalTask.h"
#include "media/import/task/MediaImportSourceRegistrationTask.h"
#include "media/import/task/MediaImportSynchronisationTask.h"
#include "media/import/task/MediaImportUpdateTask.h"
#include "threads/SingleLock.h"
#include "utils/log.h"
#include "utils/SpecialSort.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "video/VideoDatabase.h"

// TODO
#include "media/import/handler/EpisodeImportHandler.h"
#include "media/import/handler/MovieImportHandler.h"
#include "media/import/handler/MusicVideoImportHandler.h"
#include "media/import/handler/SeasonImportHandler.h"
#include "media/import/handler/TvShowImportHandler.h"

CMediaImportManager::CMediaImportManager()
  : CJobQueue(false, 1, CJob::PRIORITY_LOW_PAUSABLE)
{
  // TODO
  RegisterMediaImportHandler(new CMovieImportHandler());
  RegisterMediaImportHandler(new CTvShowImportHandler());
  RegisterMediaImportHandler(new CSeasonImportHandler());
  RegisterMediaImportHandler(new CEpisodeImportHandler());
  RegisterMediaImportHandler(new CMusicVideoImportHandler());
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
  CancelJobs();

  {
    CSingleLock lock(m_importersLock);
    m_importers.clear();
  }

  {
    // TODO
    CSingleLock lock(m_importHandlersLock);
    for (std::vector<IMediaImportHandler*>::const_iterator itHandler = m_importHandlers.begin(); itHandler != m_importHandlers.end(); ++itHandler)
      delete *itHandler;
    m_importHandlers.clear();
    m_importHandlersMap.clear();
  }

  {
    CSingleLock lock(m_sourcesLock);
    for (std::map<std::string, bool>::const_iterator it = m_sources.begin(); it != m_sources.end(); ++it)
      UnregisterSource(it->first);
    m_sources.clear();
  }

  {
    CSingleLock lock(m_importRepositoriesLock);
    m_importRepositories.clear();
  }
}

void CMediaImportManager::RegisterSourceRepository(IMediaImportRepository* importRepository)
{
  if (importRepository == NULL ||
      !importRepository->Initialize())
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
  for (std::vector<CMediaImportSource>::const_iterator itSource = sources.begin(); itSource != sources.end(); ++itSource)
    m_sources.insert(std::make_pair(itSource->GetIdentifier(), false));

  CLog::Log(LOGDEBUG, "CMediaImportManager: new import repository with %zu sources added", sources.size());
}

bool CMediaImportManager::UnregisterSourceRepository(const IMediaImportRepository* importRepository)
{
  if (importRepository == NULL)
    return false;

  CSingleLock repositoriesLock(m_importRepositoriesLock);
  std::set<IMediaImportRepository*>::const_iterator it = m_importRepositories.find(const_cast<IMediaImportRepository*>(importRepository));
  if (it == m_importRepositories.end())
    return false;

  // remove all sources from that repository
  std::vector<CMediaImportSource> sources = importRepository->GetSources();
  m_importRepositories.erase(it);

  CSingleLock sourceslock(m_sourcesLock);
  for (std::vector<CMediaImportSource>::const_iterator itSource = sources.begin(); itSource != sources.end(); ++itSource)
  {
    // only remove the source if it isn't part of another repository as well
    CMediaImportSource source(itSource->GetIdentifier());
    if (!FindSource(source.GetIdentifier(), source))
      m_sources.erase(itSource->GetIdentifier());
  }

  CLog::Log(LOGDEBUG, "CMediaImportManager: import repository with %zu sources removed", sources.size());
  return true;
}

void CMediaImportManager::RegisterImporter(const IMediaImporter* importer)
{
  if (importer == NULL)
    return;

  CSingleLock lock(m_importersLock);
  if (m_importers.find(importer->GetIdentification()) == m_importers.end())
  {
    m_importers.insert(std::make_pair(importer->GetIdentification(), importer));
    CLog::Log(LOGDEBUG, "CMediaImportManager: new importer %s added", importer->GetIdentification());
  }
}

bool CMediaImportManager::UnregisterImporter(const IMediaImporter* importer)
{
  CSingleLock lock(m_importersLock);
  std::map<const char*, const IMediaImporter*>::iterator it = m_importers.find(importer->GetIdentification());
  if (it == m_importers.end())
    return false;

  m_importers.erase(it);
  CLog::Log(LOGDEBUG, "CMediaImportManager: importer %s removed", importer->GetFriendlySourceName().c_str());
  lock.Leave();

  return true;
}

std::vector<const IMediaImporter*> CMediaImportManager::GetImporter() const
{
  std::vector<const IMediaImporter*> importer;

  CSingleLock lock(m_importersLock);
  for (std::map<const char*, const IMediaImporter*>::const_iterator it = m_importers.begin(); it != m_importers.end(); ++it)
    importer.push_back(it->second);

  return importer;
}

const IMediaImporter* CMediaImportManager::GetImporter(const std::string &path) const
{
  CSingleLock lock(m_importersLock);
  for (std::map<const char*, const IMediaImporter*>::const_iterator it = m_importers.begin(); it != m_importers.end(); ++it)
  {
    if (it->second->CanImport(path))
      return it->second;
  }

  return NULL;
}

void CMediaImportManager::RegisterMediaImportHandler(IMediaImportHandler *importHandler)
{
  if (importHandler == NULL)
    return;

  CSingleLock lock(m_importHandlersLock);
  if (m_importHandlersMap.find(importHandler->GetMediaType()) == m_importHandlersMap.end())
  {
    m_importHandlersMap.insert(make_pair(importHandler->GetMediaType(), importHandler));

    // build a dependency list
    std::vector< std::pair<MediaType, MediaType > > dependencies;
    for (std::map<MediaType, IMediaImportHandler*>::const_iterator itHandler = m_importHandlersMap.begin(); itHandler != m_importHandlersMap.end(); ++itHandler)
    {
      std::set<MediaType> mediaTypes = itHandler->second->GetDependencies();
      for (std::set<MediaType>::const_iterator itMediaType = mediaTypes.begin(); itMediaType != mediaTypes.end(); ++itMediaType)
        dependencies.push_back(make_pair(itHandler->first, *itMediaType));
    }

    // re-sort the import handlers and their dependencies
    std::vector<MediaType> result = SpecialSort::SortTopologically(dependencies);
    std::map<MediaType, IMediaImportHandler*> handlersCopy(m_importHandlersMap.begin(), m_importHandlersMap.end());
    m_importHandlers.clear();
    for (std::vector<MediaType>::const_iterator it = result.begin(); it != result.end(); ++it)
    {
      m_importHandlers.push_back(handlersCopy.find(*it)->second);
      handlersCopy.erase(*it);
    }
    for (std::map<MediaType, IMediaImportHandler*>::const_iterator itHandler = handlersCopy.begin(); itHandler != handlersCopy.end(); ++itHandler)
      m_importHandlers.push_back(itHandler->second);

    CLog::Log(LOGDEBUG, "CMediaImportManager: new import handler for %s added", importHandler->GetMediaType().c_str());
  }
}

void CMediaImportManager::UnregisterMediaImportHandler(IMediaImportHandler *importHandler)
{
  if (importHandler == NULL)
    return;

  CSingleLock lock(m_importHandlersLock);
  std::map<MediaType, IMediaImportHandler*>::iterator it = m_importHandlersMap.find(importHandler->GetMediaType());
  if (it == m_importHandlersMap.end() || it->second != importHandler)
    return;

  // remove the import handler from the map
  m_importHandlersMap.erase(it);
  // and from the sorted vector
  for (std::vector<IMediaImportHandler*>::iterator vecIt = m_importHandlers.begin(); vecIt != m_importHandlers.end(); ++vecIt)
  {
    if (*vecIt == importHandler)
    {
      m_importHandlers.erase(vecIt);
      CLog::Log(LOGDEBUG, "CMediaImportManager: import handler for %s removed", importHandler->GetMediaType().c_str());
      break;
    }
  }
}

std::vector<MediaType> CMediaImportManager::GetSupportedMediaTypes() const
{
  std::vector<MediaType> supportedMediaTypes;
  CSingleLock lock(m_importHandlersLock);
  // get all media types for which there are handlers
  for (std::vector<IMediaImportHandler*>::const_iterator itHandler = m_importHandlers.begin(); itHandler != m_importHandlers.end(); ++itHandler)
    supportedMediaTypes.push_back((*itHandler)->GetMediaType());

  return supportedMediaTypes;
}

std::vector< std::vector<MediaType> > CMediaImportManager::GetSupportedMediaTypes(const std::set<MediaType> &mediaTypes) const
{
  std::vector< std::vector<MediaType> > supportedMediaTypes;
  std::set<MediaType> handledMediaTypes;

  CSingleLock lock(m_importHandlersLock);
  // get all media types for which there are handlers
  for (std::vector<IMediaImportHandler*>::const_iterator itHandler = m_importHandlers.begin(); itHandler != m_importHandlers.end(); ++itHandler)
  {
    const IMediaImportHandler *handler = *itHandler;
    if (mediaTypes.find(handler->GetMediaType()) == mediaTypes.end())
      continue;

    bool ok = true;
    // make sure all required media types are available
    std::set<MediaType> requiredMediaTypes = handler->GetRequiredMediaTypes();
    for (std::set<MediaType>::const_iterator itMediaType = requiredMediaTypes.begin(); itMediaType != requiredMediaTypes.end(); ++itMediaType)
    {
      if (mediaTypes.find(*itMediaType) == mediaTypes.end())
      {
        ok = false;
        break;
      }
    }

    if (!ok)
      continue;

    handledMediaTypes.insert(handler->GetMediaType());
  }

  for (std::vector<IMediaImportHandler*>::const_iterator itHandler = m_importHandlers.begin(); itHandler != m_importHandlers.end(); ++itHandler)
  {
    const IMediaImportHandler *handler = *itHandler;
    std::set<MediaType>::const_iterator itHandledMediaType = handledMediaTypes.find(handler->GetMediaType());
    if (itHandledMediaType == handledMediaTypes.end())
      continue;

    std::vector<MediaType> group;
    std::vector<MediaType> groupedMediaTypes = handler->GetGroupedMediaTypes();
    for (std::vector<MediaType>::const_iterator itGroupedMediaType = groupedMediaTypes.begin(); itGroupedMediaType != groupedMediaTypes.end(); ++itGroupedMediaType)
    {
      std::set<MediaType>::const_iterator itMediaType = handledMediaTypes.find(*itGroupedMediaType);
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
  for (std::vector<IMediaImportHandler*>::const_iterator itHandler = m_importHandlers.begin(); itHandler != m_importHandlers.end(); ++itHandler)
  {
    std::set<MediaType> dependencies = (*itHandler)->GetDependencies();
    if (dependencies.find(mediaType) != dependencies.end())
      dependingMediaTypes.insert((*itHandler)->GetMediaType());
  }

  return dependingMediaTypes;
}
  
std::set<MediaType> CMediaImportManager::GetGroupedMediaTypes(const MediaType &mediaType) const
{
  std::set<MediaType> groupedMediaTypes;
  if (mediaType.empty())
    return groupedMediaTypes;

  CSingleLock lock(m_importHandlersLock);
  std::map<MediaType, IMediaImportHandler*>::const_iterator itHandler = m_importHandlersMap.find(mediaType);
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
  std::map<MediaType, IMediaImportHandler*>::const_iterator itHandler = m_importHandlersMap.find(mediaType);
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
    CLog::Log(LOGWARNING, "CMediaImportManager: unable to register source with invalid identifier \"%s\" or friendly name \"%s\"", sourceID.c_str(), friendlyName.c_str());
    return;
  }

  CMediaImportSource source(sourceID, friendlyName, iconUrl, mediaTypes);

  CSingleLock sourcesLock(m_sourcesLock);
  std::map<std::string, bool>::iterator itSource = m_sources.find(sourceID);
  if (itSource == m_sources.end() || !FindSource(sourceID, source))
  {
    CSingleLock jobsLock(m_jobsLock);
    std::map<std::string, std::set<CJob*> >::const_iterator itRegistrationJob = m_jobMap.find(sourceID);
    // if a registration job for this source is already running, there's nothing to do
    if (itRegistrationJob != m_jobMap.end())
    {
      CLog::Log(LOGDEBUG, "CMediaImportManager: source %s is already being registered, so nothing to do", sourceID.c_str());
      return;
    }

    CMediaImportTaskProcessorJob *processorJob = new CMediaImportTaskProcessorJob(sourceID, this);

    CMediaImportSourceRegistrationTask *registrationTask =
      new CMediaImportSourceRegistrationTask(CMediaImportSource(sourceID, friendlyName, iconUrl, mediaTypes));
    processorJob->SetTask(registrationTask);
    AddTaskProcessorJob(sourceID, processorJob);
    
    CLog::Log(LOGDEBUG, "CMediaImportManager: registration job for source %s queued", sourceID.c_str());
    return;
  }

  // update any possibly changed values
  CMediaImportSource updatedSource = source;
  updatedSource.SetIconUrl(iconUrl);
  updatedSource.SetFriendlyName(friendlyName);
  updatedSource.SetAvailableMediaTypes(mediaTypes);

  bool updated = false;
  CSingleLock repositoriesLock(m_importRepositoriesLock);
  // try to update the source in at least one of the repositories
  for (std::set<IMediaImportRepository*>::iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
  {
    if ((*itRepository)->UpdateSource(updatedSource))
      updated = true;
  }
  repositoriesLock.Leave();
  sourcesLock.Leave();

  if (updated)
  {
    CLog::Log(LOGDEBUG, "CMediaImportManager: source %s updated", sourceID.c_str());
    OnSourceUpdated(updatedSource);
  }

  // start processing all imports of the source
  Import(sourceID);
}

void CMediaImportManager::UnregisterSource(const std::string& sourceID)
{
  if (sourceID.empty())
    return;

  CSingleLock sourcesLock(m_sourcesLock);
  std::map<std::string, bool>::iterator itSource = m_sources.find(sourceID);
  if (itSource == m_sources.end())
    return;

  itSource->second = false;
  CLog::Log(LOGDEBUG, "CMediaImportManager: source %s disabled", sourceID.c_str());
  sourcesLock.Leave();

  // if there's a task for the source which hasn't started yet or is still
  // running, try to cancel it
  std::map<std::string, std::set<CJob*> >::const_iterator itJobs = m_jobMap.find(sourceID);
  if (itJobs != m_jobMap.end())
  {
    for (std::set<CJob*>::const_iterator itJob = itJobs->second.begin(); itJob != itJobs->second.end(); ++itJob)
    {
      if (*itJob != NULL)
        CancelJob(*itJob);
    }

    m_jobMap.erase(sourceID);
    CLog::Log(LOGDEBUG, "CMediaImportManager: source registration task for %s cancelled", sourceID.c_str());
  }

  // disable all items imported from the unregistered source
  std::vector<CMediaImport> imports = GetImportsBySource(sourceID);
  CSingleLock handlersLock(m_importHandlersLock);
  for (std::vector<CMediaImport>::const_iterator itImport = imports.begin(); itImport != imports.end(); ++itImport)
  {
    std::map<MediaType, IMediaImportHandler*>::iterator itHandler = m_importHandlersMap.find(itImport->GetMediaType());
    if (itHandler != m_importHandlersMap.end())
      itHandler->second->SetImportedItemsEnabled(*itImport, false);
  }
  handlersLock.Leave();

  CMediaImportSource source(sourceID);
  if (!FindSource(sourceID, source))
    return;

  OnSourceDeactivated(source);
}

void CMediaImportManager::RemoveSource(const std::string& sourceID, CGUIDialogProgress *progress /* = NULL */)
{
  if (sourceID.empty())
    return;

  CSingleLock sourcesLock(m_sourcesLock);
  std::map<std::string, bool>::iterator itSource = m_sources.find(sourceID);
  if (itSource == m_sources.end())
    return;

  CMediaImportSource source(sourceID);
  if (!FindSource(sourceID, source))
    return;

  // get all the imports for the source
  std::vector<CMediaImport> imports = GetImportsBySource(sourceID);
  
  // remove all imports in reverse import order
  CSingleLock importHandlersLock(m_importHandlersLock);
  for (std::vector<IMediaImportHandler*>::const_reverse_iterator itHandler = m_importHandlers.rbegin(); itHandler != m_importHandlers.rend(); ++itHandler)
  {
    for (std::vector<CMediaImport>::iterator itImport = imports.begin(); itImport != imports.end(); ++itImport)
    {
      if (MediaTypes::IsMediaType((*itHandler)->GetMediaType(), itImport->GetMediaType()))
      {
        for (std::set<IMediaImportRepository*>::iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
          (*itRepository)->RemoveImport(*itImport, progress);

        OnImportRemoved(*itImport);
        imports.erase(itImport);
        break;
      }
    }
  }
  importHandlersLock.Leave();

  CSingleLock repositoriesLock(m_importRepositoriesLock);
  for (std::set<IMediaImportRepository*>::iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
    (*itRepository)->RemoveSource(sourceID, progress);
  repositoriesLock.Leave();

  m_sources.erase(itSource);
  sourcesLock.Leave();

  OnSourceRemoved(source);
  CLog::Log(LOGDEBUG, "CMediaImportManager: source %s removed", sourceID.c_str());
}

std::vector<CMediaImportSource> CMediaImportManager::GetSources() const
{
  std::map<std::string, CMediaImportSource> mapSources;

  CSingleLock lock(m_importRepositoriesLock);
  for (std::set<IMediaImportRepository*>::const_iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
  {
    std::vector<CMediaImportSource> tmpSources = (*itRepository)->GetSources();
    for (std::vector<CMediaImportSource>::iterator itSource = tmpSources.begin(); itSource != tmpSources.end(); ++itSource)
    {
      std::map<std::string, bool>::const_iterator itSourceLocal = m_sources.find(itSource->GetIdentifier());
      if (itSourceLocal == m_sources.end())
        continue;
      
      std::map<std::string, CMediaImportSource>::iterator itSourceMap = mapSources.find(itSource->GetIdentifier());
      if (itSourceMap == mapSources.end())
      {
        // add the source to the list
        itSource->SetActive(itSourceLocal->second);
        mapSources.insert(std::make_pair(itSource->GetIdentifier(), *itSource));
      }
      else
      {
        // check if we need to update the last synced timestamp
        if (itSourceMap->second.GetLastSynced() < itSource->GetLastSynced())
          itSourceMap->second.SetLastSynced(itSource->GetLastSynced());

        // update the list of media types
        std::set<MediaType> mediaTypes = itSourceMap->second.GetAvailableMediaTypes();
        mediaTypes.insert(itSource->GetAvailableMediaTypes().begin(), itSource->GetAvailableMediaTypes().end());
        itSourceMap->second.SetAvailableMediaTypes(mediaTypes);
      }
    }
  }
  lock.Leave();

  std::vector<CMediaImportSource> sources;
  for (std::map<std::string, CMediaImportSource>::const_iterator itSourceMap = mapSources.begin(); itSourceMap != mapSources.end(); ++itSourceMap)
    sources.push_back(itSourceMap->second);

  return sources;
}

std::vector<CMediaImportSource> CMediaImportManager::GetSources(bool active) const
{
  std::map<std::string, CMediaImportSource> mapSources;

  CSingleLock lock(m_importRepositoriesLock);
  for (std::set<IMediaImportRepository*>::const_iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
  {
    std::vector<CMediaImportSource> tmpSources = (*itRepository)->GetSources();
    for (std::vector<CMediaImportSource>::iterator itSource = tmpSources.begin(); itSource != tmpSources.end(); ++itSource)
    {
      std::map<std::string, bool>::const_iterator itSourceLocal = m_sources.find(itSource->GetIdentifier());
      if (itSourceLocal == m_sources.end() ||
          itSourceLocal->second != active)
        continue;

      std::map<std::string, CMediaImportSource>::iterator itSourceMap = mapSources.find(itSource->GetIdentifier());
      if (itSourceMap == mapSources.end())
      {
        // add the source to the list
        itSource->SetActive(itSourceLocal->second);
        mapSources.insert(std::make_pair(itSource->GetIdentifier(), *itSource));
      }
      else
      {
        // check if we need to update the last synced timestamp
        if (itSourceMap->second.GetLastSynced() < itSource->GetLastSynced())
          itSourceMap->second.SetLastSynced(itSource->GetLastSynced());

        // update the list of media types
        std::set<MediaType> mediaTypes = itSourceMap->second.GetAvailableMediaTypes();
        mediaTypes.insert(itSource->GetAvailableMediaTypes().begin(), itSource->GetAvailableMediaTypes().end());
        itSourceMap->second.SetAvailableMediaTypes(mediaTypes);
      }
    }
  }
  lock.Leave();

  std::vector<CMediaImportSource> sources;
  for (std::map<std::string, CMediaImportSource>::const_iterator itSourceMap = mapSources.begin(); itSourceMap != mapSources.end(); ++itSourceMap)
    sources.push_back(itSourceMap->second);

  return sources;
}

bool CMediaImportManager::GetSource(const std::string& sourceID, CMediaImportSource& source) const
{
  if (sourceID.empty())
    return false;

  std::map<std::string, bool>::const_iterator itSource = m_sources.find(sourceID);
  if (itSource == m_sources.end())
    return false;

  if (!FindSource(sourceID, source))
    return false;

  source.SetActive(itSource->second);
  return true;
}

bool CMediaImportManager::IsSourceActive(const std::string& sourceID) const
{
  if (sourceID.empty())
    return false;

  std::map<std::string, bool>::const_iterator itSource = m_sources.find(sourceID);
  if (itSource == m_sources.end())
    return false;

  return itSource->second;
}

bool CMediaImportManager::IsSourceActive(const CMediaImportSource& source) const
{
  return IsSourceActive(source.GetIdentifier());
}

bool CMediaImportManager::HasImports(const std::string& sourceID) const
{
  if (sourceID.empty() ||
      m_sources.find(sourceID) == m_sources.end())
    return false;

  CSingleLock importRepositoriesLock(m_importRepositoriesLock);
  for (std::set<IMediaImportRepository*>::const_iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
  {
    std::vector<CMediaImport> repoImports = (*itRepository)->GetImportsBySource(sourceID);
    if (!repoImports.empty())
      return true;
  }

  return false;
}

bool CMediaImportManager::HasImports(const CMediaImportSource &source) const
{
  return HasImports(source.GetIdentifier());
}

bool CMediaImportManager::AddImport(const std::string &sourceID, const std::string &path, const MediaType &mediaType, bool synchronise /* = true */)
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
  std::map<std::string, bool>::iterator itSource = m_sources.find(sourceID);
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

  bool isSourceActive = itSource->second;
  sourcesLock.Leave();

  OnImportAdded(newImport);

  if (isSourceActive && synchronise)
    Import(newImport);

  return true;
}

bool CMediaImportManager::AddImports(const std::string &sourceID, const std::string &path, const std::set<MediaType> &mediaTypes, bool synchronise /* = true */)
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
  std::map<std::string, bool>::iterator itSource = m_sources.find(sourceID);
  if (itSource == m_sources.end() ||  // source doesn't exist
      !FindSource(sourceID, source))
  {
    CLog::Log(LOGWARNING, "CMediaImportManager: trying to add new imports from unknown source \"%s\"", sourceID.c_str());
    return false;
  }

  std::vector<CMediaImport> addedImports;
  for (std::set<MediaType>::const_iterator itMediaType = mediaTypes.begin(); itMediaType != mediaTypes.end(); ++itMediaType)
  {
    // check if the import already exists
    CMediaImport import(path, *itMediaType, sourceID);
    if (FindImport(path, *itMediaType, import))
    {
      CLog::Log(LOGERROR, "CMediaImportManager: unable to add already existing import from source \"%s\" with path \"%s\" and media type \"%s\"",
                sourceID.c_str(), path.c_str(), itMediaType->c_str());
      continue;
    }

    // check if there's an import handler that can handle the given media type
    if (m_importHandlersMap.find(*itMediaType) == m_importHandlersMap.end())
    {
      CLog::Log(LOGERROR, "CMediaImportManager: unable to add new import from source \"%s\" with path \"%s\" and media type \"%s\" because there is no matching import handler available",
                sourceID.c_str(), path.c_str(), itMediaType->c_str());
      continue;
    }

    CMediaImport newImport(path, *itMediaType, source);
    if (!AddImport(newImport))
    {
      CLog::Log(LOGERROR, "CMediaImportManager: failed to add new import for source \"%s\" with path \"%s\" and media type \"%s\" to any import repository",
                sourceID.c_str(), path.c_str(), itMediaType->c_str());
      continue;
    }

    addedImports.push_back(newImport);
  }

  bool isSourceActive = itSource->second;
  sourcesLock.Leave();

  for (std::vector<CMediaImport>::const_iterator itImport = addedImports.begin(); itImport != addedImports.end(); ++itImport)
    OnImportAdded(*itImport);

  if (!isSourceActive || !synchronise)
    return true;

  // start importing media items from all the newly added imports for the source
  // it is important to do this in the proper order
  CSingleLock importHandlersLock(m_importHandlersLock);
  for (std::vector<IMediaImportHandler*>::const_iterator itHandler = m_importHandlers.begin(); itHandler != m_importHandlers.end(); ++itHandler)
  {
    for (std::vector<CMediaImport>::iterator itImport = addedImports.begin(); itImport != addedImports.end(); ++itImport)
    {
      if (MediaTypes::IsMediaType((*itHandler)->GetMediaType(), itImport->GetMediaType()))
      {
        Import(*itImport);
        addedImports.erase(itImport);
        break;
      }
    }
  }

  return true;
}

bool CMediaImportManager::UpdateImport(const CMediaImport &import)
{
  if (import.GetPath().empty() || import.GetMediaType().empty())
    return false;

  bool updated = false;
  CSingleLock importRepositoriesLock(m_importRepositoriesLock);
  for (std::set<IMediaImportRepository*>::const_iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
    updated |= (*itRepository)->UpdateImport(import);
  importRepositoriesLock.Leave();

  OnImportUpdated(import);

  return updated;
}

void CMediaImportManager::RemoveImport(const std::string& path, const MediaType &mediaType, CGUIDialogProgress *progress /* = NULL */)
{
  if (path.empty() || mediaType == MediaTypeNone)
    return;

  CMediaImport import(path, mediaType, "");
  if (!FindImport(path, mediaType, import))
    return;

  // get all media types connected to the media type of the import to remove
  std::set<MediaType> connectedMediaTypes = GetConnectedMediaTypes(mediaType);
  connectedMediaTypes.insert(mediaType);

  // get all the imports for the source of the import to remove
  std::vector<CMediaImport> imports = GetImportsBySource(import.GetSource().GetIdentifier());
  
  // remove all imports in reverse import order
  for (std::vector<IMediaImportHandler*>::const_reverse_iterator itHandler = m_importHandlers.rbegin(); itHandler != m_importHandlers.rend(); ++itHandler)
  {
    for (std::vector<CMediaImport>::iterator itImport = imports.begin(); itImport != imports.end(); ++itImport)
    {
      if (MediaTypes::IsMediaType((*itHandler)->GetMediaType(), itImport->GetMediaType()))
      {
        if (connectedMediaTypes.find(itImport->GetMediaType()) != connectedMediaTypes.end())
        {
          for (std::set<IMediaImportRepository*>::iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
            (*itRepository)->RemoveImport(*itImport, progress);
        
          OnImportRemoved(*itImport);
          connectedMediaTypes.erase(itImport->GetMediaType());
          CLog::Log(LOGDEBUG, "CMediaImportManager: import of %s from %s removed", mediaType.c_str(), path.c_str());
        }

        imports.erase(itImport);
        break;
      }
    }
  }
}

std::vector<CMediaImport> CMediaImportManager::GetImports() const
{
  std::vector<CMediaImport> imports;
  CSingleLock importRepositoriesLock(m_importRepositoriesLock);
  CSingleLock sourcesLock(m_sourcesLock);
  for (std::set<IMediaImportRepository*>::const_iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
  {
    std::vector<CMediaImport> repoImports = (*itRepository)->GetImports();
    for (std::vector<CMediaImport>::iterator itImport = repoImports.begin(); itImport != repoImports.end(); ++itImport)
    {
      std::map<std::string, bool>::const_iterator itSource = m_sources.find(itImport->GetSource().GetIdentifier());
      if (itSource == m_sources.end())
        continue;

      itImport->SetActive(itSource->second);
      imports.push_back(*itImport);
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
  for (std::set<IMediaImportRepository*>::const_iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
  {
    std::vector<CMediaImport> repoImports = (*itRepository)->GetImportsByMediaType(mediaType);
    for (std::vector<CMediaImport>::iterator itImport = repoImports.begin(); itImport != repoImports.end(); ++itImport)
    {
      std::map<std::string, bool>::const_iterator itSource = m_sources.find(itImport->GetSource().GetIdentifier());
      if (itSource == m_sources.end())
        continue;

      itImport->SetActive(itSource->second);
      imports.push_back(*itImport);
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
  std::map<std::string, bool>::const_iterator itSource = m_sources.find(sourceID);
  if (itSource == m_sources.end())
    return imports;

  CSingleLock importRepositoriesLock(m_importRepositoriesLock);
  for (std::set<IMediaImportRepository*>::const_iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
  {
    std::vector<CMediaImport> repoImports = (*itRepository)->GetImportsBySource(sourceID);
    for (std::vector<CMediaImport>::iterator itImport = repoImports.begin(); itImport != repoImports.end(); ++itImport)
    {
      itImport->SetActive(itSource->second);
      imports.push_back(*itImport);
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
  for (std::map<std::string, bool>::const_iterator itSource = m_sources.begin(); itSource != m_sources.end(); ++itSource)
  {
    if (!itSource->second)
      continue;

    result |= Import(itSource->first);
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

  if (!m_sources[sourceID])
  {
    CMediaImportSource source(sourceID);
    if (!FindSource(sourceID, source))
      return false;

    m_sources[sourceID] = true;

    OnSourceActivated(source);
  }

  // get all the imports for the source
  std::vector<CMediaImport> imports = GetImportsBySource(sourceID);
  
  // start importing media items from all the defined imports for the source
  // it is important to do this in the proper order
  for (std::vector<IMediaImportHandler*>::const_iterator itHandler = m_importHandlers.begin(); itHandler != m_importHandlers.end(); ++itHandler)
  {
    for (std::vector<CMediaImport>::iterator itImport = imports.begin(); itImport != imports.end(); ++itImport)
    {
      if (MediaTypes::IsMediaType((*itHandler)->GetMediaType(), itImport->GetMediaType()))
      {
        Import(*itImport);
        imports.erase(itImport);
        break;
      }
    }
  }

  if (!imports.empty())
    CLog::Log(LOGWARNING, "CMediaImportManager: failed to import media items for %zu imports from %s", imports.size(), sourceID.c_str());

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
  std::map<std::string, bool>::iterator itSource = m_sources.find(import.GetSource().GetIdentifier());
  if (itSource == m_sources.end() ||  // source doesn't exist
      !itSource->second)
  {
    CLog::Log(LOGWARNING, "CMediaImportManager: unable to import from unregistered source \"%s\"",
              import.GetSource().GetIdentifier().c_str());
    return false;
  }

  // import media items from the given import and all other imports whose
  // media types depend upon this import's one or are grouped together with it
  CSingleLock lock(m_importHandlersLock);
  for (std::vector<IMediaImportHandler*>::const_iterator itHandler = m_importHandlers.begin(); itHandler != m_importHandlers.end(); ++itHandler)
  {
    IMediaImportHandler *importHandler = *itHandler;
    MediaType currentMediaType = importHandler->GetMediaType();
    if (currentMediaType == mediaType)
    {
      Import(import);
      continue;
    }

    std::set<MediaType> requiredMediaTypes = importHandler->GetRequiredMediaTypes();
    if (requiredMediaTypes.find(mediaType) == requiredMediaTypes.end())
    {
      std::vector<MediaType> groupedMediaTypes = importHandler->GetGroupedMediaTypes();
      bool found = false;
      for (std::vector<MediaType>::const_iterator itMediaType = groupedMediaTypes.begin(); itMediaType != groupedMediaTypes.end(); ++itMediaType)
      {
        if (*itMediaType == mediaType)
        {
          found = true;
          break;
        }
      }

      if (!found)
        continue;
    }

    CMediaImport tmpImport(path, currentMediaType, "");
    if (!FindImport(path, currentMediaType, tmpImport))
      continue;

    Import(tmpImport);
  }

  return true;
}

void CMediaImportManager::Import(const CMediaImport &import)
{
  CMediaImportTaskProcessorJob *processorJob = new CMediaImportTaskProcessorJob(import.GetSource().GetIdentifier(), this);

  CMediaImportRetrievalTask *retrievalTask = new CMediaImportRetrievalTask(import);
  processorJob->SetTask(retrievalTask);
  AddTaskProcessorJob(import.GetSource().GetIdentifier(), processorJob);

  CLog::Log(LOGINFO, "CMediaImportManager: import retrieval task for %s (%s) started", import.GetSource().GetIdentifier().c_str(), import.GetPath().c_str());
}

bool CMediaImportManager::AddSource(const CMediaImportSource &source)
{
  bool added = false;
  CSingleLock repositoriesLock(m_importRepositoriesLock);
  // try to add the source to at least one of the repositories
  for (std::set<IMediaImportRepository*>::iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
  {
    if ((*itRepository)->AddSource(source))
      added = true;
  }

  return added;
}

bool CMediaImportManager::FindSource(const std::string &sourceID, CMediaImportSource &source) const
{
  CSingleLock repositoriesLock(m_importRepositoriesLock);
  // try to add the source to at least one of the repositories
  for (std::set<IMediaImportRepository*>::iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
  {
    if ((*itRepository)->GetSource(sourceID, source))
      return true;
  }

  return false;
}

bool CMediaImportManager::AddImport(const CMediaImport &import)
{
  bool added = false;
  CSingleLock repositoriesLock(m_importRepositoriesLock);
  // try to add the source to at least one of the repositories
  for (std::set<IMediaImportRepository*>::iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
  {
    if ((*itRepository)->AddImport(import))
      added = true;
  }

  return added;
}

bool CMediaImportManager::FindImport(const std::string &path, const MediaType &mediaType, CMediaImport &import) const
{
  CSingleLock repositoriesLock(m_importRepositoriesLock);
  // try to add the source to at least one of the repositories
  for (std::set<IMediaImportRepository*>::const_iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
  {
    if ((*itRepository)->GetImport(path, mediaType, import))
      return true;
  }

  return false;
}

bool CMediaImportManager::UpdateImportedItem(const CFileItem &item)
{
  if (!item.IsImported())
    return false;

  std::string sourceID, importPath;
  MediaType mediaType;
  // TODO: move this logic to somewhere else
  if (item.HasVideoInfoTag())
  {
    sourceID = item.GetVideoInfoTag()->m_strSource;
    importPath = item.GetVideoInfoTag()->m_strImportPath;
    mediaType = item.GetVideoInfoTag()->m_type;
  }

  if (importPath.empty())
    return false;

  CMediaImport import(importPath, mediaType, sourceID);
  if (!FindImport(importPath, mediaType, import))
  {
    CLog::Log(LOGWARNING, "CMediaImportManager: no import found for %s of %s", importPath.c_str(), mediaType.c_str());
    return false;
  }

  /* TODO: update on source
  if (!import.GetSettings().UpdatePlaybackMetadataOnSource())
    return false;
  */

  if (!m_sources[import.GetSource().GetIdentifier()])
  {
    CLog::Log(LOGWARNING, "CMediaImportManager: unable to update item %s on inactive source %s", item.GetPath().c_str(), import.GetSource().GetIdentifier().c_str());
    return false;
  }

  const IMediaImporter *importer = GetImporter(importPath);
  if (importer == NULL)
    return false;

  CMediaImportTaskProcessorJob *processorJob = new CMediaImportTaskProcessorJob(import.GetSource().GetIdentifier(), this);

  CMediaImportUpdateTask *updateTask = new CMediaImportUpdateTask(import, item);
  processorJob->SetTask(updateTask);
  AddTaskProcessorJob(import.GetSource().GetIdentifier(), processorJob);

  CLog::Log(LOGINFO, "CMediaImportManager: import update task for %s (%s) started", import.GetSource().GetIdentifier().c_str(), import.GetPath().c_str());

  return true;
}

void CMediaImportManager::AddTaskProcessorJob(const std::string &path, CJob *job)
{
  std::map<std::string, std::set<CJob*> >::iterator itJobs = m_jobMap.find(path);
  if (itJobs == m_jobMap.end())
  {
    std::set<CJob*> jobs; jobs.insert(job);
    m_jobMap.insert(std::make_pair(path, jobs));
  }
  else
    itJobs->second.insert(job);
  AddJob(job);
}

void CMediaImportManager::RemoveTaskProcessorJob(const std::string &path, const CJob *job)
{
  std::map<std::string, std::set<CJob*> >::iterator itJobs = m_jobMap.find(path);
  if (itJobs != m_jobMap.end())
  {
    itJobs->second.erase(const_cast<CJob*>(job));
    if (itJobs->second.empty())
      m_jobMap.erase(itJobs);
  }
}

void CMediaImportManager::OnJobComplete(unsigned int jobID, bool success, CJob *job)
{
  CSingleLock jobsLock(m_jobsLock);
  if (job != NULL)
  {
    CMediaImportTaskProcessorJob *processorJob = static_cast<CMediaImportTaskProcessorJob*>(job);
    if (processorJob != NULL)
      RemoveTaskProcessorJob(processorJob->GetPath(), job);
  }

  CJobQueue::OnJobComplete(jobID, success, job);
}

void CMediaImportManager::OnJobProgress(unsigned int jobID, unsigned int progress, unsigned int total, const CJob *job)
{
  if (strcmp(job->GetType(), "MediaImportTaskProcessorJob") == 0)
  {
    const CMediaImportTaskProcessorJob* processorJob = static_cast<const CMediaImportTaskProcessorJob*>(job);
    if (processorJob->GetCurrentTask() != NULL &&
        processorJob->GetCurrentTask()->GetProgressBarHandle() != NULL)
      processorJob->GetCurrentTask()->GetProgressBarHandle()->SetProgress(progress, total);
  }

  CJobQueue::OnJobProgress(jobID, progress, total, job);
}

std::vector<IMediaImportTask*> CMediaImportManager::OnTaskComplete(bool success, IMediaImportTask *task)
{
  std::vector<IMediaImportTask*> nextTasks;
  if (task == NULL)
    return nextTasks;

  std::string taskType = task->GetType();
  if (taskType.compare("MediaImportSourceRegistrationTask") == 0)
  {
    CMediaImportSourceRegistrationTask *registrationTask = static_cast<CMediaImportSourceRegistrationTask*>(task);
    if (registrationTask == NULL)
      return nextTasks;

    const CMediaImportSource &source = registrationTask->GetImportSource();

    // nothing to do if the import job failed
    if (!success)
    {
      CLog::Log(LOGWARNING, "CMediaImportManager: source registration task for %s failed", source.GetIdentifier().c_str());
      return nextTasks;
    }

    if (!AddSource(source))
    {
      CLog::Log(LOGWARNING, "CMediaImportManager: source %s couldn't be added to any import repository", source.GetIdentifier().c_str());
      return nextTasks;
    }

    m_sources.insert(std::make_pair(source.GetIdentifier(), true));
    CLog::Log(LOGDEBUG, "CMediaImportManager: new source %s added", source.GetIdentifier().c_str());
    OnSourceAdded(source);

    bool added = false;
    // try to add the imports to at least one of the repositories
    const std::vector<CMediaImport> &imports = registrationTask->GetImports();
    std::vector<CMediaImport> imported;
    for (std::vector<CMediaImport>::const_iterator itImport = imports.begin(); itImport != imports.end(); ++itImport)
    {
      bool success = AddImport(*itImport);
      if (success)
      {
        imported.push_back(*itImport);
        OnImportAdded(*itImport);
      }

      added |= success;
    }

    if (!added)
    {
      if (!imports.empty())
        CLog::Log(LOGWARNING, "CMediaImportManager: failed to add %zu imports for source %s to any import repository", imports.size(), source.GetIdentifier().c_str());
      return nextTasks;
    }
    
    CSingleLock handlersLock(m_importHandlersLock);
    // start processing all imports of the source
    // it is important to do this in the proper order
    for (std::vector<IMediaImportHandler*>::const_iterator itHandler = m_importHandlers.begin(); itHandler != m_importHandlers.end(); ++itHandler)
    {
      for (std::vector<CMediaImport>::iterator itImport = imported.begin(); itImport != imported.end(); ++itImport)
      {
        if (MediaTypes::IsMediaType((*itHandler)->GetMediaType(), itImport->GetMediaType()))
        {
          nextTasks.push_back(new CMediaImportRetrievalTask(*itImport));
          imported.erase(itImport);
          break;
        }
      }
    }

    if (!imported.empty())
      CLog::Log(LOGWARNING, "CMediaImportManager: failed to retrieve media items for %zu imports", imported.size());
  }
  else if (taskType.compare("MediaImportRetrievalTask") == 0)
  {
    CMediaImportRetrievalTask* retrievalTask = static_cast<CMediaImportRetrievalTask*>(task);
    if (retrievalTask == NULL)
      return nextTasks;

    const IMediaImporter* importer = retrievalTask->GetImporter();
    if (importer == NULL)
      return nextTasks;

    // nothing to do if the import job failed
    if (!success)
    {
      CLog::Log(LOGWARNING, "CMediaImportManager: media retrieval task for %s using %s failed", importer->GetSourceIdentifier().c_str(), importer->GetIdentification());
      return nextTasks;
    }
  
    CLog::Log(LOGINFO, "CMediaImportManager: media retrieval task for %s using %s successfully finished", importer->GetSourceIdentifier().c_str(), importer->GetIdentification());

    CSingleLock handlersLock(m_importHandlersLock);
    // find the proper media import handler to handle the retrieved items
    std::map<MediaType, IMediaImportHandler*>::const_iterator itMediaImportHandler = m_importHandlersMap.find(retrievalTask->GetMediaType());
    if (itMediaImportHandler != m_importHandlersMap.end())
    {
      CFileItemList importedItems;
      // get the list of imported items
      if (retrievalTask->GetImportedMedia(importedItems))
        nextTasks.push_back(new CMediaImportSynchronisationTask(retrievalTask->GetImport(), itMediaImportHandler->second->Create(), importedItems));
      else
        CLog::Log(LOGINFO, "CMediaImportManager: no media items of type %s imported for %s", retrievalTask->GetMediaType().c_str(), importer->GetSourceIdentifier().c_str());
    }
    else
      CLog::Log(LOGWARNING, "CMediaImportManager: no media import handler found to handle \"%s\"", retrievalTask->GetMediaType().c_str());
  }
  else if (taskType.compare("MediaImportSynchronisationTask") == 0)
  {
    CMediaImportSynchronisationTask* synchronisationTask = static_cast<CMediaImportSynchronisationTask*>(task);
    if (synchronisationTask == NULL)
      return nextTasks;

    if (success)
    {
      CMediaImport import = synchronisationTask->GetImport();
      CSingleLock repositoriesLock(m_importRepositoriesLock);
      for (std::set<IMediaImportRepository*>::iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
      {
        if ((*itRepository)->UpdateLastSync(import))
        {
          OnSourceUpdated(import.GetSource());
          OnImportUpdated(import);
          break;
        }
      }
    }
  }

  return nextTasks;
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
