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
#include "dialogs/GUIDialogProgress.h"
#include "guilib/LocalizeStrings.h"
#include "threads/SingleLock.h"
#include "utils/StringUtils.h"

CVideoImportRepository::~CVideoImportRepository()
{
  if (!m_loaded)
    return;

  m_sources.clear();
  m_imports.clear();
}

bool CVideoImportRepository::Initialize()
{
  if (m_loaded)
    return true;

  if (!m_db.Open())
    return false;

  std::vector<CMediaImportSource> sources = m_db.GetSources();
  std::vector<CMediaImport> imports = m_db.GetImports();
  m_db.Close();
  
  CSingleLock sourcesLock(m_sourcesLock);
  for (std::vector<CMediaImportSource>::const_iterator it = sources.begin(); it != sources.end(); ++it)
    m_sources.insert(std::make_pair(it->GetIdentifier(), *it));

  {
    CSingleLock importsLock(m_importsLock);
    for (std::vector<CMediaImport>::iterator it = imports.begin(); it != imports.end(); ++it)
    {
      if (!it->GetSource().GetIdentifier().empty())
      {
        MediaImportSourceMap::const_iterator itSource = m_sources.find(it->GetSource().GetIdentifier());
        if (itSource != m_sources.end())
        {
          it->SetSource(itSource->second);
          m_imports.insert(std::make_pair(std::make_pair(it->GetPath(), it->GetMediaType()), *it));
        }
      }
    }
  }

  m_loaded = true;
  return true;
}

std::vector<CMediaImport> CVideoImportRepository::GetImports() const
{
  std::vector<CMediaImport> imports;

  if (!m_loaded)
    return imports;

  CSingleLock importsLock(m_importsLock);
  for (MediaImportMap::const_iterator it = m_imports.begin(); it != m_imports.end(); ++it)
    imports.push_back(it->second);

  return imports;
}

std::vector<CMediaImport> CVideoImportRepository::GetImportsBySource(const std::string &sourceIdentifier) const
{
  std::vector<CMediaImport> imports;

  if (!m_loaded || sourceIdentifier.empty())
    return imports;

  CSingleLock importsLock(m_importsLock);
  for (MediaImportMap::const_iterator it = m_imports.begin(); it != m_imports.end(); ++it)
  {
    if (it->second.GetSource().GetIdentifier().compare(sourceIdentifier) == 0)
      imports.push_back(it->second);
  }

  return imports;
}

std::vector<CMediaImport> CVideoImportRepository::GetImportsByMediaType(const MediaType &mediaType) const
{
  std::vector<CMediaImport> imports;

  if (!m_loaded || mediaType.empty())
    return imports;

  CSingleLock importsLock(m_importsLock);
  for (MediaImportMap::const_iterator it = m_imports.begin(); it != m_imports.end(); ++it)
  {
    if (MediaTypes::IsMediaType(it->first.second, mediaType))
      imports.push_back(it->second);
  }

  return imports;
}

bool CVideoImportRepository::GetImport(const std::string &path, const MediaType &mediaType, CMediaImport &import) const
{
  if (!m_loaded ||
      path.empty() || mediaType.empty())
    return false;
  
  CSingleLock importsLock(m_importsLock);
  MediaImportMap::const_iterator it = m_imports.find(MediaImportIdentifier(path, mediaType));
  if (it == m_imports.end())
    return false;

  import = it->second;

  return true;
}

bool CVideoImportRepository::AddImport(const CMediaImport &import)
{
  if (!m_loaded ||
      import.GetSource().GetIdentifier().empty() || import.GetPath().empty() ||
      import.GetMediaType().empty())
    return false;

  MediaImportIdentifier importIdentifier = GetMediaImportIdentifier(import);

  CSingleLock importsLock(m_importsLock);
  MediaImportMap::iterator itImport = m_imports.find(importIdentifier);
  if (itImport != m_imports.end() && itImport->second == import)
    return true;

  if (!m_db.Open())
    return false;

  if (itImport == m_imports.end())
  {
    int idImport = m_db.AddImport(import);
    m_db.Close();

    if (idImport < 0)
      return false;

    m_imports.insert(std::make_pair(importIdentifier, import));
  }
  else
  {
    bool ret = m_db.SetDetailsForImport(import);
    m_db.Close();

    if (!ret)
      return false;

    itImport->second = import;
  }

  return true;
}

bool CVideoImportRepository::UpdateImport(const CMediaImport &import)
{
  if (!m_loaded ||
      import.GetSource().GetIdentifier().empty() || import.GetPath().empty() ||
      import.GetMediaType().empty())
    return false;

  CSingleLock importsLock(m_importsLock);
  MediaImportMap::iterator itImport = m_imports.find(GetMediaImportIdentifier(import));
  if (itImport == m_imports.end())
    return false;

  if (!m_db.Open())
    return false;

  bool ret = m_db.SetDetailsForImport(import);
  m_db.Close();

  if (!ret)
    return false;

  itImport->second = import;

  return true;
}

bool CVideoImportRepository::RemoveImport(const CMediaImport &import, CGUIDialogProgress *progress /* = NULL */)
{
  if (!m_loaded ||
      import.GetPath().empty() || import.GetMediaType().empty())
    return false;

  CSingleLock importsLock(m_importsLock);
  MediaImportMap::iterator itImport = m_imports.find(GetMediaImportIdentifier(import));
  if (itImport == m_imports.end())
    return false;

  if (!m_db.Open())
    return false;

  if (progress != NULL)
    progress->SetHeading(StringUtils::Format(g_localizeStrings.Get(37046),
                         MediaTypes::GetPluralLocalization(itImport->first.second).c_str(),
                         itImport->second.GetSource().GetFriendlyName().c_str()));

  m_db.RemoveImport(import, progress);
  m_db.Close();

  m_imports.erase(itImport);

  return true;
}

bool CVideoImportRepository::UpdateLastSync(CMediaImport &import)
{
  if (!m_loaded ||
      import.GetPath().empty() || import.GetMediaType().empty())
    return false;

  CSingleLock importsLock(m_importsLock);
  MediaImportMap::iterator itImport = m_imports.find(GetMediaImportIdentifier(import));
  if (itImport == m_imports.end())
    return false;

  if (!m_db.Open())
    return false;

  CDateTime lastSynced = CDateTime::GetCurrentDateTime();
  m_db.UpdateImportLastSynced(import, lastSynced);
  m_db.Close();

  // update the cached copy of the import
  itImport->second.SetLastSynced(lastSynced);
  importsLock.Leave();

  // update the cached copy of the import's source
  CSingleLock sourcesLock(m_sourcesLock);
  MediaImportSourceMap::iterator itSource = m_sources.find(import.GetSource().GetIdentifier());
  if (itSource != m_sources.end())
    itSource->second.SetLastSynced(lastSynced);

  // update the local copy of the import and the source
  import.SetLastSynced(lastSynced);

  return true;
}

std::vector<CMediaImportSource> CVideoImportRepository::GetSources(const MediaType &mediaType /* = MediaTypeNone */) const
{
  std::vector<CMediaImportSource> sources;

  if (!m_loaded)
    return sources;

  CSingleLock sourcesLock(m_sourcesLock);
  for (MediaImportSourceMap::const_iterator it = m_sources.begin(); it != m_sources.end(); ++it)
  {
    if (mediaType.empty() ||
        it->second.GetAvailableMediaTypes().find(mediaType) != it->second.GetAvailableMediaTypes().end())
      sources.push_back(it->second);
  }

  return sources;
}

bool CVideoImportRepository::GetSource(const std::string &identifier, CMediaImportSource &source) const
{
  if (!m_loaded ||
      identifier.empty())
    return false;
  
  CSingleLock sourcesLock(m_sourcesLock);
  MediaImportSourceMap::const_iterator it = m_sources.find(identifier);
  if (it == m_sources.end())
    return false;

  source = it->second;

  return true;
}

bool CVideoImportRepository::AddSource(const CMediaImportSource &source)
{
  if (!m_loaded ||
      source.GetIdentifier().empty() || source.GetFriendlyName().empty())
    return false;
  
  CSingleLock sourcesLock(m_sourcesLock);
  MediaImportSourceMap::iterator itSource = m_sources.find(source.GetIdentifier());
  if (itSource != m_sources.end() && itSource->second == source)
    return true;

  if (!m_db.Open())
    return false;

  if (itSource == m_sources.end())
  {
    int idSource = m_db.AddSource(source);
    m_db.Close();

    if (idSource < 0)
      return false;

    m_sources.insert(std::make_pair(source.GetIdentifier(), source));
  }
  else
  {
    bool ret = m_db.SetDetailsForSource(source);
    m_db.Close();

    if (!ret)
      return false;

    itSource->second = source;
  }

  return true;
}

bool CVideoImportRepository::UpdateSource(const CMediaImportSource &source)
{
  if (!m_loaded ||
      source.GetIdentifier().empty() || source.GetFriendlyName().empty())
    return false;
  
  CSingleLock sourcesLock(m_sourcesLock);
  MediaImportSourceMap::iterator itSource = m_sources.find(source.GetIdentifier());
  if (itSource == m_sources.end())
    return false;

  if (itSource->second == source)
    return true;

  if (!m_db.Open())
    return false;

  bool ret = m_db.SetDetailsForSource(source);
  m_db.Close();

  if (!ret)
    return false;

  itSource->second = source;

  return true;
}

bool CVideoImportRepository::RemoveSource(const std::string &identifier, CGUIDialogProgress *progress /* = NULL */)
{
  if (!m_loaded ||
      identifier.empty())
    return false;
  
  CSingleLock sourcesLock(m_sourcesLock);
  MediaImportSourceMap::iterator it = m_sources.find(identifier);
  if (it == m_sources.end())
    return false;

  if (!m_db.Open())
    return false;

  if (progress != NULL)
    progress->SetHeading(StringUtils::Format(g_localizeStrings.Get(37047), it->second.GetFriendlyName().c_str()));

  m_db.RemoveSource(identifier, progress);
  m_db.Close();

  m_sources.erase(it);

  CSingleLock importsLock(m_importsLock);
  for (MediaImportMap::iterator it = m_imports.begin(); it != m_imports.end(); )
  {
    if (it->second.GetSource().GetIdentifier().compare(identifier) == 0)
      m_imports.erase(it++);
    else
      ++it;
  }

  return true;
}

CVideoImportRepository::MediaImportIdentifier CVideoImportRepository::GetMediaImportIdentifier(const CMediaImport &import)
{
  return MediaImportIdentifier(import.GetPath(), import.GetMediaType());
}
