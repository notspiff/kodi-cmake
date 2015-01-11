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

#include "GenericJoystickInputHandler.h"
#include "input/joysticks/ButtonPrimitive.h"
#include "input/joysticks/IJoystickActionHandler.h"
#include "input/joysticks/IJoystickButtonMap.h"

#include <algorithm>

#define ANALOG_DIGITAL_THRESHOLD  0.5f

CGenericJoystickInputHandler::CGenericJoystickInputHandler(IJoystickActionHandler *handler, IJoystickButtonMap* buttonMap)
 : m_handler(handler),
   m_buttonMap(buttonMap)
{
}

void CGenericJoystickInputHandler::OnButtonMotion(unsigned int index, bool bPressed)
{
  CButtonPrimitive button(index);
  JoystickActionID action = m_buttonMap->GetAction(button);

  if (action)
  {
    if (bPressed)
      m_handler->OnButtonPress(action);
    else
      m_handler->OnButtonRelease(action);
  }
}

void CGenericJoystickInputHandler::OnHatMotion(unsigned int index, HatDirection direction)
{
  if (m_hatStates.size() <= index)
    m_hatStates.resize(index + 1);

  HatDirection& oldDirection = m_hatStates[index];

  // Check left press/release
  if (!(oldDirection & HatDirectionLeft) && (direction & HatDirectionLeft))
  {
    CButtonPrimitive left(index, HatDirectionLeft);
    JoystickActionID action = m_buttonMap->GetAction(left);
    if (action)
      m_handler->OnButtonPress(action);
  }
  else if ((oldDirection & HatDirectionLeft) && !(direction & HatDirectionLeft))
  {
    CButtonPrimitive left(index, HatDirectionLeft);
    JoystickActionID action = m_buttonMap->GetAction(left);
    if (action)
      m_handler->OnButtonRelease(action);
  }

  // Check right press/release
  if (!(oldDirection & HatDirectionRight) && (direction & HatDirectionRight))
  {
    CButtonPrimitive right(index, HatDirectionRight);
    JoystickActionID action = m_buttonMap->GetAction(right);
    if (action)
      m_handler->OnButtonPress(action);
  }
  else if ((oldDirection & HatDirectionRight) && !(direction & HatDirectionRight))
  {
    CButtonPrimitive right(index, HatDirectionRight);
    JoystickActionID action = m_buttonMap->GetAction(right);
    if (action)
      m_handler->OnButtonRelease(action);
  }

  // Check up press/release
  if (!(oldDirection & HatDirectionUp) && (direction & HatDirectionUp))
  {
    CButtonPrimitive up(index, HatDirectionUp);
    JoystickActionID action = m_buttonMap->GetAction(up);
    if (action)
      m_handler->OnButtonPress(action);
  }
  else if ((oldDirection & HatDirectionUp) && !(direction & HatDirectionUp))
  {
    CButtonPrimitive up(index, HatDirectionUp);
    JoystickActionID action = m_buttonMap->GetAction(up);
    if (action)
      m_handler->OnButtonRelease(action);
  }

  // Check down press/release
  if (!(oldDirection & HatDirectionDown) && (direction & HatDirectionDown))
  {
    CButtonPrimitive down(index, HatDirectionDown);
    JoystickActionID action = m_buttonMap->GetAction(down);
    if (action)
      m_handler->OnButtonPress(action);
  }
  else if ((oldDirection & HatDirectionDown) && !(direction & HatDirectionDown))
  {
    CButtonPrimitive down(index, HatDirectionDown);
    JoystickActionID action = m_buttonMap->GetAction(down);
    if (action)
      m_handler->OnButtonRelease(action);
  }

  oldDirection = direction;
}

void CGenericJoystickInputHandler::OnAxisMotion(unsigned int index, float position)
{
  if (m_axisStates.size() <= index)
    m_axisStates.resize(index + 1);

  if (m_axisStates[index] == position)
    return;

  m_axisStates[index] = position;

  CButtonPrimitive positiveAxis(index, SemiAxisDirectionPositive);
  CButtonPrimitive negativeAxis(index, SemiAxisDirectionNegative);

  JoystickActionID positiveAction = m_buttonMap->GetAction(positiveAxis);
  JoystickActionID negativeAction = m_buttonMap->GetAction(negativeAxis);

  if (positiveAction)
    m_handler->OnButtonMotion(positiveAction, std::max(position, 0.0f));

  if (negativeAction)
    m_handler->OnButtonMotion(negativeAction, -1.0f * std::min(position, 0.0f)); // magnitude is >= 0

  if (!(positiveAction || negativeAction))
  {
    unsigned int indexHoriz, indexVert;
    JoystickActionID action = m_buttonMap->GetAnalogStick(index, indexHoriz, indexVert);
    if (action)
    {
      m_handler->OnAnalogStickMotion(action, GetAxisState(indexHoriz), GetAxisState(indexVert));
    }
    else
    {
      unsigned int indexX, indexY, indexZ;
      JoystickActionID action = m_buttonMap->GetAccelerometer(index, indexX, indexY, indexZ);
      if (action)
        m_handler->OnAccelerometerMotion(action, GetAxisState(indexX), GetAxisState(indexY), GetAxisState(indexZ));
    }
  }
}

float CGenericJoystickInputHandler::GetAxisState(unsigned int axisIndex) const
{
  return axisIndex < m_axisStates.size() ? m_axisStates[axisIndex] : 0;
}
