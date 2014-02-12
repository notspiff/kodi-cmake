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

#pragma once

#include "Joystick.h"
#include "settings/lib/ISettingCallback.h"
#include "threads/SystemClock.h"
#include "utils/Observer.h"

class CAction;

/**
 * Class to manage all connected joysticks.
 */
class CJoystickManager : public ISettingCallback, public Observer
{
private:
  CJoystickManager();

public:
  static CJoystickManager &Get();
  virtual ~CJoystickManager();

  void Reinitialize() { Initialize(); }

  void SetEnabled(bool enabled = true);
  bool IsEnabled() const { return m_bEnabled; }

  void Update();

  void Track(const CAction &action);
  void ResetActionRepeater();

  /**
   * Return the first unused joystick ID (starting from 0). APIs like XInput
   * can ignore this function and force the ID to correspond to player number.
   */
  unsigned int NextID() const;

  // Returns true if this wakes up from the screensaver
  bool Wakeup();

  // Inherited from ISettingCallback
  virtual void OnSettingChanged(const CSetting *setting);

  // Inherited from Observer
  virtual void Notify(const Observable &obs, const ObservableMessage msg);

private:
  /**
   * Track key presses for deferred action repeats.
   */
  class CActionRepeater
  {
  public:
    CActionRepeater() { Reset(); }
    void Reset();
    void Track(const CAction &action);
    void Update();

  private:
    int                  m_actionID; // Action ID, or 0 if not tracking any action
    std::string          m_name;     // Action name
    XbmcThreads::EndTime m_timeout;  // Timeout until action is repeated
  };

  void Initialize();
  void Deinitialize();

  // Allows Wakeup() to perform another wakeup check
  void ResetWakeupCheck() { m_bWakeupChecked = false; }

  bool            m_bEnabled;
  JoystickArray   m_joysticks;
  bool            m_bWakeupChecked; // true if Wakeup() has been called
  CActionRepeater m_actionRepeater;
};
