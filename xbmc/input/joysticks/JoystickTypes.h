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

#include "guilib/Key.h"

#include <stdint.h>

/*!
 * \ingroup joysticks
 * \brief Action IDs for physical elements on a joystick
 */
enum JoystickActionID
{
  JOYSTICK_ID_BUTTON_A       = KEY_BUTTON_A,
  JOYSTICK_ID_BUTTON_B       = KEY_BUTTON_B,
  JOYSTICK_ID_BUTTON_C       = KEY_BUTTON_BLACK,
  JOYSTICK_ID_BUTTON_X       = KEY_BUTTON_X,
  JOYSTICK_ID_BUTTON_Y       = KEY_BUTTON_Y,
  JOYSTICK_ID_BUTTON_Z       = KEY_BUTTON_WHITE,
  JOYSTICK_ID_BUTTON_START   = KEY_BUTTON_START,
  JOYSTICK_ID_BUTTON_SELECT  = KEY_BUTTON_BACK,
  JOYSTICK_ID_BUTTON_MODE    = KEY_BUTTON_GUIDE,
  JOYSTICK_ID_BUTTON_L       = KEY_BUTTON_LEFT_SHOULDER,
  JOYSTICK_ID_BUTTON_R       = KEY_BUTTON_RIGHT_SHOULDER,
  JOYSTICK_ID_TRIGGER_L      = KEY_BUTTON_LEFT_TRIGGER,
  JOYSTICK_ID_TRIGGER_R      = KEY_BUTTON_RIGHT_TRIGGER,
  JOYSTICK_ID_BUTTON_L_STICK = KEY_BUTTON_LEFT_THUMB_BUTTON,
  JOYSTICK_ID_BUTTON_R_STICK = KEY_BUTTON_RIGHT_THUMB_BUTTON,
  JOYSTICK_ID_BUTTON_LEFT    = KEY_BUTTON_DPAD_LEFT,
  JOYSTICK_ID_BUTTON_RIGHT   = KEY_BUTTON_DPAD_RIGHT,
  JOYSTICK_ID_BUTTON_UP      = KEY_BUTTON_DPAD_UP,
  JOYSTICK_ID_BUTTON_DOWN    = KEY_BUTTON_DPAD_DOWN,
  JOYSTICK_ID_ANALOG_STICK_L = KEY_BUTTON_LEFT_THUMB_STICK_UP,
  JOYSTICK_ID_ANALOG_STICK_R = KEY_BUTTON_RIGHT_THUMB_STICK_UP,
  JOYSTICK_ID_ACCELEROMETER  = KEY_BUTTON_ACCELEROMETER,
};

/*!
 * \ingroup joysticks
 * \brief States in which a hat (directional pad) can be
 */
enum HatDirection
{
  HatDirectionNone      = 0x0,    /*!< @brief no directions are pressed */
  HatDirectionLeft      = 0x1,    /*!< @brief only left is pressed */
  HatDirectionRight     = 0x2,    /*!< @brief only right is pressed */
  HatDirectionUp        = 0x4,    /*!< @brief only up is pressed */
  HatDirectionDown      = 0x8,    /*!< @brief only down is pressed */
  HatDirectionLeftUp    = HatDirectionLeft  | HatDirectionUp,
  HatDirectionLeftDown  = HatDirectionLeft  | HatDirectionDown,
  HatDirectionRightUp   = HatDirectionRight | HatDirectionUp,
  HatDirectionRightDown = HatDirectionRight | HatDirectionDown,
};

/*!
 * \ingroup joysticks
 * \brief Directions a semiaxis can point
 */
enum SemiAxisDirection
{
  SemiAxisDirectionNegative = -1,
  SemiAxisDirectionUnknown  =  0,
  SemiAxisDirectionPositive =  1,
};

/*!
 * \ingroup joysticks
 * \brief Container for fields uniquely identifying joysticks
 */
struct JoystickIdentifier
{
  std::string strProvider; // \brief Standard name of the API providing the joystick
  std::string strName;     // \brief Name of the joystick reported by the provider, or empty if unknown
  uint16_t    vendorId;    // \brief Vendor ID of the joystick, or 0x0000 if unknown
  uint16_t    productId;   // \brief Product ID of the joystick, or 0x0000 if unknown
  int         buttonCount; // \brief Number of raw buttons reported by the provider, or -1 if unknown
  int         hatCount;    // \brief Number of raw hats reported by the provider, or -1 if unknown
  int         axisCount;   // \brief Number of raw axes reported by the provider, or -1 if unknown
};

/*!
 * \ingroup joysticks
 * \brief Functor to sort joystick identifiers
 */
struct JoystickLessThan
{
  bool operator()(const JoystickIdentifier& lhs, const JoystickIdentifier& rhs)
  {
    if (lhs.strProvider < rhs.strProvider) return true;
    if (lhs.strProvider > rhs.strProvider) return false;

    if (!lhs.strName.empty() && !rhs.strName.empty())
    {
      if (lhs.strName < rhs.strName) return true;
      if (lhs.strName > rhs.strName) return false;
    }

    if (lhs.vendorId && rhs.vendorId)
    {
      if (lhs.vendorId < rhs.vendorId) return true;
      if (lhs.vendorId > rhs.vendorId) return false;
    }

    if (lhs.productId && rhs.productId)
    {
      if (lhs.productId < rhs.productId) return true;
      if (lhs.productId > rhs.productId) return false;
    }

    if (lhs.buttonCount > 0 && rhs.buttonCount > 0)
    {
      if (lhs.buttonCount < rhs.buttonCount) return true;
      if (lhs.buttonCount > rhs.buttonCount) return false;
    }

    if (lhs.hatCount > 0 && rhs.hatCount > 0)
    {
      if (lhs.hatCount < rhs.hatCount) return true;
      if (lhs.hatCount > rhs.hatCount) return false;
    }

    if (lhs.axisCount > 0 && rhs.axisCount > 0)
    {
      if (lhs.axisCount < rhs.axisCount) return true;
      if (lhs.axisCount > rhs.axisCount) return false;
    }

    return false;
  }
};

enum ButtonPrimitiveType
{
  ButtonPrimitiveTypeUnknown = 0,
  ButtonPrimitiveTypeButton,
  ButtonPrimitiveTypeHatDirection,
  ButtonPrimitiveTypeSemiAxis,
};

class ButtonPrimitive
{
public:
  ButtonPrimitive(void)
    : m_type(),
      m_index(0),
      m_hatDirection(),
      m_axisDirection()
  {
  }

  ButtonPrimitive(unsigned int index)
    : m_type(ButtonPrimitiveTypeButton),
      m_index(index),
      m_hatDirection(),
      m_axisDirection()
  {
  }

  ButtonPrimitive(unsigned int index, HatDirection direction)
    : m_type(ButtonPrimitiveTypeHatDirection),
      m_index(index),
      m_hatDirection(direction),
      m_axisDirection()
  {
  }

  ButtonPrimitive(unsigned int index, SemiAxisDirection direction)
    : m_type(ButtonPrimitiveTypeSemiAxis),
      m_index(index),
      m_hatDirection(),
      m_axisDirection(direction)
  {
  }

  bool operator==(const ButtonPrimitive& rhs) const
  {
    if (m_type == rhs.m_type)
    {
      switch (m_type)
      {
      case ButtonPrimitiveTypeButton:
        return m_index == rhs.m_index;
      case ButtonPrimitiveTypeHatDirection:
        return m_index == rhs.m_index && m_hatDirection == rhs.m_hatDirection;
      case ButtonPrimitiveTypeSemiAxis:
        return m_index == rhs.m_index && m_axisDirection == rhs.m_axisDirection;
      case ButtonPrimitiveTypeUnknown:
      default:
        return true;
      }
    }
    return false;
  }

  bool operator<(const ButtonPrimitive& rhs) const
  {
    if (m_type < rhs.m_type) return true;
    if (m_type > rhs.m_type) return false;

    if (m_type)
    {
      if (m_index < rhs.m_index) return true;
      if (m_index > rhs.m_index) return false;
    }

    if (m_type == ButtonPrimitiveTypeHatDirection)
    {
      if (m_hatDirection < rhs.m_hatDirection) return true;
      if (m_hatDirection > rhs.m_hatDirection) return false;
    }

    if (m_type == ButtonPrimitiveTypeSemiAxis)
    {
      if (m_axisDirection < rhs.m_axisDirection) return true;
      if (m_axisDirection > rhs.m_axisDirection) return false;
    }

    return false;
  }

  ButtonPrimitiveType Type() const        { return m_type; }
  unsigned int        Index() const       { return m_index; }
  HatDirection        HatDir() const      { return m_hatDirection; }
  SemiAxisDirection   SemiAxisDir() const { return m_axisDirection; }

private:
  ButtonPrimitiveType m_type;
  unsigned int        m_index;
  HatDirection        m_hatDirection;
  SemiAxisDirection   m_axisDirection;
};

enum ButtonModifier
{
  ButtonModifierNone = 0,
  ButtonModifierHold,
  ButtonModifierDoublePress,
  ButtonModifierDoublePressHold,
  ButtonModifierMultiPress,
};

struct ButtonRecipe
{
  ButtonModifier               modifier;
  std::vector<ButtonPrimitive> buttons;
  JoystickActionID             action;
};

typedef std::vector<ButtonRecipe> ButtonMap;
