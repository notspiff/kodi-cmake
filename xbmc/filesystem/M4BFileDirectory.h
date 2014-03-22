#pragma once
/*
 *      Copyright (C) 2005-2013 Team XBMC
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


#include "IFileDirectory.h"
#include "DllAvFormat.h"
#include "DllAvUtil.h"

namespace XFILE
{
  class CM4BFileDirectory : public IFileDirectory
  {
    public:
      CM4BFileDirectory(void);
      virtual ~CM4BFileDirectory(void);
      virtual bool GetDirectory(const CStdString& strPath, CFileItemList &items);
      virtual bool Exists(const char* strPath);
      virtual bool ContainsFiles(const CStdString& strPath);
      virtual bool IsAllowed(const CStdString &strFile) const { return true; };
    protected:
      AVIOContext* m_ioctx;
      AVFormatContext* m_fctx;
      DllAvFormat m_av;
      DllAvUtil m_avu;
  };
}
