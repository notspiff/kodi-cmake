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

#include "GUIDialogMediaImportInfo.h"
#include "FileItem.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/WindowIDs.h"
#include "input/Key.h"
#include "media/MediaType.h"
#include "media/import/IMediaImporter.h"
#include "media/import/MediaImportManager.h"
#include "utils/log.h"
#include "utils/Variant.h"

#define PROPERTY_SOURCE_IDENTIFIER                            "Source.ID"
#define PROPERTY_SOURCE_NAME                                  "Source.Name"
#define PROPERTY_SOURCE_ISACTIVE                              "Source.Active"
#define PROPERTY_SOURCE_ISACTIVE_LABEL                        "Source.ActiveLabel"
#define PROPERTY_IMPORT_PATH                                  "Import.Path"
#define PROPERTY_IMPORT_NAME                                  "Import.Name"
#define PROPERTY_IMPORT_MEDIATYPE                             "Import.MediaType"
#define PROPERTY_IMPORT_MEDIATYPE_LABEL                       "Import.MediaTypeLabel"

#define CONTROL_LIST_AREA                                     CONTROL_SETTINGS_CUSTOM + 1

#define SETTING_SYNCHRONISATION_IMPORT_TRIGGER                "synchronisation.importtrigger"
#define SETTING_SYNCHRONISATION_UPDATE_ITEMS                  "synchronisation.updateimporteditems"
#define SETTING_SYNCHRONISATION_UPDATE_PLAYBACK_FROM_SOURCE   "synchronisation.updateplaybackmetadatafromsource"
#define SETTING_SYNCHRONISATION_UPDATE_METADATA_ON_SOURCE     "synchronisation.updatemetadataonsource"
#define SETTING_SYNCHRONISATION_UPDATE_PLAYBACK_ON_SOURCE     "synchronisation.updateplaybackmetadataonsource"

CGUIDialogMediaImportInfo::CGUIDialogMediaImportInfo()
    : CGUIDialogSettingsManualBase(WINDOW_DIALOG_MEDIAIMPORT_INFO, "DialogMediaImportInfo.xml"),
      m_item(new CFileItem()),
      m_import(nullptr),
      m_source(nullptr),
      m_sourceMediaTypesList(new CFileItemList())
{
  m_loadType = KEEP_IN_MEMORY;
}

CGUIDialogMediaImportInfo::~CGUIDialogMediaImportInfo()
{
  delete m_sourceMediaTypesList;
}

bool CGUIDialogMediaImportInfo::OnMessage(CGUIMessage& message)
{
  switch (message.GetMessage())
  {
    case GUI_MSG_WINDOW_DEINIT:
    {
      // get rid of the CMediaImport instance
      if (m_import != nullptr)
      {
        delete m_import;
        m_import = nullptr;
      }
      // get rid of the CMediaImportSource instance
      else if (m_source != nullptr)
      {
        delete m_source;
        m_source = nullptr;
      }

      CGUIDialogSettingsManualBase::OnMessage(message);

      m_sourceMediaTypesViewControl.Clear();
      m_sourceMediaTypesList->Clear();
      return true;
    }

    case GUI_MSG_FOCUSED:
    {
      CGUIDialogSettingsManualBase::OnMessage(message);

      if (m_sourceMediaTypesViewControl.HasControl(message.GetControlId()) &&
               m_sourceMediaTypesViewControl.GetCurrentControl() != message.GetControlId())
        m_sourceMediaTypesViewControl.SetFocused();

      return true;
    }

    default:
        break;
  }

  return CGUIDialogSettingsManualBase::OnMessage(message);
}

bool CGUIDialogMediaImportInfo::OnAction(const CAction &action)
{
  if (action.GetID() == ACTION_SHOW_INFO)
  {
    Close();
    return true;
  }

  return CGUIDialogSettingsManualBase::OnAction(action);
}

bool CGUIDialogMediaImportInfo::ShowForMediaImport(const CFileItemPtr &item)
{
  if (item == nullptr)
    return false;

  CGUIDialogMediaImportInfo* dialog = (CGUIDialogMediaImportInfo*)g_windowManager.GetWindow(WINDOW_DIALOG_MEDIAIMPORT_INFO);
  if (!dialog)
    return false;

  if (!dialog->SetMediaImport(item))
    return false;

  dialog->DoModal();
  return true;
}

bool CGUIDialogMediaImportInfo::ShowForMediaImportSource(const CFileItemPtr &item)
{
  if (item == nullptr)
    return false;

  CGUIDialogMediaImportInfo* dialog = (CGUIDialogMediaImportInfo*)g_windowManager.GetWindow(WINDOW_DIALOG_MEDIAIMPORT_INFO);
  if (!dialog)
    return false;

  if (!dialog->SetMediaImportSource(item))
    return false;

  dialog->DoModal(); 
  return true;
}

CGUIControl *CGUIDialogMediaImportInfo::GetFirstFocusableControl(int id)
{
  if (m_sourceMediaTypesViewControl.HasControl(id))
    id = m_sourceMediaTypesViewControl.GetCurrentControl();

  return CGUIDialogSettingsManualBase::GetFirstFocusableControl(id);
}

void CGUIDialogMediaImportInfo::OnWindowLoaded()
{
  CGUIDialogSettingsManualBase::OnWindowLoaded();

  m_sourceMediaTypesViewControl.Reset();
  m_sourceMediaTypesViewControl.SetParentWindow(GetID());
  m_sourceMediaTypesViewControl.AddView(GetControl(CONTROL_LIST_AREA));
}

void CGUIDialogMediaImportInfo::OnWindowUnload()
{
  CGUIDialogSettingsManualBase::OnWindowUnload();

  m_sourceMediaTypesViewControl.Reset();
}

void CGUIDialogMediaImportInfo::OnSettingChanged(const CSetting *setting)
{
  if (setting == nullptr)
    return;

  CGUIDialogSettingsManualBase::OnSettingChanged(setting);

  if (m_import == nullptr)
    return;

  CMediaImportSettings &importSettings = m_import->GetSettings();

  const std::string &settingId = setting->GetId();
  if (settingId == SETTING_SYNCHRONISATION_IMPORT_TRIGGER)
  {
    m_synchronisationImportTrigger = static_cast<const CSettingInt*>(setting)->GetValue();
    importSettings.SetImportTrigger(static_cast<MediaImportTrigger>(m_synchronisationImportTrigger));
  }
  else if (settingId == SETTING_SYNCHRONISATION_UPDATE_ITEMS)
  {
    m_synchronisationUpdateImportedMediaItems = static_cast<const CSettingBool*>(setting)->GetValue();
    importSettings.SetUpdateImportedMediaItems(m_synchronisationUpdateImportedMediaItems);
  }
  else if (settingId == SETTING_SYNCHRONISATION_UPDATE_PLAYBACK_FROM_SOURCE)
  {
    m_synchronisationUpdatePlaybackMetadataFromSource = static_cast<const CSettingBool*>(setting)->GetValue();
    importSettings.SetUpdatePlaybackMetadataFromSource(m_synchronisationUpdatePlaybackMetadataFromSource);
  }
  else if (settingId == SETTING_SYNCHRONISATION_UPDATE_METADATA_ON_SOURCE)
  {
    m_synchronisationUpdateMetadataOnSource = static_cast<const CSettingBool*>(setting)->GetValue();
    if (m_synchronisationUpdateMetadataOnSource)
      importSettings.SetUpdatePlaybackMetadataOnSource(m_synchronisationUpdatePlaybackMetadataOnSource);
    else
      importSettings.SetUpdatePlaybackMetadataOnSource(false);
  }
  else if (settingId == SETTING_SYNCHRONISATION_UPDATE_PLAYBACK_ON_SOURCE)
  {
    m_synchronisationUpdatePlaybackMetadataOnSource = static_cast<const CSettingBool*>(setting)->GetValue();
    importSettings.SetUpdatePlaybackMetadataOnSource(m_synchronisationUpdatePlaybackMetadataOnSource);
  }

  Save();
}

void CGUIDialogMediaImportInfo::Save()
{
  if (m_import == nullptr)
    return;

  CMediaImportManager::Get().UpdateImport(*m_import);
}

void CGUIDialogMediaImportInfo::SetupView()
{
  CGUIDialogSettingsManualBase::SetupView();

  // setup media types
  SetupMediaTypes();

  if (m_source != nullptr)
    SetHeading(39200);
  else if (m_import != nullptr)
    SetHeading(39201);
}

void CGUIDialogMediaImportInfo::InitializeSettings()
{
  CGUIDialogSettingsManualBase::InitializeSettings();

  if (m_import == nullptr)
    return;

  CSettingCategory *categorySynchronisation = AddCategory("synchronisation", 39250);
  if (categorySynchronisation == nullptr)
  {
    CLog::Log(LOGERROR, "CGUIDialogMediaImportInfo: unable to setup settings");
    return;
  }

  CSettingGroup *groupImport = AddGroup(categorySynchronisation);
  if (groupImport == nullptr)
  {
    CLog::Log(LOGERROR, "CGUIDialogMediaImportInfo: unable to setup settings");
    return;
  }

  bool isContainerMediaType = MediaTypes::IsContainer(m_import->GetMediaType());

  const CMediaImportSettings &importSettings = m_import->GetSettings();
  m_synchronisationImportTrigger = static_cast<int>(importSettings.GetImportTrigger());
  m_synchronisationUpdateImportedMediaItems = importSettings.UpdateImportedMediaItems();
  m_synchronisationUpdatePlaybackMetadataFromSource = importSettings.UpdatePlaybackMetadataFromSource();
  m_synchronisationUpdatePlaybackMetadataOnSource = importSettings.UpdatePlaybackMetadataOnSource();

  StaticIntegerSettingOptions entries;
  entries.push_back(std::make_pair(39305, MediaImportTriggerAuto));
  entries.push_back(std::make_pair(39306, MediaImportTriggerManual));
  AddSpinner(groupImport, SETTING_SYNCHRONISATION_IMPORT_TRIGGER, 39304, 0, m_synchronisationImportTrigger, entries);

  CSettingGroup *groupSynchronisation = AddGroup(categorySynchronisation);
  if (groupSynchronisation == nullptr)
  {
    CLog::Log(LOGERROR, "CGUIDialogMediaImportInfo: unable to setup settings");
    return;
  }

  AddToggle(groupSynchronisation, SETTING_SYNCHRONISATION_UPDATE_ITEMS, 39300, 0, m_synchronisationUpdateImportedMediaItems);

  // playback related settings only make sense for non-container media types
  if (!isContainerMediaType)
  {
    CSettingBool *settingPlaybackFromSource =
      AddToggle(groupSynchronisation, SETTING_SYNCHRONISATION_UPDATE_PLAYBACK_FROM_SOURCE, 39301, 0, m_synchronisationUpdatePlaybackMetadataFromSource);
    // set the parent setting to SETTING_SYNCHRONISATION_UPDATE_ITEMS
    settingPlaybackFromSource->SetParent(SETTING_SYNCHRONISATION_UPDATE_ITEMS);
    // add a dependency on SETTING_SYNCHRONISATION_UPDATE_ITEMS (parent setting) being enabled
    CSettingDependency dependencySynchronisationUpdateItemsEnabled(SettingDependencyTypeEnable, m_settingsManager);
    dependencySynchronisationUpdateItemsEnabled.And()
      ->Add(CSettingDependencyConditionPtr(new CSettingDependencyCondition(SETTING_SYNCHRONISATION_UPDATE_ITEMS, "true", SettingDependencyOperatorEquals, false, m_settingsManager)));
    SettingDependencies depsSynchronisationUpdateItemsEnabled;
    depsSynchronisationUpdateItemsEnabled.push_back(dependencySynchronisationUpdateItemsEnabled);
    settingPlaybackFromSource->SetDependencies(depsSynchronisationUpdateItemsEnabled);
  }

  const std::string &importPath = m_import->GetPath();
  const MediaImporterConstPtr importer = CMediaImportManager::Get().GetImporter(importPath);

  bool canUpdatePlaybackMetadataOnSource = importer != nullptr &&
    (importer->CanUpdatePlaycountOnSource(importPath) ||
     importer->CanUpdateLastPlayedOnSource(importPath) ||
     importer->CanUpdateResumePositionOnSource(importPath));

  // playback related settings only make sense for non-container media types
  if (!isContainerMediaType)
  {
    CSettingBool *settingUpdateMetadataOnSource =
      AddToggle(groupSynchronisation, SETTING_SYNCHRONISATION_UPDATE_METADATA_ON_SOURCE, 39302, 0, m_synchronisationUpdatePlaybackMetadataOnSource);
    settingUpdateMetadataOnSource->SetEnabled(canUpdatePlaybackMetadataOnSource);

    CSettingDependency dependencySynchronisationUpdateMetadataOnSourceEnabled(SettingDependencyTypeEnable, m_settingsManager);
    dependencySynchronisationUpdateMetadataOnSourceEnabled.And()
      ->Add(CSettingDependencyConditionPtr(new CSettingDependencyCondition(SETTING_SYNCHRONISATION_UPDATE_METADATA_ON_SOURCE, "true", SettingDependencyOperatorEquals, false, m_settingsManager)));
    SettingDependencies depsSynchronisationUpdateMetadataOnSourceEnabled;
    depsSynchronisationUpdateMetadataOnSourceEnabled.push_back(dependencySynchronisationUpdateMetadataOnSourceEnabled);

    CSettingBool *settingPlaycountOnSource =
      AddToggle(groupSynchronisation, SETTING_SYNCHRONISATION_UPDATE_PLAYBACK_ON_SOURCE, 39303, 0, m_synchronisationUpdatePlaybackMetadataOnSource);
    // set the parent setting to SETTING_SYNCHRONISATION_UPDATE_METADATA_ON_SOURCE
    settingPlaycountOnSource->SetParent(SETTING_SYNCHRONISATION_UPDATE_METADATA_ON_SOURCE);
    // disable the setting if the importer doesn't support this
    if (!canUpdatePlaybackMetadataOnSource)
      settingPlaycountOnSource->SetEnabled(false);
    else
    {
      // add a dependency on SETTING_SYNCHRONISATION_UPDATE_PLAYBACK_ON_SOURCE (parent setting) being enabled
      settingPlaycountOnSource->SetDependencies(depsSynchronisationUpdateMetadataOnSourceEnabled);
    }
  }
}

bool CGUIDialogMediaImportInfo::SetMediaImport(const CFileItemPtr &item)
{
  if (!item->HasProperty(PROPERTY_SOURCE_IDENTIFIER) ||
      !item->HasProperty(PROPERTY_IMPORT_PATH) ||
      !item->HasProperty(PROPERTY_IMPORT_MEDIATYPE))
    return false;

  std::string importPath = item->GetProperty(PROPERTY_IMPORT_PATH).asString();
  MediaType mediaType = item->GetProperty(PROPERTY_IMPORT_MEDIATYPE).asString();
  if (importPath.empty() || !MediaTypes::IsValidMediaType(mediaType))
    return false;

  // get the import details
  m_import = new CMediaImport(importPath, mediaType);
  if (!CMediaImportManager::Get().GetImport(importPath, mediaType, *m_import))
  {
    delete m_import;
    m_import = nullptr;
    return false;
  }

  // copy the given item
  *m_item = *item;

  return true;
}

bool CGUIDialogMediaImportInfo::SetMediaImportSource(const CFileItemPtr &item)
{
  std::string sourceId = item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString();
  if (sourceId.empty())
    return false;

  m_source = new CMediaImportSource(sourceId);
  if (!CMediaImportManager::Get().GetSource(sourceId, *m_source))
  {
    delete m_source;
    m_source = nullptr;
    return false;
  }

  // copy the given item
  *m_item = *item;

  return true;
}

void CGUIDialogMediaImportInfo::SetupMediaTypes()
{
  if (m_source == nullptr)
    return;

  const std::set<MediaType> &availableMediaTypes = m_source->GetAvailableMediaTypes();
  if (availableMediaTypes.empty())
    return;

  // get all existing imports of the 
  std::vector<CMediaImport> imports = CMediaImportManager::Get().GetImportsBySource(m_source->GetIdentifier());

  for (const auto& itMediaType : availableMediaTypes)
  {
    CFileItemPtr item(new CFileItem(MediaTypes::GetCapitalPluralLocalization(itMediaType)));

    for (std::vector<CMediaImport>::iterator itImport = imports.begin(); itImport != imports.end(); ++itImport)
    {
      if (MediaTypes::IsMediaType(itMediaType, itImport->GetMediaType()))
      {
        item->Select(true);
        imports.erase(itImport);
        break;
      }
    }

    m_sourceMediaTypesList->Add(item);
  }

  m_sourceMediaTypesList->Sort(SortByLabel, SortOrderAscending);

  m_sourceMediaTypesViewControl.SetItems(*m_sourceMediaTypesList);
  m_sourceMediaTypesViewControl.SetCurrentView(CONTROL_LIST_AREA);
}
