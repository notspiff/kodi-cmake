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

#include "input/joysticks/IJoystickRawInputHandler.h"

/*!
 * \ingroup joysticks_generic
 * \brief Generic implementation of IJoystickActionHandler to translate
 *        joystick actions into XBMC specific and mappable actions.
 *
 * \sa IJoystickRawInputHandler
 */
class CGenericJoystickActionHandler : public IJoystickRawInputHandler // TODO: Extend IJoystickActionHandler
{
public:
  CGenericJoystickActionHandler() { }

  virtual ~CGenericJoystickActionHandler() { }

  // implementation of IJoystickActionHandler
  /*
  virtual bool OnButtonPress(unsigned int id, bool bPressed);
  virtual bool OnButtonMotion(unsigned int id, float magnitude);
  virtual bool OnAnalogStickMotion(unsigned int id, float x, float y);
  virtual bool OnAnalogStickThreshold(unsigned int id, bool bPressed, HatDirection direction = HatDirectionNone);
  virtual bool OnAccelerometerMotion(unsigned int id, float x, float y, float z);
  */
};
