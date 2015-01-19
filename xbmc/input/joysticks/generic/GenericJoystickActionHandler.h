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

#include "input/joysticks/IJoystickActionHandler.h"

/*!
 * \ingroup joysticks_generic
 * \brief Generic implementation of IJoystickActionHandler to translate
 *        joystick actions into XBMC specific and mappable actions.
 *
 * \sa IJoystickActionHandler
 */
class CGenericJoystickActionHandler : public IJoystickActionHandler
{
public:
  CGenericJoystickActionHandler() { }

  virtual ~CGenericJoystickActionHandler() { }

  // implementation of IJoystickActionHandler
  virtual bool OnButtonPress(JoystickActionID id);

  /*!
   * \brief A pressure-sensitive button has been pressed or a trigger has moved
   *
   * \param id        The ID of the button or trigger
   * \param magnitude The button pressure or trigger travel distance in the interval [0, 1]
   *
   * \return True if the event was handled otherwise false
   */
  virtual bool OnButtonMotion(JoystickActionID id, float magnitude);

  virtual bool OnButtonHold(JoystickActionID id);

  virtual bool OnButtonDoublePress(JoystickActionID id);

  virtual bool OnMultiPress(const std::vector<JoystickActionID>& ids);

  /*!
   * \brief An analog stick has moved
   *
   * \param id        The ID of the analog stick
   * \param x         The x coordinate in the closed interval [-1, 1]
   * \param y         The y coordinate in the closed interval [-1, 1]
   *
   * \return True if the event was handled otherwise false
   */
  virtual bool OnAnalogStickMotion(JoystickActionID id, float x, float y);

  /*!
   * \brief An analog stick's magnitude has crossed a threshold of 0.5 in a
   *        cardinal or intercardinal direction
   *
   * \param id        The ID of the analog stick
   * \param bPressed  True when activated (threshold >= 0.5), false when centered
   * \param direction If bPressed is false then direction is HatDirectionNone; otherwise,
   *                  direction is the direction closest to the displaced stick
   *
   * \return True if the event was handled otherwise false
   */
  virtual bool OnAnalogStickThreshold(JoystickActionID, bool bPressed, HatDirection direction);

  /*!
   * \brief An accelerometer's acceleration has changed
   *
   * \param x         The x coordinate in the closed interval [-1, 1]
   * \param y         The y coordinate in the closed interval [-1, 1]
   * \param z         The z coordinate in the closed interval [-1, 1]
   *
   * \return True if the event was handled otherwise false
   */
  virtual bool OnAccelerometerMotion(JoystickActionID, float x, float y, float z);
};
