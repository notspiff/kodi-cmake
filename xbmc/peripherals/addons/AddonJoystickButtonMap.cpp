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

#include "AddonJoystickButtonMap.h"

using namespace PERIPHERALS;

CAddonJoystickButtonMap::CAddonJoystickButtonMap(const PERIPHERALS::PeripheralAddonPtr& addon, unsigned int index)
 : m_addon(addon),
   m_index(index)
{
}

bool CAddonJoystickButtonMap::Load(void)
{
  return m_addon->GetButtonMap(m_index, m_buttonMap);
}

JoystickActionID CAddonJoystickButtonMap::GetAction(const CButtonPrimitive& source)
{
  for (ADDON::ButtonMap::const_iterator it = m_buttonMap.begin(); it != m_buttonMap.end(); ++it)
  {
    const ADDON::ButtonMapValue& value = it->second;

    bool bMatches(false);

    switch (value.Type())
    {
    case JOYSTICK_BUTTONMAP_VALUE_BUTTON:
      if (source.Index() == value.Button().index)
        bMatches = true;
      break;

    case JOYSTICK_BUTTONMAP_VALUE_HAT_DIRECTION:
      if (source.Index()  == value.Hat().index &&
          source.HatDir() == CPeripheralAddon::ToHatDirection(value.Hat().direction))
        bMatches = true;
      break;

    case JOYSTICK_BUTTONMAP_VALUE_SEMIAXIS:
      if (source.Index()       == value.SemiAxis().index &&
          source.SemiAxisDir() == CPeripheralAddon::ToSemiAxisDirection(value.SemiAxis().direction))
        bMatches = true;
      break;

    case JOYSTICK_BUTTONMAP_VALUE_ANALOG_STICK:
      if (source.Index() == value.AnalogStick().right.index ||
          source.Index() == value.AnalogStick().up.index)
        bMatches = true;
      break;

    case JOYSTICK_BUTTONMAP_VALUE_ACCELEROMETER:
      if (source.Index() == value.Accelerometer().x.index ||
          source.Index() == value.Accelerometer().y.index ||
          source.Index() == value.Accelerometer().z.index)
        bMatches = true;
      break;

    case JOYSTICK_BUTTONMAP_VALUE_UNKNOWN:
    default:
      break;
    }

    if (bMatches)
      return CPeripheralAddon::ToJoystickID(it->first);
  }
  return JOY_ID_BUTTON_UNKNOWN;
}

bool CAddonJoystickButtonMap::GetButtonPrimitive(JoystickActionID id, CButtonPrimitive& button)
{
  bool retVal(false);

  const JOYSTICK_ID addonId = CPeripheralAddon::ToAddonID(id);
  ADDON::ButtonMap::const_iterator it = m_buttonMap.find(addonId);
  if (it != m_buttonMap.end())
  {
    switch (it->second.Type())
    {
    case JOYSTICK_BUTTONMAP_VALUE_BUTTON:
      button = CButtonPrimitive(it->second.Button().index);
      retVal = true;
      break;

    case JOYSTICK_BUTTONMAP_VALUE_HAT_DIRECTION:
    {
      const HatDirection dir = CPeripheralAddon::ToHatDirection(it->second.Hat().direction);
      button = CButtonPrimitive(it->second.Hat().index, dir);
      retVal = true;
      break;
    }

    case JOYSTICK_BUTTONMAP_VALUE_SEMIAXIS:
    {
      const SemiAxisDirection dir = CPeripheralAddon::ToSemiAxisDirection(it->second.SemiAxis().direction);
      button = CButtonPrimitive(it->second.SemiAxis().index, dir);
      retVal = true;
      break;
    }

    default:
      break;
    }
  }

  return retVal;
}

bool CAddonJoystickButtonMap::GetAnalogStick(JoystickActionID id, 
    int& horizIndex, bool& horizInverted,
    int& vertIndex,  bool& vertInverted)
{
  bool retVal(false);

  const JOYSTICK_ID addonId = CPeripheralAddon::ToAddonID(id);
  ADDON::ButtonMap::const_iterator it = m_buttonMap.find(addonId);
  if (it != m_buttonMap.end())
  {
    switch (it->second.Type())
    {
    case JOYSTICK_BUTTONMAP_VALUE_ANALOG_STICK:
      horizIndex    = it->second.AnalogStick().right.index;
      horizInverted = it->second.AnalogStick().right.direction == JOYSTICK_SEMIAXIS_DIRECTION_NEGATIVE;
      vertIndex     = it->second.AnalogStick().up.index;
      vertInverted  = it->second.AnalogStick().up.direction == JOYSTICK_SEMIAXIS_DIRECTION_NEGATIVE;
      retVal        = true;
      break;

    default:
      break;
    }
  }

  return retVal;
}

bool CAddonJoystickButtonMap::GetAccelerometer(JoystickActionID id,
    int& xIndex, bool& xInverted,
    int& yIndex, bool& yInverted,
    int& zIndex, bool& zInverted)
{
  bool retVal(false);

  const JOYSTICK_ID addonId = CPeripheralAddon::ToAddonID(id);
  ADDON::ButtonMap::const_iterator it = m_buttonMap.find(addonId);
  if (it != m_buttonMap.end())
  {
    switch (it->second.Type())
    {
    case JOYSTICK_BUTTONMAP_VALUE_ACCELEROMETER:
      xIndex    = it->second.Accelerometer().x.index;
      xInverted = it->second.Accelerometer().x.direction == JOYSTICK_SEMIAXIS_DIRECTION_NEGATIVE;
      yIndex    = it->second.Accelerometer().y.index;
      yInverted = it->second.Accelerometer().y.direction == JOYSTICK_SEMIAXIS_DIRECTION_NEGATIVE;
      zIndex    = it->second.Accelerometer().z.index;
      zInverted = it->second.Accelerometer().z.direction == JOYSTICK_SEMIAXIS_DIRECTION_NEGATIVE;
      retVal    = true;
      break;

    default:
      break;
    }
  }

  return retVal;
}
