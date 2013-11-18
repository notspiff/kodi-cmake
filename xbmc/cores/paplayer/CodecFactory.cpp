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
#include "utils/StringUtils.h"
#include "addons/AddonManager.h"
#include "addons/AudioDecoder.h"

using namespace ADDON;

ICodec* CodecFactory::CreateCodec(const std::string& strFileType)
{
  std::string fileType = strFileType;
  StringUtils::ToLower(fileType);
  VECADDONS codecs;
  CAddonMgr::Get().GetAddons(ADDON_AUDIODECODER, codecs);
  for (size_t i=0;i<codecs.size();++i)
  {
    boost::shared_ptr<CAudioDecoder> dec(boost::static_pointer_cast<CAudioDecoder>(codecs[i]));
    if (dec->GetExtensions().find("."+fileType) != std::string::npos)
    {
      CAudioDecoder* result = new CAudioDecoder(*dec);
      static_cast<AudioDecoderDll&>(*result).Create();
      return result;
    }
  }
  if (fileType == "mp3" || fileType == "mp2")
    return new DVDPlayerCodec();
  else if (fileType == "pcm" || fileType == "l16")
    return new DVDPlayerCodec();
  else if (fileType == "ape" || fileType == "mac")
    return new DVDPlayerCodec();
  else if (fileType == "cdda")
    return new DVDPlayerCodec();
  else if (fileType == "mpc" || fileType == "mp+" || fileType == "mpp")
    return new DVDPlayerCodec();
  else if (fileType == "shn")
    return new DVDPlayerCodec();
  else if (fileType == "mka")
    return new DVDPlayerCodec();
  else if (fileType == "wav")
    return new DVDPlayerCodec();
  else if (fileType == "dts" || fileType == "ac3" ||
           fileType == "m4a" || fileType == "aac" ||
           fileType == "pvr")
    return new DVDPlayerCodec();
  else if (fileType == "wv")
    return new DVDPlayerCodec();
  else if (fileType == "wma")
    return new DVDPlayerCodec();
  else if (fileType == "aiff" || fileType == "aif")
    return new DVDPlayerCodec();
  else if (fileType == "tta")
    return new DVDPlayerCodec();
  else if (fileType == "tak")
    return new DVDPlayerCodec();
  else if (fileType == "opus")
    return new DVDPlayerCodec();
  else if (fileType == "dff" || fileType == "dsf")
    return new DVDPlayerCodec();

  return NULL;
}

ICodec* CodecFactory::CreateCodecDemux(const std::string& strFile, const std::string& strContent, unsigned int filecache)
{
  CURL urlFile(strFile);
  std::string content = strContent;
  StringUtils::ToLower(content);
  if (!content.empty())
  {
    VECADDONS codecs;
    CAddonMgr::Get().GetAddons(ADDON_AUDIODECODER, codecs);
    for (size_t i=0;i<codecs.size();++i)
    {
      boost::shared_ptr<CAudioDecoder> dec(boost::static_pointer_cast<CAudioDecoder>(codecs[i]));
      if (dec->GetMimetypes().find(content) != std::string::npos)
      {
        CAudioDecoder* result = new CAudioDecoder(*dec);
        static_cast<AudioDecoderDll&>(*result).Create();
        return result;
      }
    }
  }
  if( content == "audio/mpeg"
  ||  content == "audio/mpeg3"
  ||  content == "audio/mp3" )
  {
    DVDPlayerCodec *dvdcodec = new DVDPlayerCodec();
    dvdcodec->SetContentType(content);
    return dvdcodec;
  }
  else if (StringUtils::StartsWith(content, "audio/l16"))
  {
    DVDPlayerCodec *pCodec = new DVDPlayerCodec;
    pCodec->SetContentType(content);
    return pCodec;
  }
  else if(content == "audio/aac" ||
          content == "audio/aacp" ||
          content == "audio/x-ms-wma" ||
          content == "audio/x-ape" ||
          content == "audio/ape")
  {
    DVDPlayerCodec *pCodec = new DVDPlayerCodec;
    pCodec->SetContentType(content);
    return pCodec;
  }
  else if (content == "audio/x-xbmc-pcm")
  {
    // audio/x-xbmc-pcm this is the used codec for AirTunes
    // (apples audio only streaming)
    DVDPlayerCodec *dvdcodec = new DVDPlayerCodec();
    dvdcodec->SetContentType(content);
    return dvdcodec;
  }

  if (urlFile.IsProtocol("shout"))
  {
    DVDPlayerCodec *dvdcodec = new DVDPlayerCodec();
    dvdcodec->SetContentType("audio/mp3");
    return dvdcodec; // if we got this far with internet radio - content-type was wrong. gamble on mp3.
  }

  if (urlFile.IsFileType("wav") ||
      content == "audio/wav" ||
      content == "audio/x-wav")
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
    dvdcodec->SetContentType(content);
    return dvdcodec;

  }

  //default
  return CreateCodec(urlFile.GetFileType());
}
