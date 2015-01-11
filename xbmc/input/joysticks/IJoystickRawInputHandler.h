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

#include "IJoystickActionHandler.h"

/*!
 * \ingroup joysticks
 * \brief Interface defining all supported actions for raw joystick elements
 */
class IJoystickRawInputHandler : public IJoystickActionHandler
{
public:
  virtual ~IJoystickRawInputHandler(void) { }

  /*!
   * \brief A button's value has changed
   *
   * \param index     The raw button index reported by the joystick provider
   * \param bPressed  The new value of the button
   *
   * \return True if the event was handled otherwise false
   */
  virtual bool OnRawButtonPress(unsigned int index) { return true; }
  virtual bool OnRawButtonHold(unsigned int index) { return true; }
  virtual bool OnRawButtonDoublePress(unsigned int index) { return true; }
  virtual bool OnRawButtonDoublePressHold(unsigned int index) { return true; }
  virtual bool OnRawButtonRelease(unsigned int index) { return true; }

  /*!
   * \brief A hat's value has changed
   *
   * \param index     The raw hat index reported by the joystick provider
   * \param direction The new value of the hat
   *
   * \return True if the event was handled otherwise false
   */
  virtual bool OnRawHatPress(unsigned int index, HatDirection direction) { return true; }
  virtual bool OnRawHatMotion(unsigned int index, HatDirection direction) { return true; }
  virtual bool OnRawHatHold(unsigned int index, HatDirection direction) { return true; }
  virtual bool OnRawHatDoublePress(unsigned int index, HatDirection direction) { return true; }
  virtual bool OnRawHatDoublePressHold(unsigned int index, HatDirection direction) { return true; }
  virtual bool OnRawHatRelease(unsigned int index) { return true; }

  /*!
   * \brief A axis's value has changed
   *
   * \param index     The raw axis index reported by the joystick provider
   * \param position  The new value of the axis in the interval [-1, 1], inclusive
   *
   * \return True if the event was handled otherwise false
   */
  virtual bool OnRawAxisPress(unsigned int index, SemiAxisDirection direction) { return true; }
  virtual bool OnRawAxisMotion(unsigned int index, float position) { return true; }
  virtual bool OnRawAxisHold(unsigned int index, SemiAxisDirection direction) { return true; }
  virtual bool OnRawAxisDoublePress(unsigned int index, SemiAxisDirection direction) { return true; }
  virtual bool OnRawAxisDoublePressHold(unsigned int index, SemiAxisDirection direction) { return true; }
  virtual bool OnRawAxisRelease(unsigned int index) { return true; }

  /*!
   * \brief Multiple buttons were pushed within a small timeframe
   *
   * \param buttons The currently-pressed buttons, in order of activation
   *
   * \return True if the event was handled otherwise false
   */
  virtual bool OnRawMultiPress(const std::vector<ButtonPrimitive>& buttons) { return true; }
};
