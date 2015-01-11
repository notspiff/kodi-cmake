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

class CAddonJoystickButtonMap : public IJoystickButtonMap
{
public:
  CAddonJoystickButtonMap(void) { }

  virtual ~CAddonJoystickButtonMap() { }

  virtual bool Load(const std::string& strProvider,
                    const std::string& strName     = "",
                    uint16_t           vendorId    = 0x0000,
                    uint16_t           productId   = 0x0000,
                    int                buttonCount = -1,
                    int                hatCount    = -1,
                    int                axisCount   = -1);

  virtual bool Save();

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
};
