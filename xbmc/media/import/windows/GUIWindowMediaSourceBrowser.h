#pragma once

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

#include <set>
#include <string>

#include "media/MediaType.h"
#include "media/import/MediaImportSource.h"
#include "windows/GUIMediaWindow.h"

class CGUIWindowMediaSourceBrowser : public CGUIMediaWindow
{
public:
  CGUIWindowMediaSourceBrowser();
  virtual ~CGUIWindowMediaSourceBrowser();

  static std::set<MediaType> ShowAndGetMediaTypesToImport(const std::string &sourceId);
  static std::set<MediaType> ShowAndGetMediaTypesToImport(const CMediaImportSource &source);
  static bool GetMediaTypesToImport(const std::set<MediaType> &availableMediaTypes, CFileItemList &items);

  // specialization of CGUIControl
  virtual bool OnMessage(CGUIMessage& message);
  
protected:
  // specialization of CGUIMediaWindow
  virtual bool GetDirectory(const std::string &strDirectory, CFileItemList &items);
  virtual std::string GetStartFolder(const std::string &dir);

  virtual void GetContextButtons(int itemNumber, CContextButtons &buttons);
  virtual bool OnContextButton(int itemNumber, CONTEXT_BUTTON button);

  virtual bool OnClick(int iItem);
  virtual void UpdateButtons();

  bool RefreshList(bool keepSelectedItem = true);

  bool OnSourceSynchronise(const CFileItemPtr &item);
  bool OnSourceInfo(const CFileItemPtr &item);
  bool OnSourceDelete(const CFileItemPtr &item);
  bool OnImportSynchronise(const CFileItemPtr &item);
  bool OnImportInfo(const CFileItemPtr &item);
  bool OnImportDelete(const CFileItemPtr &item);

  void OnSourceAdded(const std::string &sourceId);
  void OnSourceUpdated(const std::string &sourceId);
  void OnSourceRemoved(const std::string &sourceId);
  void OnImportAdded(const std::string &importPath, const MediaType &mediaType);
  void OnImportUpdated(const std::string &importPath, const MediaType &mediaType);
  void OnImportRemoved(const std::string &importPath, const MediaType &mediaType);
  void OnSourceIsActiveChanged(const std::string &sourceId, bool isactive);
  
  CFileItemPtr GetImportItem(const std::string &importPath, const MediaType &mediaType) const;
  CFileItemPtr GetSourceItem(const std::string &sourceId) const;
};

