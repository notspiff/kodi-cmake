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

#include "MediaImportTaskProcessorJob.h"
#include "dialogs/GUIDialogExtendedProgressBar.h"
#include "guilib/GUIWindowManager.h"
#include "media/import/IMediaImportHandler.h"
#include "media/import/IMediaImportTask.h"
#include "media/import/MediaImportManager.h"
#include "media/import/task/MediaImportChangesetTask.h"
#include "media/import/task/MediaImportCleanupTask.h"
#include "media/import/task/MediaImportRemovalTask.h"
#include "media/import/task/MediaImportRetrievalTask.h"
#include "media/import/task/MediaImportScrapingTask.h"
#include "media/import/task/MediaImportSourceRegistrationTask.h"
#include "media/import/task/MediaImportSynchronisationTask.h"
#include "media/import/task/MediaImportUpdateTask.h"
#include "utils/log.h"

CMediaImportTaskProcessorJob::CMediaImportTaskProcessorJob(const std::string &path, IMediaImportTaskCallback *callback)
  : m_callback(callback),
    m_task(nullptr),
    m_progress(nullptr),
    m_path(path),
    m_importTaskData(),
    m_taskTypesToBeProcessed()
{ }

CMediaImportTaskProcessorJob::~CMediaImportTaskProcessorJob()
{
  if (m_progress != nullptr)
    m_progress->MarkFinished();
}

CMediaImportTaskProcessorJob* CMediaImportTaskProcessorJob::RegisterSource(const CMediaImportSource& source, IMediaImportTaskCallback *callback)
{
  CMediaImportTaskProcessorJob *processorJob = new CMediaImportTaskProcessorJob(source.GetIdentifier(), callback);

  CMediaImportSourceRegistrationTask *sourceRegistrationTask = new CMediaImportSourceRegistrationTask(source);
  processorJob->m_taskTypesToBeProcessed.push_back(MediaImportTaskType::SourceRegistration);
  processorJob->SetTask(sourceRegistrationTask);

  return processorJob;
}

CMediaImportTaskProcessorJob* CMediaImportTaskProcessorJob::Import(const CMediaImportSource& source, bool automatically, IMediaImportTaskCallback *callback)
{
  CMediaImportTaskProcessorJob *processorJob = new CMediaImportTaskProcessorJob(source.GetIdentifier(), callback);

  // get all the imports for the source
  std::vector<CMediaImport> imports = CMediaImportManager::Get().GetImportsBySource(source.GetIdentifier());

  bool added = false;
  for (const auto& import : imports)
  {
    if (automatically && import.GetSettings().GetImportTrigger() != MediaImportTriggerAuto)
      continue;

    if (!processorJob->AddImport(import))
    {
      CLog::Log(LOGWARNING, "CMediaImportTaskProcessorJob: failed to import %s items from %s", import.GetMediaType().c_str(), import.GetPath().c_str());
      continue;
    }

    added |= true;
  }

  if (!added)
  {
    delete processorJob;
    return nullptr;
  }

  return processorJob;
}

CMediaImportTaskProcessorJob* CMediaImportTaskProcessorJob::Import(const CMediaImport& import, bool automatically, IMediaImportTaskCallback *callback)
{
  if (automatically && import.GetSettings().GetImportTrigger() != MediaImportTriggerAuto)
    return nullptr;

  CMediaImportTaskProcessorJob *processorJob = new CMediaImportTaskProcessorJob(import.GetSource().GetIdentifier(), callback);

  // find any other media types that belong together with the media type of this import
  std::set<MediaType> groupedMediaTypes = CMediaImportManager::Get().GetGroupedMediaTypes(import.GetMediaType());
  // make sure that the media type of the provided import is also part of the grouped media types
  groupedMediaTypes.insert(import.GetMediaType());

  // go through all the grouped media types, find their import and add it to the imports to process
  for (const auto& mediaType : groupedMediaTypes)
  {
    CMediaImport mediaImport = import;
    if (mediaType != import.GetMediaType())
    {
      if (!CMediaImportManager::Get().GetImport(import.GetPath(), mediaType, mediaImport))
      {
        CLog::Log(LOGWARNING, "CMediaImportTaskProcessorJob: couldn't find import for %s items from %s", mediaType.c_str(), mediaImport.GetPath().c_str());

        delete processorJob;
        return nullptr;
      }
    }

    if (!processorJob->AddImport(mediaImport))
    {
      CLog::Log(LOGWARNING, "CMediaImportTaskProcessorJob: failed to import %s items from %s", mediaImport.GetMediaType().c_str(), mediaImport.GetPath().c_str());

      delete processorJob;
      return nullptr;
    }
  }

  return processorJob;
}

CMediaImportTaskProcessorJob* CMediaImportTaskProcessorJob::UpdateImportedItem(const CMediaImport& import, const CFileItem &item, IMediaImportTaskCallback *callback)
{
  CMediaImportTaskProcessorJob *processorJob = new CMediaImportTaskProcessorJob(import.GetSource().GetIdentifier(), callback);

  CMediaImportUpdateTask *updateTask = new CMediaImportUpdateTask(import, item);
  processorJob->m_taskTypesToBeProcessed.push_back(MediaImportTaskType::Update);
  processorJob->SetTask(updateTask);

  return processorJob;
}

CMediaImportTaskProcessorJob* CMediaImportTaskProcessorJob::Cleanup(const CMediaImportSource& source, IMediaImportTaskCallback *callback)
{
  CMediaImportTaskProcessorJob *processorJob = new CMediaImportTaskProcessorJob(source.GetIdentifier(), callback);

  // get all the imports for the source
  std::vector<CMediaImport> imports = CMediaImportManager::Get().GetImportsBySource(source.GetIdentifier());

  bool added = false;
  std::vector<MediaImportTaskType> tasksToBeProcessed; tasksToBeProcessed.push_back(MediaImportTaskType::Cleanup);
  for (const auto& import : imports)
  {
    if (!processorJob->AddImport(import, tasksToBeProcessed))
    {
      CLog::Log(LOGWARNING, "CMediaImportTaskProcessorJob: failed to cleanup imported %s items from %s", import.GetMediaType().c_str(), import.GetPath().c_str());
      continue;
    }

    added |= true;
  }

  if (!added)
  {
    delete processorJob;
    return nullptr;
  }

  return processorJob;
}

CMediaImportTaskProcessorJob* CMediaImportTaskProcessorJob::Cleanup(const CMediaImport& import, IMediaImportTaskCallback *callback)
{
  CMediaImportTaskProcessorJob *processorJob = new CMediaImportTaskProcessorJob(import.GetSource().GetIdentifier(), callback);

  // find any other media types that belong together with the media type of this import
  std::set<MediaType> groupedMediaTypes = CMediaImportManager::Get().GetGroupedMediaTypes(import.GetMediaType());
  // make sure that the media type of the provided import is also part of the grouped media types
  groupedMediaTypes.insert(import.GetMediaType());

  // go through all the grouped media types, find their import and add it to the imports to process
  std::vector<MediaImportTaskType> tasksToBeProcessed; tasksToBeProcessed.push_back(MediaImportTaskType::Cleanup);
  for (const auto& mediaType : groupedMediaTypes)
  {
    CMediaImport mediaImport = import;
    if (mediaType != import.GetMediaType())
    {
      if (!CMediaImportManager::Get().GetImport(import.GetPath(), mediaType, mediaImport))
      {
        CLog::Log(LOGWARNING, "CMediaImportTaskProcessorJob: couldn't find import for %s items from %s", mediaType.c_str(), mediaImport.GetPath().c_str());

        delete processorJob;
        return nullptr;
      }
    }

    if (!processorJob->AddImport(mediaImport, tasksToBeProcessed))
    {
      CLog::Log(LOGWARNING, "CMediaImportTaskProcessorJob: failed to cleanup imported %s items from %s", mediaImport.GetMediaType().c_str(), mediaImport.GetPath().c_str());

      delete processorJob;
      return nullptr;
    }
  }

  return processorJob;
}

CMediaImportTaskProcessorJob* CMediaImportTaskProcessorJob::Remove(const CMediaImportSource& source, const std::vector<CMediaImport> &imports, IMediaImportTaskCallback *callback)
{
  CMediaImportTaskProcessorJob *processorJob = new CMediaImportTaskProcessorJob(source.GetIdentifier(), callback);

  std::vector<MediaImportTaskType> tasksToBeProcessed; tasksToBeProcessed.push_back(MediaImportTaskType::Removal);
  for (const auto& import : imports)
  {
    if (!processorJob->AddImport(import, tasksToBeProcessed))
    {
      CLog::Log(LOGWARNING, "CMediaImportTaskProcessorJob: failed to remove imported %s items from %s", import.GetMediaType().c_str(), import.GetPath().c_str());
      continue;
    }
  }

  return processorJob;
}

CMediaImportTaskProcessorJob* CMediaImportTaskProcessorJob::Remove(const CMediaImport& import, IMediaImportTaskCallback *callback)
{
  CMediaImportTaskProcessorJob *processorJob = new CMediaImportTaskProcessorJob(import.GetSource().GetIdentifier(), callback);

  // find any other media types that belong together with the media type of this import
  std::set<MediaType> groupedMediaTypes = CMediaImportManager::Get().GetGroupedMediaTypes(import.GetMediaType());
  // make sure that the media type of the provided import is also part of the grouped media types
  groupedMediaTypes.insert(import.GetMediaType());

  // go through all the grouped media types, find their import and add it to the imports to process
  std::vector<MediaImportTaskType> tasksToBeProcessed; tasksToBeProcessed.push_back(MediaImportTaskType::Removal);
  for (const auto& mediaType : groupedMediaTypes)
  {
    CMediaImport mediaImport = import;
    if (mediaType != import.GetMediaType())
    {
      if (!CMediaImportManager::Get().GetImport(import.GetPath(), mediaType, mediaImport))
      {
        CLog::Log(LOGWARNING, "CMediaImportTaskProcessorJob: couldn't find import for %s items from %s", mediaType.c_str(), mediaImport.GetPath().c_str());

        delete processorJob;
        return nullptr;
      }
    }

    if (!processorJob->AddImport(mediaImport, tasksToBeProcessed))
    {
      CLog::Log(LOGWARNING, "CMediaImportTaskProcessorJob: failed to remove imported %s items from %s", mediaImport.GetMediaType().c_str(), mediaImport.GetPath().c_str());

      delete processorJob;
      return nullptr;
    }
  }

  return processorJob;
}

void CMediaImportTaskProcessorJob::SetTask(IMediaImportTask *task)
{
  m_task = task;
  if (m_task != nullptr)
    m_task->SetProcessorJob(this);
}

CGUIDialogProgressBarHandle* CMediaImportTaskProcessorJob::GetProgressBarHandle(const std::string &title /* = "" */)
{
  if (m_progress == nullptr)
  {
    CGUIDialogExtendedProgressBar* dialog = (CGUIDialogExtendedProgressBar*)g_windowManager.GetWindow(WINDOW_DIALOG_EXT_PROGRESS);
    if (dialog != nullptr)
      m_progress = dialog->GetHandle(title);
  }
  else if (!title.empty())
    m_progress->SetTitle(title);

  return m_progress;
}

bool CMediaImportTaskProcessorJob::DoWork()
{
  return ProcessTask();
}

bool CMediaImportTaskProcessorJob::operator==(const CJob *job) const
{
  if (strcmp(job->GetType(), GetType()) == 0)
  {
    const CMediaImportTaskProcessorJob* rjob = dynamic_cast<const CMediaImportTaskProcessorJob*>(job);
    if (rjob != nullptr)
    {
      return m_path == rjob->m_path &&
             m_callback == rjob->m_callback &&
             m_task == rjob->m_task &&
             m_progress == rjob->m_progress;
    }
  }
  return false;
}

bool CMediaImportTaskProcessorJob::ProcessTask()
{
  // check if no task is set and there are no more task types to be performed
  if (m_task == nullptr && m_taskTypesToBeProcessed.empty())
    return true;

  // if a task is set perform it
  if (m_task != nullptr)
  {
    // let the current task do its work
    bool success = ProcessTask(m_task);

    // delete the previously processed task
    delete m_task;
    m_task = nullptr;

    return success;
  }

  // if there are no media imports there's nothing to be processed
  if (m_importTaskData.empty())
    return true;

  // there's no task set and still task types to perform so go through all the media imports and perform the next task type
  MediaImportTaskType currentTaskType = m_taskTypesToBeProcessed.front();

  switch (currentTaskType)
  {
  case MediaImportTaskType::Retrieval:
    ProcessRetrievalTasks();
    break;

  case MediaImportTaskType::Changeset:
    ProcessChangesetTasks();
    break;

  case MediaImportTaskType::Scraping:
    ProcessScrapingTasks();
    break;

  case MediaImportTaskType::Synchronisation:
    ProcessSynchronisationTasks();
    break;

  case MediaImportTaskType::Cleanup:
    ProcessCleanupTasks();
    break;

  case MediaImportTaskType::Removal:
    ProcessRemovalTasks();
    break;

  default:
    CLog::Log(LOGWARNING, "CMediaImportTaskProcessorJob: unknown import task type %d", currentTaskType);
    return false;
  }

  // remove the processed task type from the list of task types to process
  m_taskTypesToBeProcessed.erase(m_taskTypesToBeProcessed.begin());

  // let's do another round of processing in case there's more to do
  return ProcessTask();
}

bool CMediaImportTaskProcessorJob::ProcessTask(IMediaImportTask* task)
{
  if (task == nullptr)
    return false;

  task->SetProcessorJob(this);

  // let the current task do its work
  CLog::Log(LOGDEBUG, "CMediaImportTaskProcessorJob: processing %s task", MediaImportTaskTypes::ToString(task->GetType()).c_str());
  bool success = task->DoWork();

  // the task has been completed
  success &= OnTaskComplete(success, task);

  return success;
}

void CMediaImportTaskProcessorJob::ProcessRetrievalTasks()
{
  for (auto&& taskData = m_importTaskData.begin(); taskData != m_importTaskData.end(); )
  {
    const CMediaImport& import = taskData->second.m_import;
    CMediaImportRetrievalTask* retrievalTask = new CMediaImportRetrievalTask(import, MediaImportHandlerPtr(taskData->second.m_importHandler->Create()));

    // if processing the task failed remove the import (no cleanup needed)
    CLog::Log(LOGINFO, "CMediaImportTaskProcessorJob: starting import retrieval task for %s items from %s...", import.GetMediaType().c_str(), import.GetPath().c_str());
    if (!ProcessTask(retrievalTask))
    {
      CLog::Log(LOGWARNING, "CMediaImportTaskProcessorJob: import retrieval task for %s items from %s failed", import.GetMediaType().c_str(), import.GetPath().c_str());
      m_importTaskData.erase(taskData++);
      delete retrievalTask;
      continue;
    }

    // get the importer used during the retrieval task
    taskData->second.m_importer = retrievalTask->GetImporter();

    // get the local and retrieved items
    taskData->second.m_localItems.Copy(retrievalTask->GetLocalItems());
    taskData->second.m_importedItems = retrievalTask->GetRetrievedItems();

    delete retrievalTask;
    ++taskData;
  }
}

void CMediaImportTaskProcessorJob::ProcessChangesetTasks()
{
  for (auto&& taskData = m_importTaskData.begin(); taskData != m_importTaskData.end(); )
  {
    // nothing to do if the importer already provides a changeset
    if (taskData->second.m_importer->ProvidesChangeset())
      continue;

    const CMediaImport& import = taskData->second.m_import;
    CMediaImportChangesetTask* changesetTask = new CMediaImportChangesetTask(import, MediaImportHandlerPtr(taskData->second.m_importHandler->Create()), taskData->second.m_localItems, taskData->second.m_importedItems);

    // if processing the task failed remove the import (no cleanup needed)
    CLog::Log(LOGINFO, "CMediaImportTaskProcessorJob: starting import changeset task for %s items from %s...", import.GetMediaType().c_str(), import.GetPath().c_str());
    if (!ProcessTask(changesetTask))
    {
      CLog::Log(LOGWARNING, "CMediaImportTaskProcessorJob: import changeset task for %s items from %s failed", import.GetMediaType().c_str(), import.GetPath().c_str());
      m_importTaskData.erase(taskData++);
      delete changesetTask;
      continue;
    }

    // get the changeset
    taskData->second.m_importedItems = changesetTask->GetChangeset();
    delete changesetTask;

    // if the changeset is empty there is nothing else to do
    if (taskData->second.m_importedItems.empty())
    {
      CLog::Log(LOGWARNING, "CMediaImportTaskProcessorJob: no %s items from %s changed", import.GetMediaType().c_str(), import.GetPath().c_str());
      m_importTaskData.erase(taskData++);
      continue;
    }

    ++taskData;
  }
}

void CMediaImportTaskProcessorJob::ProcessScrapingTasks()
{
  // TODO
}

void CMediaImportTaskProcessorJob::ProcessSynchronisationTasks()
{
  // put together a set of all media types to be synchronised
  std::set<MediaType> mediaTypes;
  for (auto&& taskData = m_importTaskData.begin(); taskData != m_importTaskData.end(); ++taskData)
    mediaTypes.insert(taskData->first);

  // retrieve the list of media types in the proper order
  std::vector<MediaType> mediaTypesOrdered = CMediaImportManager::Get().GetSupportedMediaTypesOrdered(mediaTypes);

  // go through all media types in the proper order and perform the synchronisation
  for (const auto& mediaType : mediaTypesOrdered)
  {
    const auto& taskData = m_importTaskData.find(mediaType);
    if (taskData == m_importTaskData.end())
      continue;

    const CMediaImport& import = taskData->second.m_import;
    if (taskData->second.m_importedItems.empty())
    {
      CLog::Log(LOGWARNING, "CMediaImportTaskProcessorJob: no %s items from %s changed", import.GetMediaType().c_str(), import.GetPath().c_str());
      m_importTaskData.erase(taskData->first);
      continue;
    }

    CMediaImportSynchronisationTask* synchronisationTask = new CMediaImportSynchronisationTask(import, MediaImportHandlerPtr(taskData->second.m_importHandler->Create()), taskData->second.m_importedItems);

    // if processing the task failed remove the import (no cleanup needed)
    CLog::Log(LOGINFO, "CMediaImportTaskProcessorJob: starting import synchronisation task for %s items from %s...", import.GetMediaType().c_str(), import.GetPath().c_str());
    if (!ProcessTask(synchronisationTask))
    {
      CLog::Log(LOGWARNING, "CMediaImportTaskProcessorJob: import changeset task for %s items from %s failed", import.GetMediaType().c_str(), import.GetPath().c_str());
      // don't remove the import even though it failed because we should run the cleanup
    }

    delete synchronisationTask;
  }
}

void CMediaImportTaskProcessorJob::ProcessCleanupTasks()
{
  // put together a set of all media types to be cleaned up
  std::set<MediaType> mediaTypes;
  for (auto&& taskData = m_importTaskData.begin(); taskData != m_importTaskData.end(); ++taskData)
    mediaTypes.insert(taskData->first);

  // retrieve the list of media types in the proper REVERSED order
  std::vector<MediaType> mediaTypesOrdered = CMediaImportManager::Get().GetSupportedMediaTypesOrdered(mediaTypes, true);

  // go through all media types in the proper order and perform the cleanup
  for (const auto& mediaType : mediaTypesOrdered)
  {
    const auto& taskData = m_importTaskData.find(mediaType);
    if (taskData == m_importTaskData.end())
      continue;

    const CMediaImport& import = taskData->second.m_import;
    CMediaImportCleanupTask* cleanupTask = new CMediaImportCleanupTask(import, MediaImportHandlerPtr(taskData->second.m_importHandler->Create()));

    // if processing the task failed remove the import (no cleanup needed)
    CLog::Log(LOGINFO, "CMediaImportTaskProcessorJob: starting import cleanup task for %s items from %s...", import.GetMediaType().c_str(), import.GetPath().c_str());
    if (!ProcessTask(cleanupTask))
      CLog::Log(LOGWARNING, "CMediaImportTaskProcessorJob: import cleanup task for %s items from %s failed", import.GetMediaType().c_str(), import.GetPath().c_str());

    delete cleanupTask;
  }
}

void CMediaImportTaskProcessorJob::ProcessRemovalTasks()
{
  // put together a set of all media types to be cleaned up
  std::set<MediaType> mediaTypes;
  for (auto& taskData : m_importTaskData)
    mediaTypes.insert(taskData.first);

  // retrieve the list of media types in the proper REVERSED order
  std::vector<MediaType> mediaTypesOrdered = CMediaImportManager::Get().GetSupportedMediaTypesOrdered(mediaTypes, true);

  // go through all media types in the proper order and perform the removal
  for (const auto& mediaType : mediaTypesOrdered)
  {
    const auto& taskData = m_importTaskData.find(mediaType);
    if (taskData == m_importTaskData.end())
      continue;

    const CMediaImport& import = taskData->second.m_import;
    CMediaImportRemovalTask* removalTask = new CMediaImportRemovalTask(import, MediaImportHandlerPtr(taskData->second.m_importHandler->Create()));

    // if processing the task failed remove the import
    CLog::Log(LOGINFO, "CMediaImportTaskProcessorJob: starting import removal task for %s items from %s...", import.GetMediaType().c_str(), import.GetPath().c_str());
    if (!ProcessTask(removalTask))
      CLog::Log(LOGWARNING, "CMediaImportTaskProcessorJob: import removal task for %s items from %s failed", import.GetMediaType().c_str(), import.GetPath().c_str());

    delete removalTask;
  }
}

bool CMediaImportTaskProcessorJob::OnTaskComplete(bool success, IMediaImportTask *task)
{
  if (m_callback == nullptr)
    return true;

  return m_callback->OnTaskComplete(success, task);
}

bool CMediaImportTaskProcessorJob::AddImport(const CMediaImport& import, std::vector<MediaImportTaskType> tasksToBeProcessed /* = std::vector<MediaImportTaskType>() */)
{
  // check if an import with that media type already exists
  const auto& itImportTaskData = m_importTaskData.find(import.GetMediaType());
  if (itImportTaskData != m_importTaskData.end())
    return false;

  // add the import to the map of imports to process
  MediaImportTaskData importTaskData = { import };

  // get the import handler
  importTaskData.m_importHandler = CMediaImportManager::Get().GetImportHandler(import.GetMediaType());
  if (importTaskData.m_importHandler == nullptr)
    return false;

  m_importTaskData.insert(std::make_pair(import.GetMediaType(), importTaskData));

  // determine the tasks (and their order) to process
  if (tasksToBeProcessed.empty())
  {
    // always do a retrieval
    tasksToBeProcessed.push_back(MediaImportTaskType::Retrieval);

    // also add the changeset task (even though it might not be performed depending on the importer being used)
    tasksToBeProcessed.push_back(MediaImportTaskType::Changeset);

    // TODO: check if we should look for additional metadata using a scraper

    // always do a sychronisation and cleanup
    tasksToBeProcessed.push_back(MediaImportTaskType::Synchronisation);
    tasksToBeProcessed.push_back(MediaImportTaskType::Cleanup);
  }

  // now synchronise the list of tasks to be processed for this import with the one for all imports
  if (m_taskTypesToBeProcessed.empty())
    m_taskTypesToBeProcessed = tasksToBeProcessed;
  else
  {
    size_t start_index = 0;
    for (const auto& newTask : tasksToBeProcessed)
    {
      bool found = false;
      for (std::vector<MediaImportTaskType>::const_iterator task = m_taskTypesToBeProcessed.begin() + start_index; task != m_taskTypesToBeProcessed.end(); ++task)
      {
        if (*task == newTask)
        {
          start_index = std::distance<std::vector<MediaImportTaskType>::const_iterator>(m_taskTypesToBeProcessed.begin(), task);
          found = true;
          break;
        }
      }

      // if the new task hasn't been found insert it at the earliest position
      if (!found)
      {
        m_taskTypesToBeProcessed.insert(m_taskTypesToBeProcessed.begin() + start_index, newTask);
        start_index += 1;
      }
    }
  }

  return true;
}
