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

#include "input/joysticks/JoystickTypes.h"
#include "threads/Timer.h"

class IJoystickRawInputHandler;

class CGenericRawAxisInputHandler : private ITimerCallback
{
public:
  CGenericRawAxisInputHandler(unsigned int index, IJoystickRawInputHandler* joystickHandler);

  virtual ~CGenericRawAxisInputHandler();

  unsigned int Index() const { return m_index; }

  bool OnMotion(float newValue);

private:
  // implementation of ITimerCallback
  virtual void OnTimeout();

  bool OnPress(SemiAxisDirection direction);
  bool OnHold(SemiAxisDirection direction);
  bool OnDoublePress(SemiAxisDirection direction);
  bool OnDoublePressHold(SemiAxisDirection direction);
  bool OnRelease();

  enum ButtonState
  {
    Pressed,
    Held,
    DoublePressed,
    DoublePressHeld,
    Released,
  };

  const unsigned int               m_index;
  IJoystickRawInputHandler* const m_joystickHandler;
  ButtonState                      m_state;
  SemiAxisDirection                m_direction;
  unsigned int                     m_activatedTimeMs;
  CTimer                           *m_holdTimer;
};
