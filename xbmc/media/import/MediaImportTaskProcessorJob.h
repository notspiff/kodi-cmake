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

#include "utils/Job.h"

class CGUIDialogProgressBarHandle;
class IMediaImportTask;
class IMediaImportTaskCallback;

class CMediaImportTaskProcessorJob : public CJob
{
public:
  CMediaImportTaskProcessorJob(const std::string &path, IMediaImportTaskCallback *callback);
  virtual ~CMediaImportTaskProcessorJob();

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
  bool ProcessTask();
  std::vector<IMediaImportTask*> OnTaskComplete(bool success, IMediaImportTask *task);

  IMediaImportTaskCallback *m_callback;
  IMediaImportTask *m_task;
  CGUIDialogProgressBarHandle *m_progress;
  std::string m_path;
};
