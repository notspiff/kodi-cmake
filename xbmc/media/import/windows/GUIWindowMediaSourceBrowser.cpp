/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include "GUIWindowMediaSourceBrowser.h"
#include "FileItem.h"
#include "GUIUserMessages.h"
#include "URL.h"
#include "dialogs/GUIDialogOK.h"
#include "dialogs/GUIDialogProgress.h"
#include "dialogs/GUIDialogSelect.h"
#include "dialogs/GUIDialogYesNo.h"
#include "filesystem/MediaImportDirectory.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/LocalizeStrings.h"
#include "guilib/WindowIDs.h"
#include "input/Key.h"
#include "media/import/MediaImportManager.h"
#include "media/import/dialogs/GUIDialogMediaImportInfo.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"

#define PROPERTY_SOURCE_IDENTIFIER  "Source.ID"
#define PROPERTY_SOURCE_ISACTIVE    "Source.Active"
#define PROPERTY_IMPORT_PATH        "Import.Path"
#define PROPERTY_IMPORT_MEDIATYPE   "Import.MediaType"

using namespace std;

CGUIWindowMediaSourceBrowser::CGUIWindowMediaSourceBrowser(void)
: CGUIMediaWindow(WINDOW_MEDIASOURCE_BROWSER, "MediaSourceBrowser.xml")
{ }

CGUIWindowMediaSourceBrowser::~CGUIWindowMediaSourceBrowser()
{ }

std::set<MediaType> CGUIWindowMediaSourceBrowser::ShowAndGetMediaTypesToImport(const std::string &sourceID)
{
  std::set<MediaType> mediaTypesToImport;

  if (sourceID.empty())
    return mediaTypesToImport;

  CMediaImportSource source(sourceID);
  if (!CMediaImportManager::Get().GetSource(sourceID, source))
    return mediaTypesToImport;

  return ShowAndGetMediaTypesToImport(source);
}

std::set<MediaType> CGUIWindowMediaSourceBrowser::ShowAndGetMediaTypesToImport(const CMediaImportSource &source)
{
  std::set<MediaType> mediaTypesToImport;

  if (source.GetIdentifier().empty() || source.GetFriendlyName().empty() || source.GetAvailableMediaTypes().empty())
    return mediaTypesToImport;

  // put together a list of media types that are available for the source and haven't been imported yet
  std::set<MediaType> unimportedMediaTypes = source.GetAvailableMediaTypes();
  std::vector<CMediaImport> imports = CMediaImportManager::Get().GetImportsBySource(source.GetIdentifier());
  for (const auto& import : imports)
    unimportedMediaTypes.erase(import.GetMediaType());

  // show the select dialog with all the media types available for import
  CGUIDialogSelect* pDialogSelect = (CGUIDialogSelect*)g_windowManager.GetWindow(WINDOW_DIALOG_SELECT);
  if (pDialogSelect != nullptr)
  {
    pDialogSelect->SetHeading(39005);
    pDialogSelect->SetMultiSelection(false);

    CFileItemList items;
    if (!GetMediaTypesToImport(unimportedMediaTypes, items))
      return mediaTypesToImport;

    pDialogSelect->Add(items);
    pDialogSelect->DoModal();

    if (pDialogSelect->IsConfirmed() && pDialogSelect->GetSelectedItem() != nullptr)
    {
      std::vector<MediaType> selectedMediaTypes = StringUtils::Split(pDialogSelect->GetSelectedItem()->GetPath(), ",");
      for (const auto& itMediaType : selectedMediaTypes)
        mediaTypesToImport.insert(itMediaType);
    }
  }

  return mediaTypesToImport;
}

bool CGUIWindowMediaSourceBrowser::GetMediaTypesToImport(const std::set<MediaType> &availableMediaTypes, CFileItemList &items)
{
  if (availableMediaTypes.empty())
    return false;

  std::vector< std::vector<MediaType> > supportedMediaTypes = CMediaImportManager::Get().GetSupportedMediaTypesGrouped(availableMediaTypes);
  if (supportedMediaTypes.empty())
    return false;

  for (const auto& itSupportedMediaType : supportedMediaTypes)
  {
    std::string label;
    for (std::vector<MediaType>::const_iterator itMediaType = itSupportedMediaType.begin(); itMediaType != itSupportedMediaType.end(); ++itMediaType)
    {
      std::string mediaType = MediaTypes::GetCapitalPluralLocalization(*itMediaType);
      if (itMediaType != itSupportedMediaType.begin())
      {
        int labelId;
        if (itMediaType + 1 == itSupportedMediaType.end())
          labelId = 39007;
        else
          labelId = 39006;
        label = StringUtils::Format(g_localizeStrings.Get(labelId).c_str(), label.c_str(), mediaType.c_str());
      }
      else
        label += mediaType;
    }
    CFileItemPtr pItem(new CFileItem(label));
    pItem->SetPath(StringUtils::Join(itSupportedMediaType, ",")); // abuse the path for the media type's identification

    items.Add(pItem);
  }

  return true;
}

bool CGUIWindowMediaSourceBrowser::OnMessage(CGUIMessage& message)
{
  switch (message.GetMessage())
  {
    case GUI_MSG_WINDOW_INIT:
    {
      m_rootDir.AllowNonLocalSources(false);

      // is this the first time the window is opened?
      if (m_vecItems->GetPath() == "?" && message.GetStringParam().empty())
        m_vecItems->SetPath("");
      break;
    }

    case GUI_MSG_WINDOW_DEINIT:
    {
      break;
    }

    case GUI_MSG_CLICKED:
    {
      int iControl = message.GetSenderId();
      if (m_viewControl.HasControl(iControl))  // list/thumb control
      {
        // get selected item
        int itemIndex = m_viewControl.GetSelectedItem();
        int actionId = message.GetParam1();

        if (actionId == ACTION_SHOW_INFO ||
            actionId == ACTION_DELETE_ITEM)
        {
          CFileItemPtr item = m_vecItems->Get(itemIndex);
          if (item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).empty())
            return false;

          if (item->GetProperty(PROPERTY_IMPORT_PATH).empty())
          {
            if (actionId == ACTION_SHOW_INFO)
              return OnSourceInfo(item);
            else if (actionId == ACTION_DELETE_ITEM)
              return OnSourceDelete(item);
          }
          else
          {
            if (actionId == ACTION_SHOW_INFO)
              return OnImportInfo(item);
            else if (actionId == ACTION_DELETE_ITEM)
              return OnImportDelete(item);
          }
        }
      }
      break;
    }

    case GUI_MSG_NOTIFY_ALL:
    {
      CGUIListItemPtr item = message.GetItem();
      if (item == nullptr)
        break;

      switch (message.GetParam1())
      {
        case GUI_MSG_SOURCE_ADDED:
        {
          OnSourceAdded(message.GetItem()->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString());
          return true;
        }

        case GUI_MSG_SOURCE_UPDATED:
        {
          OnSourceUpdated(message.GetItem()->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString());
          return true;
        }

        case GUI_MSG_SOURCE_REMOVED:
        {
          OnSourceRemoved(message.GetItem()->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString());
          return true;
        }

        case GUI_MSG_SOURCE_ACTIVE_CHANGED:
        {
          OnSourceIsActiveChanged(message.GetItem()->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString(), message.GetParam1() > 0);
          return true;
        }

        case GUI_MSG_IMPORT_ADDED:
        {
          OnImportAdded(item->GetProperty(PROPERTY_IMPORT_PATH).asString(), item->GetProperty(PROPERTY_IMPORT_MEDIATYPE).asString());
          return true;
        }

        case GUI_MSG_IMPORT_UPDATED:
        {
          OnImportUpdated(item->GetProperty(PROPERTY_IMPORT_PATH).asString(), item->GetProperty(PROPERTY_IMPORT_MEDIATYPE).asString());
          return true;
        }

        case GUI_MSG_IMPORT_REMOVED:
        {
          OnImportRemoved(item->GetProperty(PROPERTY_IMPORT_PATH).asString(), item->GetProperty(PROPERTY_IMPORT_MEDIATYPE).asString());
          return true;
        }

        default:
          break;
      }
    }

    default:
      break;
  }
  return CGUIMediaWindow::OnMessage(message);
}

bool CGUIWindowMediaSourceBrowser::RefreshList(bool keepSelectedItem /* = true */)
{
  std::string currentItemPath;
  if (keepSelectedItem)
  {
    int itemIndex = m_viewControl.GetSelectedItem();
    if (itemIndex >= 0 && itemIndex < m_vecItems->Size())
      currentItemPath = m_vecItems->Get(itemIndex)->GetPath();
  }

  bool result = Refresh(true);

  if (keepSelectedItem && !currentItemPath.empty())
    m_viewControl.SetSelectedItem(currentItemPath);

  return result;
}

bool CGUIWindowMediaSourceBrowser::OnSourceSynchronise(const CFileItemPtr &item)
{
  if (item == nullptr)
    return false;

  if (CMediaImportManager::Get().Import(item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString()))
    return true;

  CGUIDialogOK::ShowAndGetInput(39110, StringUtils::Format(g_localizeStrings.Get(39111).c_str(), item->GetLabel().c_str()));
  return false;
}

bool CGUIWindowMediaSourceBrowser::OnSourceInfo(const CFileItemPtr &item)
{
  if (item == nullptr)
    return false;

  return CGUIDialogMediaImportInfo::ShowForMediaImportSource(item);
}

bool CGUIWindowMediaSourceBrowser::OnSourceDelete(const CFileItemPtr &item)
{
  if (item == nullptr)
    return false;

  CGUIDialogYesNo* pDialog = (CGUIDialogYesNo*)g_windowManager.GetWindow(WINDOW_DIALOG_YES_NO);
  if (pDialog == nullptr)
    return false;

  pDialog->SetHeading(39102);
  pDialog->SetText(StringUtils::Format(g_localizeStrings.Get(39103).c_str(), item->GetLabel().c_str()));

  pDialog->DoModal();

  if (!pDialog->IsConfirmed())
    return false;

  CMediaImportManager::Get().RemoveSource(item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString());
  return true;
}

bool CGUIWindowMediaSourceBrowser::OnImportSynchronise(const CFileItemPtr &item)
{
  if (item == nullptr)
    return false;

  if (CMediaImportManager::Get().Import(item->GetProperty(PROPERTY_IMPORT_PATH).asString(),
                                        item->GetProperty(PROPERTY_IMPORT_MEDIATYPE).asString()))
    return true;

  CGUIDialogOK::ShowAndGetInput(39112, StringUtils::Format(g_localizeStrings.Get(39113).c_str(), item->GetLabel().c_str()));
  return false;
}

bool CGUIWindowMediaSourceBrowser::OnImportInfo(const CFileItemPtr &item)
{
  if (item == nullptr)
    return false;

  return CGUIDialogMediaImportInfo::ShowForMediaImport(item);
}

bool CGUIWindowMediaSourceBrowser::OnImportDelete(const CFileItemPtr &item)
{
  if (item == nullptr)
    return false;

  CGUIDialogYesNo* pDialog = (CGUIDialogYesNo*)g_windowManager.GetWindow(WINDOW_DIALOG_YES_NO);
  if (pDialog == nullptr)
    return false;

  std::string sourceID = item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString();
  std::string importPath = item->GetProperty(PROPERTY_IMPORT_PATH).asString();
  MediaType mediaType = item->GetProperty(PROPERTY_IMPORT_MEDIATYPE).asString();

  // get all media types connected to the media type of the import to delete
  std::set<MediaType> connectedMediaTypes = CMediaImportManager::Get().GetConnectedMediaTypes(mediaType);
  
  // get all imports of the same source as the import to delete
  std::set<MediaType> removedMediaTypes;
  std::vector<CMediaImport> imports = CMediaImportManager::Get().GetImportsBySource(sourceID);
  for (const auto& import : imports)
  {
    if (connectedMediaTypes.find(import.GetMediaType()) != connectedMediaTypes.end())
      removedMediaTypes.insert(import.GetMediaType());
  }

  std::string label;
  for (std::set<MediaType>::const_iterator itMediaType = removedMediaTypes.begin(); itMediaType != removedMediaTypes.end(); )
  {
    std::string mediaType = MediaTypes::GetCapitalPluralLocalization(*itMediaType);
    bool first = itMediaType == removedMediaTypes.begin();
    ++itMediaType;

    if (first)
      label += mediaType;
    else
    {
      int labelId;
      if (itMediaType == removedMediaTypes.end())
        labelId = 39007;
      else
        labelId = 39006;
      label = StringUtils::Format(g_localizeStrings.Get(labelId).c_str(), label.c_str(), mediaType.c_str());
    }
  }

  std::string text;
  if (label.empty())
    text = StringUtils::Format(g_localizeStrings.Get(39103).c_str(), item->GetLabel().c_str());
  else
    text = StringUtils::Format(g_localizeStrings.Get(39105).c_str(), item->GetLabel().c_str(), label.c_str());
  pDialog->SetHeading(39104);
  pDialog->SetText(text);

  pDialog->DoModal();

  if (!pDialog->IsConfirmed())
    return false;

  CMediaImportManager::Get().RemoveImport(importPath, mediaType);
  return true;
}

void CGUIWindowMediaSourceBrowser::OnSourceAdded(const std::string &sourceId)
{
  if (m_vecItems->GetContent() == "sources")
    RefreshList(true);
}

void CGUIWindowMediaSourceBrowser::OnSourceUpdated(const std::string &sourceId)
{
  if (m_vecItems->GetContent() == "sources")
  {
    // only refresh the list if the updated source is part of it
    CFileItemPtr item = GetSourceItem(sourceId);
    if (item != nullptr)
      RefreshList(true);
  }
}

void CGUIWindowMediaSourceBrowser::OnSourceRemoved(const std::string &sourceId)
{
  if (m_vecItems->GetContent() == "sources")
  {
    // only refresh the list if the removed source is part of it
    CFileItemPtr item = GetSourceItem(sourceId);
    if (item != nullptr)
    {
      int selectedItemIndex = m_viewControl.GetSelectedItem();
      // only update the selected item index when the deleted item is focused
      if (m_vecItems->Get(selectedItemIndex) != item)
        selectedItemIndex = -1;

      RefreshList(false);

      if (selectedItemIndex >= 0)
        m_viewControl.SetSelectedItem(selectedItemIndex);
    }
  }
}

void CGUIWindowMediaSourceBrowser::OnImportAdded(const std::string &importPath, const MediaType &mediaType)
{
  if (m_vecItems->GetContent() == "imports")
    RefreshList(true);
}

void CGUIWindowMediaSourceBrowser::OnImportUpdated(const std::string &importPath, const MediaType &mediaType)
{
  if (m_vecItems->GetContent() == "imports")
  {
    // only refresh the list if the updated import is part of it
    CFileItemPtr item = GetImportItem(importPath, mediaType);
    if (item != nullptr)
      RefreshList(true);
  }
}

void CGUIWindowMediaSourceBrowser::OnImportRemoved(const std::string &importPath, const MediaType &mediaType)
{
  if (m_vecItems->GetContent() == "imports")
  {
    // only refresh the list if the removed import is part of it
    CFileItemPtr item = GetImportItem(importPath, mediaType);
    if (item != nullptr)
    {
      int selectedItemIndex = m_viewControl.GetSelectedItem();
      // only update the selected item index when the deleted item is focused
      if (m_vecItems->Get(selectedItemIndex) != item)
        selectedItemIndex = -1;

      RefreshList(false);

      if (selectedItemIndex >= 0)
        m_viewControl.SetSelectedItem(selectedItemIndex);
    }
  }
}

void CGUIWindowMediaSourceBrowser::OnSourceIsActiveChanged(const std::string &sourceId, bool isactive)
{
  if (m_vecItems->GetContent() == "sources")
  {
    CFileItemPtr item = GetSourceItem(sourceId);
    if (item != nullptr)
      item->SetProperty(PROPERTY_SOURCE_ISACTIVE, isactive);
    else
      RefreshList(true);
  }
  else if (m_vecItems->GetContent() == "imports")
  {
    for (int index = 0; index < m_vecItems->Size(); index++)
    {
      CFileItemPtr item = m_vecItems->Get(index);
      if (item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString() == sourceId)
        item->SetProperty(PROPERTY_SOURCE_ISACTIVE, isactive);
    }
  }
}

CFileItemPtr CGUIWindowMediaSourceBrowser::GetImportItem(const std::string &importPath, const MediaType &mediaType) const
{
  for (int index = 0; index < m_vecItems->Size(); index++)
  {
    CFileItemPtr item = m_vecItems->Get(index);
    if (item->IsParentFolder())
      continue;

    if (item->GetProperty(PROPERTY_IMPORT_PATH).asString() == importPath &&
        item->GetProperty(PROPERTY_IMPORT_MEDIATYPE).asString() == mediaType)
      return item;
  }

  return CFileItemPtr();
}

CFileItemPtr CGUIWindowMediaSourceBrowser::GetSourceItem(const std::string &sourceId) const
{
  for (int index = 0; index < m_vecItems->Size(); index++)
  {
    CFileItemPtr item = m_vecItems->Get(index);
    if (item->IsParentFolder())
      continue;

    if (item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString() == sourceId)
      return item;
  }

  return CFileItemPtr();
}

bool CGUIWindowMediaSourceBrowser::GetDirectory(const std::string& strDirectory, CFileItemList& items)
{
  if (!CGUIMediaWindow::GetDirectory(strDirectory, items))
    return false;

  CURL url(strDirectory);
  // "Add import" button for imports by media provider views
  if (items.GetContent() == "imports" &&
      StringUtils::StartsWith(url.GetFileName(), "sources"))
  {
    std::string sourceDir = strDirectory;
    URIUtils::RemoveSlashAtEnd(sourceDir);
    std::string sourceID = URIUtils::GetFileName(sourceDir);
    sourceID = CURL::Decode(sourceID);

    std::string addImportPath = "newimport://" + CURL::Encode(sourceID);
    if (!items.Contains(addImportPath))
    {
      CMediaImportSource source(sourceID);
      if (CMediaImportManager::Get().GetSource(sourceID, source))
      {
        std::vector<CMediaImport> imports = CMediaImportManager::Get().GetImportsBySource(sourceID);
        if (imports.size() < source.GetAvailableMediaTypes().size())
        {
          CFileItemPtr addImport(new CFileItem(addImportPath, false));
          addImport->SetLabel(g_localizeStrings.Get(39106));
          addImport->SetLabelPreformated(true);
          addImport->SetSpecialSort(SortSpecialOnBottom);
          items.Add(addImport);
        }
      }
    }
  }

  return true;
}

std::string CGUIWindowMediaSourceBrowser::GetStartFolder(const std::string &dir)
{
  if (StringUtils::StartsWithNoCase(dir, "import://"))
    return dir;

  return CGUIMediaWindow::GetStartFolder(dir);
}

void CGUIWindowMediaSourceBrowser::GetContextButtons(int itemNumber, CContextButtons& buttons)
{
  CFileItemPtr item = m_vecItems->Get(itemNumber);
  bool isSource = !item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).empty() && !item->HasProperty(PROPERTY_IMPORT_PATH);
  bool isImport = !item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).empty() && item->HasProperty(PROPERTY_IMPORT_PATH) && item->HasProperty(PROPERTY_IMPORT_MEDIATYPE);

  if (isSource || isImport)
  {
    // only allow synchronisation of the source is active and has imports
    if (item->GetProperty(PROPERTY_SOURCE_ISACTIVE).asBoolean() &&
       ((isSource && CMediaImportManager::Get().HasImports(item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString())) || isImport))
       buttons.Add(CONTEXT_BUTTON_SCAN, g_localizeStrings.Get(39107));
    buttons.Add(CONTEXT_BUTTON_INFO, g_localizeStrings.Get(19033));
    buttons.Add(CONTEXT_BUTTON_DELETE, g_localizeStrings.Get(15015));
  }
}

bool CGUIWindowMediaSourceBrowser::OnContextButton(int itemNumber, CONTEXT_BUTTON button)
{
  CFileItemPtr item = m_vecItems->Get(itemNumber);
  bool isSource = !item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).empty() && !item->HasProperty(PROPERTY_IMPORT_PATH);

  switch (button)
  {
    case CONTEXT_BUTTON_SCAN:
      if (item->GetProperty(PROPERTY_SOURCE_ISACTIVE).asBoolean())
      {
        if (isSource)
          return OnSourceSynchronise(item);
        else
          return OnImportSynchronise(item);
      }
      break;

    case CONTEXT_BUTTON_INFO:
      if (isSource)
        return OnSourceInfo(item);
      else
        return OnImportInfo(item);
      break;

    case CONTEXT_BUTTON_DELETE:
    {
      if (isSource)
        return OnSourceDelete(item);
      else
        return OnImportDelete(item);
      break;
    }

    default:
      break;
  }

  return CGUIMediaWindow::OnContextButton(itemNumber, button);
}

bool CGUIWindowMediaSourceBrowser::OnClick(int iItem)
{
  CFileItemPtr item = m_vecItems->Get(iItem);
  CURL url(item->GetPath());

  if (url.GetProtocol() == "newimport" && !url.GetHostName().empty() && url.GetFileName().empty())
  {
    // ask the user what media type to add as an import
    std::string sourceID = CURL::Decode(url.GetHostName());
    std::set<MediaType> mediaTypesToImport = ShowAndGetMediaTypesToImport(sourceID);
    if (mediaTypesToImport.empty())
      return true;

    // add the new import (they aren't automatically synchronised)
    if (!CMediaImportManager::Get().AddImports(sourceID, sourceID, mediaTypesToImport))
    {
      CGUIDialogOK::ShowAndGetInput(39108, g_localizeStrings.Get(39109));
      return false;
    }

    if (mediaTypesToImport.size() == 1)
    {
      // show the info dialog for the new import
      CMediaImport import(sourceID, *mediaTypesToImport.begin(), sourceID);
      if (CMediaImportManager::Get().GetImport(sourceID, import.GetMediaType(), import))
      {
        CFileItemPtr importItem = XFILE::CMediaImportDirectory::FileItemFromMediaImport(import, m_vecItems->GetPath());
        if (importItem != nullptr)
        {
          // refresh the list so that the new import is listed
          RefreshList(false);

          // now select the new import so that it will be available to skins in the import info dialog
          m_viewControl.SetSelectedItem(importItem->GetPath());

          // and now finally open the import info dialog
          OnImportInfo(importItem);

          // now see if the add import button is still there and if so re-select it
          if (m_vecItems->Contains(item->GetPath()))
            m_viewControl.SetSelectedItem(item->GetPath());
        }
      }

      // now try to synchronise the newly added import
      CMediaImportManager::Get().Import(sourceID, *mediaTypesToImport.begin());
    }

    return true;
  }

  if (!item->m_bIsFolder)
  {
    // if the item is a media import, show the info dialog
    if (!item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).empty() && item->HasProperty(PROPERTY_IMPORT_PATH) && item->HasProperty(PROPERTY_IMPORT_MEDIATYPE))
      return OnImportInfo(item);
    // if the item is a media provider, show the info dialog
    else if (!item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).empty() && !item->HasProperty(PROPERTY_IMPORT_PATH))
      return OnSourceInfo(item);
  }

  return CGUIMediaWindow::OnClick(iItem);
}

void CGUIWindowMediaSourceBrowser::UpdateButtons()
{
  CGUIMediaWindow::UpdateButtons();
}
