#pragma once
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

#include "JoystickHat.h"

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#define JACTIVE_NONE            0x00000000
#define JACTIVE_BUTTON          0x00000001
#define JACTIVE_HAT             0x00000002
#define JACTIVE_AXIS            0x00000004
#define JACTIVE_HAT_UP          0x01
#define JACTIVE_HAT_RIGHT       0x02
#define JACTIVE_HAT_DOWN        0x04
#define JACTIVE_HAT_LEFT        0x08

#define GAMEPAD_BUTTON_COUNT    32
#define GAMEPAD_HAT_COUNT       4
#define GAMEPAD_AXIS_COUNT      6

#define AXIS_DIGITAL_DEADZONE   0.5f // Axis must be pushed past this for digital action repeats

class CJoystick;
typedef boost::shared_ptr<CJoystick> JoystickPtr;
typedef std::vector<JoystickPtr>     JoystickArray;

class CAction;

/**
 * Joysticks can have buttons, hats and analog axes in the range [-1, 1].
 */
class CJoystick
{
protected:
  CJoystick(const std::string& strName, unsigned int id, unsigned int buttonCount, unsigned int hatCount, unsigned int axisCount);

public:
  virtual ~CJoystick() { }

  virtual void Update() = 0;

  unsigned int ID() const { return m_id; }
  void SetID(unsigned int id) { m_id = id; }

  const std::string& Name() const { return m_name; }

protected:
  class CJoystickState
  {
  public:
    CJoystickState(unsigned int buttonCount, unsigned int hatCount, unsigned int axisCount);
  
    void Reset();

    /**
     * Helper function to normalize a value to maxAxisAmount.
     */
    void SetAxis(unsigned int axisIndex, long value, long maxAxisAmount);

    std::vector<bool>         buttons;
    std::vector<CJoystickHat> hats;
    std::vector<float>        axes;
  };

  // Methods exposed to subclasses

  /**
   * Get a new state with the same number of buttons, hats and axes as our
   * current state. By re-using an existing object we avoid the overhead of
   * allocating a new state on every update.
   */
  CJoystickState& InitialState();

  /**
   * A subclass reports the updated joystick state via UpdateState()
   */
  void UpdateState(const CJoystickState& newState);

private:
  void UpdateButton(unsigned int buttonIndex, bool newButton);
  void UpdateHat(unsigned int hatIndex, const CJoystickHat& newHat);
  void UpdateAxis(unsigned int axisIndex, float newAxis);
  void UpdateDigitalAxis(unsigned int axisIndex, bool bActiveBefore, bool bActiveNow, const CAction& action);
  void UpdateAnalogAxis(unsigned int axisIndex, float newAxis, const CAction& action);

  static bool IsGameControl(int actionID);

  const std::string m_name;
  unsigned int      m_id;
  CJoystickState    m_state;
  CJoystickState    m_initialState; // Initial state of the joystick before any buttons are pressed
};
