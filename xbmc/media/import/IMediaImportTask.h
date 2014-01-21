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
   * \brief TODO
   */
  const CMediaImport& GetImport() const { return m_import; }

  /*!
   * \brief TODO
   */
  CMediaImportTaskProcessorJob* GetProcessorJob() const { return m_processorJob; }

  /*!
   * \brief TODO
   */
  void SetProcessorJob(CMediaImportTaskProcessorJob *processorJob) { m_processorJob = processorJob; }

  /*!
   * \brief Get the progress bar handle instance used by the import task
   */
  CGUIDialogProgressBarHandle* GetProgressBarHandle() const { return m_progress; }

  /*!
   * \brief TODO
   */
  virtual bool DoWork() = 0;
  
  /*!
   * \brief TODO
   */
  virtual const char *GetType() const = 0;

  bool ShouldCancel(unsigned int progress, unsigned int total) const
  {
    if (m_processorJob == NULL)
      return false;

    return m_processorJob->ShouldCancel(progress, total);
  }

  inline void SetProgressTitle(const std::string &title)
  {
    if (m_progress != NULL)
      m_progress->SetTitle(title);
  }

  inline void SetProgressText(const std::string &text)
  {
    if (m_progress != NULL)
      m_progress->SetText(text);
  }

  inline void SetProgress(int progress, int total)
  {
    if (m_progress != NULL)
      m_progress->SetProgress(progress, total);
  }

protected:
  IMediaImportTask(const CMediaImport &import)
    : m_import(import),
      m_processorJob(NULL),
      m_progress(NULL)
  { }

  void GetProgressBarHandle(const std::string &title)
  {
    if (m_processorJob != NULL)
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

  virtual std::vector<IMediaImportTask*> OnTaskComplete(bool success, IMediaImportTask *task) = 0;
};
