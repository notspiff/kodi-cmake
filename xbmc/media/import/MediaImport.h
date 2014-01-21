#pragma once
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

#include <set>
#include <string>

#include "XBDateTime.h"
#include "media/MediaType.h"
#include "media/import/MediaImportSource.h"

class CMediaImportSettings
{
public:
  CMediaImportSettings(bool updateImportedMediaItems = true,
                       bool updatePlaybackMetadataFromSource = true
                       /* TODO: update on source
                       ,
                       bool updatePlaybackMetadataOnSource = true
                       */)
    : m_updateImportedMediaItems(updateImportedMediaItems),
      m_updatePlaybackMetadataFromSource(updatePlaybackMetadataFromSource)
      /* TODO: update on source
      ,
      m_updatePlaybackMetadataOnSource(updatePlaybackMetadataOnSource)
      */
  { }

  std::string Serialize() const;
  bool Deserialize(const std::string &xmlData);

  bool UpdateImportedMediaItems() const { return m_updateImportedMediaItems; }
  void SetUpdateImportedMediaItems(bool updateImportedMediaItems) { m_updateImportedMediaItems = updateImportedMediaItems; }
  bool UpdatePlaybackMetadataFromSource() const { return m_updatePlaybackMetadataFromSource; }
  void SetUpdatePlaybackMetadataFromSource(bool updatePlaybackMetadataFromSource) { m_updatePlaybackMetadataFromSource = updatePlaybackMetadataFromSource; }
  /* TODO: update on source
  bool UpdatePlaybackMetadataOnSource() const { return m_updatePlaybackMetadataOnSource; }
  void SetUpdatePlaybackMetadataOnSource(bool updatePlaybackMetadataOnSource) { m_updatePlaybackMetadataOnSource = updatePlaybackMetadataOnSource; }
  */

private:
  bool m_updateImportedMediaItems;
  bool m_updatePlaybackMetadataFromSource;
  /* TODO: update on source
  bool m_updatePlaybackMetadataOnSource;
  */
};

/*!
 * \brief TODO
 */
class CMediaImport
{
public:
  CMediaImport(const std::string &importPath, const MediaType& importedMediaType,
               const CMediaImportSource &source,
               const CDateTime &lastSynced = CDateTime())
    : m_importPath(importPath),
      m_mediaType(importedMediaType),
      m_source(source),
      m_lastSynced(lastSynced)
  { }
  CMediaImport(const std::string &importPath, const MediaType& importedMediaType,
               const std::string &sourceIdentifier = "",
               const CDateTime &lastSynced = CDateTime())
    : m_importPath(importPath),
      m_mediaType(importedMediaType),
      m_source(sourceIdentifier, "", "", std::set<MediaType>(), lastSynced),
      m_lastSynced(lastSynced)
  {
    std::set<MediaType> mediaTypes; mediaTypes.insert(importedMediaType);
    m_source.SetAvailableMediaTypes(mediaTypes);
  }

  bool operator==(const CMediaImport &other) const
  {
    if (m_importPath.compare(other.m_importPath) != 0 ||
        m_source != other.m_source ||
        m_mediaType != other.m_mediaType ||
        m_lastSynced != other.m_lastSynced)
      return false;

    return true;
  }
  bool operator!=(const CMediaImport &other) const { return !(*this == other); }

  const std::string& GetPath() const { return m_importPath; }

  const CMediaImportSource& GetSource() const { return m_source; }
  void SetSource(const CMediaImportSource &source)
  {
    if (source.GetIdentifier().empty())
      return;

    m_source = source;
  }

  const MediaType& GetMediaType() const { return m_mediaType; }
  void SetMediaTypes(const MediaType &mediaType) { m_mediaType = mediaType; }

  const CDateTime& GetLastSynced() const { return m_lastSynced; }
  void SetLastSynced(const CDateTime &lastSynced)
  {
    m_lastSynced = lastSynced;
    m_source.SetLastSynced(lastSynced);
  }

  bool IsActive() const { return m_source.IsActive(); }
  void SetActive(bool active) { m_source.SetActive(active); }
  
  const CMediaImportSettings& GetSettings() const { return m_settings; }
  CMediaImportSettings& GetSettings() { return m_settings; }

private:
  std::string m_importPath;
  MediaType m_mediaType;
  CMediaImportSource m_source;
  CDateTime m_lastSynced;
  CMediaImportSettings m_settings;
};
