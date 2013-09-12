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

#include "AddonCallbacksGame.h"
#include "threads/SystemClock.h"
#include "utils/log.h"

//#include "Application.h"
//#include "settings/AdvancedSettings.h"
//#include "dialogs/GUIDialogKaiToast.h"

#include <string>

using namespace GAME;
using namespace std;
using namespace XbmcThreads;

namespace ADDON
{

CAddonCallbacksGame::CAddonCallbacksGame(CAddon* addon)
{
  m_addon     = addon;
  m_callbacks = new CB_GameLib;

  /* write XBMC game specific add-on function addresses to callback table */
  m_callbacks->ShutdownFrontend               = ShutdownFrontend;
  m_callbacks->EnvironmentSetRotation         = EnvironmentSetRotation;
  m_callbacks->EnvironmentGetOverscan         = EnvironmentGetOverscan;
  m_callbacks->EnvironmentCanDupe             = EnvironmentCanDupe;
  m_callbacks->EnvironmentSetInputDescriptors = EnvironmentSetInputDescriptors;
  m_callbacks->EnvironmentSetSystemAvInfo     = EnvironmentSetSystemAvInfo;
  m_callbacks->VideoRefresh                   = VideoRefresh;
  m_callbacks->AudioSample                    = AudioSample;
  m_callbacks->AudioSampleBatch               = AudioSampleBatch;
  m_callbacks->InputState                     = InputState;
  m_callbacks->InputGetDeviceCapabilities     = InputGetDeviceCapabilities;
  m_callbacks->RumbleSetState                 = RumbleSetState;
  m_callbacks->PerfGetTimeUsec                = PerfGetTimeUsec;
  m_callbacks->PerfGetCounter                 = PerfGetCounter;
  m_callbacks->PerfGetCpuFeatures             = PerfGetCpuFeatures;
  m_callbacks->PerfLog                        = PerfLog;
  m_callbacks->PerfRegister                   = PerfRegister;
  m_callbacks->PerfStart                      = PerfStart;
  m_callbacks->PerfStop                       = PerfStop;
  m_callbacks->SensorSetState                 = SensorSetState;
  m_callbacks->SensorGetInput                 = SensorGetInput;
  m_callbacks->CameraSetInfo                  = CameraSetInfo;
  m_callbacks->CameraStart                    = CameraStart;
  m_callbacks->CameraStop                     = CameraStop;
  m_callbacks->LocationStart                  = LocationStart;
  m_callbacks->LocationStop                   = LocationStop;
  m_callbacks->LocationGetPosition            = LocationGetPosition;
  m_callbacks->LocationSetInterval            = LocationSetInterval;
  m_callbacks->LocationInitialized            = LocationInitialized;
  m_callbacks->LocationDeinitialized          = LocationDeinitialized;
  m_callbacks->FrameTimeSetReference          = FrameTimeSetReference;
  m_callbacks->HwSetInfo                      = HwSetInfo;
  m_callbacks->HwGetCurrentFramebuffer        = HwGetCurrentFramebuffer;
  m_callbacks->HwGetProcAddress               = HwGetProcAddress;
}

CAddonCallbacksGame::~CAddonCallbacksGame()
{
  /* delete the callback table */
  delete m_callbacks;
}

void CAddonCallbacksGame::ShutdownFrontend(void* addonData)
{
  // TODO: Call "ActivateWindow(shutdownmenu)"
}

void CAddonCallbacksGame::EnvironmentSetRotation(void* addonData, GAME_ROTATION rotation)
{
}

bool CAddonCallbacksGame::EnvironmentGetOverscan(void* addonData)
{
  // TODO: Move to library.xbmc.libretro and remove callback
  return true;
}

bool CAddonCallbacksGame::EnvironmentCanDupe(void* addonData)
{
  // TODO: Move to library.xbmc.libretro and remove callback
  return true;
}

void CAddonCallbacksGame::EnvironmentSetInputDescriptors(void* addonData, const game_input_descriptor* descriptor, size_t count)
{
  // TODO
}

bool CAddonCallbacksGame::EnvironmentSetSystemAvInfo(void* addonData, const game_system_av_info* info)
{
  // TODO
  return false;
}

void CAddonCallbacksGame::VideoRefresh(void* addonData, const void *data, unsigned width, unsigned height, size_t pitch, GAME_PIXEL_FORMAT pixelFormat)
{
}

void CAddonCallbacksGame::AudioSample(void* addonData, int16_t left, int16_t right)
{
}

size_t CAddonCallbacksGame::AudioSampleBatch(void* addonData, const int16_t *data, size_t frames)
{
  return 0;
}

int16_t CAddonCallbacksGame::InputState(void* addonData, unsigned port, unsigned device, unsigned index, unsigned id)
{
  return 0;
}

uint64_t CAddonCallbacksGame::InputGetDeviceCapabilities(void* addonData)
{
  // TODO
  return (1 << GAME_DEVICE_JOYPAD);
}

bool CAddonCallbacksGame::RumbleSetState(void* addonData, unsigned port, GAME_RUMBLE_EFFECT effect, uint16_t strength)
{
  return false;
}

game_time_t CAddonCallbacksGame::PerfGetTimeUsec(void* addonData)
{
  // See performance.c in RetroArch
  return 0;
}

game_perf_tick_t CAddonCallbacksGame::PerfGetCounter(void* addonData)
{
  // TODO
  return 0;
}

uint64_t CAddonCallbacksGame::PerfGetCpuFeatures(void* addonData)
{
  // TODO
  return 0;
}

void CAddonCallbacksGame::PerfLog(void* addonData)
{
  // TODO
}

void CAddonCallbacksGame::PerfRegister(void* addonData, game_perf_counter *counter)
{
  // TODO
}

void CAddonCallbacksGame::PerfStart(void* addonData, game_perf_counter *counter)
{
  // TODO
}

void CAddonCallbacksGame::PerfStop(void* addonData, game_perf_counter *counter)
{
  // TODO
}

bool CAddonCallbacksGame::SensorSetState(void* addonData, unsigned port, GAME_SENSOR_ACTION action, unsigned rate)
{
  // TODO
  return false;
}

float CAddonCallbacksGame::SensorGetInput(void* addonData, unsigned port, unsigned id)
{
  // TODO
  return 0.0f;
}

void CAddonCallbacksGame::CameraSetInfo(void* addonData, game_camera_info *camera_info)
{
  // TODO
}

bool CAddonCallbacksGame::CameraStart(void* addonData)
{
  // TODO
  return false;
}

void CAddonCallbacksGame::CameraStop(void* addonData)
{
  // TODO
}

bool CAddonCallbacksGame::LocationStart(void* addonData)
{
  // TODO
  return false;
}

void CAddonCallbacksGame::LocationStop(void* addonData)
{
  // TODO
}

bool CAddonCallbacksGame::LocationGetPosition(void* addonData, double *lat, double *lon, double *horiz_accuracy, double *vert_accuracy)
{
  // TODO
  return false;
}

void CAddonCallbacksGame::LocationSetInterval(void* addonData, unsigned interval_ms, unsigned interval_distance)
{
  // TODO
}

void CAddonCallbacksGame::LocationInitialized(void* addonData)
{
  // TODO
}

void CAddonCallbacksGame::LocationDeinitialized(void* addonData)
{
  // TODO
}

void CAddonCallbacksGame::FrameTimeSetReference(void* addonData, game_usec_t usec)
{
  // TODO
}

void CAddonCallbacksGame::HwSetInfo(void* addonData, const game_hw_info *hw_info)
{
  // TODO
}

uintptr_t CAddonCallbacksGame::HwGetCurrentFramebuffer(void* addonData)
{
  // TODO
  return 0;
}

game_proc_address_t CAddonCallbacksGame::HwGetProcAddress(void* addonData, const char *sym)
{
  // TODO
  return NULL;
}

}; /* namespace ADDON */
