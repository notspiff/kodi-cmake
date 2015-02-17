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
#include "input/joysticks/InputPrimitive.h"
#include "input/joysticks/IJoystickFeatureHandler.h"
#include "input/joysticks/IJoystickButtonMap.h"
#include "input/joysticks/JoystickTypes.h"
#include "utils/log.h"

#include <algorithm>

#define ANALOG_DIGITAL_THRESHOLD  0.5f

CGenericJoystickInputHandler::CGenericJoystickInputHandler(IJoystickFeatureHandler *handler, IJoystickButtonMap* buttonMap)
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

  CInputPrimitive button(index);
  JoystickFeatureID feature = m_buttonMap->GetFeature(button);

  if (feature != JOY_ID_BUTTON_UNKNOWN)
  {
    CLog::Log(LOGDEBUG, "CGenericJoystickInputHandler: Feature %d %s",
              feature, bPressed ? "pressed" : "released");

    if (!oldState && pressed)
      m_handler->OnButtonPress(feature);
    else if (oldState && !pressed)
      m_handler->OnButtonRelease(feature);
  }
  else if (bPressed)
  {
    CLog::Log(LOGDEBUG, "CGenericJoystickInputHandler: No feature for button %u",
              index);
  }

  oldState = pressed;
}

void CGenericJoystickInputHandler::OnHatMotion(unsigned int index, HatDirection newDirection)
{
  if (m_hatStates.size() <= index)
    m_hatStates.resize(index + 1);

  HatDirection& oldDirection = m_hatStates[index];

  ProcessHatDirection(index, oldDirection, newDirection, HatDirectionUp);
  ProcessHatDirection(index, oldDirection, newDirection, HatDirectionRight);
  ProcessHatDirection(index, oldDirection, newDirection, HatDirectionDown);
  ProcessHatDirection(index, oldDirection, newDirection, HatDirectionLeft);

  oldDirection = newDirection;
}

void CGenericJoystickInputHandler::ProcessHatDirection(int index,
    HatDirection oldDir, HatDirection newDir, HatDirection targetDir)
{
  if ((oldDir & targetDir) == HatDirectionNone &&
      (newDir & targetDir) != HatDirectionNone)
  {
    CInputPrimitive left(index, HatDirectionLeft);
    JoystickFeatureID feature = m_buttonMap->GetFeature(left);
    if (feature != JOY_ID_BUTTON_UNKNOWN)
    {
      CLog::Log(LOGDEBUG, "CGenericJoystickInputHandler: Feature %d activated",
                feature);
      m_handler->OnButtonPress(feature);
    }
    else
    {
      CLog::Log(LOGDEBUG, "CGenericJoystickInputHandler: No feature for hat %u %s",
                index, CJoystickTranslator::HatDirectionToString(targetDir));
    }
  }
  else if ((oldDir & targetDir) != HatDirectionNone &&
           (newDir & targetDir) == HatDirectionNone)
  {
    CInputPrimitive left(index, HatDirectionLeft);
    JoystickFeatureID feature = m_buttonMap->GetFeature(left);
    if (feature != JOY_ID_BUTTON_UNKNOWN)
    {
      CLog::Log(LOGDEBUG, "CGenericJoystickInputHandler: Feature %d deactivated",
                feature);
      m_handler->OnButtonRelease(feature);
    }
  }
}

void CGenericJoystickInputHandler::OnAxisMotion(unsigned int index, float newPosition)
{
  if (m_axisStates.size() <= index)
    m_axisStates.resize(index + 1);

  if (m_axisStates[index] == 0.0f && newPosition == 0.0f)
    return;

  float oldPosition = m_axisStates[index];
  m_axisStates[index] = newPosition;

  CInputPrimitive positiveAxis(index, SemiAxisDirectionPositive);
  CInputPrimitive negativeAxis(index, SemiAxisDirectionNegative);

  JoystickFeatureID positiveFeature = m_buttonMap->GetFeature(positiveAxis);
  JoystickFeatureID negativeFeature = m_buttonMap->GetFeature(negativeAxis);

  if (!positiveFeature && !negativeFeature)
  {
    // No features to send to callback
  }
  else if (positiveFeature == negativeFeature)
  {
    // Feature uses multiple axes, add to OnAxisMotions() batch process
    if (std::find(m_featuresWithMotion.begin(), m_featuresWithMotion.end(), positiveFeature) == m_featuresWithMotion.end())
      m_featuresWithMotion.push_back(positiveFeature);
  }
  else // positiveFeature != negativeFeature
  {
    // Positive and negative directions are mapped to different features, so we
    // must be dealing with a button or trigger

    if (positiveFeature)
    {
      // If new position passes through zero, 0.0f is sent exactly once until
      // the position becomes positive again
      if (newPosition > 0)
        m_handler->OnButtonMotion(positiveFeature, newPosition);
      else if (oldPosition > 0)
        m_handler->OnButtonMotion(positiveFeature, 0.0f);
    }

    if (negativeFeature)
    {
      // If new position passes through zero, 0.0f is sent exactly once until
      // the position becomes negative again
      if (newPosition < 0)
        m_handler->OnButtonMotion(negativeFeature, -1.0f * newPosition); // magnitude is >= 0
      else if (oldPosition < 0)
        m_handler->OnButtonMotion(negativeFeature, 0.0f);
    }
  }
}

void CGenericJoystickInputHandler::ProcessAxisMotions()
{
  std::vector<JoystickFeatureID> featuresToProcess;
  featuresToProcess.swap(m_featuresWithMotion);

  for (std::vector<JoystickFeatureID>::const_iterator it = featuresToProcess.begin(); it != featuresToProcess.end(); ++it)
  {
    const JoystickFeatureID feature = *it;
    switch (feature)
    {
    case JOY_ID_ANALOG_STICK_L:
    case JOY_ID_ANALOG_STICK_R:
    {
      int  horizIndex;
      bool horizInverted;
      int  vertIndex;
      bool vertInverted;

      if (m_buttonMap->GetAnalogStick(feature,
                                      horizIndex, horizInverted,
                                      vertIndex,  vertInverted))
      {
        const float horizPos = GetAxisState(horizIndex) * (horizInverted ? -1.0f : 1.0f);
        const float vertPos  = GetAxisState(vertIndex)  * (vertInverted  ? -1.0f : 1.0f);
        m_handler->OnAnalogStickMotion(feature, horizPos, vertPos);
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

      if (m_buttonMap->GetAccelerometer(feature,
                                        xIndex, xInverted,
                                        yIndex, yInverted,
                                        zIndex, zInverted))
      {
        const float xPos = GetAxisState(xIndex) * (xInverted ? -1.0f : 1.0f);
        const float yPos = GetAxisState(yIndex) * (yInverted ? -1.0f : 1.0f);
        const float zPos = GetAxisState(zIndex) * (zInverted ? -1.0f : 1.0f);
        m_handler->OnAccelerometerMotion(feature, xPos, yPos, zPos);
      }
      break;
    }

    default:
      break;
    }
  }
}

float CGenericJoystickInputHandler::GetAxisState(int axisIndex) const
{
  return (0 <= axisIndex && axisIndex < (int)m_axisStates.size()) ? m_axisStates[axisIndex] : 0;
}
