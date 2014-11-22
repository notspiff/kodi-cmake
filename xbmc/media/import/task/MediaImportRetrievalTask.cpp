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

#include "MediaImportRetrievalTask.h"
#include "guilib/LocalizeStrings.h"
#include "media/import/IMediaImporter.h"
#include "media/import/IMediaImportHandler.h"
#include "media/import/MediaImportManager.h"
#include "utils/log.h"
#include "utils/StringUtils.h"

CMediaImportRetrievalTask::CMediaImportRetrievalTask(const CMediaImport &import, MediaImportHandlerPtr importHandler)
  : IMediaImportTask(import),
    m_importer(),
    m_importHandler(importHandler),
    m_retrievedItems(),
    m_localItems()
{ }

CMediaImportRetrievalTask::~CMediaImportRetrievalTask()
{ }

bool CMediaImportRetrievalTask::DoWork()
{
  if (m_importer == nullptr)
  {
    // look for an importer than can handle the given path
    const MediaImporterConstPtr importer = CMediaImportManager::Get().GetImporter(m_import.GetPath());
    if (importer == nullptr)
    {
      CLog::Log(LOGERROR, "CMediaImportRetrievalTask: no importer capable of handling %s (%s) found", m_import.GetSource().GetFriendlyName().c_str(), m_import.GetPath().c_str());
      return false;
    }
    
    // try to create a new instance of the matching importer for this import
    m_importer = MediaImporterPtr(importer->Create(m_import));
    if (m_importer == nullptr)
    {
      CLog::Log(LOGERROR, "CMediaImportRetrievalTask: failed to create importer %s to handle %s", importer->GetIdentification(), m_import.GetPath().c_str());
      return false;
    }
  }

  GetProgressBarHandle(StringUtils::Format(g_localizeStrings.Get(39008).c_str(), m_import.GetSource().GetFriendlyName().c_str()));

  // first get a list of items previously imported from the media import
  if (m_importHandler != nullptr)
  {
    if (!m_importHandler->GetLocalItems(m_import, m_localItems))
    {
      CLog::Log(LOGERROR, "CMediaImportRetrievalTask: failed to get previously imported items of type %s from %s", m_import.GetMediaType().c_str(), m_import.GetPath().c_str());
      return false;
    }
  }

  return m_importer->Import(this);
}

void CMediaImportRetrievalTask::AddItem(const CFileItemPtr& item, MediaImportChangesetType changesetType /* = MediaImportChangesetTypeNone */)
{
  m_retrievedItems.push_back(std::make_pair(changesetType, item));
}

void CMediaImportRetrievalTask::AddItems(const CFileItemList& items, MediaImportChangesetType changesetType /* = MediaImportChangesetTypeNone */)
{
  for (int index = 0; index < items.Size(); ++index)
    AddItem(items.Get(index), changesetType);
}

void CMediaImportRetrievalTask::SetItems(const ChangesetItems& items)
{
  m_retrievedItems = items;
}
