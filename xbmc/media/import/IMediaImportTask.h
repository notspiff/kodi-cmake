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

#include <vector>

#include "dialogs/GUIDialogExtendedProgressBar.h"
#include "media/import/MediaImport.h"
#include "media/import/MediaImportTaskProcessorJob.h"
#include "media/import/MediaImportTaskTypes.h"
#include "utils/Job.h"

/*!
 * \brief Interface of a media importer capable of importing media items from
 * a specific source into the local library.
 */
class IMediaImportTask
{
public:
  virtual ~IMediaImportTask() { }

  /*!
   * \brief Gets the import processed by the import task.
   */
  const CMediaImport& GetImport() const { return m_import; }

  /*!
   * \brief Gets the processor job which is executing the import task.
   */
  CMediaImportTaskProcessorJob* GetProcessorJob() const { return m_processorJob; }

  /*!
   * \brief Sets the processor job which will be executing the import task.
   */
  void SetProcessorJob(CMediaImportTaskProcessorJob *processorJob) { m_processorJob = processorJob; }

  /*!
   * \brief Get the progress bar handle instance used by the import task.
   */
  CGUIDialogProgressBarHandle* GetProgressBarHandle() const { return m_progress; }

  /*!
  * \brief Gets the type of the import task.
  */
  virtual MediaImportTaskType GetType() = 0;

  /*!
   * \brief Executes the import task.
   *
   * \return True if the task succeeded, false otherwise
   */
  virtual bool DoWork() = 0;

  /*
   * \brief Updates the progress of the task and checks if it should be cancelled.
   *
   * \param progress Current progress
   * \param total Total progress steps
   * \return True if the task should be cancelled, false otherwise
   */
  bool ShouldCancel(unsigned int progress, unsigned int total) const
  {
    if (m_processorJob == nullptr)
      return false;

    return m_processorJob->ShouldCancel(progress, total);
  }

  /*
   * \brief Sets the title of the progress indicator.
   */
  inline void SetProgressTitle(const std::string &title)
  {
    if (m_progress != nullptr)
      m_progress->SetTitle(title);
  }

  /*
  * \brief Sets the text/description of the progress indicator.
  */
  inline void SetProgressText(const std::string &text)
  {
    if (m_progress != nullptr)
      m_progress->SetText(text);
  }
  
  /*
   * \brief Sets the progress of the progress indicator.
   *
   * \param progress Current progress
   * \param total Total progress steps
   */
  inline void SetProgress(int progress, int total)
  {
    if (m_progress != nullptr)
      m_progress->SetProgress(progress, total);
  }

protected:
  IMediaImportTask(const CMediaImport &import)
    : m_import(import),
      m_processorJob(nullptr),
      m_progress(nullptr)
  { }

  /*
   * \brief Prepares a progress indicator with the given title set.
   *
   * \param title Title to be set in the progress indicator.
   */
  void GetProgressBarHandle(const std::string &title)
  {
    if (m_processorJob != nullptr)
      m_progress = m_processorJob->GetProgressBarHandle(title);
  }

  CMediaImport m_import;
  CMediaImportTaskProcessorJob *m_processorJob;
  CGUIDialogProgressBarHandle *m_progress;
};

class IMediaImportTaskCallback
{
public:
  virtual ~IMediaImportTaskCallback() { }

  /*
   * \brief Callback for completed tasks.
   *
   * \param success Whether the task succeeded or failed.
   * \param task The task that has completed
   * \return True if the callback succeeded, false otherwise
   */
  virtual bool OnTaskComplete(bool success, IMediaImportTask *task) = 0;
};
