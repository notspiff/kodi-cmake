#pragma once
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

#include <string>
#include <vector>

#include "FileItem.h"
#include "filesystem/IDirectory.h"
#include "media/MediaType.h"

class CMediaImport;
class CMediaImportSource;
class IMediaImportRepository;

namespace XFILE
{
  class CMediaImportDirectory : public IDirectory
  {
  public:
    CMediaImportDirectory();
    virtual ~CMediaImportDirectory();

    virtual bool GetDirectory(const CURL& url, CFileItemList &items);
    virtual bool Create(const CURL& url) { return true; }
    virtual bool Exists(const CURL& url) { return true; }
    virtual bool IsAllowed(const CURL& url) const { return true; };

    static CFileItemPtr FileItemFromMediaImport(const CMediaImport &import, const std::string &basePath, bool bySource = false);

  private:
    static void HandleSources(const std::string &strPath, const std::vector<CMediaImportSource> &sources, CFileItemList &items, bool asFolder = false);
    static CFileItemPtr FileItemFromMediaImportSource(const CMediaImportSource &source, const std::string &basePath, bool asFolder = false);

    static void HandleImports(const std::string &strPath, const std::vector<CMediaImport> &imports, CFileItemList &items, bool bySource = false);
  };
}
