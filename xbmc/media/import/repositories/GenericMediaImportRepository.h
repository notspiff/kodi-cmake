#pragma once
/*
 *      Copyright (C) 2014 Team XBMC
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

#include <map>
#include <set>

#include "media/import/IMediaImportRepository.h"
#include "threads/CriticalSection.h"

class CGenericMediaImportRepository : public IMediaImportRepository
{
public:
  CGenericMediaImportRepository()
    : m_loaded(false)
  { }
  virtual ~CGenericMediaImportRepository();

  virtual bool Initialize();

  virtual std::vector<CMediaImport> GetImports() const;
  virtual std::vector<CMediaImport> GetImportsBySource(const std::string &sourceIdentifier) const;
  virtual std::vector<CMediaImport> GetImportsByMediaType(const MediaType &mediaType) const;
  virtual bool GetImport(const std::string &path, const MediaType &mediaType, CMediaImport &import) const;

  virtual bool AddImport(const CMediaImport &import);
  virtual bool UpdateImport(const CMediaImport &import);
  virtual bool RemoveImport(const CMediaImport &import);
  
  virtual bool UpdateLastSync(CMediaImport &import);

  virtual std::vector<CMediaImportSource> GetSources(const MediaType &mediaType = MediaTypeNone) const;
  virtual bool GetSource(const std::string &identifier, CMediaImportSource &source) const;

  virtual bool AddSource(const CMediaImportSource &source);
  virtual bool UpdateSource(const CMediaImportSource &source);
  virtual bool RemoveSource(const std::string &identifier);

protected:
  virtual std::set<MediaType> getSupportedMediaTypes() const = 0;

  virtual bool openRepository() = 0;
  virtual void closeRepository() = 0;

  virtual std::vector<CMediaImportSource> getSources() = 0;
  virtual int addSource(const CMediaImportSource &source) = 0;
  virtual bool updateSource(const CMediaImportSource &source) = 0;
  virtual void removeSource(const CMediaImportSource &source) = 0;
  virtual std::vector<CMediaImport> getImports() = 0;
  virtual int addImport(const CMediaImport &import) = 0;
  virtual bool updateImport(const CMediaImport &import) = 0;
  virtual void removeImport(const CMediaImport &import) = 0;
  virtual void updateLastSync(const CMediaImport &import, const CDateTime &lastSync) = 0;

  typedef std::pair<std::string, MediaType> MediaImportIdentifier;
  static MediaImportIdentifier GetMediaImportIdentifier(const CMediaImport &import);

  bool m_loaded;

  typedef std::map<MediaImportIdentifier, CMediaImport> MediaImportMap;
  CCriticalSection m_importsLock;
  MediaImportMap m_imports;

  typedef std::map<std::string, CMediaImportSource> MediaImportSourceMap;
  CCriticalSection m_sourcesLock;
  MediaImportSourceMap m_sources;
};
