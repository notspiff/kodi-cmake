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

#include "MediaImportRemovalTask.h"
#include "guilib/LocalizeStrings.h"
#include "media/import/IMediaImportHandler.h"
#include "utils/log.h"
#include "utils/StringUtils.h"

CMediaImportRemovalTask::CMediaImportRemovalTask(const CMediaImport &import, MediaImportHandlerPtr importHandler)
  : IMediaImportTask(import),
    m_importHandler(importHandler)
{ }

CMediaImportRemovalTask::~CMediaImportRemovalTask()
{ }

bool CMediaImportRemovalTask::DoWork()
{
  if (m_importHandler == nullptr)
    return false;

  // prepare the progress bar
  GetProgressBarHandle(StringUtils::Format(g_localizeStrings.Get(39016).c_str(),
    MediaTypes::GetPluralLocalization(m_import.GetMediaType()).c_str(), m_import.GetSource().GetFriendlyName().c_str()));
  SetProgressText("");

  CLog::Log(LOGINFO, "CMediaImportRemovalTask: removing imported %s items from %s",
    m_importHandler->GetMediaType().c_str(), m_import.GetSource().GetIdentifier().c_str());

  return m_importHandler->RemoveImportedItems(m_import);
}
