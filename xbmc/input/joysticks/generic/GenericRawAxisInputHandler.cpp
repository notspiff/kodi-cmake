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

#include "GenericRawAxisInputHandler.h"
#include "input/joysticks/IJoystickRawInputHandler.h"
#include "threads/SystemClock.h"

using namespace XbmcThreads;

#define BUTTON_HOLD_TIMEOUT_MS          1000
#define BUTTON_DOUBLE_PRESS_TIMEOUT_MS  500
#define ANALOG_DIGITAL_THRESHOLD        0.5f // TODO

#ifndef ABS
  #define ABS(x)  ((x) >= 0 ? (x) : -(x))
#endif

CGenericRawAxisInputHandler::CGenericRawAxisInputHandler(unsigned int index, IJoystickRawInputHandler* joystickHandler)
  : m_index(index),
    m_joystickHandler(joystickHandler),
    m_state(),
    m_direction(),
    m_activatedTimeMs(0)
{
  m_holdTimer = new CTimer(this);
}

CGenericRawAxisInputHandler::~CGenericRawAxisInputHandler()
{
  delete m_holdTimer;
}

bool CGenericRawAxisInputHandler::OnMotion(float newValue)
{
  m_joystickHandler->OnRawAxisMotion(m_index, newValue);

  const unsigned int motionTimeMs = SystemClockMillis();

  SemiAxisDirection newDirection = SemiAxisDirectionUnknown;
  if (ABS(newValue) >= ANALOG_DIGITAL_THRESHOLD)
    newDirection = (newValue > 0 ? SemiAxisDirectionPositive : SemiAxisDirectionNegative);

  switch (m_state)
  {
    case Released:
      if (newDirection)
      {
        if (motionTimeMs - m_activatedTimeMs <= BUTTON_DOUBLE_PRESS_TIMEOUT_MS)
        {
          m_activatedTimeMs = 0;
          OnDoublePress(newDirection);
        }
        else
        {
          m_activatedTimeMs = motionTimeMs;
          OnPress(newDirection);
        }

        m_holdTimer->Start(BUTTON_HOLD_TIMEOUT_MS);
      }
      break;

    default:
      if (!newDirection)
      {
        OnRelease();
        m_holdTimer->Stop(true);
      }
      else if (newDirection != m_direction)
      {
        // Axis must have passed through origin
        OnRelease();

        m_activatedTimeMs = motionTimeMs;
        OnPress(newDirection);
        m_holdTimer->Stop(true);
        m_holdTimer->Start(BUTTON_HOLD_TIMEOUT_MS);
      }
      break;
  }

  m_direction = newDirection;
  return true;
}

void CGenericRawAxisInputHandler::OnTimeout()
{
  switch (m_state)
  {
    case Pressed:
      OnHold(m_direction);
      break;

    case DoublePressed:
      OnDoublePressHold(m_direction);
      break;

    default:
      break;
  }
}

bool CGenericRawAxisInputHandler::OnPress(SemiAxisDirection direction)
{
  m_state = Pressed;
  return m_joystickHandler->OnRawAxisPress(m_index, direction);
}

bool CGenericRawAxisInputHandler::OnHold(SemiAxisDirection direction)
{
  m_state = Held;
  return m_joystickHandler->OnRawAxisHold(m_index, direction);
}

bool CGenericRawAxisInputHandler::OnDoublePress(SemiAxisDirection direction)
{
  m_state = DoublePressed;
  return m_joystickHandler->OnRawAxisDoublePress(m_index, direction);
}

bool CGenericRawAxisInputHandler::OnDoublePressHold(SemiAxisDirection direction)
{
  m_state = DoublePressHeld;
  return m_joystickHandler->OnRawAxisDoublePressHold(m_index, direction);
}

bool CGenericRawAxisInputHandler::OnRelease()
{
  m_state = Released;
  return m_joystickHandler->OnRawAxisRelease(m_index);
}
