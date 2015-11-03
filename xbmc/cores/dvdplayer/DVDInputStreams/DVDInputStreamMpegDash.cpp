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

#if (defined HAVE_CONFIG_H) && (!defined TARGET_WINDOWS)
  #include "config.h"
#endif

#include "settings/AdvancedSettings.h"
#include "DVDInputStreamMpegDash.h"
#include "threads/SingleLock.h"
#include "utils/log.h"
#include "utils/Variant.h"

#include <string>

using namespace XFILE;

CDVDInputStreamMpegDash::CDVDInputStreamMpegDash()
  : CDVDInputStream(DVDSTREAM_TYPE_DASH)
  , m_canSeek(true)
  , m_canPause(true)
{
  m_eof = true;
  m_bPaused = false;
}

CDVDInputStreamMpegDash::~CDVDInputStreamMpegDash()
{
  Close();
  m_bPaused = false;
}

bool CDVDInputStreamMpegDash::IsEOF()
{
  return m_eof;
}

bool CDVDInputStreamMpegDash::Open(const char* strFile, const std::string& content, bool contentLookup)
{
  if (!CDVDInputStream::Open(strFile, content, contentLookup))
    return false;

  m_eof = false;

  return true;
}

// close file and reset everything
void CDVDInputStreamMpegDash::Close()
{
  m_eof = true;
  m_bPaused = false;
}

int CDVDInputStreamMpegDash::Read(uint8_t* buf, int buf_size)
{
  return 0;
}

int64_t CDVDInputStreamMpegDash::Seek(int64_t offset, int whence)
{
  if (whence == SEEK_POSSIBLE)
    return 0;
  else
    return -1;
}

bool CDVDInputStreamMpegDash::SeekTime(int iTimeInMsec)
{
  return false;
}

int64_t CDVDInputStreamMpegDash::GetLength()
{
  return -1;
}

bool CDVDInputStreamMpegDash::Pause(double dTime)
{
  m_bPaused = !m_bPaused;

  return true;
}
