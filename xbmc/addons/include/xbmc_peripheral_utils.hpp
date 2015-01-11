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

  typedef PeripheralVector<Peripheral, PERIPHERAL_INFO>  Peripherals;

  /*!
   * ADDON::JoystickButton
   *
   * Wrapper class providing button information. Classes can extend
   * JoystickButton to inherit button properties.
   */
  class JoystickButton
  {
  public:
    JoystickButton(JOYSTICK_ID          id       = JOYSTICK_ID(),
                   JOYSTICK_BUTTON_TYPE type     = JOYSTICK_BUTTON_TYPE(),
                   const std::string&   strLabel = "")
    : m_id(id),
      m_type(type),
      m_strLabel(strLabel)
    {
    }

    JoystickButton(const JOYSTICK_BUTTON& button)
    : m_id(button.id),
      m_type(button.type),
      m_strLabel(button.label ? button.label : "")
    {
    }

    JOYSTICK_ID          ID(void) const    { return m_id; }
    JOYSTICK_BUTTON_TYPE Type(void) const  { return m_type; }
    const std::string&   Label(void) const { return m_strLabel; }

    void SetID(JOYSTICK_ID id)                 { m_id = id; }
    void SetType(JOYSTICK_BUTTON_TYPE type)    { m_type = type; }
    void SetLabel(const std::string& strLabel) { m_strLabel = strLabel; }

    void ToStruct(JOYSTICK_BUTTON& button) const
    {
      button.id    = m_id;
      button.type  = m_type;
      button.label = new char[m_strLabel.size() + 1];

      std::strcpy(button.label, m_strLabel.c_str());
    }

    static void FreeStruct(JOYSTICK_BUTTON& button)
    {
      SAFE_DELETE_ARRAY(button.label);
    }

  private:
    JOYSTICK_ID          m_id;
    JOYSTICK_BUTTON_TYPE m_type;
    std::string          m_strLabel;
  };

  typedef PeripheralVector<JoystickButton, JOYSTICK_BUTTON>  JoystickButtons;

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
    Joystick(const std::string& strName = "")
    : Peripheral(PERIPHERAL_TYPE_JOYSTICK, strName),
      m_requestedPlayer(0),
      m_buttonCount(0),
      m_hatCount(0),
      m_axisCount(0)
    {
    }

    Joystick(JOYSTICK_INFO& info)
    : Peripheral(info.peripheral_info),
      m_requestedPlayer(info.requested_player_num),
      m_buttonCount(info.virtual_layout.button_count),
      m_hatCount(info.virtual_layout.hat_count),
      m_axisCount(info.virtual_layout.axis_count)
    {
      SetType(PERIPHERAL_TYPE_JOYSTICK);

      if (info.physical_layout.buttons)
      {
        for (unsigned int i = 0; i < info.physical_layout.button_count; i++)
          m_buttons.push_back(info.physical_layout.buttons[i]);
      }
    }

    virtual ~Joystick(void) { }

    unsigned int       RequestedPlayer(void) const { return m_requestedPlayer; }
    unsigned int       ButtonCount(void) const     { return m_buttonCount; }
    unsigned int       HatCount(void) const        { return m_hatCount; }
    unsigned int       AxisCount(void) const       { return m_axisCount; }

    const std::vector<JoystickButton>& Buttons(void) const { return m_buttons; }

    void SetRequestedPlayer(unsigned int requestedPlayer) { m_requestedPlayer = requestedPlayer; }
    void SetButtonCount(unsigned int buttonCount)         { m_buttonCount     = buttonCount; }
    void SetHatCount(unsigned int hatCount)               { m_hatCount        = hatCount; }
    void SetAxisCount(unsigned int axisCount)             { m_axisCount       = axisCount; }

    std::vector<JoystickButton>& Buttons(void) { return m_buttons; }

    void ToStruct(JOYSTICK_INFO& info) const
    {
      Peripheral::ToStruct(info.peripheral_info);

      info.requested_player_num         = m_requestedPlayer;
      info.virtual_layout.button_count  = m_buttonCount;
      info.virtual_layout.hat_count     = m_hatCount;
      info.virtual_layout.axis_count    = m_axisCount;
      info.physical_layout.button_count = m_buttons.size();

      JoystickButtons::ToStructs(m_buttons, &info.physical_layout.buttons);
    }

    static void FreeStruct(JOYSTICK_INFO& info)
    {
      Peripheral::FreeStruct(info.peripheral_info);
      JoystickButtons::FreeStructs(info.physical_layout.button_count, info.physical_layout.buttons);
    }

  private:
    unsigned int                m_requestedPlayer;
    unsigned int                m_buttonCount;
    unsigned int                m_hatCount;
    unsigned int                m_axisCount;
    std::vector<JoystickButton> m_buttons;
  };

  typedef PeripheralVector<Joystick, JOYSTICK_INFO>    Joysticks;

  /*!
   * ADDON::PeripheralEvent
   *
   * Wrapper class for peripheral events. Classes can extend PeripheralEvent to
   * inherit event properties.
   */
  /*
  class PeripheralEvent
  {
  public:
    PeripheralEvent(JOYSTICK_EVENT_TYPE type = JOYSTICK_EVENT_TYPE_NONE, unsigned int peripheralIndex = 0)
    : m_type(type),
      m_peripheralIndex(peripheralIndex),
      m_virtualIndex(0),
      m_buttonId(),
      m_digitalState(),
      m_analogState1(),
      m_analogState2(),
      m_analogState3()
    {
    }

    PeripheralEvent(const PERIPHERAL_EVENT& event)
    : m_type(event.type),
      m_peripheralIndex(event.peripheral_index),
      m_virtualIndex(0),
      m_buttonId(),
      m_digitalState(),
      m_hatState(),
      m_analogState1(),
      m_analogState2(),
      m_analogState3()
    {
      switch (m_type)
      {
      case JOYSTICK_EVENT_TYPE_VIRTUAL_BUTTON:
        m_virtualIndex = event.virtual_index;
        SetDigitalState(event.digital_state);
        break;
      case JOYSTICK_EVENT_TYPE_VIRTUAL_HAT:
        m_virtualIndex = event.virtual_index;
        SetHatState(event.hat_state);
        break;
      case JOYSTICK_EVENT_TYPE_VIRTUAL_AXIS:
        m_virtualIndex = event.virtual_index;
        SetAnalogState(event.analog_state);
        break;
      case JOYSTICK_EVENT_TYPE_BUTTON_DIGITAL:
        m_buttonId = event.button_id;
        SetDigitalState(event.digital_state);
        break;
      case JOYSTICK_EVENT_TYPE_BUTTON_ANALOG:
        m_buttonId = event.button_id;
        SetAnalogState(event.analog_state);
        break;
      case JOYSTICK_EVENT_TYPE_ANALOG_STICK:
      case JOYSTICK_EVENT_TYPE_ANALOG_STICK_THRESHOLD:
        m_buttonId = event.button_id;
        SetAnalogStick(event.analog_stick.horiz, event.analog_stick.vert);
        break;
      case JOYSTICK_EVENT_TYPE_ACCELEROMETER:
        m_buttonId = event.button_id;
        SetAccelerometer(event.accelerometer.x, event.accelerometer.y, event.accelerometer.z);
        break;
      case JOYSTICK_EVENT_TYPE_NONE:
      default:
        break;
      }
    }

    virtual ~PeripheralEvent(void) { }

    JOYSTICK_EVENT_TYPE   Type(void) const            { return m_type; }
    unsigned int          PeripheralIndex(void) const { return m_peripheralIndex; }
    unsigned int          VirtualIndex(void) const    { return m_virtualIndex; }
    JOYSTICK_ID           ButtonID(void) const        { return m_buttonId; }
    JOYSTICK_STATE_BUTTON DigitalState(void) const    { return m_digitalState; }
    JOYSTICK_STATE_HAT    HatState(void) const        { return m_hatState; }
    JOYSTICK_STATE_ANALOG AnalogState(void) const     { return m_analogState1; }
    JOYSTICK_STATE_ANALOG HorizontalState(void) const { return m_analogState1; }
    JOYSTICK_STATE_ANALOG VerticalState(void) const   { return m_analogState2; }
    JOYSTICK_STATE_ANALOG X(void) const               { return m_analogState1; }
    JOYSTICK_STATE_ANALOG Y(void) const               { return m_analogState2; }
    JOYSTICK_STATE_ANALOG Z(void) const               { return m_analogState3; }

    void SetType(JOYSTICK_EVENT_TYPE type)                { m_type            = type; }
    void SetPeripheralIndex(unsigned int peripheralIndex) { m_peripheralIndex = peripheralIndex; }
    void SetVirtualIndex(unsigned int virtualIndex)       { m_virtualIndex    = virtualIndex; }
    void SetButtonID(JOYSTICK_ID buttonId)                { m_buttonId        = buttonId; }

    void SetDigitalState(JOYSTICK_STATE_BUTTON digitalState)
    {
      m_digitalState = digitalState;
      m_analogState1 = digitalState ? 1.0f : 0.0f;
    }

    void SetHatState(JOYSTICK_STATE_HAT hatState)
    {
      m_hatState     = hatState;
      m_digitalState = hatState ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;
    }

    void SetAnalogState(JOYSTICK_STATE_ANALOG analogState)
    {
      m_analogState1 = CONSTRAIN(-1.0f, analogState, 1.0f);
      m_digitalState = analogState > DIGITAL_ANALOG_THRESHOLD ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;
    }

    void SetAnalogStick(JOYSTICK_STATE_ANALOG horizontal, JOYSTICK_STATE_ANALOG vertical)
    {
      m_analogState1 = CONSTRAIN(-1.0f, horizontal, 1.0f);
      m_analogState2 = CONSTRAIN(-1.0f, vertical,   1.0f);
      m_digitalState = std::sqrt(m_analogState1 * m_analogState1 +
                                 m_analogState2 * m_analogState2) > DIGITAL_ANALOG_THRESHOLD ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;
    }

    void SetAccelerometer(JOYSTICK_STATE_ANALOG x, JOYSTICK_STATE_ANALOG y, JOYSTICK_STATE_ANALOG z)
    {
      m_analogState1 = CONSTRAIN(-1.0f, x, 1.0f);
      m_analogState2 = CONSTRAIN(-1.0f, y, 1.0f);
      m_analogState3 = CONSTRAIN(-1.0f, z, 1.0f);
    }

    void ToStruct(PERIPHERAL_EVENT& event) const
    {
      event.type             = m_type;
      event.peripheral_index = m_peripheralIndex;

      switch (m_type)
      {
      case JOYSTICK_EVENT_TYPE_VIRTUAL_BUTTON:
        event.virtual_index = m_virtualIndex;
        event.digital_state = m_digitalState;
        break;
      case JOYSTICK_EVENT_TYPE_VIRTUAL_HAT:
        event.virtual_index = m_virtualIndex;
        event.hat_state     = m_hatState;
        break;
      case JOYSTICK_EVENT_TYPE_VIRTUAL_AXIS:
        event.virtual_index = m_virtualIndex;
        event.analog_state  = m_analogState1;
        break;
      case JOYSTICK_EVENT_TYPE_BUTTON_DIGITAL:
        event.button_id     = m_buttonId;
        event.digital_state = m_digitalState;
        break;
      case JOYSTICK_EVENT_TYPE_BUTTON_ANALOG:
        event.button_id    = m_buttonId;
        event.analog_state = m_analogState1;
        break;
      case JOYSTICK_EVENT_TYPE_ANALOG_STICK:
      case JOYSTICK_EVENT_TYPE_ANALOG_STICK_THRESHOLD:
        event.button_id          = m_buttonId;
        event.analog_stick.horiz = m_analogState1;
        event.analog_stick.vert  = m_analogState2;
        break;
      case JOYSTICK_EVENT_TYPE_ACCELEROMETER:
        event.button_id       = m_buttonId;
        event.accelerometer.x = m_analogState1;
        event.accelerometer.y = m_analogState2;
        event.accelerometer.z = m_analogState3;
        break;
      case JOYSTICK_EVENT_TYPE_NONE:
      default:
        break;
      }
    }

    static void FreeStruct(PERIPHERAL_EVENT& event)
    {
      (void)event;
    }

  private:
    JOYSTICK_EVENT_TYPE    m_type;
    unsigned int           m_peripheralIndex;
    unsigned int           m_virtualIndex;
    JOYSTICK_ID            m_buttonId;
    JOYSTICK_STATE_BUTTON  m_digitalState;
    JOYSTICK_STATE_HAT     m_hatState;
    JOYSTICK_STATE_ANALOG  m_analogState1;
    JOYSTICK_STATE_ANALOG  m_analogState2;
    JOYSTICK_STATE_ANALOG  m_analogState3;
  };

  typedef PeripheralVector<PeripheralEvent, PERIPHERAL_EVENT> PeripheralEvents;

  /**/
}
