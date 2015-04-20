/*
 *      Copyright (C) 2014 Team XBMC
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

#include "MediaImportSynchronisationTask.h"
#include "guilib/LocalizeStrings.h"
#include "media/import/IMediaImportHandler.h"
#include "utils/log.h"
#include "utils/StringUtils.h"

CMediaImportSynchronisationTask::CMediaImportSynchronisationTask(const CMediaImport &import, MediaImportHandlerPtr importHandler, const ChangesetItems &items)
  : IMediaImportTask(import),
    m_importHandler(importHandler),
    m_items(items)
{ }

CMediaImportSynchronisationTask::~CMediaImportSynchronisationTask()
{ }

bool CMediaImportSynchronisationTask::DoWork()
{
  if (m_importHandler == nullptr)
    return false;

  if (!m_importHandler->StartSynchronisation(m_import))
  {
    CLog::Log(LOGINFO, "CMediaImportSynchronisationTask: failed to initialize synchronisation of imported %s items from %s",
      m_importHandler->GetMediaType().c_str(), m_import.GetSource().GetIdentifier().c_str());
    return false;
  }

  // prepare the progress bar
  GetProgressBarHandle(StringUtils::Format(g_localizeStrings.Get(39011).c_str(),
    MediaTypes::GetPluralLocalization(m_import.GetMediaType()).c_str(), m_import.GetSource().GetFriendlyName().c_str()));
  SetProgressText("");

  if (ShouldCancel(0, m_items.size()))
    return false;

  CLog::Log(LOGINFO, "CMediaImportSynchronisationTask: handling %d imported %s items from %s",
    static_cast<int>(m_items.size()), m_importHandler->GetMediaType().c_str(), m_import.GetSource().GetIdentifier().c_str());
  // handle the imported items of a specific media type
  size_t total = m_items.size();
  size_t progress = 0;
  for (const auto& item : m_items)
  {
    // check if we should cancel
    if (ShouldCancel(progress, total))
      return false;

    // get the item label to be used in the progress bar text
    std::string itemLabel = m_importHandler->GetItemLabel(item.second.get());

    // process the item depending on its changeset state
    switch (item.first)
    {
      case MediaImportChangesetTypeAdded:
        SetProgressText(StringUtils::Format(g_localizeStrings.Get(39012).c_str(), itemLabel.c_str()));
        m_importHandler->AddImportedItem(m_import, item.second.get());
        break;

      case MediaImportChangesetTypeChanged:
        SetProgressText(StringUtils::Format(g_localizeStrings.Get(39013).c_str(), itemLabel.c_str()));
        m_importHandler->UpdateImportedItem(m_import, item.second.get());
        break;

      case MediaImportChangesetTypeRemoved:
        SetProgressText(StringUtils::Format(g_localizeStrings.Get(39014).c_str(), itemLabel.c_str()));
        m_importHandler->RemoveImportedItem(m_import, item.second.get());
        break;

      case MediaImportChangesetTypeNone:
      default:
        CLog::Log(LOGWARNING, "CMediaImportSynchronisationTask: ignoring imported item with unknown changeset type %d", item.first);
        break;
    }

    ++progress;
    SetProgress(progress, total);
  }

  if (!m_importHandler->FinishSynchronisation(m_import))
  {
    CLog::Log(LOGINFO, "CMediaImportSynchronisationTask: failed to finalize synchronisation of imported %s items from %s",
      m_importHandler->GetMediaType().c_str(), m_import.GetSource().GetIdentifier().c_str());
    return false;
  }

  return true;
}
