/*
 *      Copyright (C) 2012-2014 Team XBMC
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

#include "RetroPlayerBuffer.h"
#include "addons/include/xbmc_game_types.h"
#include "linux/PlatformDefs.h" // Must be included before RenderManager.h
#include "cores/VideoRenderers/RenderManager.h"
#include "threads/Thread.h"

#include <stdint.h>

struct DVDVideoPicture;
struct SwsContext;

class CRetroPlayerVideo : public CThread
{
  struct VideoInfo
  {
    unsigned int      width;
    unsigned int      height;
    size_t            pitch;
    GAME_PIXEL_FORMAT format;
  };

  class CRetroPlayerVideoBuffer : public CRetroPlayerBuffer
  {
  protected:
    // We only buffer a single video frame
    virtual bool IsFull() const { return GetCount() > 0; }

  public:
    virtual ~CRetroPlayerVideoBuffer() { }
  };

  typedef CRetroPlayerPacket<VideoInfo> VideoFrame;

public:
  CRetroPlayerVideo();
  ~CRetroPlayerVideo();

  /**
   * Begin doing what a RetroPlayerVideo does best.
   */
  void GoForth(double framerate, bool fullscreen);

  /**
   * Send a video frame to be rendered by this class. The pixel format is
   * specified by m_pixelFormat.
   */
  void SendVideoFrame(const uint8_t *data, unsigned width, unsigned height, size_t pitch, GAME_PIXEL_FORMAT format);

protected:
  virtual void Process();

private:
  bool ProcessFrame(const VideoFrame &frame);
  bool CheckConfiguration(const DVDVideoPicture &picture, GAME_PIXEL_FORMAT sourceFormat);
  void ColorspaceConversion(const VideoFrame &input, const DVDVideoPicture &output);

  SwsContext*             m_swsContext;

  CRetroPlayerVideoBuffer m_buffer;
  CEvent                  m_frameEvent;  // Set immediately when a new frame is queued

  bool                    m_bAllowFullscreen;
  double                  m_framerate;

  // Output variables are used to store the current output configuration. Each
  // frame the picture's configuration is compared to the output configuration,
  // and a discrepancy will reconfigure the render manager, as well as saving
  // the new state to these variables.
  unsigned int            m_outputWidth;
  unsigned int            m_outputHeight;
  double                  m_outputFramerate;
  GAME_PIXEL_FORMAT       m_pixelFormat;
};
