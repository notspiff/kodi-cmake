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

#include "GenericRawButtonInputHandler.h"
#include "input/joysticks/IJoystickRawInputHandler.h"
#include "threads/SystemClock.h"

using namespace XbmcThreads;

#define BUTTON_HOLD_TIMEOUT_MS          1000
#define BUTTON_DOUBLE_PRESS_TIMEOUT_MS  500

CGenericRawButtonInputHandler::CGenericRawButtonInputHandler(unsigned int index, IJoystickRawInputHandler* joystickHandler)
  : m_index(index),
    m_joystickHandler(joystickHandler),
    m_state(),
    m_activatedTimeMs(0)
{
  m_holdTimer = new CTimer(this);
}

CGenericRawButtonInputHandler::~CGenericRawButtonInputHandler()
{
  delete m_holdTimer;
}

bool CGenericRawButtonInputHandler::OnMotion(bool newValue)
{
  const unsigned int motionTimeMs = SystemClockMillis();

  switch (m_state)
  {
    case Released:
      if (newValue)
      {
        if (motionTimeMs - m_activatedTimeMs <= BUTTON_DOUBLE_PRESS_TIMEOUT_MS)
        {
          OnDoublePress();
          m_activatedTimeMs = 0;
        }
        else
        {
          OnPress();
          m_activatedTimeMs = motionTimeMs;
        }

        m_holdTimer->Start(BUTTON_HOLD_TIMEOUT_MS);
      }
      break;

    default:
      if (!newValue)
      {
        OnRelease();
        m_holdTimer->Stop(true);
      }
      break;
  }

  return true;
}

void CGenericRawButtonInputHandler::OnTimeout()
{
  switch (m_state)
  {
    case Pressed:
      OnHold();
      break;

    case DoublePressed:
      OnDoublePressHold();
      break;

    default:
      break;
  }
}

bool CGenericRawButtonInputHandler::OnPress()
{
  m_state = Pressed;
  return m_joystickHandler->OnRawButtonPress(m_index);
}

bool CGenericRawButtonInputHandler::OnHold()
{
  m_state = Held;
  return m_joystickHandler->OnRawButtonHold(m_index);
}

bool CGenericRawButtonInputHandler::OnDoublePress()
{
  m_state = DoublePressed;
  return m_joystickHandler->OnRawButtonDoublePress(m_index);
}

bool CGenericRawButtonInputHandler::OnDoublePressHold()
{
  m_state = DoublePressHeld;
  return m_joystickHandler->OnRawButtonDoublePressHold(m_index);
}

bool CGenericRawButtonInputHandler::OnRelease()
{
  m_state = Released;
  return m_joystickHandler->OnRawButtonRelease(m_index);
}
