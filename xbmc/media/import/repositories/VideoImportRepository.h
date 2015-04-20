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

#include <map>

#include "media/import/repositories/GenericMediaImportRepository.h"
#include "threads/CriticalSection.h"
#include "video/VideoDatabase.h"

class CVideoImportRepository : public CGenericMediaImportRepository
{
public:
  CVideoImportRepository();
  virtual ~CVideoImportRepository();
  
  virtual bool GetMediaType(const CFileItem &item, MediaType &mediaType) const;

protected:
  virtual std::set<MediaType> getSupportedMediaTypes() const;

  virtual bool openRepository() { return m_db.Open(); }
  virtual void closeRepository() { m_db.Close(); }

  virtual std::vector<CMediaImportSource> getSources();
  virtual int addSource(const CMediaImportSource &source);
  virtual bool updateSource(const CMediaImportSource &source);
  virtual void removeSource(const CMediaImportSource &source);
  virtual std::vector<CMediaImport> getImports();
  virtual int addImport(const CMediaImport &import);
  virtual bool updateImport(const CMediaImport &import);
  virtual void removeImport(const CMediaImport &import);
  virtual void updateLastSync(const CMediaImport &import, const CDateTime &lastSync);

  CVideoDatabase m_db;
};
