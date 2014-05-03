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


#include "system.h"
#include "Util.h"
#include "utils/URIUtils.h"
#include "FileDirectoryFactory.h"
#ifdef HAS_FILESYSTEM
#include "UDFDirectory.h"
#include "RSSDirectory.h"
#endif
#if defined(TARGET_ANDROID)
#include "APKDirectory.h"
#endif
#include "ZipDirectory.h"
#include "SmartPlaylistDirectory.h"
#include "playlists/SmartPlayList.h"
#include "PlaylistFileDirectory.h"
#include "playlists/PlayListFactory.h"
#include "Directory.h"
#include "File.h"
#include "ZipManager.h"
#include "settings/AdvancedSettings.h"
#include "FileItem.h"
#include "utils/StringUtils.h"
#include "addons/AddonManager.h"
#include "addons/AudioDecoder.h"
#include "addons/VFSEntry.h"
#include "M4BFileDirectory.h"

using namespace ADDON;
using namespace XFILE;
using namespace PLAYLIST;
using namespace std;

CFileDirectoryFactory::CFileDirectoryFactory(void)
{}

CFileDirectoryFactory::~CFileDirectoryFactory(void)
{}

// return NULL + set pItem->m_bIsFolder to remove it completely from list.
IFileDirectory* CFileDirectoryFactory::Create(const CStdString& strPath, CFileItem* pItem, const CStdString& strMask)
{
  if (URIUtils::IsStack(strPath)) // disqualify stack as we need to work with each of the parts instead
    return NULL;

  CStdString strExtension=URIUtils::GetExtension(strPath);
  StringUtils::ToLower(strExtension);

#ifdef HAS_FILESYSTEM
  VECADDONS codecs;
  CAddonMgr::Get().GetAddons(ADDON_AUDIODECODER, codecs);
  for (size_t i=0;i<codecs.size();++i)
  {
    boost::shared_ptr<CAudioDecoder> dec(boost::static_pointer_cast<CAudioDecoder>(codecs[i]));
    if (!strExtension.empty() && dec->HasTracks() &&
        dec->GetExtensions().find(strExtension) != std::string::npos)
    {
      CAudioDecoder* result = new CAudioDecoder(*dec);
      static_cast<AudioDecoderDll&>(*result).Create();
      if (result->ContainsFiles(strPath))
        return result;
      delete result;
      return NULL;
    }
  }

  CAddonMgr::Get().GetAddons(ADDON_VFS, codecs);
  for (size_t i=0;i<codecs.size();++i)
  {
    boost::shared_ptr<CVFSEntry> dec(boost::static_pointer_cast<CVFSEntry>(codecs[i]));
    if (!strExtension.empty() && dec->HasFileDirectories() &&
        dec->GetExtensions().find(strExtension) != std::string::npos)
    {
      CVFSEntryIFileDirectoryWrapper* wrap = new CVFSEntryIFileDirectoryWrapper(CVFSEntryManager::Get().GetAddon(dec->ID()));
      if (wrap->ContainsFiles(strPath))
      {
        if (wrap->m_items.Size() == 1)
        {
          // one STORED file - collapse it down
          *pItem = *wrap->m_items[0];
        }
        else
        { // compressed or more than one file -> create a dir
          pItem->SetPath(wrap->m_items.GetPath());
          return wrap;
        }
      }
      else
        pItem->m_bIsFolder = true;

      delete wrap;
      return NULL;
    }
  }

  if (pItem->IsRSS())
    return new CRSSDirectory();

  if (pItem->IsDVDImage())
    return new CUDFDirectory();

#endif
#if defined(TARGET_ANDROID)
  if (strExtension.Equals(".apk"))
  {
    CStdString strUrl;
    URIUtils::CreateArchivePath(strUrl, "apk", strPath, "");

    CFileItemList items;
    CDirectory::GetDirectory(strUrl, items, strMask);
    if (items.Size() == 0) // no files
      pItem->m_bIsFolder = true;
    else if (items.Size() == 1 && items[0]->m_idepth == 0 && !items[0]->m_bIsFolder)
    {
      // one STORED file - collapse it down
      *pItem = *items[0];
    }
    else
    { // compressed or more than one file -> create a apk dir
      pItem->SetPath(strUrl);
      return new CAPKDirectory;
    }
    return NULL;
  }
#endif
  if (strExtension.Equals(".zip"))
  {
    CStdString strUrl;
    URIUtils::CreateArchivePath(strUrl, "zip", strPath, "");

    CFileItemList items;
    CDirectory::GetDirectory(strUrl, items, strMask);
    if (items.Size() == 0) // no files
      pItem->m_bIsFolder = true;
    else if (items.Size() == 1 && items[0]->m_idepth == 0 && !items[0]->m_bIsFolder)
    {
      // one STORED file - collapse it down
      *pItem = *items[0];
    }
    else
    { // compressed or more than one file -> create a zip dir
      pItem->SetPath(strUrl);
      return new CZipDirectory;
    }
    return NULL;
  }
  if (strExtension.Equals(".xsp"))
  { // XBMC Smart playlist - just XML renamed to XSP
    // read the name of the playlist in
    CSmartPlaylist playlist;
    if (playlist.OpenAndReadName(strPath))
    {
      pItem->SetLabel(playlist.GetName());
      pItem->SetLabelPreformated(true);
    }
    IFileDirectory* pDir=new CSmartPlaylistDirectory;
    return pDir; // treat as directory
  }
  if (CPlayListFactory::IsPlaylist(strPath))
  { // Playlist file
    // currently we only return the directory if it contains
    // more than one file.  Reason is that .pls and .m3u may be used
    // for links to http streams etc.
    IFileDirectory *pDir = new CPlaylistFileDirectory();
    CFileItemList items;
    if (pDir->GetDirectory(strPath, items))
    {
      if (items.Size() > 1)
        return pDir;
    }
    delete pDir;
    return NULL;
  }

  if (pItem->IsAudioBook() || strExtension.Equals(".mka") || pItem->IsType(".mka"))
  {
    if (!pItem->HasMusicInfoTag() || pItem->m_lEndOffset <= 0)
    {
      CM4BFileDirectory* pDir = new CM4BFileDirectory;
      if (pDir->ContainsFiles(strPath))
        return pDir;
      delete pDir;
    }
    return NULL;
  }
  return NULL;
}

