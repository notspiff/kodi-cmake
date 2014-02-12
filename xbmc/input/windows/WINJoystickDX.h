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

#pragma once

#include "input/Joystick.h"

#include <string>

class CJoystickDX : public CJoystick
{
public:
  static void Initialize(JoystickArray &joysticks);
  static void Deinitialize(JoystickArray &joysticks);

  virtual ~CJoystickDX() { Release(); }

  virtual void Update();

private:
  CJoystickDX(LPDIRECTINPUTDEVICE8 joystickDevice, const std::string &name, const DIDEVCAPS &devCaps);

  static BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE *pdidInstance, VOID *pContext);
  static BOOL CALLBACK EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE *pdidoi, VOID *pContext);
  static bool IsXInputDevice(const GUID *pGuidProductFromDirectInput);
  bool InitAxes();
  void Release();

  static LPDIRECTINPUT8 m_pDirectInput;
  LPDIRECTINPUTDEVICE8  m_joystickDevice;
};
