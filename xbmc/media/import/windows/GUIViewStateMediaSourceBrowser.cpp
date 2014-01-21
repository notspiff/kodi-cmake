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

#include "GUIViewStateMediaSourceBrowser.h"
#include "FileItem.h"
#include "URL.h"
#include "guilib/GraphicContext.h"
#include "guilib/WindowIDs.h"
#include "view/ViewState.h"

CGUIViewStateMediaSourceBrowser::CGUIViewStateMediaSourceBrowser(const CFileItemList& items)
  : CGUIViewState(items)
{
  CURL url(items.GetPath());

  AddSortMethod(SortByLabel, SortAttributeIgnoreFolders, 551, LABEL_MASKS("%L", "%d", "%L", "%d")); // Filename, Date Time | Foldername, Date Time
  if (items.GetContent() == "sources" || items.GetContent() == "imports")
    AddSortMethod(SortByDate, SortAttributeIgnoreFolders, 578, LABEL_MASKS("%L", "%d", "%L", "%d"));   // Filename, Date Time | Foldername, Date Time
  
  SetSortMethod(SortByLabel, SortAttributeIgnoreFolders);
  SetSortOrder(SortOrderAscending);

  SetViewAsControl(DEFAULT_VIEW_AUTO);

  LoadViewState(items.GetPath(), WINDOW_MEDIASOURCE_BROWSER);
}

void CGUIViewStateMediaSourceBrowser::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_MEDIASOURCE_BROWSER);
}

CStdString CGUIViewStateMediaSourceBrowser::GetExtensions()
{
  return "";
}

VECSOURCES& CGUIViewStateMediaSourceBrowser::GetSources()
{
  m_sources.clear();

  // all registered sources
  {
    CMediaSource source;
    source.strPath = "import://sources/";
    source.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
    source.strName = "Media providers"; // TODO: localization
    m_sources.push_back(source);
  }

  // all imports
  {
    CMediaSource source;
    source.strPath = "import://imports/";
    source.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
    source.strName = "Media imports"; // TODO: localization
    m_sources.push_back(source);
  }

  return CGUIViewState::GetSources();
}

