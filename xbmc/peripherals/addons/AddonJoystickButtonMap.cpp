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

JoystickActionID CAddonJoystickButtonMap::GetAction(const CButtonPrimitive& source)
{
  return m_addon->GetAction(m_index, source);
}

JoystickActionID CAddonJoystickButtonMap::GetAnalogStick(unsigned int  axisIndex, unsigned int& indexHoriz, unsigned int& indexVert)
{
  return m_addon->GetAnalogStick(m_index, axisIndex, indexHoriz, indexVert);
}

JoystickActionID CAddonJoystickButtonMap::GetAccelerometer(unsigned int  axisIndex, unsigned int& indexX, unsigned int& indexY, unsigned int& indexZ)
{
  return m_addon->GetAccelerometer(m_index, axisIndex, indexX, indexY, indexZ);
}
