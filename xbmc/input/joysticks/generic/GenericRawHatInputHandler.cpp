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

#include "GenericRawHatInputHandler.h"
#include "input/joysticks/IJoystickRawInputHandler.h"
#include "threads/SystemClock.h"

using namespace XbmcThreads;

#define BUTTON_HOLD_TIMEOUT_MS          1000
#define BUTTON_DOUBLE_PRESS_TIMEOUT_MS  500

CGenericRawHatInputHandler::CGenericRawHatInputHandler(unsigned int index, IJoystickRawInputHandler* joystickHandler)
  : m_index(index),
    m_joystickHandler(joystickHandler),
    m_state(),
    m_direction(),
    m_activatedTimeMs(0)
{
  m_holdTimer = new CTimer(this);
}

CGenericRawHatInputHandler::~CGenericRawHatInputHandler()
{
  delete m_holdTimer;
}

bool CGenericRawHatInputHandler::OnMotion(HatDirection newValue)
{
  const unsigned int motionTimeMs = SystemClockMillis();

  switch (m_state)
  {
    case Released:
      if (newValue)
      {
        if (motionTimeMs - m_activatedTimeMs <= BUTTON_DOUBLE_PRESS_TIMEOUT_MS)
        {
          m_activatedTimeMs = 0;
          OnDoublePress(newValue);
        }
        else
        {
          m_activatedTimeMs = motionTimeMs;
          OnPress(newValue);
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
      else if (newValue != m_direction)
      {
        m_activatedTimeMs = motionTimeMs;
        OnDirectionChange(newValue);
        m_holdTimer->Stop(true);
        m_holdTimer->Start(BUTTON_HOLD_TIMEOUT_MS);
      }
      break;
  }

  m_direction = newValue;
  return true;
}

void CGenericRawHatInputHandler::OnTimeout()
{
  switch (m_state)
  {
    case Pressed:
    case Moved:
      OnHold(m_direction);
      break;

    case DoublePressed:
      OnDoublePressHold(m_direction);
      break;

    default:
      break;
  }
}

bool CGenericRawHatInputHandler::OnPress(HatDirection direction)
{
  m_state = Pressed;
  return m_joystickHandler->OnRawHatPress(m_index, direction);
}

bool CGenericRawHatInputHandler::OnDirectionChange(HatDirection direction)
{
  m_state = Moved;
  return m_joystickHandler->OnRawHatMotion(m_index, direction);
}

bool CGenericRawHatInputHandler::OnHold(HatDirection direction)
{
  m_state = Held;
  return m_joystickHandler->OnRawHatHold(m_index, direction);
}

bool CGenericRawHatInputHandler::OnDoublePress(HatDirection direction)
{
  m_state = DoublePressed;
  return m_joystickHandler->OnRawHatDoublePress(m_index, direction);
}

bool CGenericRawHatInputHandler::OnDoublePressHold(HatDirection direction)
{
  m_state = DoublePressHeld;
  return m_joystickHandler->OnRawHatDoublePressHold(m_index, direction);
}

bool CGenericRawHatInputHandler::OnRelease()
{
  m_state = Released;
  return m_joystickHandler->OnRawHatRelease(m_index);
}
