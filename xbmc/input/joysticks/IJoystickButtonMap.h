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

#include "ButtonPrimitive.h"

#include <stdint.h>
#include <string>

/*!
 * \ingroup joysticks
 *
 * \brief IJoystickButtonMap
 */
class IJoystickButtonMap
{
public:
  virtual ~IJoystickButtonMap() { }
  
#if 0 // TODO
  /*!
   * \brief Load()
   *
   * \param strProvider Standard name of the API providing the joystick
   * \param strName     Name of the joystick reported by the provider, or empty if unknown
   * \param vendorId    Vendor ID of the joystick, or 0x0000 if unknown
   * \param productId   Product ID of the joystick, or 0x0000 if unknown
   * \param buttonCount Number of raw buttons reported by the provider, or -1 if unknown
   * \param hatCount    Number of raw hats reported by the provider, or -1 if unknown
   * \param axisCount   Number of raw axes reported by the provider, or -1 if unknown
   *
   * \return True if the button map was loaded
   */
  virtual bool Load(const std::string& strProvider,
                    const std::string& strName     = "",
                    uint16_t           vendorId    = 0x0000,
                    uint16_t           productId   = 0x0000,
                    int                buttonCount = -1,
                    int                hatCount    = -1,
                    int                axisCount   = -1) = 0;
#endif

  virtual JoystickActionID GetAction(const CButtonPrimitive& source) = 0;

  virtual JoystickActionID GetAnalogStick(unsigned int  axisIndex,
                                          unsigned int& indexHoriz,
                                          unsigned int& indexVert) = 0;

  virtual JoystickActionID GetAccelerometer(unsigned int  axisIndex,
                                            unsigned int& indexX,
                                            unsigned int& indexY,
                                            unsigned int& indexZ) = 0;
};
