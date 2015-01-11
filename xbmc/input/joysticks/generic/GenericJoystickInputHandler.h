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

#include "input/joysticks/IJoystickInputHandler.h"

#include <vector>

class IJoystickActionHandler;
class IJoystickButtonMap;

/*!
 * \ingroup joysticks_generic
 * \brief Generic implementation of IJoystickInputHandler to handle low level (raw)
 *        joystick events and translate them into joystick actions which are passed
 *        on to the registered IJoystickActionHandler implementation.
 *
 * The generic implementation supports single a double touch and hold
 * actions and basic gesture recognition for panning, swiping, pinching/zooming
 * and rotating.
 *
 * \sa IJoystickInputHandler
 */
class CGenericJoystickInputHandler : public IJoystickInputHandler
{
public:
  CGenericJoystickInputHandler(IJoystickActionHandler *handler, IJoystickButtonMap *buttonMap);

  virtual ~CGenericJoystickInputHandler() { }



  // implementation of IJoystickInputHandler
  virtual void OnButtonMotion(unsigned int index, bool bPressed);
  virtual void OnHatMotion(unsigned int index, HatDirection direction);
  virtual void OnAxisMotion(unsigned int index, float position);

private:
  float GetAxisState(unsigned int axisIndex) const;

  IJoystickActionHandler    *m_handler;
  IJoystickButtonMap        *m_buttonMap;
  std::vector<HatDirection> m_hatStates;
  std::vector<float>        m_axisStates;
};
