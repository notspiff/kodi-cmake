/*
 *      Copyright (C) 2012-2013 Team XBMC
 *      http://www.xbmc.org
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

#include "WINJoystickXInput.h"
#include "system.h"
#include "utils/log.h"

#include <Xinput.h>

#pragma comment(lib, "XInput.lib")

#define XINPUT_ALIAS  "XBMC-Compatible XInput Controller"
#define MAX_JOYSTICKS 4
#define MAX_AXIS      32768
#define MAX_TRIGGER   255
#define BUTTON_COUNT  10
#define HAT_COUNT     1
#define AXIS_COUNT    5

CJoystickXInput::CJoystickXInput(unsigned int controllerID, unsigned int id)
 : CJoystick(XINPUT_ALIAS, id, BUTTON_COUNT, HAT_COUNT, AXIS_COUNT),
   m_controllerID(controllerID),
   m_dwPacketNumber(0)
{
}

/* static */
void CJoystickXInput::Initialize(JoystickArray& joysticks)
{
  Deinitialize(joysticks);

  XINPUT_STATE controllerState; // No need to memset, only checking for controller existence

  for (unsigned int i = 0; i < MAX_JOYSTICKS; i++)
  {
    DWORD result = XInputGetState(i, &controllerState);
    if (result != ERROR_SUCCESS)
    {
      if (result == ERROR_DEVICE_NOT_CONNECTED)
        CLog::Log(LOGNOTICE, "No XInput devices on port %u", i);
      continue;
    }

    // That's all it takes to check controller existence... I <3 XInput
    CLog::Log(LOGNOTICE, "CJoystickXInput: Found a 360-compatible XInput controller on port %u", i);
    joysticks.push_back(JoystickPtr(new CJoystickXInput(i, joysticks.size())));
  }
}

/* static */
void CJoystickXInput::Deinitialize(JoystickArray& joysticks)
{
  for (int i = 0; i < (int)joysticks.size(); i++)
  {
    if (boost::dynamic_pointer_cast<CJoystickXInput>(joysticks[i]))
      joysticks.erase(joysticks.begin() + i--);
  }
}

void CJoystickXInput::Update()
{
  CJoystickState &state = InitialState();

  XINPUT_STATE controllerState;

  DWORD result = XInputGetState(m_controllerID, &controllerState);
  if (result != ERROR_SUCCESS)
    return;

  m_dwPacketNumber = controllerState.dwPacketNumber;

  // Map to DirectInput controls
  state.buttons[0] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_A) ? true : false;
  state.buttons[1] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_B) ? true : false;
  state.buttons[2] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_X) ? true : false;
  state.buttons[3] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_Y) ? true : false;
  state.buttons[4] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? true : false;
  state.buttons[5] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? true : false;
  state.buttons[6] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ? true : false;
  state.buttons[7] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_START) ? true : false;
  state.buttons[8] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) ? true : false;
  state.buttons[9] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) ? true : false;

  state.hats[0][CJoystickHat::UP]    = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) ? true : false;
  state.hats[0][CJoystickHat::RIGHT] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) ? true : false;
  state.hats[0][CJoystickHat::DOWN]  = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) ? true : false;
  state.hats[0][CJoystickHat::LEFT]  = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) ? true : false;

  // Combine triggers into a single axis, like DirectInput
  const long triggerAxis = (long)controllerState.Gamepad.bLeftTrigger - (long)controllerState.Gamepad.bRightTrigger;
  state.SetAxis(0, controllerState.Gamepad.sThumbLX, MAX_AXIS);
  state.SetAxis(1, -controllerState.Gamepad.sThumbLY, MAX_AXIS);
  state.SetAxis(2, triggerAxis, MAX_TRIGGER); 
  state.SetAxis(3, controllerState.Gamepad.sThumbRX, MAX_AXIS);
  state.SetAxis(4, -controllerState.Gamepad.sThumbRY, MAX_AXIS);

  UpdateState(state);
}
