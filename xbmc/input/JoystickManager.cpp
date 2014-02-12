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

#include "JoystickManager.h"
#include "guilib/Key.h"
#include "Application.h"
#include "peripherals/Peripherals.h"
#include "peripherals/devices/PeripheralImon.h"
#include "settings/lib/Setting.h"
#include "utils/log.h"

// Include joystick APIs
#if defined(TARGET_WINDOWS)
  #include "input/windows/WINJoystickXInput.h"
  #include "input/windows/WINJoystickDX.h"
#else
  #if defined(HAS_LINUX_JOYSTICK)
    #include "input/linux/LinuxJoystick.h"
  #elif defined(HAS_SDL_JOYSTICK)
    #include "input/linux/LinuxJoystickSDL.h"
  #endif
#endif

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(x)  (sizeof((x)) / sizeof((x)[0]))
#endif

#define ACTION_FIRST_DELAY      500 // ms
#define ACTION_REPEAT_DELAY     100 // ms

using namespace PERIPHERALS;
using namespace std;

void CJoystickManager::CActionRepeater::Reset()
{
  m_actionID = 0;
  m_name.clear();
  m_timeout.SetInfinite();
}

void CJoystickManager::CActionRepeater::Track(const CAction &action)
{
  if (m_actionID != action.GetID())
  {
    // A new button was pressed, start tracking it
    m_actionID = action.GetID();
    m_name = action.GetName();
    m_timeout.Set(ACTION_FIRST_DELAY);
  }
  else
  {
    // Already tracking the action, update timer if expired
    if (m_timeout.IsTimePast())
      m_timeout.Set(ACTION_REPEAT_DELAY);
  }
}

void CJoystickManager::CActionRepeater::Update()
{
  // If tracking an action and the time has elapsed, execute the action now
  if (m_actionID && m_timeout.IsTimePast())
  {
    CAction action(m_actionID, 1.0f, 0.0f, m_name);
    g_application.ExecuteInputAction(action);
    Track(action); // Update the timer
  }
}

CJoystickManager::CJoystickManager()
 : m_bEnabled(false),
   m_bWakeupChecked(false)
{
  PERIPHERALS::CPeripherals::Get().RegisterObserver(this);
}

CJoystickManager &CJoystickManager::Get()
{
  static CJoystickManager joystickManager;
  return joystickManager;
}

CJoystickManager::~CJoystickManager()
{
  PERIPHERALS::CPeripherals::Get().UnregisterObserver(this);
  Deinitialize();
}

void CJoystickManager::Initialize()
{
  if (!IsEnabled())
    return;

  Deinitialize();

  // Initialize joystick APIs
#if defined(TARGET_WINDOWS)
  // Initialize XInput before DX. DX knows what to do if XInput joysticks are encountered
  CJoystickXInput::Initialize(m_joysticks);
  CJoystickDX::Initialize(m_joysticks);
#else
  #if defined(HAS_LINUX_JOYSTICK)
    CLinuxJoystick::Initialize(m_joysticks);
  #elif defined(HAS_SDL_JOYSTICK)
    CLinuxJoystickSDL::Initialize(m_joysticks);
  #endif
#endif
}

void CJoystickManager::Deinitialize()
{
  // De-initialize joystick APIs
#if defined(TARGET_WINDOWS)
  CJoystickXInput::Deinitialize(m_joysticks);
  CJoystickDX::Deinitialize(m_joysticks);
#else
  #if defined(HAS_LINUX_JOYSTICK)
    CLinuxJoystick::Deinitialize(m_joysticks);
  #elif defined(HAS_SDL_JOYSTICK)
    CLinuxJoystickSDL::Deinitialize(m_joysticks);
  #endif
#endif

  m_joysticks.clear();

  ResetActionRepeater();
}

void CJoystickManager::Update()
{
  if (!IsEnabled() || m_joysticks.empty())
    return;

  for (JoystickArray::iterator it = m_joysticks.begin(); it != m_joysticks.end(); it++)
    (*it)->Update();

  m_actionRepeater.Update();

  // Reset the wakeup check, so that the check will be performed for the next button press also
  ResetWakeupCheck();
}

void CJoystickManager::ResetActionRepeater()
{
  m_actionRepeater.Reset();
}

void CJoystickManager::Track(const CAction &action)
{
  m_actionRepeater.Track(action);
}

unsigned int CJoystickManager::NextID() const
{
  for (unsigned int id = 0; id < m_joysticks.size(); id++)
  {
    // Look for ID
    bool bFound = false;
    for (JoystickArray::const_iterator it = m_joysticks.begin(); it != m_joysticks.end(); ++it)
    {
      if (id == (*it)->ID())
      {
        bFound = true;
        break;
      }
    }
    // If ID isn't in use, we found our next ID
    if (!bFound)
      return id;
  }
  return m_joysticks.size();
}

bool CJoystickManager::Wakeup()
{
  static bool bWokenUp = false;

  // Refresh bWokenUp after every call to ResetWakeup() (which sets m_bWakeupChecked to false)
  if (!m_bWakeupChecked)
  {
    m_bWakeupChecked = true;

    // Reset the timers and check to see if we have woken the application
    g_application.ResetSystemIdleTimer();
    g_application.ResetScreenSaver();
    bWokenUp = g_application.WakeUpScreenSaverAndDPMS();
  }
  return bWokenUp;
}

void CJoystickManager::SetEnabled(bool enabled /* = true */)
{
  if (enabled && !m_bEnabled)
  {
    m_bEnabled = true;
    Initialize();
  }
  else if (!enabled && m_bEnabled)
  {
    Deinitialize();
    m_bEnabled = false;
  }
}

void CJoystickManager::OnSettingChanged(const CSetting *setting)
{
  if (setting == NULL)
    return;

  const std::string &settingId = setting->GetId();
  if (settingId == "input.enablejoystick")
  {
    SetEnabled(((CSettingBool*)setting)->GetValue() && CPeripheralImon::GetCountOfImonsConflictWithDInput() == 0);
  }
}

void CJoystickManager::Notify(const Observable &obs, const ObservableMessage msg)
{
  if (msg == ObservableMessagePeripheralsChanged)
    Reinitialize();
}
