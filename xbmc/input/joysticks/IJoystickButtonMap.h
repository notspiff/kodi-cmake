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
#pragma once

#include "ButtonPrimitive.h"

#include <stdint.h>
#include <string>

/*!
 * \ingroup joysticks
 *
 * \brief IJoystickButtonMap
 */
class IJoystickButtonMap
{
public:
  virtual ~IJoystickButtonMap() { }
  
  virtual bool Load(void) = 0;

  virtual JoystickActionID GetAction(const CButtonPrimitive& button) = 0;

  virtual bool GetButtonPrimitive(JoystickActionID id, CButtonPrimitive& button) = 0;

  virtual bool GetAnalogStick(JoystickActionID id, int& horizIndex, bool& horizInverted,
                                                    int& vertIndex,  bool& vertInverted) = 0;

  virtual bool GetAccelerometer(JoystickActionID id, int& xIndex, bool& xInverted,
                                                      int& yIndex, bool& yInverted,
                                                      int& zIndex, bool& zInverted) = 0;
};
