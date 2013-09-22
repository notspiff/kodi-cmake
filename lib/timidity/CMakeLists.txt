set(SOURCES timidity/aq.c
            timidity/audio_cnv.c
            timidity/buffer_a.c
            timidity/common.c
            timidity/controls.c
            timidity/sdl_c.c
            timidity/effect.c
            timidity/filter.c
            timidity/freq.c
            timidity/instrum.c
            timidity/miditrace.c
            timidity/mix.c
            timidity/modmid_a.c
            timidity/mt19937ar.c
            timidity/optcode.c
            timidity/output.c
            timidity/playmidi.c
            timidity/quantity.c
            timidity/readmidi.c
            timidity/recache.c
            timidity/resample.c
            timidity/reverb.c
            timidity/sbkconv.c
            timidity/sffile.c
            timidity/sfitem.c
            timidity/smfconv.c
            timidity/smplfile.c
            timidity/sndfont.c
            timidity/tables.c
            timidity/timidity.c
            timidity/version.c
            timidity/wave_a.c
            timidity/wrdt.c
            libarc/url.c
            libarc/url_mem.c
            libarc/url_file.c
            utils/mblock.c
            utils/strtab.c
            utils/fft4g.c
            utils/timer.c
            utils/getopt.c
            timidity_codec.c)
 
add_options(C ALL_BUILDS "-fPIC")

add_definitions(-DHAVE_GETTIMEOFDAY=1 -DHAVE_ALSA_ASOUNDLIB=1 -DHAVE_LIBASOUND=1)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} timidity libarc utils)

xbmc_add_library(timidity 1)
