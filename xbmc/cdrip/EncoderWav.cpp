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

#include "EncoderWav.h"
#include "filesystem/File.h"
#include "utils/log.h"

CEncoderWav::CEncoderWav()
{
  m_iBytesWritten = 0;
  first = true;
}

bool CEncoderWav::Init()
{
  m_iBytesWritten = 0;

  // we only accept 2 / 44100 / 16 atm
  if (m_iInChannels != 2 ||
      m_iInSampleRate != 44100 ||
      m_iInBitsPerSample != 16)
    return false;

  // write dummy header file
  first = true;

  return true;
}

int CEncoderWav::Encode(int nNumBytesRead, uint8_t* pbtStream, uint8_t* buffer)
{
  int extra=0;
  if (first)
  {
    memset(buffer, 0, sizeof(WAVHDR));
    buffer += sizeof(WAVHDR);
    first = false;
    extra = sizeof(WAVHDR);
  }

  memcpy(buffer, pbtStream, nNumBytesRead);
  m_iBytesWritten += nNumBytesRead;

  return nNumBytesRead+extra;
}

int CEncoderWav::Flush(uint8_t* buffer)
{
  return 0;
}

bool CEncoderWav::Close()
{
  WAVHDR wav;
  int bps = 1;

  XFILE::CFile file;
  if (!file.OpenForWrite(m_strFile))
    return false;

  memcpy(wav.riff, "RIFF", 4);
  wav.len = m_iBytesWritten + 44 - 8;
  memcpy(wav.cWavFmt, "WAVEfmt ", 8);
  wav.dwHdrLen = 16;
  wav.wFormat = WAVE_FORMAT_PCM;
  wav.wNumChannels = m_iInChannels;
  wav.dwSampleRate = m_iInSampleRate;
  wav.wBitsPerSample = m_iInBitsPerSample;
  if (wav.wBitsPerSample == 16) bps = 2;
  wav.dwBytesPerSec = m_iInBitsPerSample * m_iInChannels * bps;
  wav.wBlockAlign = 4;
  memcpy(wav.cData, "data", 4);
  wav.dwDataLen = m_iBytesWritten;

  // write header to beginning of stream
  file.Seek(0, FILE_BEGIN);
  file.Write(&wav, sizeof(wav));

  return true;
}
