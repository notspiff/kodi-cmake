#pragma once
/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include "Peripheral.h"
#include "input/joysticks/IJoystickInputHandler.h"

namespace PERIPHERALS
{
  class CPeripheralJoystick : public CPeripheral // TODO: extend CPeripheralHID
  {
  public:
    CPeripheralJoystick(const PeripheralScanResult& scanResult);
    virtual ~CPeripheralJoystick(void);

    virtual bool InitialiseFeature(const PeripheralFeature feature);

    /*
    bool HandleJoystickEvent(JoystickEvent event,
                             unsigned int  index,
                             int64_t       timeNs,
                             bool          bPressed  = false,
                             HatDirection  direction = HatDirectionNone,
                             float         axisPos   = 0.0f);
    */
  protected:
    IJoystickInputHandler* m_inputHandler;
  };
}
