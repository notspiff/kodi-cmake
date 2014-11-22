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

#include <string>
#include <vector>

#include "media/MediaType.h"
#include "media/import/IMediaImporter.h"
#include "media/import/IMediaImportHandler.h"
#include "media/import/MediaImport.h"
#include "media/import/MediaImportChangesetTypes.h"
#include "media/import/MediaImportTaskTypes.h"
#include "LibraryQueue.h"

class CFileItem;
class CGUIDialogProgressBarHandle;
class CMediaImportSource;
class IMediaImportTask;
class IMediaImportTaskCallback;

class CMediaImportTaskProcessorJob : public CLibraryJob
{
public:
  virtual ~CMediaImportTaskProcessorJob();

  static CMediaImportTaskProcessorJob* RegisterSource(const CMediaImportSource& source, IMediaImportTaskCallback *callback);

  static CMediaImportTaskProcessorJob* Import(const CMediaImportSource& source, bool automatically, IMediaImportTaskCallback *callback);
  static CMediaImportTaskProcessorJob* Import(const CMediaImport& import, bool automatically, IMediaImportTaskCallback *callback);

  static CMediaImportTaskProcessorJob* UpdateImportedItem(const CMediaImport& import, const CFileItem &item, IMediaImportTaskCallback *callback);

  static CMediaImportTaskProcessorJob* Cleanup(const CMediaImportSource& source, IMediaImportTaskCallback *callback);
  static CMediaImportTaskProcessorJob* Cleanup(const CMediaImport& import, IMediaImportTaskCallback *callback);

  static CMediaImportTaskProcessorJob* Remove(const CMediaImportSource& source, const std::vector<CMediaImport> &imports, IMediaImportTaskCallback *callback);
  static CMediaImportTaskProcessorJob* Remove(const CMediaImport& import, IMediaImportTaskCallback *callback);

  const std::string& GetPath() const { return m_path; }

  void SetTask(IMediaImportTask *task);
  const IMediaImportTask* GetCurrentTask() const { return m_task; }

  /*!
   * \brief Get the progress bar handle instance used by the import task
   */
  CGUIDialogProgressBarHandle* GetProgressBarHandle(const std::string &title = "");

  // implementation of CJob
  virtual bool DoWork();
  virtual const char *GetType() const { return "MediaImportTaskProcessorJob"; }
  virtual bool operator==(const CJob *job) const;

protected:
  CMediaImportTaskProcessorJob(const std::string &path, IMediaImportTaskCallback *callback);

  bool ProcessTask();
  bool ProcessTask(IMediaImportTask* task);
  void ProcessRetrievalTasks();
  void ProcessChangesetTasks();
  void ProcessScrapingTasks();
  void ProcessSynchronisationTasks();
  void ProcessCleanupTasks();
  void ProcessRemovalTasks();
  bool OnTaskComplete(bool success, IMediaImportTask *task);

  bool AddImport(const CMediaImport& import, std::vector<MediaImportTaskType> tasksToBeProcessed = std::vector<MediaImportTaskType>());

  IMediaImportTaskCallback *m_callback;
  IMediaImportTask *m_task;
  CGUIDialogProgressBarHandle *m_progress;
  std::string m_path;

  typedef struct MediaImportTaskData {
    CMediaImport m_import;
    MediaImporterConstPtr m_importer;
    MediaImportHandlerConstPtr m_importHandler;

    CFileItemList m_localItems;
    ChangesetItems m_importedItems;
  } MediaImportTaskData;

  std::map<MediaType, MediaImportTaskData> m_importTaskData;
  std::vector<MediaImportTaskType> m_taskTypesToBeProcessed;
};
