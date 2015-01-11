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

#include "IJoystickInputHandling.h"

#include <stdlib.h>

IJoystickInputHandling::IJoystickInputHandling(void)
  : m_handler(NULL)
{
}

void IJoystickInputHandling::RegisterHandler(IJoystickRawInputHandler *joystickInputHandler)
{
  m_handler = joystickInputHandler;
}

void IJoystickInputHandling::UnregisterHandler()
{
  m_handler = NULL;
}

bool IJoystickInputHandling::OnButtonPress(JoystickActionID id, bool bPressed)
{
  if (m_handler)
    return m_handler->OnButtonPress(id, bPressed);

  return true;
}

bool IJoystickInputHandling::OnButtonMotion(JoystickActionID id, float magnitude)
{
  if (m_handler)
    return m_handler->OnButtonMotion(id, magnitude);

  return true;
}

bool IJoystickInputHandling::OnAnalogStickMotion(JoystickActionID id, float x, float y)
{
  if (m_handler)
    return m_handler->OnAnalogStickMotion(id, x, y);

  return true;
}

bool IJoystickInputHandling::OnAnalogStickThreshold(JoystickActionID id, bool bPressed, HatDirection direction /* = HatDirectionNone */)
{
  if (m_handler)
    return m_handler->OnAnalogStickThreshold(id, bPressed, direction);

  return true;
}

bool IJoystickInputHandling::OnAccelerometerMotion(JoystickActionID id, float x, float y, float z)
{
  if (m_handler)
    return m_handler->OnAccelerometerMotion(id, x, y, z);

  return true;
}

bool IJoystickInputHandling::OnRawButtonPress(unsigned int index)
{
  if (m_handler)
    return m_handler->OnRawButtonPress(index);

  return true;
}

bool IJoystickInputHandling::OnRawButtonHold(unsigned int index)
{
  if (m_handler)
    return m_handler->OnRawButtonHold(index);

  return true;
}

bool IJoystickInputHandling::OnRawButtonDoublePress(unsigned int index)
{
  if (m_handler)
    return m_handler->OnRawButtonDoublePress(index);

  return true;
}

bool IJoystickInputHandling::OnRawButtonDoublePressHold(unsigned int index)
{
  if (m_handler)
    return m_handler->OnRawButtonDoublePressHold(index);

  return true;
}

bool IJoystickInputHandling::OnRawButtonRelease(unsigned int index)
{
  if (m_handler)
    return m_handler->OnRawButtonRelease(index);

  return true;
}

bool IJoystickInputHandling::OnRawHatPress(unsigned int index, HatDirection direction)
{
  if (m_handler)
    return m_handler->OnRawHatPress(index, direction);

  return true;
}

bool IJoystickInputHandling::OnRawHatMotion(unsigned int index, HatDirection direction)
{
  if (m_handler)
    return m_handler->OnRawHatMotion(index, direction);

  return true;
}

bool IJoystickInputHandling::OnRawHatHold(unsigned int index, HatDirection direction)
{
  if (m_handler)
    return m_handler->OnRawHatHold(index, direction);

  return true;
}

bool IJoystickInputHandling::OnRawHatDoublePress(unsigned int index, HatDirection direction)
{
  if (m_handler)
    return m_handler->OnRawHatDoublePress(index, direction);

  return true;
}

bool IJoystickInputHandling::OnRawHatDoublePressHold(unsigned int index, HatDirection direction)
{
  if (m_handler)
    return m_handler->OnRawHatDoublePressHold(index, direction);

  return true;
}

bool IJoystickInputHandling::OnRawHatRelease(unsigned int index)
{
  if (m_handler)
    return m_handler->OnRawHatRelease(index);

  return true;
}

bool IJoystickInputHandling::OnRawAxisPress(unsigned int index, SemiAxisDirection direction)
{
  if (m_handler)
    return m_handler->OnRawAxisPress(index, direction);

  return true;
}

bool IJoystickInputHandling::OnRawAxisMotion(unsigned int index, float position)
{
  if (m_handler)
    return m_handler->OnRawAxisMotion(index, position);

  return true;
}

bool IJoystickInputHandling::OnRawAxisHold(unsigned int index, SemiAxisDirection direction)
{
  if (m_handler)
    return m_handler->OnRawAxisHold(index, direction);

  return true;
}

bool IJoystickInputHandling::OnRawAxisDoublePress(unsigned int index, SemiAxisDirection direction)
{
  if (m_handler)
    return m_handler->OnRawAxisDoublePress(index, direction);

  return true;
}

bool IJoystickInputHandling::OnRawAxisDoublePressHold(unsigned int index, SemiAxisDirection direction)
{
  if (m_handler)
    return m_handler->OnRawAxisDoublePressHold(index, direction);

  return true;
}

bool IJoystickInputHandling::OnRawAxisRelease(unsigned int index)
{
  if (m_handler)
    return m_handler->OnRawAxisRelease(index);

  return true;
}

bool IJoystickInputHandling::OnRawMultiPress(const std::vector<ButtonPrimitive>& buttons)
{
  if (m_handler)
    return m_handler->OnRawMultiPress(buttons);

  return true;
}
