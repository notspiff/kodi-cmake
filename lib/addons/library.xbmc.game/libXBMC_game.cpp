/*
 *      Copyright (C) 2014 Team XBMC
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

#include "addons/AddonCallbacks.h"
#include "addons/include/xbmc_game_types.h"

#include <stdio.h>

#ifdef _WIN32
  #include <windows.h>
  #define DLLEXPORT __declspec(dllexport)
#else
  #define DLLEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

DLLEXPORT CB_GameLib* GAME_register_me(AddonCB* frontend)
{
  CB_GameLib* cb = NULL;
  if (!frontend)
    fprintf(stderr, "ERROR: GAME_register_frontend is called with NULL handle!!!\n");
  else
  {
    cb = frontend->GameLib_RegisterMe(frontend->addonData);
    if (!cb)
      fprintf(stderr, "ERROR: GAME_register_frontend can't get callback table from frontend!!!\n");
  }
  return cb;
}

DLLEXPORT void GAME_unregister_me(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return;
  return frontend->GameLib_UnRegisterMe(frontend->addonData, cb);
}

DLLEXPORT void GAME_shutdown_frontend(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->ShutdownFrontend(frontend->addonData);
}

DLLEXPORT void GAME_environment_set_rotation(AddonCB* frontend, CB_GameLib* cb, GAME_ROTATION rotation)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->EnvironmentSetRotation(frontend->addonData, rotation);
}

DLLEXPORT bool GAME_environment_get_overscan(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return false;
  return cb->EnvironmentGetOverscan(frontend->addonData);
}

DLLEXPORT bool GAME_environment_can_dupe(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return false;
  return cb->EnvironmentCanDupe(frontend->addonData);
}

DLLEXPORT void GAME_environment_set_input_descriptors(AddonCB* frontend, CB_GameLib* cb, const struct game_input_descriptor* descriptor, size_t count)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->EnvironmentSetInputDescriptors(frontend->addonData, descriptor, count);
}

DLLEXPORT bool GAME_environment_set_system_av_info(AddonCB* frontend, CB_GameLib* cb, const struct game_system_av_info* info)
{
  if (frontend == NULL || cb == NULL)
    return false;
  return cb->EnvironmentSetSystemAvInfo(frontend->addonData, info);
}

DLLEXPORT void GAME_video_refresh(AddonCB* frontend, CB_GameLib* cb, const void *data, unsigned width, unsigned height, size_t pitch, unsigned int pixelFormat)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->VideoRefresh(frontend->addonData, data, width, height, pitch, (GAME_PIXEL_FORMAT)pixelFormat);
}

DLLEXPORT void GAME_audio_sample(AddonCB* frontend, CB_GameLib* cb, int16_t left, int16_t right)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->AudioSample(frontend->addonData, left, right);
}

DLLEXPORT size_t GAME_audio_sample_batch(AddonCB* frontend, CB_GameLib* cb, const int16_t *data, size_t frames)
{
  if (frontend == NULL || cb == NULL)
    return 0;
  return cb->AudioSampleBatch(frontend->addonData, data, frames);
}

DLLEXPORT int16_t GAME_input_state(AddonCB* frontend, CB_GameLib* cb, unsigned port, unsigned device, unsigned index, unsigned id)
{
  if (frontend == NULL || cb == NULL)
    return 0;
  return cb->InputState(frontend->addonData, port, device, index, id);
}

DLLEXPORT uint64_t GAME_input_get_device_capabilities(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return 0;
  return cb->InputGetDeviceCapabilities(frontend->addonData);
}

DLLEXPORT bool GAME_rumble_set_state(AddonCB* frontend, CB_GameLib* cb, unsigned port, GAME_RUMBLE_EFFECT effect, uint16_t strength)
{
  if (frontend == NULL || cb == NULL)
    return false;
  return cb->RumbleSetState(frontend->addonData, port, effect, strength);
}

DLLEXPORT game_time_t GAME_perf_get_time_usec(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return 0;
  return cb->PerfGetTimeUsec(frontend->addonData);
}

DLLEXPORT game_perf_tick_t GAME_perf_get_counter(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return 0;
  return cb->PerfGetCounter(frontend->addonData);
}

DLLEXPORT uint64_t GAME_perf_get_cpu_features(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return 0;
  return cb->PerfGetCpuFeatures(frontend->addonData);
}

DLLEXPORT void GAME_perf_log(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->PerfLog(frontend->addonData);
}

DLLEXPORT void GAME_perf_register(AddonCB* frontend, CB_GameLib* cb, game_perf_counter *counter)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->PerfRegister(frontend->addonData, counter);
}

DLLEXPORT void GAME_perf_start(AddonCB* frontend, CB_GameLib* cb, game_perf_counter *counter)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->PerfStart(frontend->addonData, counter);
}

DLLEXPORT void GAME_perf_stop(AddonCB* frontend, CB_GameLib* cb, game_perf_counter *counter)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->PerfStop(frontend->addonData, counter);
}

DLLEXPORT bool GAME_sensor_set_state(AddonCB* frontend, CB_GameLib* cb, unsigned port, GAME_SENSOR_ACTION action, unsigned rate)
{
  if (frontend == NULL || cb == NULL)
    return false;
  return cb->SensorSetState(frontend->addonData, port, action, rate);
}

DLLEXPORT float GAME_sensor_get_input(AddonCB* frontend, CB_GameLib* cb, unsigned port, unsigned id)
{
  if (frontend == NULL || cb == NULL)
    return 0.0f;
  return cb->SensorGetInput(frontend->addonData, port, id);
}

DLLEXPORT void GAME_camera_set_info(AddonCB* frontend, CB_GameLib* cb, game_camera_info *camera_info)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->CameraSetInfo(frontend->addonData, camera_info);
}

DLLEXPORT bool GAME_camera_start(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return false;
  return cb->CameraStart(frontend->addonData);
}

DLLEXPORT void GAME_camera_stop(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->CameraStop(frontend->addonData);
}

DLLEXPORT bool GAME_location_start(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return false;
  return cb->LocationStart(frontend->addonData);
}

DLLEXPORT void GAME_location_stop(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->LocationStop(frontend->addonData);
}

DLLEXPORT bool GAME_location_get_position(AddonCB* frontend, CB_GameLib* cb, double *lat, double *lon, double *horiz_accuracy, double *vert_accuracy)
{
  if (frontend == NULL || cb == NULL)
    return false;
  return cb->LocationGetPosition(frontend->addonData, lat, lon, horiz_accuracy, vert_accuracy);
}

DLLEXPORT void GAME_location_set_interval(AddonCB* frontend, CB_GameLib* cb, unsigned interval_ms, unsigned interval_distance)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->LocationSetInterval(frontend->addonData, interval_ms, interval_distance);
}

DLLEXPORT void GAME_location_initialized(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->LocationInitialized(frontend->addonData);
}

DLLEXPORT void GAME_location_deinitialized(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->LocationDeinitialized(frontend->addonData);
}

DLLEXPORT void GAME_frame_time_set_reference(AddonCB* frontend, CB_GameLib* cb, game_usec_t usec)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->FrameTimeSetReference(frontend->addonData, usec);
}

DLLEXPORT void GAME_hw_set_info(AddonCB* frontend, CB_GameLib* cb, game_hw_info *hw_info)
{
  if (frontend == NULL || cb == NULL)
    return;
  return cb->HwSetInfo(frontend->addonData, hw_info);
}

DLLEXPORT uintptr_t GAME_hw_get_current_framebuffer(AddonCB* frontend, CB_GameLib* cb)
{
  if (frontend == NULL || cb == NULL)
    return 0;
  return cb->HwGetCurrentFramebuffer(frontend->addonData);
}

DLLEXPORT game_proc_address_t GAME_hw_get_proc_address(AddonCB* frontend, CB_GameLib* cb, const char *sym)
{
  if (frontend == NULL || cb == NULL)
    return NULL;
  return cb->HwGetProcAddress(frontend->addonData, sym);
}

#ifdef __cplusplus
}
#endif
