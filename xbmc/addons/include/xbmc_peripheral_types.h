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
#ifndef __PERIPHERAL_TYPES_H__
#define __PERIPHERAL_TYPES_H__

#ifdef TARGET_WINDOWS
  #include <windows.h>
#else
  #ifndef __cdecl
    #define __cdecl
  #endif
  #ifndef __declspec
    #define __declspec(X)
  #endif
#endif

#undef ATTRIBUTE_PACKED
#undef PRAGMA_PACK_BEGIN
#undef PRAGMA_PACK_END

#if defined(__GNUC__)
  #if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)
    #define ATTRIBUTE_PACKED __attribute__ ((packed))
    #define PRAGMA_PACK 0
  #endif
#endif

#if !defined(ATTRIBUTE_PACKED)
  #define ATTRIBUTE_PACKED
  #define PRAGMA_PACK 1
#endif

//#define PERIPHERAL_NAME_STRING_LENGTH   1024

/* current Peripheral API version */
#define PERIPHERAL_API_VERSION "1.0.0"

/* min. Peripheral API version */
#define PERIPHERAL_MIN_API_VERSION "1.0.0"

#ifdef __cplusplus
extern "C"
{
#endif

  /// @name Peripheral types
  ///{
  typedef enum PERIPHERAL_ERROR
  {
    PERIPHERAL_NO_ERROR                      =  0, // no error occurred
    PERIPHERAL_ERROR_UNKNOWN                 = -1, // an unknown error occurred
    PERIPHERAL_ERROR_FAILED                  = -2, // the command failed
    PERIPHERAL_ERROR_INVALID_PARAMETERS      = -3, // the parameters of the method are invalid for this operation
    PERIPHERAL_ERROR_NOT_IMPLEMENTED         = -4, // the method that the frontend called is not implemented
    PERIPHERAL_ERROR_NOT_CONNECTED           = -5, // no peripherals are connected
    PERIPHERAL_ERROR_CONNECTION_FAILED       = -6, // peripherals are connected, but command was interrupted
  } PERIPHERAL_ERROR;

  typedef enum PERIPHERAL_TYPE
  {
    PERIPHERAL_TYPE_UNKNOWN = 0,
    PERIPHERAL_TYPE_JOYSTICK,
  } PERIPHERAL_TYPE;

  typedef struct PERIPHERAL_INFO
  {
    PERIPHERAL_TYPE type;
    char*           name;
    unsigned int    index;
    unsigned int    vendor_id;
    unsigned int    product_id;
  } ATTRIBUTE_PACKED PERIPHERAL_INFO;

  /*!
   * @brief Properties passed to the Create() method of an add-on.
   */
  typedef struct PERIPHERAL_PROPERTIES
  {
    const char* user_path;              /*!< @brief path to the user profile */
    const char* addon_path;             /*!< @brief path to this add-on */
  } ATTRIBUTE_PACKED PERIPHERAL_PROPERTIES;

  /*!
   * @brief Peripheral add-on capabilities.
   * If a capability is set to true, then the corresponding methods from
   * xbmc_peripheral_dll.h need to be implemented.
   */
  typedef struct PERIPHERAL_CAPABILITIES
  {
    bool provides_joysticks;            /*!< @brief true if the add-on provides joysticks */
  } ATTRIBUTE_PACKED PERIPHERAL_CAPABILITIES;
  ///}

  /// @name Joystick types
  typedef enum JOYSTICK_ID
  {
    JOYSTICK_ID_BUTTON_UNKNOWN = 0,      /*!< @brief no data exists to associate button with ID */
    JOYSTICK_ID_BUTTON_A,                /*!< @brief corresponds to A (generic) or Cross (Sony) */
    JOYSTICK_ID_BUTTON_B,                /*!< @brief corresponds to B (generic) or Circle (Sony) */
    JOYSTICK_ID_BUTTON_X,                /*!< @brief corresponds to C or X (generic), Square (Sony), C-down (N64) or One (Wii)*/
    JOYSTICK_ID_BUTTON_Y,                /*!< @brief corresponds to Y (generic), Triangle (Sony), C-left (N64) or Two (Wii) */
    JOYSTICK_ID_BUTTON_5,                /*!< @brief corresponds to Black (Xbox) or C-right (N64) */
    JOYSTICK_ID_BUTTON_6,                /*!< @brief corresponds to White (Xbox) or C-up (N64) */
    JOYSTICK_ID_BUTTON_START,            /*!< @brief corresponds to Start (generic) */
    JOYSTICK_ID_BUTTON_SELECT,           /*!< @brief corresponds to Select (generic) or Back (Xbox) */
    JOYSTICK_ID_BUTTON_HOME,             /*!< @brief corresponds to Guide (Xbox) or Analog (Sony) */
    JOYSTICK_ID_BUTTON_UP,               /*!< @brief corresponds to Up on the directional pad */
    JOYSTICK_ID_BUTTON_DOWN,             /*!< @brief corresponds to Down on the directional pad */
    JOYSTICK_ID_BUTTON_LEFT,             /*!< @brief corresponds to Left on the directional pad */
    JOYSTICK_ID_BUTTON_RIGHT,            /*!< @brief corresponds to Right on the directional pad */
    JOYSTICK_ID_BUTTON_L,                /*!< @brief corresponds to Left shoulder button (generic) */
    JOYSTICK_ID_BUTTON_R,                /*!< @brief corresponds to Right shoulder button (generic) */
    JOYSTICK_ID_BUTTON_L_STICK,          /*!< @brief corresponds to Left stick (Xbox, Sony) */
    JOYSTICK_ID_BUTTON_R_STICK,          /*!< @brief corresponds to Right stick (Xbox, Sony) */
    JOYSTICK_ID_TRIGGER_L,               /*!< @brief corresponds to Left trigger (generic) or L2 (Sony) */
    JOYSTICK_ID_TRIGGER_R,               /*!< @brief corresponds to Right trigger (generic) or R2 (Sony) */
    JOYSTICK_ID_ANALOG_STICK_LEFT,       /*!< @brief corresponds to Left analog stick */
    JOYSTICK_ID_ANALOG_STICK_RIGHT,      /*!< @brief corresponds to Right analog stick */
    JOYSTICK_ID_ACCELEROMETER,           /*!< @brief corresponds to Accelerometer (Wii/Sixaxis) */
  } JOYSTICK_ID;

  typedef enum JOYSTICK_BUTTON_TYPE
  {
    JOYSTICK_BUTTON_TYPE_UNKNOWN = 0,   /*!< @brief No data on button */
    JOYSTICK_BUTTON_TYPE_DIGITAL,       /*!< @brief Digital button */
    JOYSTICK_BUTTON_TYPE_ANALOG,        /*!< @brief Pressure-sensitive button or trigger */
  } JOYSTICK_BUTTON_TYPE;

  typedef struct JOYSTICK_BUTTON
  {
    JOYSTICK_ID             id;           /*!< @brief the physical button ID */
    JOYSTICK_BUTTON_TYPE    type;         /*!< @brief button type */
    char*                   label;        /*!< @brief the label, e.g. "X" or "Square" for JOYSTICK_ID_BUTTON_3 */
  } ATTRIBUTE_PACKED JOYSTICK_BUTTON;

  typedef struct JOYSTICK_VIRTUAL_LAYOUT
  {
    unsigned int    button_count;     /*!< @brief number of buttons reported by the driver */
    unsigned int    hat_count;        /*!< @brief number of hats reported by the driver */
    unsigned int    axis_count;       /*!< @brief number of axes reported by the driver */
  } ATTRIBUTE_PACKED JOYSTICK_VIRTUAL_LAYOUT;

  typedef struct JOYSTICK_PHYSICAL_LAYOUT
  {
    unsigned int           button_count;       /*!< @brief the number of buttons in this joystick map */
    JOYSTICK_BUTTON*       buttons;            /*!< @brief the button mappings */
  } ATTRIBUTE_PACKED JOYSTICK_PHYSICAL_LAYOUT;

  typedef struct JOYSTICK_INFO
  {
    PERIPHERAL_INFO          peripheral_info;
    unsigned int             requested_player_num;
    JOYSTICK_VIRTUAL_LAYOUT  virtual_layout;
    JOYSTICK_PHYSICAL_LAYOUT physical_layout;
  } ATTRIBUTE_PACKED JOYSTICK_INFO;

  typedef enum JOYSTICK_EVENT_TYPE
  {
    JOYSTICK_EVENT_TYPE_NONE = 0,            /*!< @brief unknown event */
    JOYSTICK_EVENT_TYPE_RAW_BUTTON,          /*!< @brief state changed for raw button, reported by its index */
    JOYSTICK_EVENT_TYPE_RAW_HAT,             /*!< @brief state changed for raw hat, reported by its index */
    JOYSTICK_EVENT_TYPE_RAW_AXIS,            /*!< @brief state changed for raw axis, reported by its index */
  } JOYSTICK_EVENT_TYPE;

  typedef enum JOYSTICK_STATE_BUTTON
  {
    JOYSTICK_STATE_BUTTON_UNPRESSED = 0x0,    /*!< @brief button is unpressed */
    JOYSTICK_STATE_BUTTON_PRESSED   = 0x1,    /*!< @brief button is pressed */
  } JOYSTICK_STATE_BUTTON;

  typedef enum JOYSTICK_STATE_HAT
  {
    JOYSTICK_STATE_HAT_UNPRESSED  = 0x0,    /*!< @brief no directions are pressed */
    JOYSTICK_STATE_HAT_LEFT       = 0x4,    /*!< @brief only left is pressed */
    JOYSTICK_STATE_HAT_RIGHT      = 0x8,    /*!< @brief only right is pressed */
    JOYSTICK_STATE_HAT_UP         = 0x1,    /*!< @brief only up is pressed */
    JOYSTICK_STATE_HAT_DOWN       = 0x2,    /*!< @brief only down is pressed */
    JOYSTICK_STATE_HAT_LEFT_UP    = JOYSTICK_STATE_HAT_LEFT  | JOYSTICK_STATE_HAT_UP,
    JOYSTICK_STATE_HAT_LEFT_DOWN  = JOYSTICK_STATE_HAT_LEFT  | JOYSTICK_STATE_HAT_DOWN,
    JOYSTICK_STATE_HAT_RIGHT_UP   = JOYSTICK_STATE_HAT_RIGHT | JOYSTICK_STATE_HAT_UP,
    JOYSTICK_STATE_HAT_RIGHT_DOWN = JOYSTICK_STATE_HAT_RIGHT | JOYSTICK_STATE_HAT_DOWN,
  } JOYSTICK_STATE_HAT;

  typedef float JOYSTICK_STATE_AXIS;     /*!< @brief value in the interval [-1, 1], inclusive */

  typedef struct PERIPHERAL_EVENT
  {
    JOYSTICK_EVENT_TYPE      type;
    unsigned int             peripheral_index;
    unsigned int             raw_index;
    union
    {
      JOYSTICK_STATE_BUTTON  button_state;
      JOYSTICK_STATE_HAT     hat_state;
      JOYSTICK_STATE_AXIS    axis_state;
    };
  } ATTRIBUTE_PACKED PERIPHERAL_EVENT;

  typedef enum JOYSTICK_SEMI_AXIS_DIRECTION
  {
    JOYSTICK_SEMI_AXIS_DIRECTION_NEGATIVE = -1,     /*!< @brief negative part of the axis in the interval [-1, 0) */
    JOYSTICK_SEMI_AXIS_DIRECTION_UNKNOWN  =  0,     /*!< @brief positive part of the axis in the interval (0, 1] */
    JOYSTICK_SEMI_AXIS_DIRECTION_POSITIVE =  1,     /*!< @brief positive part of the axis in the interval (0, 1] */
  } JOYSTICK_SEMI_AXIS_DIRECTION;

  typedef enum JOYSTICK_BUTTON_PRIMITIVE_TYPE
  {
    JOYSTICK_BUTTON_MAP_VALUE_UNKNOWN = 0,
    JOYSTICK_BUTTON_MAP_VALUE_BUTTON,
    JOYSTICK_BUTTON_MAP_VALUE_HAT_DIRECTION,
    JOYSTICK_BUTTON_MAP_VALUE_SEMI_AXIS,
  } JOYSTICK_BUTTON_MAP_VALUE_TYPE;

  typedef struct JOYSTICK_BUTTON_PRIMITIVE
  {
    JOYSTICK_BUTTON_PRIMITIVE_TYPE type;
    unsigned int                   index;
    JOYSTICK_STATE_HAT             hat_direction;
    JOYSTICK_SEMI_AXIS_DIRECTION   semi_axis_dir;
  } ATTRIBUTE_PACKED JOYSTICK_BUTTON_PRIMITIVE;
  ///}

  // TODO: Mouse, light gun, multitouch

  /*!
   * @brief Structure to transfer the methods from xbmc_peripheral_dll.h to the frontend
   */
  typedef struct PeripheralAddon
  {
    const char*      (__cdecl* GetPeripheralAPIVersion)(void);
    const char*      (__cdecl* GetMinimumPeripheralAPIVersion)(void);
    PERIPHERAL_ERROR (__cdecl* GetAddonCapabilities)(PERIPHERAL_CAPABILITIES*);
    PERIPHERAL_ERROR (__cdecl* PerformDeviceScan)(unsigned int*, PERIPHERAL_INFO**);
    void             (__cdecl* FreeScanResults)(unsigned int, PERIPHERAL_INFO*);

    /// @name Joystick operations
    ///{
    PERIPHERAL_ERROR (__cdecl* GetJoystickInfo)(unsigned int, JOYSTICK_INFO*);
    void             (__cdecl* FreeJoystickInfo)(JOYSTICK_INFO*);
    PERIPHERAL_ERROR (__cdecl* GetEvents)(unsigned int*, PERIPHERAL_EVENT**);
    void             (__cdecl* FreeEvents)(unsigned int, PERIPHERAL_EVENT*);
    JOYSTICK_ID      (__cdecl* GetAction)(JOYSTICK_BUTTON_PRIMITIVE*);
    JOYSTICK_ID      (__cdecl* GetAnalogStick)(unsigned int, unsigned int*, unsigned int*);
    JOYSTICK_ID      (__cdecl* GetAccelerometer)(unsigned int, unsigned int*, unsigned int*, unsigned int*);
    ///}
  } PeripheralAddon;

#ifdef __cplusplus
}
#endif

#endif // __PERIPHERAL_TYPES_H__
