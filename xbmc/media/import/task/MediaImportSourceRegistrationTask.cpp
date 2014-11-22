/*
 *      Copyright (C) 2015 Team XBMC
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

#include "MediaImportSourceRegistrationTask.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "guilib/LocalizeStrings.h"
#include "media/import/MediaImportManager.h"
#include "utils/log.h"

CMediaImportSourceRegistrationTask::CMediaImportSourceRegistrationTask(const CMediaImportSource &source)
  : IMediaImportTask(CMediaImport(source.GetIdentifier(), MediaTypeNone, source))
{ }

CMediaImportSourceRegistrationTask::~CMediaImportSourceRegistrationTask()
{ }

bool CMediaImportSourceRegistrationTask::DoWork()
{
  const CMediaImportSource& source = m_import.GetSource();

  if (CMediaImportManager::Get().GetImporter(source.GetIdentifier()) == nullptr)
  {
    CLog::Log(LOGINFO, "CMediaImportManager: unable to find an importer for source %s (%s)",
      source.GetFriendlyName().c_str(), source.GetIdentifier().c_str());
    CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, g_localizeStrings.Get(39001).c_str(), source.GetFriendlyName());
    return false;
  }

  return true;
}
