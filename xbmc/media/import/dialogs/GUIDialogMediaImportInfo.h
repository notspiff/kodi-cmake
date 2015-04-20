#pragma once
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

#include "settings/dialogs/GUIDialogSettingsManualBase.h"
#include "view/GUIViewControl.h"

class CFileItemList;
class CMediaImport;
class CMediaImportSource;

class CGUIDialogMediaImportInfo : public CGUIDialogSettingsManualBase
{
public:
  CGUIDialogMediaImportInfo();
  virtual ~CGUIDialogMediaImportInfo();

  // specialization of CGUIControl
  virtual bool OnMessage(CGUIMessage& message);
  virtual bool OnAction(const CAction &action);

  static bool ShowForMediaImport(const CFileItemPtr &item);
  static bool ShowForMediaImportSource(const CFileItemPtr &item);

protected:
  // specialization of CGUIControlGroup
  virtual CGUIControl* GetFirstFocusableControl(int id);

  // specializations of CGUIWindow
  virtual void OnWindowLoaded();
  virtual void OnWindowUnload();

  // implementations of ISettingCallback
  virtual void OnSettingChanged(const CSetting *setting);

  // specialization of CGUIDialogSettingsBase
  virtual bool AllowResettingSettings() const { return false; }
  virtual void Save();
  virtual void SetupView();

  // specialization of CGUIDialogSettingsManualBase
  virtual void InitializeSettings();

  bool SetMediaImport(const CFileItemPtr &item);
  bool SetMediaImportSource(const CFileItemPtr &item);

  // media types handling
  void SetupMediaTypes();
  
  CFileItemPtr m_item;
  CMediaImport *m_import;
  CMediaImportSource *m_source;

  // media import settings related
  int m_synchronisationImportTrigger;
  bool m_synchronisationUpdateImportedMediaItems;
  bool m_synchronisationUpdatePlaybackMetadataFromSource;
  bool m_synchronisationUpdateMetadataOnSource;
  bool m_synchronisationUpdatePlaybackMetadataOnSource;

  // media import source related
  CFileItemList* m_sourceMediaTypesList;
  CGUIViewControl m_sourceMediaTypesViewControl;
};

