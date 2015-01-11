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

/*!
 * \ingroup joysticks
 * \brief Action IDs for physical elements on a joystick
 */
enum JoystickActionID
{
  JOYSTICK_ID_BUTTON_UNKNOWN = 0,
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
