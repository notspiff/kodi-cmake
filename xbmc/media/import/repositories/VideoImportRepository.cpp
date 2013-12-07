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

#include "VideoImportRepository.h"
#include "FileItem.h"
#include "video/VideoInfoTag.h"

CVideoImportRepository::CVideoImportRepository()
{ }

CVideoImportRepository::~CVideoImportRepository()
{ }

bool CVideoImportRepository::GetMediaType(const CFileItem &item, MediaType &mediaType) const
{
  if (!item.HasVideoInfoTag())
    return false;

  mediaType = item.GetVideoInfoTag()->m_type;
  return !mediaType.empty();
}

std::set<MediaType> CVideoImportRepository::getSupportedMediaTypes() const
{
  std::set<MediaType> supportedMediaTypes;
  supportedMediaTypes.insert(MediaTypeMovie);
  supportedMediaTypes.insert(MediaTypeTvShow);
  supportedMediaTypes.insert(MediaTypeSeason);
  supportedMediaTypes.insert(MediaTypeEpisode);
  supportedMediaTypes.insert(MediaTypeMusicVideo);
  return supportedMediaTypes;
}

std::vector<CMediaImportSource> CVideoImportRepository::getSources()
{
  return m_db.GetSources();
}

int CVideoImportRepository::addSource(const CMediaImportSource &source)
{
  return m_db.AddSource(source);
}

bool CVideoImportRepository::updateSource(const CMediaImportSource &source)
{
  return m_db.SetDetailsForSource(source);
}

void CVideoImportRepository::removeSource(const CMediaImportSource &source)
{
  m_db.RemoveSource(source.GetIdentifier());
}

std::vector<CMediaImport> CVideoImportRepository::getImports()
{
  return m_db.GetImports();
}

int CVideoImportRepository::addImport(const CMediaImport &import)
{
  return m_db.AddImport(import);
}

bool CVideoImportRepository::updateImport(const CMediaImport &import)
{
  return m_db.SetDetailsForImport(import);
}

void CVideoImportRepository::removeImport(const CMediaImport &import)
{
  m_db.RemoveImport(import);
}

void CVideoImportRepository::updateLastSync(const CMediaImport &import, const CDateTime &lastSync)
{
  m_db.UpdateImportLastSynced(import, lastSync);
}
