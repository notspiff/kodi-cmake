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
#include "input/joysticks/JoystickTypes.h"

#include <string>

class IJoystickInputHandler;
class IJoystickActionHandler;
class IJoystickButtonMap;

namespace PERIPHERALS
{
  class CPeripheralJoystick : public CPeripheral // TODO: extend CPeripheralHID
  {
  public:
    CPeripheralJoystick(const PeripheralScanResult& scanResult);
    virtual ~CPeripheralJoystick(void);

    virtual bool InitialiseFeature(const PeripheralFeature feature);

    const std::string& Provider(void) const      { return m_strProvider; }
    unsigned int       RequestedPort(void) const { return m_requestedPort; }
    unsigned int       ButtonCount(void) const   { return m_buttonCount; }
    unsigned int       HatCount(void) const      { return m_hatCount; }
    unsigned int       AxisCount(void) const     { return m_axisCount; }

    void SetDeviceName(const std::string& strName)    { m_strDeviceName = strName; } // Override value in peripherals.xml
    void SetProvider(const std::string& strProvider)  { m_strProvider   = strProvider; }
    void SetRequestedPort(unsigned int requestedPort) { m_requestedPort = requestedPort; }
    void SetButtonCount(unsigned int buttonCount)     { m_buttonCount   = buttonCount; }
    void SetHatCount(unsigned int hatCount)           { m_hatCount      = hatCount; }
    void SetAxisCount(unsigned int axisCount)         { m_axisCount     = axisCount; }

    virtual void OnButtonMotion(unsigned int index, bool bPressed);
    virtual void OnHatMotion(unsigned int index, HatDirection direction);
    virtual void OnAxisMotion(unsigned int index, float position);

  protected:
    IJoystickInputHandler*  m_inputHandler;
    IJoystickActionHandler* m_actionHandler;
    IJoystickButtonMap*     m_buttonMap;
    std::string             m_strProvider;
    unsigned int            m_requestedPort;
    unsigned int            m_buttonCount;
    unsigned int            m_hatCount;
    unsigned int            m_axisCount;
  };
}
