/*
 *      Copyright (C) 2007-2013 Team XBMC
 *      http://www.xbmc.org
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

#include "Joystick.h"
#include "JoystickManager.h"
#include "ButtonTranslator.h"
#include "IInputHandler.h"
#include "MouseStat.h"
#include "Application.h"
#include "guilib/Key.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"

#include <string.h>

using namespace std;

#ifndef ABS
#define ABS(x)  ((x) >= 0 ? (x) : (-(x)))
#endif

#ifndef CLAMP
#define CLAMP(value, min, max)  ((value) > (max) ? (max) : (value) < (min) ? (min) : (value))
#endif

CJoystick::CJoystickState::CJoystickState(unsigned int buttonCount, unsigned int hatCount, unsigned int axisCount)
{
  buttons.resize(buttonCount);
  hats.resize(hatCount);
  axes.resize(axisCount);
}

void CJoystick::CJoystickState::Reset()
{
  for (vector<bool>::iterator it = buttons.begin(); it != buttons.end(); ++it)
    *it = false;
  for (vector<CJoystickHat>::iterator it = hats.begin(); it != hats.end(); ++it)
    it->Center();
  for (vector<float>::iterator it = axes.begin(); it != axes.end(); ++it)
    *it = 0.0f;
}

void CJoystick::CJoystickState::SetAxis(unsigned int axisIndex, long value, long maxAxisAmount)
{
  if (axisIndex >= axes.size())
    return;

  value = CLAMP(value, -maxAxisAmount, maxAxisAmount);

  const long deadzoneRange = (long)(g_advancedSettings.m_controllerDeadzone * maxAxisAmount);

  if (value > deadzoneRange)
    axes[axisIndex] = (float)(value - deadzoneRange) / (float)(maxAxisAmount - deadzoneRange);
  else if (value < -deadzoneRange)
    axes[axisIndex] = (float)(value + deadzoneRange) / (float)(maxAxisAmount - deadzoneRange);
  else
    axes[axisIndex] = 0.0f;
}

CJoystick::CJoystick(const std::string& strName, unsigned int id, unsigned int buttonCount, unsigned int hatCount, unsigned int axisCount)
 : m_name(strName),
   m_id(id),
   m_state(buttonCount, hatCount, axisCount),
   m_initialState(buttonCount, hatCount, axisCount)
{
}

CJoystick::CJoystickState& CJoystick::InitialState()
{
  m_initialState.Reset();
  return m_initialState;
}

void CJoystick::UpdateState(const CJoystickState& newState)
{
  assert(m_state.buttons.size() == newState.buttons.size());
  assert(m_state.hats.size() == newState.hats.size());
  assert(m_state.axes.size() == newState.axes.size());

  for (unsigned int i = 0; i < m_state.buttons.size() && i < newState.buttons.size(); i++)
    UpdateButton(i, newState.buttons[i]);
  for (unsigned int i = 0; i < m_state.hats.size() && i < newState.hats.size(); i++)
    UpdateHat(i, newState.hats[i]);
  for (unsigned int i = 0; i < m_state.axes.size() && i < newState.axes.size(); i++)
    UpdateAxis(i, newState.axes[i]);
}

void CJoystick::UpdateButton(unsigned int buttonIndex, bool newButton)
{
  IInputHandler* inputHandler = g_application.m_pPlayer->GetInputHandler();

  const bool oldButton = m_state.buttons[buttonIndex];
  if (oldButton == newButton)
    return; // Nothing changed

  m_state.buttons[buttonIndex] = newButton;

  const int buttonId = buttonIndex + 1;
  CLog::Log(LOGDEBUG, "Joystick %d button %d %s", m_id, buttonId, newButton ? "pressed" : "unpressed");

  // Check to see if an action is registered for the button first
  int        actionID;
  CStdString actionName;
  bool       fullrange;
  if (!CButtonTranslator::GetInstance().TranslateJoystickString(g_application.GetActiveWindowID(),
    m_name.c_str(), buttonId, JACTIVE_BUTTON, actionID, actionName, fullrange))
  {
    CLog::Log(LOGDEBUG, "-> Joystick %d button %d no registered action", m_id, buttonId);
    return;
  }

  g_Mouse.SetActive(false);

  // Ignore all button presses during this ProcessStateChanges() if we woke
  // up the screensaver (but always send joypad unpresses)
  if (!CJoystickManager::Get().Wakeup() && newButton)
  {
    CAction action(actionID, 1.0f, 0.0f, actionName);

    if (IsGameControl(actionID))
    {
      if (inputHandler)
        inputHandler->ProcessButtonDown(m_id, buttonIndex, action);
      CJoystickManager::Get().ResetActionRepeater(); // Don't track game control actions
    }
    else
    {
      g_application.ExecuteInputAction(action);
      // Track the button press for deferred repeated execution
      CJoystickManager::Get().Track(action);
    }
  }
  else if (!newButton)
  {
    if (IsGameControl(actionID))
    {
      // Allow game input to record button release
      if (inputHandler)
        inputHandler->ProcessButtonUp(m_id, buttonIndex);
    }
    CJoystickManager::Get().ResetActionRepeater(); // If a button was released, reset the tracker
  }
}

void CJoystick::UpdateHat(unsigned int hatIndex, const CJoystickHat& newHat)
{
  IInputHandler *inputHandler = g_application.m_pPlayer->GetInputHandler();

  CJoystickHat& oldHat = m_state.hats[hatIndex];
  if (oldHat == newHat)
    return;

  const int hatId = hatIndex + 1;
  CLog::Log(LOGDEBUG, "Joystick %d hat %d new direction: %s", m_id, hatId, newHat.GetDirection());

  for (unsigned int directionIndex = 0; directionIndex < 4; directionIndex++) // up, right, down, left
  {
    CJoystickHat::Direction direction = (CJoystickHat::Direction)directionIndex;
    if (oldHat[direction] == newHat[direction])
      continue;
    oldHat[direction] = newHat[direction];

    int        actionID;
    CStdString actionName;
    bool       fullrange;

    // Up is (1 << 0), right (1 << 1), down (1 << 2), left (1 << 3)
    const int buttonID = (1 << directionIndex) << 16 | hatId;
    if (!buttonID || !CButtonTranslator::GetInstance().TranslateJoystickString(g_application.GetActiveWindowID(),
      m_name.c_str(), buttonID, JACTIVE_HAT, actionID, actionName, fullrange))
    {
      static const char *dirs[] = {"UP", "RIGHT", "DOWN", "LEFT"};
      CLog::Log(LOGDEBUG, "-> Joystick %d hat %d direction %s no registered action", m_id, hatId, dirs[directionIndex]);
      continue;
    }

    g_Mouse.SetActive(false);

    // Ignore all button presses during this ProcessStateChanges() if we woke
    // up the screensaver (but always send joypad unpresses)
    if (!CJoystickManager::Get().Wakeup() && newHat[direction])
    {
      CAction action(actionID, 1.0f, 0.0f, actionName);

      if (IsGameControl(actionID))
      {
        if (inputHandler)
          inputHandler->ProcessHatDown(m_id, hatIndex, directionIndex, action);
        CJoystickManager::Get().ResetActionRepeater(); // Don't track game control actions
      }
      else
      {
        g_application.ExecuteInputAction(action);
        // Track the hat press for deferred repeated execution
        CJoystickManager::Get().Track(action);
      }
    }
    else if (!newHat[direction])
    {
      if (IsGameControl(actionID))
      {
        // Allow game input to record hat release
        if (inputHandler)
          inputHandler->ProcessHatUp(m_id, hatIndex, directionIndex);
      }
      // If a hat was released, reset the tracker
      CJoystickManager::Get().ResetActionRepeater();
    }
  }
}

void CJoystick::UpdateAxis(unsigned int axisIndex, float newAxis)
{
  const float oldAxis = m_state.axes[axisIndex];
  const float absNewAxis = ABS(newAxis);
  const float absOldAxis = ABS(oldAxis);

  const int axisId = axisIndex + 1;

  if (absOldAxis == 0.0f && absNewAxis == 0.0f)
    return;

  int        actionID;
  CStdString actionName;
  bool       fullrange;
  // Axis ID is i + 1, and negative if newState.axes[i] < 0
  if (!CButtonTranslator::GetInstance().TranslateJoystickString(g_application.GetActiveWindowID(),
    m_name.c_str(), newAxis >= 0.0f ? axisId : -axisId, JACTIVE_AXIS, actionID, actionName, fullrange))
  {
    return;
  }

  g_Mouse.SetActive(false);

  // Use newAxis as the second amount so subscribers can recover the original value
  CAction action(actionID, fullrange ? (newAxis + 1.0f) / 2.0f : absNewAxis, newAxis, actionName);

  if (!CJoystickManager::Get().Wakeup())
  {
    // For digital event, we treat action repeats like buttons and hats
    if (!CButtonTranslator::IsAnalog(actionID))
    {
      const bool bActiveBefore = (absOldAxis >= AXIS_DIGITAL_DEADZONE);
      const bool bActiveNow = (absNewAxis >= AXIS_DIGITAL_DEADZONE);
      UpdateDigitalAxis(axisIndex, bActiveBefore, bActiveNow, action);
    }
    else
      UpdateAnalogAxis(axisIndex, newAxis, action);
  }

  m_state.axes[axisIndex] = newAxis;
}

void CJoystick::UpdateDigitalAxis(unsigned int axisIndex, bool bActiveBefore, bool bActiveNow, const CAction& action)
{
  IInputHandler *inputHandler = g_application.m_pPlayer->GetInputHandler();

  if (bActiveBefore == bActiveNow)
    return;
  
  const int axisId = axisIndex + 1;
  CLog::Log(LOGDEBUG, "Joystick %d axis %d %s", m_id, axisId, bActiveNow ? "activated" : "deactivated");

  if (bActiveNow)
  {
    if (IsGameControl(action.GetID()))
    {
      if (inputHandler)
      {
        // Because an axis's direction can reverse and the button ID will be
        // given a different action, record the button up event first
        inputHandler->ProcessDigitalAxisUp(m_id, axisIndex);
        inputHandler->ProcessDigitalAxisDown(m_id, axisIndex, action);
      }
      CJoystickManager::Get().ResetActionRepeater(); // Don't track game control actions
    }
    else
    {
      g_application.ExecuteInputAction(action);
      CJoystickManager::Get().Track(action);
    }
  }
  else
  {
    if (IsGameControl(action.GetID()))
    {
      if (inputHandler)
        inputHandler->ProcessDigitalAxisUp(m_id, axisIndex);
    }
    CJoystickManager::Get().ResetActionRepeater();
  }
}

void CJoystick::UpdateAnalogAxis(unsigned int axisIndex, float newAxis, const CAction& action)
{
  IInputHandler *inputHandler = g_application.m_pPlayer->GetInputHandler();
  
  if (ABS(newAxis) == 0.0f)
  {
    const int axisId = axisIndex + 1;
    CLog::Log(LOGDEBUG, "Joystick %d axis %d centered", m_id, axisId);
  }

  if (IsGameControl(action.GetID()))
  {
    if (inputHandler)
      inputHandler->ProcessAnalogAxis(m_id, axisIndex, action);
  }
  else if (newAxis != 0.0f)
    g_application.ExecuteInputAction(action);

  // The presence of analog actions disables others from being tracked
  CJoystickManager::Get().ResetActionRepeater();
}

/* static */
bool CJoystick::IsGameControl(int actionID)
{
  return ACTION_GAME_CONTROL_START <= actionID && actionID <= ACTION_GAME_CONTROL_END;
}
