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
#include "input/joysticks/JoystickTypes.h"

#include <map>

// TODO
class IButtonMapper
{
public:
  virtual ~IButtonMapper() { }

  virtual bool LoadButtonMap(const JoystickIdentifier& joystick, ButtonMap& loadedButtonMap) = 0;
};

class CGenericJoystickActionHandler;

/*!
 * \ingroup joysticks_generic
 * \brief Interface defining methods to perform button mapping
 */
class CGenericJoystickButtonMapper : public IJoystickInputHandling
{
public:
  CGenericJoystickButtonMapper(IButtonMapper *buttonMapper);

  virtual ~CGenericJoystickButtonMapper();

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

protected:
  IButtonMapper* const          m_buttonMapper;
  std::map<ButtonPrimitive, std::vector<ButtonRecipe> > m_buttonMap;
  CGenericJoystickActionHandler *m_handler;
};
