/*
 *      Copyright (C) 2015 Team Kodi
 *      http://kodi.tv
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
 *  along with Kodi; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "DVDInputStreamMpegDash.h"
#include "utils/XBMCTinyXML.h"
#include "settings/Settings.h"
#include "utils/URIUtils.h"

CDVDInputStreamMpegDash::CDVDInputStreamMpegDash()
  : CDVDInputStream(DVDSTREAM_TYPE_DASH)
{
}

CDVDInputStreamMpegDash::~CDVDInputStreamMpegDash()
{
  Close();
}

bool CDVDInputStreamMpegDash::IsEOF()
{
  return false;
}

bool CDVDInputStreamMpegDash::Open(const char* strFile, const std::string& content, bool contentLookup)
{
  if (!CDVDInputStream::Open(strFile, content, contentLookup))
    return false;

  CXBMCTinyXML doc;
  doc.LoadFile(strFile);

  if (!doc.RootElement() || strcmp(doc.RootElement()->Value(), "MPD"))
    return false;

  const TiXmlElement* child = doc.RootElement()->FirstChildElement("BaseURL");
  if (child && child->FirstChild())
    m_mpd.base = child->FirstChild()->Value();

  child = doc.RootElement()->FirstChildElement("Period");
  if (!child)
    return false;

  child = child->FirstChildElement("AdaptationSet");
  if (!child)
    return false;

  child = child->FirstChildElement("Representation");
  while (child)
  {
    m_mpd.sets.resize(m_mpd.sets.size()+1);
    if (child->Attribute("bandwidth"))
      m_mpd.sets.back().bandwidth = strtol(child->Attribute("bandwidth"), nullptr, 10);
    const TiXmlElement* slist = child->FirstChildElement("SegmentList");
    if (slist)
    {
      if (slist->Attribute("duration"))
        m_mpd.sets.back().duration = strtol(slist->Attribute("duration"), nullptr, 10);
      const TiXmlElement* init = slist->FirstChildElement("Initialization");
      if (init && init->Attribute("sourceURL"))
        m_mpd.sets.back().segments.push_back(init->Attribute("sourceURL"));
      const TiXmlElement* seg = slist->FirstChildElement("SegmentURL");
      while (seg)
      {
        if (seg->Attribute("media"))
          m_mpd.sets.back().segments.push_back(seg->Attribute("media"));
        seg = seg->NextSiblingElement();
      }
    }
    child = child->NextSiblingElement();
  }

  if (m_mpd.sets.empty())
    return false;

  std::sort(m_mpd.sets.begin(), m_mpd.sets.end(),
            [](const MPD::Representation& a, const MPD::Representation& b)
            {
              return a.bandwidth < b.bandwidth;
            });

  std::cout << "Found MPD with base path: " << m_mpd.base << std::endl;
  for (auto& it : m_mpd.sets)
    std::cout << "\t Representation with bw " << it.bandwidth
               << ", segment duration " << it.duration
               << ", " << it.segments.size() << " segments" << std::endl;

  int bw = CSettings::GetInstance().GetInt("network.bandwidth")*1000/8;
  if (bw == 0)
    bw = m_mpd.sets.back().bandwidth+1;

  m_crepId = m_cSegId = 0;
  while (m_mpd.sets[m_crepId].bandwidth < bw && m_crepId+1 < m_mpd.sets.size())
    ++m_crepId;

  std::cout << "Using representation " << m_crepId << ", bandwidth "
            << m_mpd.sets[m_crepId].bandwidth << std::endl;

  return m_file.Open(URIUtils::AddFileToFolder(m_mpd.base,
                     m_mpd.sets[m_crepId].segments[m_cSegId]));
}

// close file and reset everything
void CDVDInputStreamMpegDash::Close()
{
  m_file.Close();
  m_mpd.base.clear();
  m_mpd.sets.clear();
}

int CDVDInputStreamMpegDash::Read(uint8_t* buf, int buf_size)
{
  if (buf_size == 0)
    return 0;

  ssize_t read = m_file.Read(buf, buf_size);

  if (read < 0)
    return read;

  if (read == 0) // assume eof - open next segment
  {
    ++m_cSegId;
    if (m_cSegId >= m_mpd.sets[m_crepId].segments.size())
      return 0;
    m_file.Close();
    m_file.Open(URIUtils::AddFileToFolder(m_mpd.base,
                m_mpd.sets[m_crepId].segments[m_cSegId]));
    read = m_file.Read(buf, buf_size);
  }

  return read;
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
  return true;
}
