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

#include "JoystickTypes.h"

/*!
 * \ingroup joysticks
 * \brief Interface (implements IJoystickInputHandling) defining methods to handle
 *        raw joystick events (buttons, hats, axes).
 *
 * This interface should be implemented on platforms only supporting low level
 * (raw) joystick events for buttons, hats and axes and with no button mapping
 * or gesture recognition logic.
 */
class IJoystickInputHandler
{
public:
  virtual ~IJoystickInputHandler() { }

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
  virtual void OnButtonMotion(unsigned int index, bool bPressed) = 0;

  virtual void OnHatMotion(unsigned int index, HatDirection direction) = 0;

  virtual void OnAxisMotion(unsigned int index, float position) = 0;
};
