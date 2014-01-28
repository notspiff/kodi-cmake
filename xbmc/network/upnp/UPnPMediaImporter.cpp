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

#include "UPnPMediaImporter.h"
#include "FileItem.h"
#include "URL.h"
#include "Platinum.h"
#include "dialogs/GUIDialogExtendedProgressBar.h"
#include "guilib/LocalizeStrings.h"
#include "media/import/task/MediaImportRetrievalTask.h"
#include "media/import/task/MediaImportUpdateTask.h"
#include "network/upnp/UPnP.h"
#include "network/upnp/UPnPInternal.h"
#include "settings/Settings.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "video/VideoInfoTag.h"

using namespace UPNP;

#define UPNP_ROOT_CONTAINER_ID "0"

typedef struct {
  MediaType mediaType;
  const char* objectIdentification;
} SupportedMediaType;

static SupportedMediaType SupportedMediaTypes[] = {
  { MediaTypeMovie,       "object.item.videoItem.movie" },
  { MediaTypeTvShow,      "object.container.album.videoAlbum.videoBroadcastShow" },
  { MediaTypeSeason,      "object.container.album.videoAlbum.videoBroadcastSeason" },
  { MediaTypeEpisode,     "object.item.videoItem.videoBroadcast" },
  { MediaTypeMusicVideo,  "object.item.videoItem.musicVideoClip" },
  //{ MediaTypeArtist,      "object.container.person.musicArtist" },
  //{ MediaTypeAlbum,       "object.container.album.musicAlbum" },
  //{ MediaTypeSong,        "object.item.audioItem" }
};

#define SupportedMediaTypesSize sizeof(SupportedMediaTypes) / sizeof(SupportedMediaType)

static bool FindServer(const std::string &deviceUUID, PLT_DeviceDataReference &device)
{
  if (deviceUUID.empty())
    return false;

  return CUPnP::GetInstance()->m_MediaBrowser->FindServer(deviceUUID.c_str(), device) == NPT_SUCCESS;
}

static CFileItemPtr ConstructItem(PLT_DeviceDataReference &device, PLT_MediaObject *object)
{
  CFileItemPtr pItem = BuildObject(object);
  CStdString id;
  if (object->m_ReferenceID.IsEmpty())
    id = (const char*)object->m_ObjectID;
  else
    id = (const char*)object->m_ReferenceID;

  CURL::Encode(id);
  URIUtils::AddSlashAtEnd(id);
  pItem->SetPath("upnp://" + CStdString(device->GetUUID()) + "/" + id);
  pItem->GetVideoInfoTag()->m_strPath = pItem->GetVideoInfoTag()->m_strFileNameAndPath = pItem->GetPath();
  
  return pItem;
}

static void ConstructList(PLT_DeviceDataReference &device, PLT_MediaObjectListReference &list, std::vector<CFileItemPtr> &items)
{
  if (list.IsNull())
    return;

  for (PLT_MediaObjectList::Iterator entry = list->GetFirstItem() ; entry ; ++entry)
  {
    CFileItemPtr item(ConstructItem(device, *entry));
    if (item != NULL)
      items.push_back(item);
  }
}

static bool Search(PLT_DeviceDataReference &device, const std::string &search_criteria, std::vector<CFileItemPtr> &items)
{
  PLT_SyncMediaBrowser *mediaBrowser = CUPnP::GetInstance()->m_MediaBrowser;
  if (mediaBrowser == NULL)
    return false;

  PLT_MediaObjectListReference list;
  if (mediaBrowser->SearchSync(device, UPNP_ROOT_CONTAINER_ID, search_criteria.c_str(), list) == NPT_SUCCESS)
  {
    ConstructList(device, list, items);
    return true;
  }

  return false;
}

static bool Search(CMediaImportRetrievalTask *task, PLT_DeviceDataReference &device, const MediaType &mediaType, std::vector<CFileItemPtr> &items)
{
  SupportedMediaType* supportedMediaType = NULL;
  for (size_t i = 0; i < SupportedMediaTypesSize; i++)
  {
    if (mediaType == SupportedMediaTypes[i].mediaType)
    {
      supportedMediaType = &SupportedMediaTypes[i];
      break;
    }
  }

  if (supportedMediaType == NULL)
    return false;

  task->GetProgressBarHandle()->SetText(StringUtils::Format(g_localizeStrings.Get(37043), MediaTypes::GetPluralLocalization(supportedMediaType->mediaType).c_str()));
  if (!Search(device, StringUtils::Format("upnp:class = \"%s\"", supportedMediaType->objectIdentification), items))
    return false;

  if (mediaType == MediaTypeTvShow || mediaType == MediaTypeSeason)
  {
    std::vector<CFileItemPtr> showItems;
    for (std::vector<CFileItemPtr>::iterator it = items.begin() ; it != items.end() ; ++it)
    { // discard video albums that are NOT tv shows
      if ((*it)->HasVideoInfoTag() && (*it)->GetVideoInfoTag()->m_type == mediaType)
        showItems.push_back(*it);
    }

    items.clear();
    items.assign(showItems.begin(), showItems.end());
  }

  return true;
}

CUPnPMediaImporter::CUPnPMediaImporter()
  : IMediaImporter(CMediaImport("", MediaTypeNone))
{ }

CUPnPMediaImporter::CUPnPMediaImporter(const CMediaImport &import)
  : IMediaImporter(import)
{
  CURL url(import.GetPath());
  if (url.GetProtocol() == "upnp")
    m_deviceUUID = url.GetHostName();
}

bool CUPnPMediaImporter::CanImport(const std::string& path) const
{
  PLT_DeviceDataReference device;
  if (!validatePath(path, device))
    return false;

  if (!CSettings::Get().GetBool("services.upnpimportcompatibleonly"))
    return true;

  PLT_SyncMediaBrowser *mediaBrowser = CUPnP::GetInstance()->m_MediaBrowser;
  if (mediaBrowser == NULL)
    return false;

  NPT_String searchCapabilities;
  if (NPT_FAILED(mediaBrowser->GetSearchCapabilitiesSync(device, searchCapabilities)))
    return false;

  return !searchCapabilities.IsEmpty() && searchCapabilities.Find("upnp:class") >= 0;
}

/* TODO: update on source
bool CUPnPMediaImporter::CanUpdatePlaycountOnSource(const std::string& path) const
{
  // TODO: find a more generic condition
  return isXbmcServer(path);
}

bool CUPnPMediaImporter::CanUpdateLastPlayedOnSource(const std::string& path) const
{
  // TODO: find a more generic condition
  return isXbmcServer(path);
}

bool CUPnPMediaImporter::CanUpdateResumePositionOnSource(const std::string& path) const
{
  // TODO: find a more generic condition
  return isXbmcServer(path);
}
*/

IMediaImporter* CUPnPMediaImporter::Create(const CMediaImport &import) const
{
  if (!CanImport(import.GetPath()))
    return NULL;

  return new CUPnPMediaImporter(import);
}

bool CUPnPMediaImporter::Import(CMediaImportRetrievalTask* task) const
{
  if (task == NULL)
    return false;

  PLT_DeviceDataReference device;
  if (!FindServer(m_deviceUUID, device))
  {
    CLog::Log(LOGINFO, "CUPnPMediaImporter: unable to import media items from unavailable source %s", m_deviceUUID.c_str());
    return false;
  }

  std::set<MediaType> handledMediaTypes;
  const MediaType &importedMediaType = GetImport().GetMediaType();
  bool success = false;
  for (size_t i = 0; i < SupportedMediaTypesSize; i++)
  {
    if (MediaTypes::IsMediaType(importedMediaType, SupportedMediaTypes[i].mediaType))
    {
      success = true;
      break;
    }
  }

  if (!success)
    return false;

  success = false;
  if (task->ShouldCancel(0, 1))
    return false;

  std::vector<CFileItemPtr> items;
  if (!Search(task, device, importedMediaType, items))
    return false;

  // remove any items that are not part of the requested path
  for (std::vector<CFileItemPtr>::iterator itItem = items.begin(); itItem != items.end(); )
  {
    if (!URIUtils::IsInPath((*itItem)->GetPath(), GetPath()))
      itItem = items.erase(itItem);
    else
      ++itItem;
  }

  task->SetItems(items);
  return true;
}

bool CUPnPMediaImporter::UpdateOnSource(CMediaImportUpdateTask* task) const
{
  if (task == NULL || !task->GetItem().IsImported())
    return false;

  PLT_DeviceDataReference device;
  if (!FindServer(m_deviceUUID, device))
  {
    CLog::Log(LOGINFO, "CUPnPMediaImporter: unable to update imported media items on unavailable source %s", m_deviceUUID.c_str());
    return false;
  }

  /* TODO: update on source
  const CMediaImportSettings &importSettings = task->GetImport().GetSettings();
  if (!importSettings.UpdatePlaybackMetadataOnSource())
    return false;
  */
  
  const std::string &importPath = task->GetImport().GetPath();
  if (!CanUpdatePlaycountOnSource(importPath) &&
      !CanUpdateLastPlayedOnSource(importPath) &&
      !CanUpdateResumePositionOnSource(importPath))
    return false;

  /* TODO: we need a way to get the original path/object identifier from the server
  return CUPnP::GetInstance()->UpdateItem("TODO", task->GetItem());
  */

  return false;
}

bool CUPnPMediaImporter::validatePath(const std::string& path, PLT_DeviceDataReference &device)
{
  CURL url(path);
  if (path.empty() || url.GetProtocol() != "upnp")
    return false;

  std::string deviceUUID = url.GetHostName();
  return FindServer(deviceUUID, device);
}

bool CUPnPMediaImporter::isXbmcServer(const std::string &path)
{
  PLT_DeviceDataReference device;
  if (!validatePath(path, device))
    return false;

  PLT_SyncMediaBrowser *mediaBrowser = CUPnP::GetInstance()->m_MediaBrowser;
  if (mediaBrowser == NULL)
    return false;

  NPT_String sortCapabilities;
  if (NPT_FAILED(mediaBrowser->GetSortCapabilitiesSync(device, sortCapabilities)))
    return false;

  return !sortCapabilities.IsEmpty() && sortCapabilities.Find("xbmc:") >= 0;
}
