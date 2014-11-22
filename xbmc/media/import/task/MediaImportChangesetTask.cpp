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

#include "MediaImportChangesetTask.h"
#include "guilib/LocalizeStrings.h"
#include "media/import/IMediaImportHandler.h"
#include "utils/log.h"
#include "utils/StringUtils.h"

CMediaImportChangesetTask::CMediaImportChangesetTask(const CMediaImport &import, MediaImportHandlerPtr importHandler, const CFileItemList &localItems, const ChangesetItems &retrievedItems)
  : IMediaImportTask(import),
    m_importHandler(importHandler),
    m_localItems(localItems),
    m_retrievedItems(retrievedItems)
{ }

CMediaImportChangesetTask::~CMediaImportChangesetTask()
{ }

bool CMediaImportChangesetTask::DoWork()
{
  size_t total = m_retrievedItems.size();
  size_t progress = 0;

  // prepare the progress bar
  GetProgressBarHandle(StringUtils::Format(g_localizeStrings.Get(39009).c_str(), m_import.GetSource().GetFriendlyName().c_str()));
  SetProgressText(StringUtils::Format(g_localizeStrings.Get(39010).c_str(), MediaTypes::GetPluralLocalization(m_import.GetMediaType()).c_str()));

  if (ShouldCancel(0, total))
    return false;

  for (ChangesetItems::iterator item = m_retrievedItems.begin(); item != m_retrievedItems.end();)
  {
    // check if we should cancel
    if (ShouldCancel(progress, total))
      return false;

    // determine the changeset state of the item
    item->first = m_importHandler->DetermineChangeset(m_import, item->second.get(), m_localItems);

    // if the changeset state couldn't be determined, ignore the item
    if (item->first == MediaImportChangesetTypeNone)
      item = m_retrievedItems.erase(item);
    else
      ++item;

    ++progress;
    SetProgress(progress, total);
  }

  // all local items left need to be removed
  for (int index = 0; index < m_localItems.Size(); ++index)
    m_retrievedItems.push_back(std::make_pair(MediaImportChangesetTypeRemoved, m_localItems.Get(index)));

  m_localItems.Clear();

  return true;
}
