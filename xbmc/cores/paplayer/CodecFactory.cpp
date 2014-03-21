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
#include "MP3codec.h"
#include "OGGcodec.h"
#include "NSFCodec.h"
#ifdef HAS_SPC_CODEC
#include "SPCCodec.h"
#endif
#include "SIDCodec.h"
#include "VGMCodec.h"
#include "TimidityCodec.h"
#ifdef HAS_ASAP_CODEC
#include "ASAPCodec.h"
#endif
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
  if (strFileType.Equals("mp3") || strFileType.Equals("mp2"))
    return new MP3Codec();
  else if (strFileType.Equals("pcm") || strFileType.Equals("l16"))
    return new PCMCodec();
  else if (strFileType.Equals("ape") || strFileType.Equals("mac"))
    return new DVDPlayerCodec();
  else if (strFileType.Equals("cdda"))
    return new DVDPlayerCodec();
  else if (strFileType.Equals("mpc") || strFileType.Equals("mp+") || strFileType.Equals("mpp"))
    return new DVDPlayerCodec();
  else if (strFileType.Equals("shn"))
    return new DVDPlayerCodec();
  else if (strFileType.Equals("mka"))
    return new DVDPlayerCodec();
  else if (strFileType.Equals("flac"))
    return new DVDPlayerCodec();
  else if (strFileType.Equals("wav"))
    return new DVDPlayerCodec();
  else if (strFileType.Equals("dts") || strFileType.Equals("ac3") ||
           strFileType.Equals("m4a") || strFileType.Equals("aac") ||
           strFileType.Equals("pvr"))
    return new DVDPlayerCodec();
  else if (strFileType.Equals("wv"))
    return new DVDPlayerCodec();
  else if (strFileType.Equals("nsf") || strFileType.Equals("nsfstream"))
    return new NSFCodec();
#ifdef HAS_SPC_CODEC
  else if (strFileType.Equals("spc"))
    return new SPCCodec();
#endif
  else if (strFileType.Equals("sid") || strFileType.Equals("sidstream"))
    return new SIDCodec();
  else if (VGMCodec::IsSupportedFormat(strFileType))
    return new VGMCodec();
  else if (strFileType.Equals("wma"))
    return new DVDPlayerCodec();
  else if (strFileType.Equals("aiff") || strFileType.Equals("aif"))
    return new DVDPlayerCodec();
  else if (strFileType.Equals("xwav"))
    return new DVDPlayerCodec();
  else if (TimidityCodec::IsSupportedFormat(strFileType))
    return new TimidityCodec();
#ifdef HAS_ASAP_CODEC
  else if (ASAPCodec::IsSupportedFormat(strFileType) || strFileType.Equals("asapstream"))
    return new ASAPCodec();
#endif
  else if (strFileType.Equals("tta"))
    return new DVDPlayerCodec();
  else if (strFileType.Equals("tak"))
    return new DVDPlayerCodec();

  return NULL;
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
  if( strContent.Equals("audio/mpeg")
  ||  strContent.Equals("audio/mpeg3")
  ||  strContent.Equals("audio/mp3") )
    return new MP3Codec();
  else if (StringUtils::StartsWithNoCase(strContent, "audio/l16"))
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
  else if( strContent.Equals("application/ogg") || strContent.Equals("audio/ogg"))
    return CreateOGGCodec(strFile,filecache);
  else if (strContent.Equals("audio/x-xbmc-pcm"))
  {
    // audio/x-xbmc-pcm this is the used codec for AirTunes
    // (apples audio only streaming)
    DVDPlayerCodec *dvdcodec = new DVDPlayerCodec();
    dvdcodec->SetContentType(strContent);
    return dvdcodec;
  }
  else if (strContent.Equals("audio/flac") || strContent.Equals("audio/x-flac") || strContent.Equals("application/x-flac"))
  {
    DVDPlayerCodec *dvdcodec = new DVDPlayerCodec();
    dvdcodec->SetContentType(strContent);
    return dvdcodec;
  }

  if (urlFile.GetProtocol() == "shout")
  {
    return new MP3Codec(); // if we got this far with internet radio - content-type was wrong. gamble on mp3.
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

    dvdcodec = new DVDPlayerCodec();
    dvdcodec->SetContentType(strContent);
    return dvdcodec;

  }
  else if (urlFile.GetFileType().Equals("ogg") || urlFile.GetFileType().Equals("oggstream") || urlFile.GetFileType().Equals("oga"))
    return CreateOGGCodec(strFile,filecache);

  //default
  return CreateCodec(urlFile.GetFileType());
}

ICodec* CodecFactory::CreateOGGCodec(const CStdString& strFile,
                                     unsigned int filecache)
{
  // oldnemesis: we want to use OGGCodec() for OGG music since unlike DVDCodec 
  // it provides better timings for Karaoke. However OGGCodec() cannot handle 
  // ogg-flac and ogg videos, that's why this block.
  ICodec* codec = new OGGCodec();
  try
  {
    if (codec->Init(strFile, filecache))
      return codec;
  }
  catch( ... )
  {
  }
  delete codec;
  return new DVDPlayerCodec();
}

