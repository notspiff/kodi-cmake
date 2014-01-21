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

#include "MediaImportSourceRegistrationTask.h"
#include "ApplicationMessenger.h"
#include "FileItem.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "dialogs/GUIDialogSelect.h"
#include "dialogs/GUIDialogYesNo.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/LocalizeStrings.h"
#include "media/import/MediaImportManager.h"
#include "media/import/task/MediaImportRetrievalTask.h"
#include "utils/log.h"
#include "utils/StringUtils.h"

CMediaImportSourceRegistrationTask::CMediaImportSourceRegistrationTask(const CMediaImportSource &source)
  : IMediaImportTask(CMediaImport(source.GetIdentifier(), "", source)),
    m_source(source)
{ }

std::set<MediaType> CMediaImportSourceRegistrationTask::ShowAndGetMediaTypesToImport(const std::string &sourceID, bool allowMultipleImports /*= true */)
{
  std::set<MediaType> mediaTypesToImport;

  if (sourceID.empty())
    return mediaTypesToImport;

  CMediaImportSource source(sourceID);
  if (!CMediaImportManager::Get().GetSource(sourceID, source))
    return mediaTypesToImport;

  return ShowAndGetMediaTypesToImport(source, allowMultipleImports);
}

std::set<MediaType> CMediaImportSourceRegistrationTask::ShowAndGetMediaTypesToImport(const CMediaImportSource &source, bool allowMultipleImports /*= true */)
{
  std::set<MediaType> mediaTypesToImport;

  if (source.GetIdentifier().empty() || source.GetFriendlyName().empty() || source.GetAvailableMediaTypes().empty())
    return mediaTypesToImport;

  // put together a list of media types that are available for the source and haven't been imported yet
  std::set<MediaType> unimportedMediaTypes = source.GetAvailableMediaTypes();
  std::vector<CMediaImport> imports = CMediaImportManager::Get().GetImportsBySource(source.GetIdentifier());
  for (std::vector<CMediaImport>::const_iterator itImport = imports.begin(); itImport != imports.end(); ++itImport)
    unimportedMediaTypes.erase(itImport->GetMediaType());

  // show the select dialog with all the media types available for import
  CGUIDialogSelect* pDialogSelect = (CGUIDialogSelect*)g_windowManager.GetWindow(WINDOW_DIALOG_SELECT);
  if (pDialogSelect != NULL)
  {
    pDialogSelect->SetHeading(37042);
    pDialogSelect->SetMultiSelection(allowMultipleImports);

    CFileItemList items;
    if (!GetMediaTypesToImport(unimportedMediaTypes, items))
      return mediaTypesToImport;

    // select all items if allowed
    if (allowMultipleImports)
    {
      for (int index = 0; index < items.Size(); index++)
        items.Get(index)->Select(true);
    }

    pDialogSelect->Add(items);
    pDialogSelect->DoModal();

    if (pDialogSelect->IsConfirmed() && pDialogSelect->GetSelectedItems().Size() > 0)
    {
      const CFileItemList &selectedItems = pDialogSelect->GetSelectedItems();
      for (int index = 0; index < selectedItems.Size(); ++index)
      {
        std::vector<MediaType> selectedMediaTypes = StringUtils::Split(selectedItems.Get(index)->GetPath(), ",");
        for (std::vector<MediaType>::const_iterator itMediaType = selectedMediaTypes.begin(); itMediaType != selectedMediaTypes.end(); ++itMediaType)
          mediaTypesToImport.insert(*itMediaType);
      }
    }
  }

  return mediaTypesToImport;
}

bool CMediaImportSourceRegistrationTask::GetMediaTypesToImport(const std::set<MediaType> &availableMediaTypes, CFileItemList &items)
{
  if (availableMediaTypes.empty())
    return false;

  std::vector< std::vector<MediaType> > supportedMediaTypes = CMediaImportManager::Get().GetSupportedMediaTypes(availableMediaTypes);
  if (supportedMediaTypes.empty())
    return false;

  for (std::vector< std::vector<MediaType> >::const_iterator itSupportedMediaType = supportedMediaTypes.begin(); itSupportedMediaType != supportedMediaTypes.end(); ++itSupportedMediaType)
  {
    std::string label;
    for (std::vector<MediaType>::const_iterator itMediaType = itSupportedMediaType->begin(); itMediaType != itSupportedMediaType->end(); ++itMediaType)
    {
      std::string mediaType = MediaTypes::GetCapitalPluralLocalization(*itMediaType);
      if (itMediaType != itSupportedMediaType->begin())
      {
        int labelId;
        if (itMediaType + 1 == itSupportedMediaType->end())
          labelId = 37045;
        else
          labelId = 37044;
        label = StringUtils::Format(g_localizeStrings.Get(labelId), label.c_str(), mediaType.c_str());
      }
      else
        label += mediaType;
    }
    CFileItemPtr pItem(new CFileItem(label));
    pItem->SetPath(StringUtils::Join(*itSupportedMediaType, ",")); // abuse the path for the media type's identification

    items.Add(pItem);
  }

  return true;
}

bool CMediaImportSourceRegistrationTask::DoWork()
{
  if (CMediaImportManager::Get().GetImporter(m_import.GetPath()) == NULL)
  {
    CLog::Log(LOGINFO, "CMediaImportSourceRegistrationTask: unable to find an importer for source %s", m_source.GetIdentifier().c_str());
    CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, g_localizeStrings.Get(37037), StringUtils::Format(g_localizeStrings.Get(37038), m_source.GetFriendlyName().c_str()));
    return false;
  }

  CGUIDialogYesNo* pDialog = (CGUIDialogYesNo*)g_windowManager.GetWindow(WINDOW_DIALOG_YES_NO);
  if (pDialog == NULL)
    return false;

  pDialog->SetHeading(37037);
  pDialog->SetText(StringUtils::Format(g_localizeStrings.Get(37039), m_source.GetFriendlyName().c_str()));
  pDialog->SetChoice(0, 37040);
  pDialog->SetChoice(1, 37041);

  if (ShouldCancel(0, 2))
    return false;

  // send message and wait for user input
  ThreadMessage tMsg = { TMSG_DIALOG_DOMODAL, WINDOW_DIALOG_YES_NO, (unsigned int)g_windowManager.GetActiveWindow() };
  CApplicationMessenger::Get().SendMessage(tMsg, true);

  if (pDialog->IsConfirmed())
  {
    std::set<MediaType> mediaTypesToImport = ShowAndGetMediaTypesToImport(m_source);
    for (std::set<MediaType>::const_iterator itMediaType = mediaTypesToImport.begin(); itMediaType != mediaTypesToImport.end(); ++itMediaType)
      m_imports.push_back(CMediaImport(m_source.GetIdentifier(), *itMediaType, m_source));
  }

  return true;
}
