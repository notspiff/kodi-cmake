/*
 * various utility functions used within FFmpeg
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* Functions defined here are functions that cannot be resolved through the
 * ffmpeg shared libraries, yet are used in XBMC.
 */

#if (defined HAVE_CONFIG_H) && (!defined WIN32)
  #include "config.h"
#endif

#include <libavformat/avformat.h>

/* Taken from libavformat/utils.c */
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(54,0,0)
static void flush_packet_queue(AVFormatContext *s)
{
    AVPacketList *pktl;

    for(;;) {
        pktl = s->packet_buffer;
        if (!pktl)
            break;
        s->packet_buffer = pktl->next;
        av_free_packet(&pktl->pkt);
        av_free(pktl);
    }
    while(s->raw_packet_buffer){
        pktl = s->raw_packet_buffer;
        s->raw_packet_buffer = pktl->next;
        av_free_packet(&pktl->pkt);
        av_free(pktl);
    }
    s->packet_buffer_end=
    s->raw_packet_buffer_end= NULL;
#ifdef RAW_PACKET_BUFFER_SIZE
    // Added on: 2009-06-25
    s->raw_packet_buffer_remaining_size = RAW_PACKET_BUFFER_SIZE;
#endif
}
#else
static void free_packet_buffer(AVPacketList **pkt_buf, AVPacketList **pkt_buf_end)
{
    while (*pkt_buf) {
        AVPacketList *pktl = *pkt_buf;
        *pkt_buf = pktl->next;
        av_free_packet(&pktl->pkt);
        av_freep(&pktl);
    }
    *pkt_buf_end = NULL;
}
/* XXX: suppress the packet queue */
static void flush_packet_queue(AVFormatContext *s)
{
    free_packet_buffer(&s->parse_queue,       &s->parse_queue_end);
    free_packet_buffer(&s->packet_buffer,     &s->packet_buffer_end);
    free_packet_buffer(&s->raw_packet_buffer, &s->raw_packet_buffer_end);

    s->raw_packet_buffer_remaining_size = RAW_PACKET_BUFFER_SIZE;
}
#endif

/* Taken from libavformat/utils.c
 * Original name is ff_read_frame_flush
 * */
void xbmc_read_frame_flush(AVFormatContext *s)
{
    AVStream *st;
    int i, j;

    flush_packet_queue(s);

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(54,0,0)
    s->cur_st = NULL;
#endif

    /* for each stream, reset read state */
    for(i = 0; i < s->nb_streams; i++) {
        st = s->streams[i];

        if (st->parser) {
            av_parser_close(st->parser);
            st->parser = NULL;
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(54,0,0)
            av_free_packet(&st->cur_pkt);
#endif
        }
        st->last_IP_pts = AV_NOPTS_VALUE;
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(54,0,0)
        st->cur_dts = AV_NOPTS_VALUE; /* we set the current DTS to an unspecified origin */
        st->reference_dts = AV_NOPTS_VALUE;
        /* fail safe */
        st->cur_ptr = NULL;
        st->cur_len = 0;
#else
#define RELATIVE_TS_BASE (INT64_MAX - (1LL<<48))
        if(st->first_dts == AV_NOPTS_VALUE) st->cur_dts = RELATIVE_TS_BASE;
        else                                st->cur_dts = AV_NOPTS_VALUE; /* we set the current DTS to an unspecified origin */
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(55,0,0)
        st->reference_dts = AV_NOPTS_VALUE;
#endif

        st->probe_packets = MAX_PROBE_PACKETS;
#endif

        for(j=0; j<MAX_REORDER_DELAY+1; j++)
            st->pts_buffer[j]= AV_NOPTS_VALUE;
    }
}
