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

#include "DVDInputStreamTV.h"
#include "filesystem/MythFile.h"
#include "pvr/channels/PVRChannel.h"
#include "URL.h"
#include "addons/VFSEntry.h"
#include "addons/AddonManager.h"
#include "utils/StringUtils.h"

using namespace ADDON;
using namespace XFILE;

CDVDInputStreamTV::CDVDInputStreamTV() : CDVDInputStream(DVDSTREAM_TYPE_TV)
{
  m_pFile = NULL;
  m_pRecordable = NULL;
  m_pLiveTV = NULL;
  m_eof = true;
}

CDVDInputStreamTV::~CDVDInputStreamTV()
{
  Close();
}

bool CDVDInputStreamTV::IsEOF()
{
  return !m_pFile || m_eof;
}

bool CDVDInputStreamTV::Open(const char* strFile, const std::string& content)
{
  if (!CDVDInputStream::Open(strFile, content)) return false;

  CURL url(strFile);
  std::string strProtocol = url.GetProtocol();
  StringUtils::ToLower(strProtocol);
  if (!strProtocol.empty())
  {
    VECADDONS addons;
    CAddonMgr::Get().GetAddons(ADDON_VFS, addons);
    for (size_t i=0;i<addons.size();++i)
    {
      VFSEntryPtr vfs(boost::static_pointer_cast<CVFSEntry>(addons[i]));
      if (vfs->HasLiveTV() && vfs->GetProtocols().find(strProtocol) != std::string::npos)
      {
        CVFSEntryILiveTVWrapper* wrap = new CVFSEntryILiveTVWrapper(CVFSEntryManager::Get().GetAddon(vfs->ID()));
        m_pFile = wrap;
        m_pLiveTV = wrap;
        m_pRecordable = NULL;
      }
    }
  }

  if (!m_pFile)
  {
    m_pFile       = new CMythFile();
    m_pLiveTV     = ((CMythFile*)m_pFile)->GetLiveTV();
    m_pRecordable = ((CMythFile*)m_pFile)->GetRecordable();
  }

  // open file in binary mode
  if (!m_pFile->Open(url))
  {
    delete m_pFile;
    m_pFile = NULL;
    return false;
  }
  m_eof = false;
  return true;
}

// close file and reset everyting
void CDVDInputStreamTV::Close()
{
  if (m_pFile)
  {
    m_pFile->Close();
    delete m_pFile;
  }

  CDVDInputStream::Close();
  m_pFile = NULL;
  m_pLiveTV = NULL;
  m_eof = true;
}

int CDVDInputStreamTV::Read(uint8_t* buf, int buf_size)
{
  if(!m_pFile) return -1;

  ssize_t ret = m_pFile->Read(buf, buf_size);

  if (ret < 0)
    return -1; // player will retry read in case of error until playback is stopped

  /* we currently don't support non completing reads */
  if (ret == 0)
    m_eof = true;

  return (int)ret;
}

int64_t CDVDInputStreamTV::Seek(int64_t offset, int whence)
{
  if(!m_pFile) return -1;
  int64_t ret = m_pFile->Seek(offset, whence);

  /* if we succeed, we are not eof anymore */
  if( ret >= 0 ) m_eof = false;

  return ret;
}

int64_t CDVDInputStreamTV::GetLength()
{
  if (!m_pFile) return 0;
  return m_pFile->GetLength();
}


int CDVDInputStreamTV::GetTotalTime()
{
  if(!m_pLiveTV) return -1;
  return m_pLiveTV->GetTotalTime();
}

int CDVDInputStreamTV::GetTime()
{
  if(!m_pLiveTV) return -1;
  return m_pLiveTV->GetStartTime();
}

bool CDVDInputStreamTV::NextChannel(bool preview/* = false*/)
{
  if(!m_pLiveTV) return false;
  return m_pLiveTV->NextChannel();
}

bool CDVDInputStreamTV::PrevChannel(bool preview/* = false*/)
{
  if(!m_pLiveTV) return false;
  return m_pLiveTV->PrevChannel();
}

bool CDVDInputStreamTV::SelectChannelByNumber(unsigned int channel)
{
  if(!m_pLiveTV) return false;
  return m_pLiveTV->SelectChannel(channel);
}

bool CDVDInputStreamTV::UpdateItem(CFileItem& item)
{
  if(m_pLiveTV)
    return m_pLiveTV->UpdateItem(item);
  return false;
}

bool CDVDInputStreamTV::SeekTime(int iTimeInMsec)
{
  return false;
}

CDVDInputStream::ENextStream CDVDInputStreamTV::NextStream()
{
  if(!m_pFile) return NEXTSTREAM_NONE;
  if(m_pFile->SkipNext())
  {
    m_eof = false;
    return NEXTSTREAM_OPEN;
  }
  return NEXTSTREAM_NONE;
}

bool CDVDInputStreamTV::CanRecord()
{
  if(m_pRecordable)
    return m_pRecordable->CanRecord();
  return false;
}
bool CDVDInputStreamTV::IsRecording()
{
  if(m_pRecordable)
    return m_pRecordable->IsRecording();
  return false;
}
bool CDVDInputStreamTV::Record(bool bOnOff)
{
  if(m_pRecordable)
    return m_pRecordable->Record(bOnOff);
  return false;
}

int CDVDInputStreamTV::GetBlockSize()
{
  if(m_pFile)
    return m_pFile->GetChunkSize();
  else
    return 0;
}

