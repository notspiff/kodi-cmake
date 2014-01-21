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

#include "MediaImport.h"
#include "utils/StringUtils.h"
#include "utils/XMLUtils.h"

std::string CMediaImportSettings::Serialize() const
{
  TiXmlElement root("settings");

  switch (m_importTrigger)
  {
    case MediaImportTriggerManual:
      XMLUtils::SetString(&root, "importTrigger", "manual");
      break;

    default:
    case MediaImportTriggerAuto:
      XMLUtils::SetString(&root, "importTrigger", "auto");
      break;
  }

  XMLUtils::SetBoolean(&root, "updateImportedMediaItems", m_updateImportedMediaItems);
  XMLUtils::SetBoolean(&root, "updatePlaybackMetadataFromSource", m_updatePlaybackMetadataFromSource);
  XMLUtils::SetBoolean(&root, "updatePlaybackMetadataOnSource", m_updatePlaybackMetadataOnSource);

  std::stringstream stream;
  stream << root;
  return stream.str();
}

bool CMediaImportSettings::Deserialize(const std::string &xmlData)
{
  CXBMCTinyXML doc;
  if (!doc.Parse(xmlData))
    return false;
  const TiXmlElement *root = doc.RootElement();

  std::string mediaImportTrigger;
  if (XMLUtils::GetString(root, "importTrigger", mediaImportTrigger))
  {
    if (StringUtils::EqualsNoCase(mediaImportTrigger, "manual"))
      m_importTrigger = MediaImportTriggerManual;
    else
      m_importTrigger = MediaImportTriggerAuto;
  }

  XMLUtils::GetBoolean(root, "updateImportedMediaItems", m_updateImportedMediaItems);
  XMLUtils::GetBoolean(root, "updatePlaybackMetadataFromSource", m_updatePlaybackMetadataFromSource);
  XMLUtils::GetBoolean(root, "updatePlaybackMetadataOnSource", m_updatePlaybackMetadataOnSource);

  return true;
}
