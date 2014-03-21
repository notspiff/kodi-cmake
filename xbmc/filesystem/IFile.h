/*
 *      Copyright (c) 2002 Frodo
 *      Portions Copyright (c) by the authors of ffmpeg and xvid
 *      Copyright (C) 2002-2013 Team XBMC
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

// IFile.h: interface for the IFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IFILE_H__7EE73AC7_36BC_4822_93FF_44F3B0C766F6__INCLUDED_)
#define AFX_IFILE_H__7EE73AC7_36BC_4822_93FF_44F3B0C766F6__INCLUDED_

#pragma once

#ifdef TARGET_POSIX
#include "PlatformDefs.h" // for __stat64
#endif

#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string>

#include "IFileTypes.h"

class CURL;

namespace XFILE
{

class IFile
{
public:
  IFile();
  virtual ~IFile();

  virtual bool Open(const CURL& url) = 0;
  virtual bool OpenForWrite(const CURL& url, bool bOverWrite = false) { return false; };
  virtual bool Exists(const CURL& url) = 0;
  virtual int Stat(const CURL& url, struct __stat64* buffer) = 0;
  virtual int Stat(struct __stat64* buffer);
  virtual unsigned int Read(void* lpBuf, int64_t uiBufSize) = 0;
  virtual int Write(const void* lpBuf, int64_t uiBufSize) { return -1;};
  virtual bool ReadString(char *szLine, int iLineLength);
  virtual int64_t Seek(int64_t iFilePosition, int iWhence = SEEK_SET) = 0;
  virtual void Close() = 0;
  virtual int64_t GetPosition() = 0;
  virtual int64_t GetLength() = 0;
  virtual void Flush() { }
  virtual int Truncate(int64_t size) { return -1;};

  /* Returns the minium size that can be read from input stream.   *
   * For example cdrom access where access could be sector based.  *
   * This will cause file system to buffer read requests, to       *
   * to meet the requirement of CFile.                             *
   * It can also be used to indicate a file system is non buffered *
   * but accepts any read size, have it return the value 1         */
  virtual int  GetChunkSize() {return 0;}

  virtual bool SkipNext(){return false;}

  virtual bool Delete(const CURL& url) { return false; }
  virtual bool Rename(const CURL& url, const CURL& urlnew) { return false; }
  virtual bool SetHidden(const CURL& url, bool hidden) { return false; }

  virtual int IoControl(EIoControl request, void* param) { return -1; }

  virtual std::string GetContent()                            { return "application/octet-stream"; }
  virtual std::string GetContentCharset(void)                { return ""; }
};

class CRedirectException
{
public:
  IFile *m_pNewFileImp;
  CURL  *m_pNewUrl;

  CRedirectException();
  
  CRedirectException(IFile *pNewFileImp, CURL *pNewUrl=NULL);
};

}

#endif // !defined(AFX_IFILE_H__7EE73AC7_36BC_4822_93FF_44F3B0C766F6__INCLUDED_)
