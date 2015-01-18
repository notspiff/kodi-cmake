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

#include "xbmc_peripheral_types.h"

#include <cmath>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#define DIGITAL_ANALOG_THRESHOLD  0.5f // TODO

#ifndef SAFE_DELETE
  #define SAFE_DELETE(x)  do { delete (x); (x) = NULL; } while (0)
#endif

#ifndef SAFE_DELETE_ARRAY
  #define SAFE_DELETE_ARRAY(x)  do { delete[] (x); (x) = NULL; } while (0)
#endif

#ifndef CONSTRAIN
  #define CONSTRAIN(min, value, max)  ((value) < (min) ? (min) : (max) < (value) ? (max) : (value))
#endif

namespace ADDON
{
  /*!
   * Utility class to manipulate arrays of peripheral types.
   */
  template <class THE_CLASS, typename THE_STRUCT>
  class PeripheralVector
  {
  public:
    static void ToStructs(const std::vector<THE_CLASS>& vecObjects, THE_STRUCT** pStructs)
    {
      if (!pStructs)
        return;

      if (vecObjects.empty())
      {
        *pStructs = NULL;
      }
      else
      {
        (*pStructs) = new THE_STRUCT[vecObjects.size()];
        for (unsigned int i = 0; i < vecObjects.size(); i++)
          vecObjects.at(i).ToStruct((*pStructs)[i]);
      }
    }

    static void ToStructs(const std::vector<THE_CLASS*>& vecObjects, THE_STRUCT** pStructs)
    {
      if (!pStructs)
        return;

      if (vecObjects.empty())
      {
        *pStructs = NULL;
      }
      else
      {
        *pStructs = new THE_STRUCT[vecObjects.size()];
        for (unsigned int i = 0; i < vecObjects.size(); i++)
          vecObjects.at(i)->ToStruct((*pStructs)[i]);
      }
    }

    static void FreeStructs(unsigned int structCount, THE_STRUCT* structs)
    {
      if (structs)
      {
        for (unsigned int i = 0; i < structCount; i++)
          THE_CLASS::FreeStruct(structs[i]);
      }
      SAFE_DELETE_ARRAY(structs);
    }
  };

  /*!
   * ADDON::Peripheral
   *
   * Wrapper class providing peripheral information. Classes can extend
   * Peripheral to inherit peripheral properties.
   */
  class Peripheral
  {
  public:
    Peripheral(PERIPHERAL_TYPE type = PERIPHERAL_TYPE_UNKNOWN, const std::string& strName = "")
    : m_type(type),
      m_strName(strName),
      m_index(0),
      m_vendorId(0),
      m_productId(0)
    {
    }

    Peripheral(PERIPHERAL_INFO& info)
    : m_type(info.type),
      m_strName(info.name ? info.name : ""),
      m_index(info.index),
      m_vendorId(info.vendor_id),
      m_productId(info.product_id)
    {
    }

    virtual ~Peripheral(void) { }

    PERIPHERAL_TYPE    Type(void) const      { return m_type; }
    const std::string& Name(void) const      { return m_strName; }
    unsigned int       Index(void) const     { return m_index; }
    unsigned int       VendorID(void) const  { return m_vendorId; }
    unsigned int       ProductID(void) const { return m_productId; }

    void SetType(PERIPHERAL_TYPE type)        { m_type      = type; }
    void SetName(const std::string& strName)  { m_strName   = strName; }
    void SetIndex(unsigned int index)         { m_index     = index; }
    void SetVendorID(unsigned int vendorId)   { m_vendorId  = vendorId; }
    void SetProductID(unsigned int productId) { m_productId = productId; }

    void ToStruct(PERIPHERAL_INFO& info) const
    {
      info.type       = m_type;
      info.name       = new char[m_strName.size() + 1];
      info.index      = m_index;
      info.vendor_id  = m_vendorId;
      info.product_id = m_productId;

      std::strcpy(info.name, m_strName.c_str());
    }

    static void FreeStruct(PERIPHERAL_INFO& info)
    {
      SAFE_DELETE_ARRAY(info.name);
    }

  private:
    PERIPHERAL_TYPE             m_type;
    std::string                 m_strName;
    unsigned int                m_index;
    unsigned int                m_vendorId;
    unsigned int                m_productId;
  };

  typedef PeripheralVector<Peripheral, PERIPHERAL_INFO> Peripherals;

  /*!
   * ADDON::Joystick
   *
   * Wrapper class providing additional joystick information not provided by
   * Peripheral. Classes can extend Joystick to inherit peripheral and joystick
   * properties.
   */
  class Joystick : public Peripheral
  {
  public:
    Joystick(const std::string& strProvider = "", const std::string& strName = "")
    : Peripheral(PERIPHERAL_TYPE_JOYSTICK, strName),
      m_strProvider(strProvider),
      m_requestedPort(0),
      m_buttonCount(0),
      m_hatCount(0),
      m_axisCount(0)
    {
    }

    Joystick(JOYSTICK_INFO& info)
    : Peripheral(info.peripheral_info),
      m_strProvider(info.provider ? info.provider : ""),
      m_requestedPort(info.requested_port_num),
      m_buttonCount(info.button_count),
      m_hatCount(info.hat_count),
      m_axisCount(info.axis_count)
    {
      SetType(PERIPHERAL_TYPE_JOYSTICK);
    }

    virtual ~Joystick(void) { }

    const std::string& Provider(void) const      { return m_strProvider; }
    unsigned int       RequestedPort(void) const { return m_requestedPort; }
    unsigned int       ButtonCount(void) const   { return m_buttonCount; }
    unsigned int       HatCount(void) const      { return m_hatCount; }
    unsigned int       AxisCount(void) const     { return m_axisCount; }

    void SetProvider(const std::string& strProvider)  { m_strProvider   = strProvider; }
    void SetRequestedPort(unsigned int requestedPort) { m_requestedPort = requestedPort; }
    void SetButtonCount(unsigned int buttonCount)     { m_buttonCount   = buttonCount; }
    void SetHatCount(unsigned int hatCount)           { m_hatCount      = hatCount; }
    void SetAxisCount(unsigned int axisCount)         { m_axisCount     = axisCount; }

    void ToStruct(JOYSTICK_INFO& info) const
    {
      Peripheral::ToStruct(info.peripheral_info);

      info.provider           = new char[m_strProvider.size() + 1];
      info.requested_port_num = m_requestedPort;
      info.button_count       = m_buttonCount;
      info.hat_count          = m_hatCount;
      info.axis_count         = m_axisCount;

      std::strcpy(info.provider, m_strProvider.c_str());
    }

    static void FreeStruct(JOYSTICK_INFO& info)
    {
      Peripheral::FreeStruct(info.peripheral_info);
      SAFE_DELETE_ARRAY(info.provider);
    }

  private:
    std::string  m_strProvider;
    unsigned int m_requestedPort;
    unsigned int m_buttonCount;
    unsigned int m_hatCount;
    unsigned int m_axisCount;
  };

  typedef PeripheralVector<Joystick, JOYSTICK_INFO> Joysticks;

  class PeripheralEvent
  {
  public:
    PeripheralEvent(void)
    : m_type(),
      m_peripheralIndex(0),
      m_rawIndex(0),
      m_buttonState(),
      m_hatState(),
      m_axisState()
    {
    }

    PeripheralEvent(unsigned int peripheralIndex, unsigned int buttonIndex, JOYSTICK_STATE_BUTTON state)
    : m_type(JOYSTICK_EVENT_TYPE_RAW_BUTTON),
      m_peripheralIndex(peripheralIndex),
      m_rawIndex(buttonIndex),
      m_buttonState(state),
      m_hatState(),
      m_axisState()
    {
    }

    PeripheralEvent(unsigned int peripheralIndex, unsigned int hatIndex, JOYSTICK_STATE_HAT state)
    : m_type(JOYSTICK_EVENT_TYPE_RAW_HAT),
      m_peripheralIndex(peripheralIndex),
      m_rawIndex(hatIndex),
      m_buttonState(),
      m_hatState(state),
      m_axisState()
    {
    }

    PeripheralEvent(unsigned int peripheralIndex, unsigned int axisIndex, JOYSTICK_STATE_AXIS state)
    : m_type(JOYSTICK_EVENT_TYPE_RAW_AXIS),
      m_peripheralIndex(peripheralIndex),
      m_rawIndex(axisIndex),
      m_buttonState(),
      m_hatState(),
      m_axisState(state)
    {
    }

    PeripheralEvent(const PERIPHERAL_EVENT& event)
    : m_type(event.type),
      m_peripheralIndex(event.peripheral_index),
      m_rawIndex(event.raw_index),
      m_buttonState(),
      m_hatState(),
      m_axisState()
    {
      switch (m_type)
      {
        case JOYSTICK_EVENT_TYPE_RAW_BUTTON: m_buttonState = event.button_state; break;
        case JOYSTICK_EVENT_TYPE_RAW_HAT:    m_hatState    = event.hat_state;    break;
        case JOYSTICK_EVENT_TYPE_RAW_AXIS:   m_axisState   = event.axis_state;   break;
        case JOYSTICK_EVENT_TYPE_NONE:
        default: break;
      }
    }

    JOYSTICK_EVENT_TYPE   Type(void) const            { return m_type; }
    unsigned int          PeripheralIndex(void) const { return m_peripheralIndex; }
    unsigned int          RawIndex(void) const        { return m_rawIndex; }
    JOYSTICK_STATE_BUTTON ButtonState(void) const     { return m_buttonState; }
    JOYSTICK_STATE_HAT    HatState(void) const        { return m_hatState; }
    JOYSTICK_STATE_AXIS   AxisState(void) const       { return m_axisState; }

    void SetType(JOYSTICK_EVENT_TYPE type)           { m_type            = type; }
    void SetPeripheralIndex(unsigned int index)      { m_peripheralIndex = index; }
    void SetRawIndex(unsigned int index)             { m_rawIndex        = index; }
    void SetButtonState(JOYSTICK_STATE_BUTTON state) { m_buttonState     = state; }
    void SetHatState(JOYSTICK_STATE_HAT state)       { m_hatState        = state; }
    void SetAxisState(JOYSTICK_STATE_AXIS state)     { m_axisState       = state; }

    void ToStruct(PERIPHERAL_EVENT& event) const
    {
      event.type             = m_type;
      event.peripheral_index = m_peripheralIndex;
      event.raw_index        = m_rawIndex;
      switch (m_type)
      {
        case JOYSTICK_EVENT_TYPE_RAW_BUTTON: event.button_state = m_buttonState; break;
        case JOYSTICK_EVENT_TYPE_RAW_HAT:    event.hat_state    = m_hatState;    break;
        case JOYSTICK_EVENT_TYPE_RAW_AXIS:   event.axis_state   = m_axisState;   break;
        case JOYSTICK_EVENT_TYPE_NONE:
        default: break;
      }
    }

    static void FreeStruct(PERIPHERAL_EVENT& event)
    {
      (void)event;
    }

  private:
    JOYSTICK_EVENT_TYPE   m_type;
    unsigned int          m_peripheralIndex;
    unsigned int          m_rawIndex;
    JOYSTICK_STATE_BUTTON m_buttonState;
    JOYSTICK_STATE_HAT    m_hatState;
    JOYSTICK_STATE_AXIS   m_axisState;
  };

  typedef PeripheralVector<PeripheralEvent, PERIPHERAL_EVENT> PeripheralEvents;

  class ButtonMapValue
  {
  public:
    ButtonMapValue(void)
    : m_value()
    {
    }

    ButtonMapValue(unsigned int buttonIndex)
    : m_value()
    {
      SetButtonIndex(buttonIndex);
    }

    ButtonMapValue(unsigned int hatIndex, JOYSTICK_HAT_DIRECTION direction)
    : m_value()
    {
      SetHatDirection(hatIndex, direction);
    }

    ButtonMapValue(unsigned int axisIndex, bool bInverted)
    : m_value()
    {
      SetSemiAxis(axisIndex, bInverted);
    }

    ButtonMapValue(unsigned int horizIndex, bool horizInverted, unsigned int vertIndex, bool vertInverted)
    : m_value()
    {
      SetAnalogStick(horizIndex, horizInverted, vertIndex, vertInverted);
    }

    ButtonMapValue(unsigned int xIndex, bool xInverted, unsigned int yIndex, bool yInverted, unsigned int zIndex, bool zInverted)
    : m_value()
    {
      SetAccelerometer(xIndex, xInverted, yIndex, yInverted, zIndex, zInverted);
    }

    ButtonMapValue(const JOYSTICK_BUTTONMAP_VALUE& value)
    : m_value(value)
    {
    }

    JOYSTICK_BUTTONMAP_VALUE_TYPE Type(void) const { return m_value.type; }
    
    JOYSTICK_BUTTONMAP_VALUE::button&        Button(void)        { return m_value.button_value; }
    JOYSTICK_BUTTONMAP_VALUE::hat&           Hat(void)           { return m_value.hat_value; }
    JOYSTICK_BUTTONMAP_VALUE::semiaxis&      SemiAxis(void)      { return m_value.semiaxis_value; }
    JOYSTICK_BUTTONMAP_VALUE::analog_stick&  AnalogStick(void)   { return m_value.analog_stick_value; }
    JOYSTICK_BUTTONMAP_VALUE::accelerometer& Accelerometer(void) { return m_value.accelerometer_value; }

    const JOYSTICK_BUTTONMAP_VALUE::button&        Button(void)        const { return m_value.button_value; }
    const JOYSTICK_BUTTONMAP_VALUE::hat&           Hat(void)           const { return m_value.hat_value; }
    const JOYSTICK_BUTTONMAP_VALUE::semiaxis&      SemiAxis(void)      const { return m_value.semiaxis_value; }
    const JOYSTICK_BUTTONMAP_VALUE::analog_stick&  AnalogStick(void)   const { return m_value.analog_stick_value; }
    const JOYSTICK_BUTTONMAP_VALUE::accelerometer& Accelerometer(void) const { return m_value.accelerometer_value; }

    void SetButtonIndex(unsigned int buttonIndex)
    {
      m_value.type   = JOYSTICK_BUTTONMAP_VALUE_BUTTON;
      Button().index = buttonIndex;
    }

    void SetHatDirection(unsigned int hatIndex, JOYSTICK_HAT_DIRECTION direction)
    {
      m_value.type    = JOYSTICK_BUTTONMAP_VALUE_HAT_DIRECTION;
      Hat().index     = hatIndex;
      Hat().direction = direction;
    }

    void SetSemiAxis(unsigned int axisIndex, bool bInverted)
    {
      m_value.type         = JOYSTICK_BUTTONMAP_VALUE_SEMIAXIS;
      SemiAxis().index     = axisIndex;
      SemiAxis().direction = bInverted ? JOYSTICK_SEMIAXIS_DIRECTION_NEGATIVE : JOYSTICK_SEMIAXIS_DIRECTION_POSITIVE;
    }

    void SetAnalogStick(unsigned int horizIndex, bool horizInverted, 
                        unsigned int vertIndex,  bool vertInverted)
    {
      m_value.type                  = JOYSTICK_BUTTONMAP_VALUE_ANALOG_STICK;
      AnalogStick().right.index     = horizIndex;
      AnalogStick().right.direction = horizInverted ? JOYSTICK_SEMIAXIS_DIRECTION_NEGATIVE : JOYSTICK_SEMIAXIS_DIRECTION_POSITIVE;
      AnalogStick().up.index        = vertIndex;
      AnalogStick().up.direction    = vertInverted ? JOYSTICK_SEMIAXIS_DIRECTION_NEGATIVE : JOYSTICK_SEMIAXIS_DIRECTION_POSITIVE;
    }

    void SetAccelerometer(unsigned int xIndex, bool xInverted, 
                          unsigned int yIndex, bool yInverted,
                          unsigned int zIndex, bool zInverted)
    {
      m_value.type                = JOYSTICK_BUTTONMAP_VALUE_ACCELEROMETER;
      Accelerometer().x.index     = xIndex;
      Accelerometer().x.direction = xInverted ? JOYSTICK_SEMIAXIS_DIRECTION_NEGATIVE : JOYSTICK_SEMIAXIS_DIRECTION_POSITIVE;
      Accelerometer().y.index     = yIndex;
      Accelerometer().y.direction = yInverted ? JOYSTICK_SEMIAXIS_DIRECTION_NEGATIVE : JOYSTICK_SEMIAXIS_DIRECTION_POSITIVE;
      Accelerometer().z.index     = zIndex;
      Accelerometer().z.direction = zInverted ? JOYSTICK_SEMIAXIS_DIRECTION_NEGATIVE : JOYSTICK_SEMIAXIS_DIRECTION_POSITIVE;
    }

    void ToStruct(JOYSTICK_BUTTONMAP_VALUE& value) const
    {
      value = m_value;
    }

  private:
    JOYSTICK_BUTTONMAP_VALUE m_value;
  };

  typedef std::map<JOYSTICK_ID, ButtonMapValue> ButtonMap;

  class ButtonMaps
  {
  public:
    static void ToStruct(const ButtonMap& buttonMap, JOYSTICK_BUTTONMAP& buttonMapStruct)
    {
      buttonMapStruct.size = 0;
      if (buttonMap.empty())
      {
        buttonMapStruct.keys   = NULL;
        buttonMapStruct.values = NULL;
      }
      else
      {
        buttonMapStruct.keys   = new JOYSTICK_BUTTONMAP_KEY[buttonMapStruct.size];
        buttonMapStruct.values = new JOYSTICK_BUTTONMAP_VALUE[buttonMapStruct.size];
        for (ButtonMap::const_iterator it = buttonMap.begin(); it != buttonMap.end(); ++it)
        {
          unsigned int i = buttonMapStruct.size++;
          buttonMapStruct.keys[i] = it->first;
          it->second.ToStruct(buttonMapStruct.values[i]);
        }
      }
    }

    static void ToMap(const JOYSTICK_BUTTONMAP& buttonMapStruct, ButtonMap& buttonMap)
    {
      buttonMap.clear();
      if (buttonMapStruct.keys && buttonMapStruct.values)
      {
        for (unsigned int i = 0; i < buttonMapStruct.size; i++)
          buttonMap[buttonMapStruct.keys[i]] = buttonMapStruct.values[i];
      }
    }

    static void FreeStruct(JOYSTICK_BUTTONMAP& buttonMapStruct)
    {
      SAFE_DELETE_ARRAY(buttonMapStruct.keys);
      SAFE_DELETE_ARRAY(buttonMapStruct.values);
    }
  };
}
