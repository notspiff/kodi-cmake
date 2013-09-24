set(SOURCES AEFactory.cpp
            Utils/AEChannelInfo.cpp
            Utils/AEBuffer.cpp
            Utils/AEConvert.cpp
            Utils/AERemap.cpp
            Utils/AEStreamInfo.cpp
            Utils/AEUtil.cpp
            Utils/AEPackIEC61937.cpp
            Utils/AEBitstreamPacker.cpp
            Utils/AEWAVLoader.cpp
            Utils/AEELDParser.cpp
            Utils/AEDeviceInfo.cpp
            Utils/AELimiter.cpp
            Encoders/AEEncoderFFmpeg.cpp)

if(APPLE)
  list(APPEND SOURCES Engines/CoreAudio/CoreAudioAE.cpp
                      Engines/CoreAudio/CoreAudioAEHAL.cpp
                      Engines/CoreAudio/CoreAudioAEHALOSX.cpp
                      Engines/CoreAudio/CoreAudioAESound.cpp
                      Engines/CoreAudio/CoreAudioAEStream.cpp
                      Engines/CoreAudio/CoreAudioChannelLayout.cpp
                      Engines/CoreAudio/CoreAudioDevice.cpp
                      Engines/CoreAudio/CoreAudioGraph.cpp
                      Engines/CoreAudio/CoreAudioHardware.cpp
                      Engines/CoreAudio/CoreAudioMixMap.cpp
                      Engines/CoreAudio/CoreAudioStream.cpp
                      Engines/CoreAudio/CoreAudioUnit.cpp)
else()
  list(APPEND SOURCES AESinkFactory.cpp
                      Sinks/AESinkNULL.cpp
                      Sinks/AESinkOSS.cpp
                      Sinks/AESinkProfiler.cpp
                      Engines/ActiveAE/ActiveAE.cpp
                      Engines/ActiveAE/ActiveAESink.cpp
                      Engines/ActiveAE/ActiveAEStream.cpp
                      Engines/ActiveAE/ActiveAESound.cpp
                      Engines/ActiveAE/ActiveAEResample.cpp
                      Engines/ActiveAE/ActiveAEBuffer.cpp
                      Engines/SoftAE/SoftAE.cpp
                      Engines/SoftAE/SoftAEStream.cpp
                      Engines/SoftAE/SoftAESound.cpp)
endif()

#if(ALSA_FOUND)
  list(APPEND SOURCES Sinks/AESinkALSA.cpp)
#endif()

if(PULSEAUDIO_FOUND)
  list(APPEND SOURCES Engines/PulseAE/PulseAE.cpp
                      Engines/PulseAE/PulseAEStream.cpp
                      Engines/PulseAE/PulseAESound.cpp)
  add_definitions(-D__STDC_LIMIT_MACROS)
endif()

if(WIN32)
  list(APPEND SOURCES Sinks/AESinkDirectSound.cpp
                      Sinks/AESinkWASAPI.cpp)
endif()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

core_add_library(audioengine)
add_dependencies(audioengine ffmpeg)
