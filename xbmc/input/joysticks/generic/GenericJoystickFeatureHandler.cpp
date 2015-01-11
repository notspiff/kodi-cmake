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

#include "GenericJoystickFeatureHandler.h"
#include "guilib/GUIWindowManager.h"
#include "input/ButtonTranslator.h"
#include "input/Key.h"
#include "threads/SingleLock.h"
#include "ApplicationMessenger.h"

#include <algorithm>

#define HOLD_TIMEOUT_MS    500 // TODO
#define REPEAT_TIMEOUT_MS  250 // TODO

#ifndef ABS
#define ABS(x)  ((x) >= 0 ? (x) : (-x))
#endif

#ifndef MAX
#define MAX(x, y)  ((x) >= (y) ? (x) : (y))
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)  (sizeof(x) / sizeof((x)[0]))
#endif

CGenericJoystickFeatureHandler::CGenericJoystickFeatureHandler(void) :
  m_holdTimer(this),
  m_lastButtonPress(0)
{
}

bool CGenericJoystickFeatureHandler::OnButtonPress(JoystickFeatureID id)
{
  unsigned int buttonKeyId = GetButtonKeyID(id);
  if (buttonKeyId != 0)
  {
    CAction action(CButtonTranslator::GetInstance().GetAction(g_windowManager.GetActiveWindowID(), CKey(buttonKeyId, 0)));
    if (action.GetID() > 0)
    {
      if (action.IsAnalog())
      {
        const float amount = 1.0f;
        CAction actionWithAmount(action.GetID(), amount, 0.0f, action.GetName());
        CApplicationMessenger::Get().SendAction(action);
      }
      else
      {
        if (std::find(m_pressedButtons.begin(), m_pressedButtons.end(), buttonKeyId) == m_pressedButtons.end())
          ProcessButtonPress(action);
      }
    }
  }

  return true;
}

bool CGenericJoystickFeatureHandler::OnButtonRelease(JoystickFeatureID id)
{
  unsigned int buttonKeyId = GetButtonKeyID(id);
  if (buttonKeyId != 0)
  {
    CAction action(CButtonTranslator::GetInstance().GetAction(g_windowManager.GetActiveWindowID(), CKey(buttonKeyId, 0)));
    if (action.GetID() > 0)
    {
      if (action.IsAnalog())
      {
        const float amount = 0.0f;
        CAction actionWithAmount(action.GetID(), amount, 0.0f, action.GetName());
        CApplicationMessenger::Get().SendAction(action);
      }
      else
      {
        ProcessButtonRelease(buttonKeyId);
      }
    }
  }

  return true;
}

bool CGenericJoystickFeatureHandler::OnButtonMotion(JoystickFeatureID id, float magnitude)
{
  unsigned int buttonKeyId = GetButtonKeyID(id);
  if (buttonKeyId != 0)
  {
    CAction action(CButtonTranslator::GetInstance().GetAction(g_windowManager.GetActiveWindowID(), CKey(buttonKeyId, 0)));
    if (action.GetID() > 0)
    {
      if (action.IsAnalog())
      {
        CAction actionWithAmount(action.GetID(), magnitude, 0.0f, action.GetName());
        CApplicationMessenger::Get().SendAction(action);
      }
      else
      {
        std::vector<unsigned int>::iterator it = std::find(m_pressedButtons.begin(), m_pressedButtons.end(), buttonKeyId);

        if (magnitude >= 0.5f && it == m_pressedButtons.end())
          ProcessButtonPress(action);
        else if (magnitude < 0.5f && it != m_pressedButtons.end())
          ProcessButtonRelease(buttonKeyId);
      }
    }
  }

  return true;
}

bool CGenericJoystickFeatureHandler::OnAnalogStickMotion(JoystickFeatureID id, float x, float y)
{
  unsigned int buttonKeyId  = GetButtonKeyID(id, x, y);

  float magnitude = MAX(ABS(x), ABS(y));

  unsigned int buttonRightId = GetButtonKeyID(id,  1.0f,  0.0f);
  unsigned int buttonUpId    = GetButtonKeyID(id,  0.0f,  1.0f);
  unsigned int buttonLeftId  = GetButtonKeyID(id, -1.0f,  0.0f);
  unsigned int buttonDownId  = GetButtonKeyID(id,  0.0f, -1.0f);

  unsigned int buttonKeyIds[] = {buttonRightId, buttonUpId, buttonLeftId, buttonDownId};

  for (unsigned int i = 0; i < ARRAY_SIZE(buttonKeyIds); i++)
  {
    if (!buttonKeyIds[i])
      continue;

    CAction action(CButtonTranslator::GetInstance().GetAction(g_windowManager.GetActiveWindowID(), CKey(buttonKeyId, 0)));
    if (action.GetID() > 0)
    {
      if (action.IsAnalog())
      {
        if (buttonKeyId == buttonKeyIds[i])
        {
          CAction actionWithAmount(action.GetID(), magnitude, 0.0f, action.GetName());
          CApplicationMessenger::Get().SendAction(action);
        }
      }
    }
    else
    {
      std::vector<unsigned int>::iterator it = std::find(m_pressedButtons.begin(), m_pressedButtons.end(), buttonKeyIds[i]);

      if (buttonKeyId == buttonKeyIds[i])
      {
        if (magnitude >= 0.5f && it == m_pressedButtons.end())
          ProcessButtonPress(action);
        else if (magnitude < 0.5f && it != m_pressedButtons.end())
          ProcessButtonRelease(buttonKeyId);
      }
      else if (it != m_pressedButtons.end())
      {
        ProcessButtonRelease(buttonKeyId);
      }
    }
  }

  return true;
}

bool CGenericJoystickFeatureHandler::OnAccelerometerMotion(JoystickFeatureID id, float x, float y, float z)
{
  return OnAnalogStickMotion(id, x, y); // TODO
}

void CGenericJoystickFeatureHandler::OnTimeout(void)
{
  CSingleLock lock(m_digitalMutex);

  const unsigned int holdTimeMs = (unsigned int)m_holdTimer.GetElapsedMilliseconds();

  if (m_lastButtonPress != 0 && holdTimeMs >= HOLD_TIMEOUT_MS)
  {
    CAction action(CButtonTranslator::GetInstance().GetAction(g_windowManager.GetActiveWindowID(), CKey(m_lastButtonPress, holdTimeMs)));
    if (action.GetID() > 0)
      CApplicationMessenger::Get().SendAction(action);
  }
}

void CGenericJoystickFeatureHandler::ProcessButtonPress(const CAction& action)
{
  ClearHoldTimer();

  CApplicationMessenger::Get().SendAction(action);

  CSingleLock lock(m_digitalMutex);

  m_pressedButtons.push_back(action.GetButtonCode());
  StartHoldTimer(action.GetButtonCode());
}

void CGenericJoystickFeatureHandler::ProcessButtonRelease(unsigned int buttonKeyId)
{
  std::vector<unsigned int>::iterator it = std::find(m_pressedButtons.begin(), m_pressedButtons.end(), buttonKeyId);
  if (it != m_pressedButtons.end())
    m_pressedButtons.erase(it);

  if (buttonKeyId == m_lastButtonPress || m_pressedButtons.empty())
    ClearHoldTimer();
}

void CGenericJoystickFeatureHandler::StartHoldTimer(unsigned int buttonKeyId)
{
  m_holdTimer.Start(REPEAT_TIMEOUT_MS, true);
  m_lastButtonPress = buttonKeyId;
}

void CGenericJoystickFeatureHandler::ClearHoldTimer(void)
{
  m_holdTimer.Stop(true);
  m_lastButtonPress = 0;
}

unsigned int CGenericJoystickFeatureHandler::GetButtonKeyID(JoystickFeatureID id, float x /* = 0.0f */, float y /* = 0.0f */, float z /* = 0.0f */)
{
  switch (id)
  {
  case JOY_ID_BUTTON_A:
    return KEY_BUTTON_A;
  case JOY_ID_BUTTON_B:
    return KEY_BUTTON_B;
  case JOY_ID_BUTTON_X:
    return KEY_BUTTON_X;
  case JOY_ID_BUTTON_Y:
    return KEY_BUTTON_Y;
  case JOY_ID_BUTTON_C:
    return KEY_BUTTON_BLACK;
  case JOY_ID_BUTTON_Z:
    return KEY_BUTTON_WHITE;
  case JOY_ID_BUTTON_START:
    return KEY_BUTTON_START;
  case JOY_ID_BUTTON_SELECT:
    return KEY_BUTTON_BACK;
  case JOY_ID_BUTTON_MODE:
    return KEY_BUTTON_GUIDE;
  case JOY_ID_BUTTON_L:
    return KEY_BUTTON_LEFT_SHOULDER;
  case JOY_ID_BUTTON_R:
    return KEY_BUTTON_RIGHT_SHOULDER;
  case JOY_ID_TRIGGER_L:
    return KEY_BUTTON_LEFT_TRIGGER;
  case JOY_ID_TRIGGER_R:
    return KEY_BUTTON_RIGHT_TRIGGER;
  case JOY_ID_BUTTON_L_STICK:
    return KEY_BUTTON_LEFT_THUMB_BUTTON;
  case JOY_ID_BUTTON_R_STICK:
    return KEY_BUTTON_RIGHT_THUMB_BUTTON;
  case JOY_ID_BUTTON_LEFT:
    return KEY_BUTTON_DPAD_LEFT;
  case JOY_ID_BUTTON_RIGHT:
    return KEY_BUTTON_DPAD_RIGHT;
  case JOY_ID_BUTTON_UP:
    return KEY_BUTTON_DPAD_UP;
  case JOY_ID_BUTTON_DOWN:
    return KEY_BUTTON_DPAD_DOWN;
  case JOY_ID_ANALOG_STICK_L:
    if (y >= x && y > -x)
      return KEY_BUTTON_LEFT_THUMB_STICK_UP;
    else if (y < x && y >= -x)
      return KEY_BUTTON_LEFT_THUMB_STICK_RIGHT;
    else if (y <= x && y < -x)
      return KEY_BUTTON_LEFT_THUMB_STICK_DOWN;
    else if (y > x && y <= -x)
      return KEY_BUTTON_LEFT_THUMB_STICK_LEFT;
    break;
  case JOY_ID_ANALOG_STICK_R:
    if (y >= x && y > -x)
      return KEY_BUTTON_RIGHT_THUMB_STICK_UP;
    else if (y < x && y >= -x)
      return KEY_BUTTON_RIGHT_THUMB_STICK_RIGHT;
    else if (y <= x && y < -x)
      return KEY_BUTTON_RIGHT_THUMB_STICK_DOWN;
    else if (y > x && y <= -x)
      return KEY_BUTTON_RIGHT_THUMB_STICK_LEFT;
    break;
  case JOY_ID_ACCELEROMETER:
    return 0; // TODO
  case JOY_ID_BUTTON_UNKNOWN:
  default:
    break;
  }

  return 0;
}
