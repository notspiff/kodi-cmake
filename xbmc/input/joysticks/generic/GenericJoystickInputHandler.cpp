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

#define ANALOG_DIGITAL_THRESHOLD  0.5f

CGenericJoystickInputHandler::CGenericJoystickInputHandler(IJoystickActionHandler *handler, IJoystickButtonMap* buttonMap)
 : m_handler(handler),
   m_buttonMap(buttonMap)
{
}

void CGenericJoystickInputHandler::OnButtonMotion(unsigned int index, bool bPressed)
{
  const char pressed = bPressed ? 1 : 0;

  if (m_buttonStates.size() <= index)
    m_buttonStates.resize(index + 1);

  if (m_buttonStates[index] == pressed)
    return;
  
  char& oldState = m_buttonStates[index];

  CButtonPrimitive button(index);
  JoystickActionID action = m_buttonMap->GetAction(button);

  if (action)
  {
    if (!oldState && pressed)
      m_handler->OnButtonPress(action);
    else if (oldState && !pressed)
      m_handler->OnButtonRelease(action);
  }

  oldState = pressed;
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

  float& oldPosition = m_axisStates[index];

  // If the action ID requires multiple axes (analog stick, accelerometer, etc),
  // then both positive and negative axis directions will resolve to the same
  // ID, so we start with the positive direction.
  //
  // If the action ID corresponds to a semi-axis, then the axis's positive and
  // negative directions may correspond to different IDs. (For example, in
  // DirectInput, axis 3 in the positive direction is right trigger, axis 3 in
  // the negative direction is left trigger.) In this case, we need the ID for
  // both directions so we can invoke callbacks on both IDs when the position
  // changes from positive to negative and back without passing through zero.
  CButtonPrimitive positiveAxis(index, SemiAxisDirectionPositive);
  JoystickActionID positiveAction = m_buttonMap->GetAction(positiveAxis);

  switch (positiveAction)
  {
  case JOY_ID_ANALOG_STICK_L:
  case JOY_ID_ANALOG_STICK_R:
  {
    int  horizIndex;
    bool horizInverted;
    int  vertIndex;
    bool vertInverted;

    if (m_buttonMap->GetAnalogStick(positiveAction,
                                    horizIndex, horizInverted,
                                    vertIndex,  vertInverted))
    {
      const float horizPos = GetAxisState(horizIndex) * (horizInverted ? -1.0f : 1.0f);
      const float vertPos  = GetAxisState(vertIndex)  * (vertInverted  ? -1.0f : 1.0f);
      m_handler->OnAnalogStickMotion(positiveAction, horizPos, vertPos);
    }
    break;
  }

  case JOY_ID_ACCELEROMETER:
  {
    int  xIndex;
    bool xInverted;
    int  yIndex;
    bool yInverted;
    int  zIndex;
    bool zInverted;

    if (m_buttonMap->GetAccelerometer(positiveAction,
                                      xIndex, xInverted,
                                      yIndex, yInverted,
                                      zIndex, zInverted))
    {
      const float xPos = GetAxisState(xIndex) * (xInverted ? -1.0f : 1.0f);
      const float yPos = GetAxisState(yIndex) * (yInverted ? -1.0f : 1.0f);
      const float zPos = GetAxisState(zIndex) * (zInverted ? -1.0f : 1.0f);
      m_handler->OnAccelerometerMotion(positiveAction, xPos, yPos, zPos);
    }
    break;
  }

  default:
  {
    // Axis might be overloaded in the positive and negative directions, so
    // lookup the negative direction too
    CButtonPrimitive negativeAxis(index, SemiAxisDirectionNegative);
    JoystickActionID negativeAction = m_buttonMap->GetAction(negativeAxis);

    if (positiveAction)
    {
      // Notify callback of positive motion. Also, passing through zero will
      // emit a 0.0f position exactly once until next positive motion.
      if (position > 0)
        m_handler->OnButtonMotion(positiveAction, position);
      else if (oldPosition > 0)
        m_handler->OnButtonMotion(positiveAction, 0.0f);
    }

    if (negativeAction)
    {
      // Notify callback of negative motion. Also, passing through zero will
      // emit a 0.0f position exactly once until next negative motion.
      if (position < 0)
        m_handler->OnButtonMotion(negativeAction, -1.0f * position); // magnitude is >= 0
      else if (oldPosition < 0)
        m_handler->OnButtonMotion(negativeAction, 0.0f);
    }

    break;
  }
  }

  oldPosition = position;
}

float CGenericJoystickInputHandler::GetAxisState(int axisIndex) const
{
  return (0 <= axisIndex && axisIndex < (int)m_axisStates.size()) ? m_axisStates[axisIndex] : 0;
}
