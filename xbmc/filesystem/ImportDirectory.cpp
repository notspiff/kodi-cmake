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

#include "ImportDirectory.h"
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

CImportDirectory::CImportDirectory(void)
{ }

CImportDirectory::~CImportDirectory(void)
{ }

bool CImportDirectory::GetDirectory(const CStdString& strPath, CFileItemList &items)
{
  CURL url(strPath);
  if (url.GetHostName() == "imports")
  {
    std::string filename = url.GetFileName();
    URIUtils::RemoveSlashAtEnd(filename);
    if (filename.empty())
    {
      // All
      {
        CFileItemPtr item(new CFileItem(URIUtils::AddFileToFolder(strPath, "all"), true));
        item->SetLabel("All"); // TODO: localization
        item->SetSpecialSort(SortSpecialOnTop);
        items.Add(item);
      }

      // by media provider
      {
        CFileItemPtr item(new CFileItem(URIUtils::AddFileToFolder(strPath, "sources"), true));
        item->SetLabel("By media providers"); // TODO: localization
        item->SetSpecialSort(SortSpecialOnTop);
        items.Add(item);
      }

      // by media provider
      {
        CFileItemPtr item(new CFileItem(URIUtils::AddFileToFolder(strPath, "mediatypes"), true));
        item->SetLabel("By media types"); // TODO: localization
        item->SetSpecialSort(SortSpecialOnTop);
        items.Add(item);
      }

      items.SetLabel("Media imports"); // TODO: localization
      return true;
    }

    if (filename == "all" || StringUtils::StartsWith(filename, "sources") || StringUtils::StartsWith(filename, "mediatypes"))
    {
      if (filename == "all" ||
          StringUtils::StartsWith(filename, "sources/") ||
          StringUtils::StartsWith(filename, "mediatypes/"))
      {
        std::vector<CMediaImport> imports;
        if (filename == "all")
        {
          items.SetLabel("All imports"); // TODO: localization
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
      if (filename == "sources")
      {
        items.SetLabel("By media providers"); // TODO: localization
        HandleSources(strPath, CMediaImportManager::Get().GetSources(), items, true);
        return true;
      }
      if (filename == "mediatypes")
      {
        items.SetLabel("By media types");
        std::vector<MediaType> mediaTypes = CMediaImportManager::Get().GetSupportedMediaTypes();
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
      {
        CFileItemPtr item(new CFileItem(URIUtils::AddFileToFolder(strPath, "all"), true));
        item->SetLabel("All"); // TODO: localization
        item->SetSpecialSort(SortSpecialOnTop);
        items.Add(item);
      }

      // Active
      {
        CFileItemPtr item(new CFileItem(URIUtils::AddFileToFolder(strPath, "active"), true));
        item->SetLabel("Active"); // TODO: localization
        items.Add(item);
      }

      // Inactive
      {
        CFileItemPtr item(new CFileItem(URIUtils::AddFileToFolder(strPath, "inactive"), true));
        item->SetLabel("Inactive"); // TODO: localization
        items.Add(item);
      }

      items.SetLabel("Media providers"); // TODO: localization
      return true;
    }

    if (filename == "all" || filename == "active" || filename == "inactive")
    {
      std::vector<CMediaImportSource> sources;
      if (filename == "all")
      {
        items.SetLabel("All media providers"); // TODO: localization
        sources = CMediaImportManager::Get().GetSources();
      }
      else
      {
        if (filename == "active")
          items.SetLabel("Active media providers"); // TODO: localization
        else
          items.SetLabel("Inactive media providers"); // TODO: localization

        sources = CMediaImportManager::Get().GetSources(filename == "active");
      }

      HandleSources(strPath, sources, items);
      return true;
    }
  }

  /* TODO
  // list all items for a media type of an import
  string dbPath = library + "db://";
  if (mediaType == "season")
    dbPath += "tvshows/titles/-1";
  else if (mediaType == "episode")
    dbPath += "tvshows/titles/-1/-1";
  else
    dbPath += MediaTypes::ToPlural(mediaType) + "/titles";

  if (dbUrl->FromString(dbPath))
  {
    dbUrl->AddOption("source", strSource);
    dbUrl->AddOption("import", strImport);

    result = CDirectory::GetDirectory(dbUrl->ToString(), items);
  }
  */

  return false;
}

void CImportDirectory::HandleSources(const std::string &strPath, const std::vector<CMediaImportSource> &sources, CFileItemList &items, bool asFolder /* = false */)
{
  for (std::vector<CMediaImportSource>::const_iterator itSource = sources.begin(); itSource != sources.end(); ++itSource)
  {
    CFileItemPtr item = FileItemFromMediaImportSource(*itSource, strPath, asFolder);
    if (item != NULL)
      items.Add(item);
  }

  items.SetContent("sources");
}

CFileItemPtr CImportDirectory::FileItemFromMediaImportSource(const CMediaImportSource &source, const std::string &basePath, bool asFolder /* = false */)
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
  item->SetProperty(PROPERTY_SOURCE_ISACTIVE_LABEL, source.IsActive() ? "Active" : "Inactive"); // TODO: localization

  return item;
}

void CImportDirectory::HandleImports(const std::string &strPath, const std::vector<CMediaImport> &imports, CFileItemList &items, bool bySource /* = false */)
{
  for (std::vector<CMediaImport>::const_iterator itImport = imports.begin(); itImport != imports.end(); ++itImport)
  {
    CFileItemPtr item = FileItemFromMediaImport(*itImport, strPath, bySource);
    if (item != NULL)
      items.Add(item);
  }

  items.SetContent("imports");
}

CFileItemPtr CImportDirectory::FileItemFromMediaImport(const CMediaImport &import, const std::string &basePath, bool bySource /* = false */)
{
  if (import.GetPath().empty() || import.GetMediaType().empty())
    return CFileItemPtr();

  const CMediaImportSource &source = import.GetSource();

  // TODO: prepare the path
  CURL url(URIUtils::AddFileToFolder(basePath, CURL::Encode(import.GetPath())));
  url.SetOption("mediatype", import.GetMediaType());
  std::string path = url.Get();
  std::string label;
  if (!bySource)
    label = source.GetFriendlyName() + ": "; // TODO: localization
  label += MediaTypes::GetCapitalPluralLocalization(import.GetMediaType());

  CFileItemPtr item(new CFileItem(path, false));
  item->SetLabel(label);
  item->m_dateTime = import.GetLastSynced();

  if (!source.GetIconUrl().empty())
    item->SetArt("thumb", source.GetIconUrl());

  item->SetProperty(PROPERTY_IMPORT_PATH, import.GetPath());
  item->SetProperty(PROPERTY_IMPORT_MEDIATYPE, import.GetMediaType());
  item->SetProperty(PROPERTY_IMPORT_MEDIATYPE_LABEL, MediaTypes::GetCapitalPluralLocalization(import.GetMediaType()));
  item->SetProperty(PROPERTY_IMPORT_NAME, import.GetSource().GetFriendlyName() + ": " + MediaTypes::GetCapitalPluralLocalization(import.GetMediaType())); // TODO: localization
  item->SetProperty(PROPERTY_SOURCE_IDENTIFIER, source.GetIdentifier());
  item->SetProperty(PROPERTY_SOURCE_NAME, source.GetFriendlyName());
  item->SetProperty(PROPERTY_SOURCE_ISACTIVE, import.IsActive());
  item->SetProperty(PROPERTY_SOURCE_ISACTIVE_LABEL, import.IsActive() ? "Active" : "Inactive"); // TODO: localization

  return item;
}
