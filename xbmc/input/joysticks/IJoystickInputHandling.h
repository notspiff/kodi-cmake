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

#include "input/joysticks/IJoystickRawInputHandler.h"

/*!
 * \ingroup joysticks
 * \brief Convenience interface implementing IJoystickRawInputHandler with an
 *        implementation that forwards any IJoystickRawInputHandler-related calls
 *        to a previously registered IJoystickRawInputHandler
 *
 * \sa IJoystickRawInputHandler
 */
class IJoystickInputHandling : public IJoystickRawInputHandler
{
public:
  IJoystickInputHandling(void);

  virtual ~IJoystickInputHandling(void) { }

  /*!
   * \brief Register a joystick handler
   *
   * There can only be one joystick handler.
   *
   * \param joystickHandler An instance of a joystick handler implementing the
   *                        IJoystickActionHandler interface
   *
   * \sa UnregisterHandler
   */
  void RegisterHandler(IJoystickRawInputHandler *joystickInputHandler);

  /*!
   * \brief Unregister the previously registered joystick handler
   *
   * \sa RegisterHandler
   */
  void UnregisterHandler();

  // Implementation of IJoystickActionHandler
  virtual bool OnButtonPress(JoystickActionID id, bool bPressed);
  virtual bool OnButtonMotion(JoystickActionID id, float magnitude);
  virtual bool OnAnalogStickMotion(JoystickActionID id, float x, float y);
  virtual bool OnAnalogStickThreshold(JoystickActionID id, bool bPressed, HatDirection direction = HatDirectionNone);
  virtual bool OnAccelerometerMotion(JoystickActionID id, float x, float y, float z);

  // Implementation of IJoystickRawInputHandler
  virtual bool OnRawButtonPress(unsigned int index);
  virtual bool OnRawButtonHold(unsigned int index);
  virtual bool OnRawButtonDoublePress(unsigned int index);
  virtual bool OnRawButtonDoublePressHold(unsigned int index);
  virtual bool OnRawButtonRelease(unsigned int index);

  virtual bool OnRawHatPress(unsigned int index, HatDirection direction);
  virtual bool OnRawHatMotion(unsigned int index, HatDirection direction);
  virtual bool OnRawHatHold(unsigned int index, HatDirection direction);
  virtual bool OnRawHatDoublePress(unsigned int index, HatDirection direction);
  virtual bool OnRawHatDoublePressHold(unsigned int index, HatDirection direction);
  virtual bool OnRawHatRelease(unsigned int index);

  virtual bool OnRawAxisPress(unsigned int index, SemiAxisDirection direction);
  virtual bool OnRawAxisMotion(unsigned int index, float position);
  virtual bool OnRawAxisHold(unsigned int index, SemiAxisDirection direction);
  virtual bool OnRawAxisDoublePress(unsigned int index, SemiAxisDirection direction);
  virtual bool OnRawAxisDoublePressHold(unsigned int index, SemiAxisDirection direction);
  virtual bool OnRawAxisRelease(unsigned int index);

  virtual bool OnRawMultiPress(const std::vector<ButtonPrimitive>& buttons);

private:
  IJoystickRawInputHandler *m_handler;
};
