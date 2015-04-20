/*
 *      Copyright (C) 2011-2013 Team XBMC
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

#include "MediaImportDirectory.h"
#include "DatabaseManager.h"
#include "DbUrl.h"
#include "FileItem.h"
#include "guilib/LocalizeStrings.h"
#include "filesystem/Directory.h"
#include "media/MediaType.h"
#include "media/import/IMediaImportRepository.h"
#include "media/import/MediaImportManager.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "video/VideoDbUrl.h"

#define PROPERTY_SOURCE_IDENTIFIER      "Source.ID"
#define PROPERTY_SOURCE_NAME            "Source.Name"
#define PROPERTY_SOURCE_ISACTIVE        "Source.Active"
#define PROPERTY_SOURCE_ISACTIVE_LABEL  "Source.ActiveLabel"
#define PROPERTY_IMPORT_PATH            "Import.Path"
#define PROPERTY_IMPORT_MEDIATYPE       "Import.MediaType"
#define PROPERTY_IMPORT_MEDIATYPE_LABEL "Import.MediaTypeLabel"
#define PROPERTY_IMPORT_NAME            "Import.Name"

using namespace std;
using namespace XFILE;

CMediaImportDirectory::CMediaImportDirectory(void)
{ }

CMediaImportDirectory::~CMediaImportDirectory(void)
{ }

bool CMediaImportDirectory::GetDirectory(const CURL& url, CFileItemList &items)
{
  const std::string& strPath = url.Get();
  if (url.GetHostName() == "imports")
  {
    std::string filename = url.GetFileName();
    URIUtils::RemoveSlashAtEnd(filename);
    if (filename.empty())
    {
      // All
      if (CMediaImportManager::Get().HasImports())
      {
        CFileItemPtr item(new CFileItem(URIUtils::AddFileToFolder(strPath, "all"), true));
        item->SetLabel(g_localizeStrings.Get(39020));
        item->SetSpecialSort(SortSpecialOnTop);
        items.Add(item);
      }

        // by media provider
      if (CMediaImportManager::Get().HasSources())
      {
        CFileItemPtr item(new CFileItem(URIUtils::AddFileToFolder(strPath, "sources"), true));
        item->SetLabel(g_localizeStrings.Get(39021));
        item->SetSpecialSort(SortSpecialOnTop);
        items.Add(item);
      }

      // by media types
      if (CMediaImportManager::Get().HasImports())
      {
        CFileItemPtr item(new CFileItem(URIUtils::AddFileToFolder(strPath, "mediatypes"), true));
        item->SetLabel(g_localizeStrings.Get(39022));
        item->SetSpecialSort(SortSpecialOnTop);
        items.Add(item);
      }

      items.SetLabel(g_localizeStrings.Get(39101));
      return true;
    }
    else if (filename == "all" || StringUtils::StartsWith(filename, "sources") || StringUtils::StartsWith(filename, "mediatypes"))
    {
      if (filename == "all" ||
          StringUtils::StartsWith(filename, "sources/") ||
          StringUtils::StartsWith(filename, "mediatypes/"))
      {
        std::vector<CMediaImport> imports;
        if (filename == "all")
        {
          items.SetLabel(g_localizeStrings.Get(39020));
          imports = CMediaImportManager::Get().GetImports();
        }
        else if (StringUtils::StartsWith(filename, "sources/"))
        {
          std::string sourceID = StringUtils::Mid(filename, 8);
          URIUtils::RemoveSlashAtEnd(sourceID);
          if (sourceID.find('/') != std::string::npos)
            return false;

          sourceID = CURL::Decode(sourceID);
          CMediaImportSource source(sourceID);
          if (!CMediaImportManager::Get().GetSource(sourceID, source))
            return false;

          items.SetLabel(source.GetFriendlyName());
          imports = CMediaImportManager::Get().GetImportsBySource(sourceID);
        }
        else if (StringUtils::StartsWith(filename, "mediatypes/"))
        {
          std::string mediaType = StringUtils::Mid(filename, 11);
          URIUtils::RemoveSlashAtEnd(mediaType);
          mediaType = CURL::Decode(mediaType);
          if (mediaType.find('/') != std::string::npos ||
              !MediaTypes::IsValidMediaType(mediaType))
            return false;

          items.SetLabel(MediaTypes::GetCapitalPluralLocalization(mediaType));
          imports = CMediaImportManager::Get().GetImportsByMediaType(mediaType);
        }
        else
          return false;

        HandleImports(strPath, imports, items);
        return true;
      }
      else if (filename == "sources")
      {
        items.SetLabel(g_localizeStrings.Get(39021));
        HandleSources(strPath, CMediaImportManager::Get().GetSources(), items, true);
        return true;
      }
      else if (filename == "mediatypes")
      {
        items.SetLabel(g_localizeStrings.Get(39022));
        std::vector<MediaType> mediaTypes = CMediaImportManager::Get().GetSupportedMediaTypesOrdered();
        for (std::vector<MediaType>::const_iterator itMediaType = mediaTypes.begin(); itMediaType != mediaTypes.end(); ++itMediaType)
        {
          std::string path = URIUtils::AddFileToFolder(strPath, CURL::Encode(*itMediaType));
          CFileItemPtr item(new CFileItem(path, true));
          item->SetLabel(MediaTypes::GetCapitalPluralLocalization(*itMediaType));

          items.Add(item);
        }

        return true;
      }
    }
  }
  else if (url.GetHostName() == "sources")
  {
    std::string filename = url.GetFileName();
    URIUtils::RemoveSlashAtEnd(filename);
    if (filename.empty())
    {
      // All
      if (CMediaImportManager::Get().HasSources())
      {
        {
          CFileItemPtr item(new CFileItem(URIUtils::AddFileToFolder(strPath, "all"), true));
          item->SetLabel(g_localizeStrings.Get(39023));
          item->SetSpecialSort(SortSpecialOnTop);
          items.Add(item);
        }

        // Active
        if (CMediaImportManager::Get().HasSources(true))
        {
          CFileItemPtr item(new CFileItem(URIUtils::AddFileToFolder(strPath, "active"), true));
          item->SetLabel(g_localizeStrings.Get(39024));
          items.Add(item);
        }

        // Inactive
        if (CMediaImportManager::Get().HasSources(false))
        {
          CFileItemPtr item(new CFileItem(URIUtils::AddFileToFolder(strPath, "inactive"), true));
          item->SetLabel(g_localizeStrings.Get(39025));
          items.Add(item);
        }
      }

      items.SetLabel(g_localizeStrings.Get(39100));
      return true;
    }
    else if (filename == "all" || filename == "active" || filename == "inactive")
    {
      std::vector<CMediaImportSource> sources;
      if (filename == "all")
      {
        items.SetLabel(g_localizeStrings.Get(39023));
        sources = CMediaImportManager::Get().GetSources();
      }
      else
      {
        if (filename == "active")
          items.SetLabel(g_localizeStrings.Get(39024));
        else
          items.SetLabel(g_localizeStrings.Get(39025));

        sources = CMediaImportManager::Get().GetSources(filename == "active");
      }

      HandleSources(strPath, sources, items);
      return true;
    }
  }

  return false;
}

void CMediaImportDirectory::HandleSources(const std::string &strPath, const std::vector<CMediaImportSource> &sources, CFileItemList &items, bool asFolder /* = false */)
{
  for (std::vector<CMediaImportSource>::const_iterator itSource = sources.begin(); itSource != sources.end(); ++itSource)
  {
    CFileItemPtr item = FileItemFromMediaImportSource(*itSource, strPath, asFolder);
    if (item != NULL)
      items.Add(item);
  }

  items.SetContent("sources");
}

CFileItemPtr CMediaImportDirectory::FileItemFromMediaImportSource(const CMediaImportSource &source, const std::string &basePath, bool asFolder /* = false */)
{
  if (source.GetIdentifier().empty() || source.GetFriendlyName().empty())
    return CFileItemPtr();
  
  // prepare the path
  std::string path = URIUtils::AddFileToFolder(basePath, CURL::Encode(source.GetIdentifier()));
  if (asFolder)
    URIUtils::AddSlashAtEnd(path);

  CFileItemPtr item(new CFileItem(path, asFolder));
  item->SetLabel(source.GetFriendlyName());
  item->m_dateTime = source.GetLastSynced();

  if (!source.GetIconUrl().empty())
    item->SetArt("thumb", source.GetIconUrl());

  item->SetProperty(PROPERTY_SOURCE_IDENTIFIER, source.GetIdentifier());
  item->SetProperty(PROPERTY_SOURCE_NAME, source.GetFriendlyName());
  item->SetProperty(PROPERTY_SOURCE_ISACTIVE, source.IsActive());
  item->SetProperty(PROPERTY_SOURCE_ISACTIVE_LABEL, source.IsActive() ? g_localizeStrings.Get(39026) : g_localizeStrings.Get(39027));

  return item;
}

void CMediaImportDirectory::HandleImports(const std::string &strPath, const std::vector<CMediaImport> &imports, CFileItemList &items, bool bySource /* = false */)
{
  for (std::vector<CMediaImport>::const_iterator itImport = imports.begin(); itImport != imports.end(); ++itImport)
  {
    CFileItemPtr item = FileItemFromMediaImport(*itImport, strPath, bySource);
    if (item != NULL)
      items.Add(item);
  }

  items.SetContent("imports");
}

CFileItemPtr CMediaImportDirectory::FileItemFromMediaImport(const CMediaImport &import, const std::string &basePath, bool bySource /* = false */)
{
  if (import.GetPath().empty() || import.GetMediaType().empty())
    return CFileItemPtr();

  const CMediaImportSource &source = import.GetSource();

  CURL url(URIUtils::AddFileToFolder(basePath, CURL::Encode(import.GetPath())));
  url.SetOption("mediatype", import.GetMediaType());
  std::string path = url.Get();
  std::string label = MediaTypes::GetCapitalPluralLocalization(import.GetMediaType());
  if (!bySource)
    label = StringUtils::Format(g_localizeStrings.Get(39015).c_str(), source.GetFriendlyName().c_str(), label.c_str());

  CFileItemPtr item(new CFileItem(path, false));
  item->SetLabel(label);
  item->m_dateTime = import.GetLastSynced();

  if (!source.GetIconUrl().empty())
    item->SetArt("thumb", source.GetIconUrl());

  item->SetProperty(PROPERTY_IMPORT_PATH, import.GetPath());
  item->SetProperty(PROPERTY_IMPORT_MEDIATYPE, import.GetMediaType());
  item->SetProperty(PROPERTY_IMPORT_MEDIATYPE_LABEL, MediaTypes::GetCapitalPluralLocalization(import.GetMediaType()));
  item->SetProperty(PROPERTY_IMPORT_NAME, StringUtils::Format(g_localizeStrings.Get(39015).c_str(),
    source.GetFriendlyName().c_str(),
    MediaTypes::GetCapitalPluralLocalization(import.GetMediaType()).c_str()));
  item->SetProperty(PROPERTY_SOURCE_IDENTIFIER, source.GetIdentifier());
  item->SetProperty(PROPERTY_SOURCE_NAME, source.GetFriendlyName());
  item->SetProperty(PROPERTY_SOURCE_ISACTIVE, import.IsActive());
  item->SetProperty(PROPERTY_SOURCE_ISACTIVE_LABEL, import.IsActive() ? g_localizeStrings.Get(39026) : g_localizeStrings.Get(39027));

  return item;
}
