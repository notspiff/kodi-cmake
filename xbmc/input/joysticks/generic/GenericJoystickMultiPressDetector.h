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

#include "input/joysticks/IJoystickInputHandling.h"

class CGenericJoystickButtonMapper;
class IButtonMapper;

/*!
 * \ingroup joysticks_generic
 * \brief Implementation of IJoystickInputHandling to detect multi-presses
 *
 * \sa IJoystickInputHandling
 */
class CGenericJoystickMultiPressDetector : public IJoystickInputHandling
{
public:
  CGenericJoystickMultiPressDetector(IButtonMapper* buttonMapper);

  virtual ~CGenericJoystickMultiPressDetector();

  // Implementation of IJoystickRawInputHandler
  virtual bool OnRawButtonPress(unsigned int index);
  virtual bool OnRawButtonDoublePress(unsigned int index);
  virtual bool OnRawButtonRelease(unsigned int index);

  virtual bool OnRawHatPress(unsigned int index, HatDirection direction);
  virtual bool OnRawHatMotion(unsigned int index, HatDirection direction);
  virtual bool OnRawHatDoublePress(unsigned int index, HatDirection direction);
  virtual bool OnRawHatRelease(unsigned int index);

  virtual bool OnRawAxisPress(unsigned int index, SemiAxisDirection direction);
  virtual bool OnRawAxisDoublePress(unsigned int index, SemiAxisDirection direction);
  virtual bool OnRawAxisRelease(unsigned int index);

private:
  bool OnPress(const ButtonPrimitive& button);

  void ClearButton(unsigned int index);
  void ClearHat(unsigned int index);
  void ClearAxis(unsigned int index);

  std::vector<ButtonPrimitive> m_currentCombo; // sorted in order of activation
  unsigned int                 m_lastPressMs;
  CGenericJoystickButtonMapper *m_buttonMapper;
};
