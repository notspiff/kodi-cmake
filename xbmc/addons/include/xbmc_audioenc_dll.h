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

#ifndef __XBMC_AUDIOENC_H__
#define __XBMC_AUDIOENC_H__

#include <stdint.h>
#include "xbmc_addon_dll.h"
#include "xbmc_audioenc_types.h"

extern "C"
{
  //! \copydoc AudioEncoder::Init
  bool Init(int iInChannels, int iInRate, int iInBits,
            const char* title, const char* artist,
            const char* albumartist, const char* album,
            const char* year, const char* track,
            const char* genre, const char* comment, int iTrackLength);

  //! \copydoc AudioEncoder::Encode
  int Encode(int nNumBytesRead, uint8_t* pbtStream, uint8_t* buffer);

  //! \copydoc AudioEncoder::Flush
  int Flush(uint8_t* buffer);

  //! \copydoc AudioEncoder::Close
  bool Close(const char* File);

  // function to export the above structure to XBMC
  void __declspec(dllexport) get_addon(struct AudioEncoder* pScr)
  {
    pScr->Init = Init;
    pScr->Encode = Encode;
    pScr->Flush = Flush;
    pScr->Close = Close;
  };
};

#endif
