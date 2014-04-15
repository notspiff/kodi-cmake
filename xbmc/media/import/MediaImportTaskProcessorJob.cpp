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
#include "media/import/IMediaImportTask.h"
#include "utils/log.h"

CMediaImportTaskProcessorJob::CMediaImportTaskProcessorJob(const std::string &path, IMediaImportTaskCallback *callback)
  : m_callback(callback),
    m_task(NULL),
    m_progress(NULL),
    m_path(path)
{ }

CMediaImportTaskProcessorJob::~CMediaImportTaskProcessorJob()
{
  if (m_progress != NULL)
    m_progress->MarkFinished();
}

void CMediaImportTaskProcessorJob::SetTask(IMediaImportTask *task)
{
  m_task = task;
  if (m_task != NULL)
    m_task->SetProcessorJob(this);
}

CGUIDialogProgressBarHandle* CMediaImportTaskProcessorJob::GetProgressBarHandle(const std::string &title /* = "" */)
{
  if (m_progress == NULL)
  {
    CGUIDialogExtendedProgressBar* dialog = (CGUIDialogExtendedProgressBar*)g_windowManager.GetWindow(WINDOW_DIALOG_EXT_PROGRESS);
    if (dialog != NULL)
      m_progress = dialog->GetHandle(title);
  }
  else if (!title.empty())
    m_progress->SetTitle(title);

  return m_progress;
}

bool CMediaImportTaskProcessorJob::DoWork()
{
  if (m_task == NULL)
    return false;

  return ProcessTask();
}

bool CMediaImportTaskProcessorJob::operator==(const CJob *job) const
{
  if (strcmp(job->GetType(), GetType()) == 0)
  {
    const CMediaImportTaskProcessorJob* rjob = dynamic_cast<const CMediaImportTaskProcessorJob*>(job);
    if (rjob != NULL)
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
  // let the current task do its work
  CLog::Log(LOGDEBUG, "CMediaImportTaskProcessorJob: processing task %s", m_task->GetType());
  bool success = m_task->DoWork();

  // the task has been completed
  std::vector<IMediaImportTask*> tasks = OnTaskComplete(success, m_task);

  // delete the previously processed task
  delete m_task;
  m_task = NULL;

  // abort processing if the task failed
  if (!success)
  {
    for (std::vector<IMediaImportTask*>::const_iterator itTask = tasks.begin(); itTask != tasks.end(); ++itTask)
      delete *itTask;
    return false;
  }

  // go through all the following tasks
  for (std::vector<IMediaImportTask*>::const_iterator itTask = tasks.begin(); itTask != tasks.end(); ++itTask)
  {
    SetTask(*itTask);
    if (m_task == NULL)
      continue;

    // process the next task
    if (!ProcessTask())
      return false;
  }

  return true;
}

std::vector<IMediaImportTask*> CMediaImportTaskProcessorJob::OnTaskComplete(bool success, IMediaImportTask *task)
{
  if (m_callback == NULL)
    return std::vector<IMediaImportTask*>();

  return m_callback->OnTaskComplete(success, task);
}
