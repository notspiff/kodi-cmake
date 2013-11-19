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

#include "system.h"
#include "CodecFactory.h"
#include "URL.h"
#include "DVDPlayerCodec.h"
#include "PCMCodec.h"
#include "utils/StringUtils.h"
#include "addons/AddonManager.h"
#include "addons/AudioDecoder.h"

using namespace ADDON;

ICodec* CodecFactory::CreateCodec(const CStdString& strFileType)
{
  VECADDONS codecs;
  CAddonMgr::Get().GetAddons(ADDON_AUDIODECODER, codecs);
  for (size_t i=0;i<codecs.size();++i)
  {
    boost::shared_ptr<CAudioDecoder> dec(boost::static_pointer_cast<CAudioDecoder>(codecs[i]));
    if (dec->GetExtensions().find("."+strFileType) != std::string::npos)
    {
      CAudioDecoder* result = new CAudioDecoder(*dec);
      static_cast<AudioDecoderDll&>(*result).Create();
      return result;
    }
  }

  if (strFileType.Equals("pcm") || strFileType.Equals("l16"))
    return new PCMCodec();

  return new DVDPlayerCodec();
}

ICodec* CodecFactory::CreateCodecDemux(const CStdString& strFile, const CStdString& strContent, unsigned int filecache)
{
  CURL urlFile(strFile);

  VECADDONS codecs;
  CAddonMgr::Get().GetAddons(ADDON_AUDIODECODER, codecs);
  for (size_t i=0;i<codecs.size();++i)
  {
    boost::shared_ptr<CAudioDecoder> dec(boost::static_pointer_cast<CAudioDecoder>(codecs[i]));
    if (dec->GetMimetypes().find(strContent) != std::string::npos)
    {
      CAudioDecoder* result = new CAudioDecoder(*dec);
      static_cast<AudioDecoderDll&>(*result).Create();
      return result;
    }
  }
  if (StringUtils::StartsWithNoCase(strContent, "audio/l16"))
  {
    PCMCodec * pcm_codec = new PCMCodec();
    pcm_codec->SetMimeParams(strContent);
    return pcm_codec;
  }
  else if( strContent.Equals("audio/aac") || strContent.Equals("audio/aacp") ||
      strContent.Equals("audio/x-ms-wma") ||
      strContent.Equals("audio/x-ape") || strContent.Equals("audio/ape"))
  {
    DVDPlayerCodec *pCodec = new DVDPlayerCodec;
    pCodec->SetContentType(strContent);
    return pCodec;
  }
  else if (strContent.Equals("audio/x-xbmc-pcm"))
  {
    // audio/x-xbmc-pcm this is the used codec for AirTunes
    // (apples audio only streaming)
    DVDPlayerCodec *dvdcodec = new DVDPlayerCodec();
    dvdcodec->SetContentType(strContent);
    return dvdcodec;
  }

  if (urlFile.GetFileType().Equals("wav") || strContent.Equals("audio/wav") || strContent.Equals("audio/x-wav"))
  {
    //lets see what it contains...
    //this kinda sucks 'cause if it's a plain wav file the file
    //will be opened, sniffed and closed 2 times before it is opened *again* for wav
    //would be better if the papcodecs could work with bitstreams instead of filenames.
    DVDPlayerCodec *dvdcodec = new DVDPlayerCodec();
    dvdcodec->SetContentType("audio/x-spdif-compressed");
    if (dvdcodec->Init(strFile, filecache))
    {
      return dvdcodec;
    }
    delete dvdcodec;

    dvdcodec = new DVDPlayerCodec();
    dvdcodec->SetContentType(strContent);
    return dvdcodec;

  }

  //default
  return CreateCodec(urlFile.GetFileType());
}

