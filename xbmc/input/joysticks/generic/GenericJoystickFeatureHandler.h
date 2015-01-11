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

#include "input/joysticks/IJoystickFeatureHandler.h"
#include "threads/CriticalSection.h"
#include "threads/Timer.h"

#include <vector>

class CAction;

/*!
 * \ingroup joysticks_generic
 * \brief Generic implementation of IJoystickFeatureHandler to translate
 *        joystick features into joystick.xml buttons
 *
 * \sa IJoystickFeatureHandler
 */
class CGenericJoystickFeatureHandler : public IJoystickFeatureHandler, public ITimerCallback
{
public:
  CGenericJoystickFeatureHandler(void);

  virtual ~CGenericJoystickFeatureHandler(void) { }

  // implementation of IJoystickFeatureHandler
  virtual bool OnButtonPress(JoystickFeatureID id);
  virtual bool OnButtonRelease(JoystickFeatureID id);
  virtual bool OnButtonMotion(JoystickFeatureID id, float magnitude);
  virtual bool OnAnalogStickMotion(JoystickFeatureID id, float x, float y);
  virtual bool OnAccelerometerMotion(JoystickFeatureID id, float x, float y, float z);

  // implementation of ITimerCallback
  virtual void OnTimeout(void);

private:
  void ProcessButtonPress(const CAction& action);
  void ProcessButtonRelease(unsigned int buttonKeyId);

  void StartHoldTimer(unsigned int buttonKeyId);
  void ClearHoldTimer(void);

  /*!
   * \brief Get the button key, as defined in guilib/Key.h, for the specified
   *        joystick feature/direction
   *
   * A direction vector of the feature's position can be used to obtain keys
   * for analog stick directions (e.g. "rightthumbstickup").
   *
   * Ties are resolved in the clockwise direction. A right thumb stick at (0.5, 0.5)
   * will resolve to "rightthumbstickright".
   *
   * \param id        The joystick feature ID
   * \param x         The x component of the direction vector being queried
   * \param y         The y component of the direction vector being queried
   * \param z         The z component of the direction vector being queried
   *
   * \return True if the event was handled otherwise false
   */
  static unsigned int GetButtonKeyID(JoystickFeatureID id, float x = 0.0f, float y = 0.0f, float z = 0.0f);

  CTimer                    m_holdTimer;
  unsigned int              m_lastButtonPress;
  std::vector<unsigned int> m_pressedButtons;
  CCriticalSection          m_digitalMutex;
};
