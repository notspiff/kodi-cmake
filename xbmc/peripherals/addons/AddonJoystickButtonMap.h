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

#include "input/joysticks/IJoystickButtonMap.h"
#include "peripherals/addons/PeripheralAddon.h"

namespace PERIPHERALS
{
  class CAddonJoystickButtonMap : public IJoystickButtonMap
  {
  public:
    CAddonJoystickButtonMap(const PERIPHERALS::PeripheralAddonPtr& addon, unsigned int index);

    virtual ~CAddonJoystickButtonMap(void) { }

    JoystickActionID GetAction(const CButtonPrimitive& source);

    JoystickActionID GetAnalogStick(unsigned int  axisIndex,
                                    unsigned int& indexHoriz,
                                    unsigned int& indexVert);

    JoystickActionID GetAccelerometer(unsigned int  axisIndex,
                                      unsigned int& indexX,
                                      unsigned int& indexY,
                                      unsigned int& indexZ);

  private:
    PERIPHERALS::PeripheralAddonPtr m_addon;
    unsigned int                    m_index;
  };
}
