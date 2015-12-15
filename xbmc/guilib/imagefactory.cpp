/*
 *      Copyright (C) 2012-2013 Team XBMC
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

#include "imagefactory.h"
#include "guilib/FFmpegImage.h"
#include "addons/AddonManager.h"
#include "addons/ImageEncoder.h"
#include "utils/Mime.h"
#include "utils/StringUtils.h"
#if defined(HAS_GIFLIB)
#include "guilib/Gif.h"
#endif//HAS_GIFLIB
#include "FFmpegImage.h"

#include <algorithm>

using namespace ADDON;

IImage* ImageFactory::CreateLoader(const std::string& strFileName)
{
  CURL url(strFileName);
  return CreateLoader(url);
}

IImage* ImageFactory::CreateLoader(const CURL& url)
{
  if(!url.GetFileType().empty())
    return CreateLoaderFromMimeType("image/"+url.GetFileType());

  return CreateLoaderFromMimeType(CMime::GetMimeType(url));
}

IImage* ImageFactory::CreateLoaderFromMimeType(const std::string& strMimeType)
{
  VECADDONS codecs;
  CAddonMgr::GetInstance().GetAddons(codecs, ADDON_IMAGE_ENCODER);
  for (auto& codec : codecs)
  {
    std::shared_ptr<CImageEncoder> enc(std::static_pointer_cast<CImageEncoder>(codec));
    std::vector<std::string> mime = StringUtils::Split(enc->GetMimetypes(), "|");
    if (std::find(mime.begin(), mime.end(), strMimeType) != mime.end())
    {
      CImageEncoder* result = new CImageEncoder(*enc);
      result->Create(strMimeType);
      return result;
    }
  }

#if defined(HAS_GIFLIB)
  if (strMimeType == "image/gif")
    return new Gif();
#endif//HAS_GIFLIB

  return new CFFmpegImage(strMimeType);
}
