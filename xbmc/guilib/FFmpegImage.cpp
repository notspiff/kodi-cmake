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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */
#include "FFmpegImage.h"
#include "utils/log.h"
#include "cores/FFmpeg.h"
extern "C"
{
#include <libavutil/imgutils.h>
}

struct MemBuffer
{
  uint8_t* data;
  size_t size;
  size_t pos;
};

static int mem_file_read(void *h, uint8_t* buf, int size)
{
  MemBuffer* mbuf = static_cast<MemBuffer*>(h);
  size_t tocopy = std::min((size_t)size, mbuf->size-mbuf->pos);
  memcpy(buf, mbuf->data+mbuf->pos, tocopy);
  mbuf->pos += tocopy;
  return tocopy;
}

static off_t mem_file_seek(void *h, off_t pos, int whence)
{
  MemBuffer* mbuf = static_cast<MemBuffer*>(h);
  if(whence == AVSEEK_SIZE)
    return mbuf->size;

  whence &= ~AVSEEK_FORCE;
  if (whence == SEEK_SET)
    mbuf->pos = pos;
  else if (whence == SEEK_CUR)
    mbuf->pos = std::min(mbuf->pos+pos, mbuf->size-1);
  else
    mbuf->pos = mbuf->size+pos;

  return mbuf->pos;
}

CFFmpegImage::CFFmpegImage()
{
  m_hasAlpha = false;
}

CFFmpegImage::~CFFmpegImage()
{
}

bool CFFmpegImage::LoadImageFromMemory(unsigned char* buffer, unsigned int bufSize,
                                      unsigned int width, unsigned int height)
{
  uint8_t* fbuffer = (uint8_t*)av_malloc(32768);
  MemBuffer buf;
  buf.data = buffer;
  buf.size = bufSize;
  buf.pos = 0;

  AVIOContext* ioctx = avio_alloc_context(fbuffer, 32768, 0, &buf,
                                          mem_file_read, NULL, mem_file_seek);

  AVFormatContext* fctx = avformat_alloc_context();
  fctx->pb = ioctx;

  ioctx->max_packet_size = 32768;

  AVInputFormat* iformat = av_find_input_format("image");

  if (avformat_open_input(&fctx, "", iformat, NULL) < 0)
  {
    if (fctx)
      avformat_close_input(&fctx);
    av_free(ioctx->buffer);
    av_free(ioctx);
    return false;
  }
  AVCodecContext* codec_ctx = fctx->streams[0]->codec;
  AVCodec* codec = avcodec_find_decoder(codec_ctx->codec_id);
  if (avcodec_open2(codec_ctx, codec, NULL) < 0)
  {
    if (fctx)
      avformat_close_input(&fctx);
    av_free(ioctx->buffer);
    av_free(ioctx);
    return false;
  }

  AVFrame* frame = av_frame_alloc();
  AVPacket pkt;
  av_read_frame(fctx, &pkt);
  int frame_decoded;
  avcodec_decode_video2(codec_ctx, m_frame, &frame_decoded, &pkt);
  av_free_packet(&pkt);
  avcodec_close(codec_ctx);
  avformat_close_input(&fctx);
  av_free(ioctx->buffer);
  av_free(ioctx);

  return true;
}

bool CFFmpegImage::Decode(const unsigned char *pixels, unsigned int pitch,
                          unsigned int format)
{
  av_image_copy_to_buffer(const_cast<uint8_t*>(pixels), pitch,
                          m_frame->data, m_frame->linesize,
                          (AVPixelFormat)m_frame->format,
                          m_frame->width, m_frame->height, pitch);
  return true;
}

bool CFFmpegImage::CreateThumbnailFromSurface(unsigned char* bufferin, unsigned int width,
                                             unsigned int height, unsigned int format,
                                             unsigned int pitch,
                                             const std::string& destFile,
                                             unsigned char* &bufferout,
                                             unsigned int &bufferoutSize)
{
  return false;
}

void CFFmpegImage::ReleaseThumbnailBuffer()
{
  av_freep(&m_frame);
}
