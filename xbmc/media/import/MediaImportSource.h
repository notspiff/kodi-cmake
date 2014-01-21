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

/*!
 * \brief TODO
 */
class CMediaImportSource
{
public:
  explicit CMediaImportSource(const std::string &identifier, const std::string &friendlyName = "",
                              const std::string &iconUrl = "",
                              const std::set<MediaType>& availableMediaTypes = std::set<MediaType>(),
                              const CDateTime &lastSynced = CDateTime())
    : m_identifier(identifier),
      m_friendlyName(friendlyName),
      m_iconUrl(iconUrl),
      m_availableMediaTypes(availableMediaTypes),
      m_lastSynced(lastSynced),
      m_active(false)
  { }

  bool operator==(const CMediaImportSource &other) const
  {
    if (m_identifier.compare(other.m_identifier) != 0 ||
        m_friendlyName.compare(other.m_friendlyName) != 0 ||
        m_iconUrl.compare(other.m_iconUrl) != 0 ||
        m_availableMediaTypes != other.m_availableMediaTypes ||
        m_lastSynced != other.m_lastSynced)
      return false;

    return true;
  }
  bool operator!=(const CMediaImportSource &other) const { return !(*this == other); }

  const std::string& GetIdentifier() const { return m_identifier; }
  const std::string& GetFriendlyName() const { return m_friendlyName; }
  void SetFriendlyName(const std::string &friendlyName) { m_friendlyName = friendlyName; }
  const std::string& GetIconUrl() const { return m_iconUrl; }
  void SetIconUrl(const std::string &iconUrl) { m_iconUrl = iconUrl; }
  const std::set<MediaType>& GetAvailableMediaTypes() const { return m_availableMediaTypes; }
  void SetAvailableMediaTypes(const std::set<MediaType> &mediaTypes) { m_availableMediaTypes = mediaTypes; }
  const CDateTime& GetLastSynced() const { return m_lastSynced; }
  void SetLastSynced(const CDateTime &lastSynced) { m_lastSynced = lastSynced; }
  bool IsActive() const { return m_active; }
  void SetActive(bool active) { m_active = active; }

private:
  std::string m_identifier;
  std::string m_friendlyName;
  std::string m_iconUrl;
  std::set<MediaType> m_availableMediaTypes;
  CDateTime m_lastSynced;
  bool m_active;
};
