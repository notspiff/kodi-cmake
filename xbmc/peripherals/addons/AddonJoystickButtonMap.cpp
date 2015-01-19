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
  return m_addon->GetJoystickFeatures(m_index, m_features);
}

JoystickActionID CAddonJoystickButtonMap::GetAction(const CButtonPrimitive& source)
{
  for (JoystickFeatureMap::const_iterator it = m_features.begin(); it != m_features.end(); ++it)
  {
    const ADDON::JoystickFeature* feature = it->second.get();

    bool bMatches(false);

    switch (feature->Type())
    {
    case JOYSTICK_DRIVER_TYPE_BUTTON:
    {
      const ADDON::DriverButton* button = static_cast<const ADDON::DriverButton*>(feature);
      if (source.Index() == button->Index())
        bMatches = true;
      break;
    }

    case JOYSTICK_DRIVER_TYPE_HAT_DIRECTION:
    {
      const ADDON::DriverHat* hat = static_cast<const ADDON::DriverHat*>(feature);
      if (source.Index()  == hat->Index() &&
          source.HatDir() == CPeripheralAddon::ToHatDirection(hat->Direction()))
        bMatches = true;
      break;
    }

    case JOYSTICK_DRIVER_TYPE_SEMIAXIS:
    {
      const ADDON::DriverSemiAxis* semiaxis = static_cast<const ADDON::DriverSemiAxis*>(feature);
      if (source.Index()       == semiaxis->Index() &&
          source.SemiAxisDir() == CPeripheralAddon::ToSemiAxisDirection(semiaxis->Direction()))
        bMatches = true;
      break;
    }

    case JOYSTICK_DRIVER_TYPE_ANALOG_STICK:
    {
      const ADDON::DriverAnalogStick* analogStick = static_cast<const ADDON::DriverAnalogStick*>(feature);
      if (source.Index() == analogStick->XIndex() ||
          source.Index() == analogStick->YIndex())
        bMatches = true;
      break;
    }

    case JOYSTICK_DRIVER_TYPE_ACCELEROMETER:
    {
      const ADDON::DriverAccelerometer* accelerometer = static_cast<const ADDON::DriverAccelerometer*>(feature);
      if (source.Index() == accelerometer->XIndex() ||
          source.Index() == accelerometer->YIndex() ||
          source.Index() == accelerometer->ZIndex())
        bMatches = true;
      break;
    }

    default:
      break;
    }

    if (bMatches)
      return it->first;
  }
  return JOY_ID_BUTTON_UNKNOWN;
}

bool CAddonJoystickButtonMap::GetButtonPrimitive(JoystickActionID id, CButtonPrimitive& button)
{
  bool retVal(false);

  JoystickFeatureMap::const_iterator it = m_features.find(id);
  if (it != m_features.end())
  {
    const ADDON::JoystickFeature* feature = it->second.get();

    switch (feature->Type())
    {
    case JOYSTICK_DRIVER_TYPE_BUTTON:
    {
      const ADDON::DriverButton* driverButton = static_cast<const ADDON::DriverButton*>(feature);
      button = CButtonPrimitive(driverButton->Index());
      retVal = true;
      break;
    }

    case JOYSTICK_DRIVER_TYPE_HAT_DIRECTION:
    {
      const ADDON::DriverHat* driverHat = static_cast<const ADDON::DriverHat*>(feature);
      const HatDirection dir = CPeripheralAddon::ToHatDirection(driverHat->Direction());
      button = CButtonPrimitive(driverHat->Index(), dir);
      retVal = true;
      break;
    }

    case JOYSTICK_DRIVER_TYPE_SEMIAXIS:
    {
      const ADDON::DriverSemiAxis* driverSemiAxis = static_cast<const ADDON::DriverSemiAxis*>(feature);
      const SemiAxisDirection dir = CPeripheralAddon::ToSemiAxisDirection(driverSemiAxis->Direction());
      button = CButtonPrimitive(driverSemiAxis->Index(), dir);
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

  JoystickFeatureMap::const_iterator it = m_features.find(id);
  if (it != m_features.end())
  {
    const ADDON::JoystickFeature* feature = it->second.get();

    switch (feature->Type())
    {
    case JOYSTICK_DRIVER_TYPE_ANALOG_STICK:
    {
      const ADDON::DriverAnalogStick* driverAnalogStick = static_cast<const ADDON::DriverAnalogStick*>(feature);
      horizIndex    = driverAnalogStick->XIndex();
      horizInverted = driverAnalogStick->XInverted();
      vertIndex     = driverAnalogStick->YIndex();
      vertInverted  = driverAnalogStick->YInverted();
      retVal        = true;
      break;
    }

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

  JoystickFeatureMap::const_iterator it = m_features.find(id);
  if (it != m_features.end())
  {
    const ADDON::JoystickFeature* feature = it->second.get();

    switch (feature->Type())
    {
    case JOYSTICK_DRIVER_TYPE_ACCELEROMETER:
    {
      const ADDON::DriverAccelerometer* driverAccelerometer = static_cast<const ADDON::DriverAccelerometer*>(feature);
      xIndex    = driverAccelerometer->XIndex();
      xInverted = driverAccelerometer->XInverted();
      yIndex    = driverAccelerometer->YIndex();
      yInverted = driverAccelerometer->YInverted();
      zIndex    = driverAccelerometer->ZIndex();
      zInverted = driverAccelerometer->ZInverted();
      retVal    = true;
      break;
    }

    default:
      break;
    }
  }

  return retVal;
}
