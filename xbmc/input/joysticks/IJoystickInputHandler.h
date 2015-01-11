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
#pragma once

#include "IJoystickInputHandling.h"
#include "windowing/XBMC_events.h"

/*!
 * \ingroup joysticks
 * \brief Events for raw joystick features (buttons, hats and axes)
 */
enum JoystickEvent
{
  JoystickEventRawButton,
  JoystickEventRawHat,
  JoystickEventRawAxis,
};

/*!
 * \ingroup joysticks
 * \brief Events for physical joystick features (TODO)
 */
enum JoystickActionEvent
{
  JoystickActionEventButtonDigital,
  JoystickActionEventButtonAnalog,
  JoystickActionEventAnalogStick,
  JoystickActionEventAnalogStickThreshold,
  JoystickActionEventAccelerometer,
};

/*!
 * \ingroup joysticks
 * \brief Interface (implements IJoystickInputHandling) defining methods to handle
 *        raw joystick events (buttons, hats, axes).
 *
 * This interface should be implemented on platforms only supporting low level
 * (raw) joystick events for buttons, hats and axes and with no button mapping
 * or gesture recognition logic.
 */
class IJoystickInputHandler : public IJoystickInputHandling
{
public:
  IJoystickInputHandler()/*
    : m_buttonCount(0),
      m_hatCount(0),
      m_axisCount(0)*/
  {
  }

  virtual ~IJoystickInputHandler() { }

  /*
  unsigned int ButtonCount(void) const { return m_buttonCount; }
  unsigned int HatCount(void) const    { return m_hatCount; }
  unsigned int AxisCount(void) const   { return m_axisCount; }
  */

  /*
  void SetButtonCount(unsigned int buttonCount) { m_buttonCount = buttonCount; }
  void SetHatCount(unsigned int hatCount)       { m_hatCount    = hatCount; }
  void SetAxisCount(unsigned int axisCount)     { m_axisCount   = axisCount; }
  */

  /*!
   * \brief Handle a joystick event
   *
   * TODO: doxy
   *
   * Handles the given joystick event at the given location.
   * This takes into account all the currently active pointers
   * which need to be updated before calling this method to
   * actually interpret and handle the changes in touch.
   *
   * \param event     The event type (button, hat, axis)
   * \param index     The index of the button/hat/axis
   * \param timeNs    The time (in nanoseconds) when this touch occurred
   * \param bPressed  The state of the button for button events
   * \param direction The state of the hat for hat events
   * \param axisPos   The state of the axis for axis events
   *
   * \return True if the event was handled otherwise false.
   */
  virtual bool HandleJoystickEvent(JoystickEvent event,
                                   unsigned int  index,
                                   int64_t       timeNs,
                                   bool          bPressed  = false,
                                   HatDirection  direction = HatDirectionNone,
                                   float         axisPos   = 0.0f) = 0;

protected:
  /*!
   * \brief Counts of driver features
   */
  /*
  unsigned int m_buttonCount;
  unsigned int m_hatCount;
  unsigned int m_axisCount;
  */
};
