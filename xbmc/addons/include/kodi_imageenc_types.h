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

#pragma once

#include <stdint.h>

extern "C"
{
  struct IMAGEENC_INFO
  {
    int dummy;
  };

  struct IMAGEENC_PROPS
  {
    const char* mimetype;
  };

  struct ImageEncoder
  {
    //! \brief Initialize an encoder
    //! \param buffer The data to read from memory
    //! \param bufSize The buffer size
    //! \param width The optimal width of image on entry, obtained width on return
    //! \param height The optimal height of image, actual obtained height on return
    //! \return Image or nullptr on error
    void* (__cdecl* LoadImage) (unsigned char* buffer, unsigned int bufSize,
                                unsigned int* width, unsigned int* height);

    //! \brief Decode previously loaded image
    //! \param image Image to decode
    //! \param pixels Output buffer
    //! \param width Width of output image
    //! \param height Height of output image
    //! \param pitch Pitch of output image
    //! \param format Format of output image
    bool (__cdecl* Decode) (void* image, unsigned char* pixels,
                            unsigned int width, unsigned int height,
                            unsigned int pitch, unsigned int format);

   /*!
   \brief Encodes an thumbnail from raw bits of given memory location
   \remarks Caller need to call ReleaseThumbnailBuffer() afterwards to free the output buffer
   \param bufferin The memory location where the image data can be found
   \param width The width of the thumbnail
   \param height The height of the thumbnail
   \param pitch The pitch of the input texture stored in bufferin
   \param format The format of the input buffer
   \param destFile The destination path of the thumbnail to determine the image format from the extension
   \return true if the thumbnail was successfully created
   */
    bool (__cdecl* CreateThumbnail) (unsigned char* bufferin,
                                     unsigned int width,
                                     unsigned int height,
                                     unsigned int pitch,
                                     unsigned int format,
                                     const char* destFile);

    //! \brief Close an opened image
    //! \param image Image to close
    //! \return True on success, false on failure
    void (__cdecl* Close)(void* image);
  };
}
