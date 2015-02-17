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
#ifndef XBMC_GAME_TYPES_H_
#define XBMC_GAME_TYPES_H_

/* current game API version */
#define GAME_API_VERSION                "1.0.0"

/* min. game API version */
#define GAME_MIN_API_VERSION            "1.0.0"

/* magic number for empty tray */
#define GAME_NO_DISK                   ((unsigned)-1)

#include <stddef.h>
#include <stdint.h>

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

#ifdef __cplusplus
extern "C" {
#endif

/*! Game add-on error codes */
typedef enum GAME_ERROR
{
  GAME_ERROR_NO_ERROR                =  0, // no error occurred
  GAME_ERROR_UNKNOWN                 = -1, // an unknown error occurred
  GAME_ERROR_NOT_IMPLEMENTED         = -2, // the method that the frontend called is not implemented
  GAME_ERROR_REJECTED                = -3, // the command was rejected by the game client
  GAME_ERROR_INVALID_PARAMETERS      = -4, // the parameters of the method that was called are invalid for this operation
  GAME_ERROR_FAILED                  = -5, // the command failed
} GAME_ERROR;

/*! Fundamental device abstractions */
typedef enum GAME_DEVICE
{
  GAME_DEVICE_NONE                   = 0,

  /*!
    * The RetroPad, essentially a Super Nintendo controller with additional
    * L2/R2/L3/R3 buttons, similar to a PS1 DualShock
    */
  GAME_DEVICE_JOYPAD                 = 1,

  /*!
    * A simple mouse, similar to Super Nintendo's mouse. X and Y coordinates
    * are reported relative to last poll (poll callback). It is up to the
    * game client to keep track of where the mouse pointer is
    * supposed to be on the screen. The frontend must make sure not to
    * interfere with its own hardware mouse pointer.
    */
  GAME_DEVICE_MOUSE                  = 2,

  /*!
    * A keyboard device that lets one poll for raw key pressed. It is poll-
    * based, so input callback will return with the current pressed state.
    */
  GAME_DEVICE_KEYBOARD               = 3,

  /*!
    * Lightgun X/Y coordinates are reported relatively to last poll, similar
    * to mouse.
    */
  GAME_DEVICE_LIGHTGUN               = 4,

  /*!
    * An extension to JOYPAD (RetroPad). Similar to DualShock it adds two
    * analog sticks. This is treated as a separate device type as it returns
    * values in the full analog range of [-0x8000, 0x7fff]. Positive X axis
    * is right. Positive Y axis is down. Only use ANALOG type when polling
    * for analog values of the axes.
    */
  GAME_DEVICE_ANALOG                 = 5,

  /*!
    * Abstracts the concept of a pointing mechanism, e.g. touch. This allows
    * the frontend to query in absolute coordinates where on the screen a mouse
    * (or something similar) is being placed. For a touch-centric device,
    * coordinates reported are the coordinates of the press.
    *
    * Coordinates in X and Y are reported as [-0x7fff, 0x7fff]. -0x7fff
    * corresponds to the far left/top of the screen, and 0x7fff corresponds
    * to the far right/bottom of the screen. The "screen" is the area that is
    * passed to the frontend and later displayed on the monitor. The frontend
    * is free to scale/resize this screen as it sees fit, however,
    * (X, Y) = (-0x7fff, -0x7fff) will correspond to the top-left pixel of
    * the game image, etc.
    *
    * To check if the pointer coordinates are valid (e.g. a touch display
    * actually being touched), PRESSED returns 1 or 0. If using a mouse,
    * PRESSED will usually correspond to the left mouse button. PRESSED will
    * only return 1 if the pointer is inside the game screen.
    *
    * For multi-touch, the index variable can be used to successively query
    * more presses. If index = 0 returns true for _PRESSED, coordinates can
    * be extracted with _X, _Y for index = 0. One can then query _PRESSED,
    * _X, _Y with index = 1, and so on. Eventually _PRESSED will return false
    * for an index. No further presses are registered at this point.
    */
  GAME_DEVICE_POINTER                = 6,

  /*!
    * These device types are specializations of the base types above. They
    * should only be used in game_set_controller_type() (TODO) to inform game
    * clients about use of a very specific device type.
    *
    * In input state callback, however, only the base type should be used in
    * the 'device' field.
    */
  GAME_DEVICE_JOYPAD_MULTITAP        = ((1 << 8) | GAME_DEVICE_JOYPAD),
  GAME_DEVICE_LIGHTGUN_SUPER_SCOPE   = ((1 << 8) | GAME_DEVICE_LIGHTGUN),
  GAME_DEVICE_LIGHTGUN_JUSTIFIER     = ((2 << 8) | GAME_DEVICE_LIGHTGUN),
  GAME_DEVICE_LIGHTGUN_JUSTIFIERS    = ((3 << 8) | GAME_DEVICE_LIGHTGUN),

  GAME_DEVICE_MASK                   = 0xff
} GAME_DEVICE;

typedef enum GAME_DEVICE_ID
{
  /*!
    * Buttons for the RetroPad (JOYPAD). The placement of these is equivalent
    * to placements on the Super Nintendo controller. L2/R2/L3/R3 buttons
    * correspond to the PS1 DualShock.
    */
  GAME_DEVICE_ID_JOYPAD_B            = 0,
  GAME_DEVICE_ID_JOYPAD_Y            = 1,
  GAME_DEVICE_ID_JOYPAD_SELECT       = 2,
  GAME_DEVICE_ID_JOYPAD_START        = 3,
  GAME_DEVICE_ID_JOYPAD_UP           = 4,
  GAME_DEVICE_ID_JOYPAD_DOWN         = 5,
  GAME_DEVICE_ID_JOYPAD_LEFT         = 6,
  GAME_DEVICE_ID_JOYPAD_RIGHT        = 7,
  GAME_DEVICE_ID_JOYPAD_A            = 8,
  GAME_DEVICE_ID_JOYPAD_X            = 9,
  GAME_DEVICE_ID_JOYPAD_L            = 10,
  GAME_DEVICE_ID_JOYPAD_R            = 11,
  GAME_DEVICE_ID_JOYPAD_L2           = 12,
  GAME_DEVICE_ID_JOYPAD_R2           = 13,
  GAME_DEVICE_ID_JOYPAD_L3           = 14,
  GAME_DEVICE_ID_JOYPAD_R3           = 15,

  /*! Index / Id values for ANALOG device */
  GAME_DEVICE_INDEX_ANALOG_LEFT      = 0,
  GAME_DEVICE_INDEX_ANALOG_RIGHT     = 1,
  GAME_DEVICE_ID_ANALOG_X            = 0,
  GAME_DEVICE_ID_ANALOG_Y            = 1,

  /*! Id values for MOUSE */
  GAME_DEVICE_ID_MOUSE_X             = 0,
  GAME_DEVICE_ID_MOUSE_Y             = 1,
  GAME_DEVICE_ID_MOUSE_LEFT          = 2,
  GAME_DEVICE_ID_MOUSE_RIGHT         = 3,

  /*! Id values for LIGHTGUN types */
  GAME_DEVICE_ID_LIGHTGUN_X          = 0,
  GAME_DEVICE_ID_LIGHTGUN_Y          = 1,
  GAME_DEVICE_ID_LIGHTGUN_TRIGGER    = 2,
  GAME_DEVICE_ID_LIGHTGUN_CURSOR     = 3,
  GAME_DEVICE_ID_LIGHTGUN_TURBO      = 4,
  GAME_DEVICE_ID_LIGHTGUN_PAUSE      = 5,
  GAME_DEVICE_ID_LIGHTGUN_START      = 6,

  /*! Id values for POINTER */
  GAME_DEVICE_ID_POINTER_X           = 0,
  GAME_DEVICE_ID_POINTER_Y           = 1,
  GAME_DEVICE_ID_POINTER_PRESSED     = 2,
} GAME_DEVICE_ID;

/*! Returned from game_get_region() (TODO) */
typedef enum GAME_REGION
{
  GAME_REGION_NTSC                   = 0,
  GAME_REGION_PAL                    = 1,
} GAME_REGION;

typedef enum GAME_MEMORY
{
  /*!
    * Passed to game_get_memory_data/size(). If the memory type doesn't apply
    * to the implementation NULL/0 can be returned.
    */
  GAME_MEMORY_MASK                   = 0xff,

  /*!
    * Regular save ram. This ram is usually found on a game cartridge, backed
    * up by a battery. If save game data is too complex for a single memory
    * buffer, the SYSTEM_DIRECTORY environment callback can be used.
    */
  GAME_MEMORY_SAVE_RAM               = 0,

  /*!
    * Some games have a built-in clock to keep track of time. This memory is
    * usually just a couple of bytes to keep track of time.
    */
  GAME_MEMORY_RTC                    = 1,

  /*! System ram lets a frontend peek into a game systems main RAM */
  GAME_MEMORY_SYSTEM_RAM             = 2,

  /*! Video ram lets a frontend peek into a game systems video RAM (VRAM) */
  GAME_MEMORY_VIDEO_RAM              = 3,

  /*! Special memory types */
  GAME_MEMORY_SNES_BSX_RAM           = ((1 << 8) | GAME_MEMORY_SAVE_RAM),
  GAME_MEMORY_SNES_BSX_PRAM          = ((2 << 8) | GAME_MEMORY_SAVE_RAM),
  GAME_MEMORY_SNES_SUFAMI_TURBO_A_RAM= ((3 << 8) | GAME_MEMORY_SAVE_RAM),
  GAME_MEMORY_SNES_SUFAMI_TURBO_B_RAM= ((4 << 8) | GAME_MEMORY_SAVE_RAM),
  GAME_MEMORY_SNES_GAME_BOY_RAM      = ((5 << 8) | GAME_MEMORY_SAVE_RAM),
  GAME_MEMORY_SNES_GAME_BOY_RTC      = ((6 << 8) | GAME_MEMORY_RTC)
} GAME_MEMORY;

/*!
  * Special game types passed into game_load_game_special(). Only used when
  * multiple ROMs are required.
  */
typedef enum GAME_TYPE
{
  GAME_TYPE_BSX                      = 0x101,
  GAME_TYPE_BSX_SLOTTED              = 0x102,
  GAME_TYPE_SUFAMI_TURBO             = 0x103,
  GAME_TYPE_SUPER_GAME_BOY           = 0x104
} GAME_TYPE;

/*! Keysyms used for ID in input state callback when polling GAME_KEYBOARD */
typedef enum GAME_KEY /* retro_key in libretro */
{
  GAME_KEY_UNKNOWN                   = 0,
  GAME_KEY_FIRST                     = 0,
  GAME_KEY_BACKSPACE                 = 8,
  GAME_KEY_TAB                       = 9,
  GAME_KEY_CLEAR                     = 12,
  GAME_KEY_RETURN                    = 13,
  GAME_KEY_PAUSE                     = 19,
  GAME_KEY_ESCAPE                    = 27,
  GAME_KEY_SPACE                     = 32,
  GAME_KEY_EXCLAIM                   = 33,
  GAME_KEY_QUOTEDBL                  = 34,
  GAME_KEY_HASH                      = 35,
  GAME_KEY_DOLLAR                    = 36,
  GAME_KEY_AMPERSAND                 = 38,
  GAME_KEY_QUOTE                     = 39,
  GAME_KEY_LEFTPAREN                 = 40,
  GAME_KEY_RIGHTPAREN                = 41,
  GAME_KEY_ASTERISK                  = 42,
  GAME_KEY_PLUS                      = 43,
  GAME_KEY_COMMA                     = 44,
  GAME_KEY_MINUS                     = 45,
  GAME_KEY_PERIOD                    = 46,
  GAME_KEY_SLASH                     = 47,
  GAME_KEY_0                         = 48,
  GAME_KEY_1                         = 49,
  GAME_KEY_2                         = 50,
  GAME_KEY_3                         = 51,
  GAME_KEY_4                         = 52,
  GAME_KEY_5                         = 53,
  GAME_KEY_6                         = 54,
  GAME_KEY_7                         = 55,
  GAME_KEY_8                         = 56,
  GAME_KEY_9                         = 57,
  GAME_KEY_COLON                     = 58,
  GAME_KEY_SEMICOLON                 = 59,
  GAME_KEY_LESS                      = 60,
  GAME_KEY_EQUALS                    = 61,
  GAME_KEY_GREATER                   = 62,
  GAME_KEY_QUESTION                  = 63,
  GAME_KEY_AT                        = 64,
  GAME_KEY_LEFTBRACKET               = 91,
  GAME_KEY_BACKSLASH                 = 92,
  GAME_KEY_RIGHTBRACKET              = 93,
  GAME_KEY_CARET                     = 94,
  GAME_KEY_UNDERSCORE                = 95,
  GAME_KEY_BACKQUOTE                 = 96,
  GAME_KEY_a                         = 97,
  GAME_KEY_b                         = 98,
  GAME_KEY_c                         = 99,
  GAME_KEY_d                         = 100,
  GAME_KEY_e                         = 101,
  GAME_KEY_f                         = 102,
  GAME_KEY_g                         = 103,
  GAME_KEY_h                         = 104,
  GAME_KEY_i                         = 105,
  GAME_KEY_j                         = 106,
  GAME_KEY_k                         = 107,
  GAME_KEY_l                         = 108,
  GAME_KEY_m                         = 109,
  GAME_KEY_n                         = 110,
  GAME_KEY_o                         = 111,
  GAME_KEY_p                         = 112,
  GAME_KEY_q                         = 113,
  GAME_KEY_r                         = 114,
  GAME_KEY_s                         = 115,
  GAME_KEY_t                         = 116,
  GAME_KEY_u                         = 117,
  GAME_KEY_v                         = 118,
  GAME_KEY_w                         = 119,
  GAME_KEY_x                         = 120,
  GAME_KEY_y                         = 121,
  GAME_KEY_z                         = 122,
  GAME_KEY_DELETE                    = 127,

  GAME_KEY_KP0                       = 256,
  GAME_KEY_KP1                       = 257,
  GAME_KEY_KP2                       = 258,
  GAME_KEY_KP3                       = 259,
  GAME_KEY_KP4                       = 260,
  GAME_KEY_KP5                       = 261,
  GAME_KEY_KP6                       = 262,
  GAME_KEY_KP7                       = 263,
  GAME_KEY_KP8                       = 264,
  GAME_KEY_KP9                       = 265,
  GAME_KEY_KP_PERIOD                 = 266,
  GAME_KEY_KP_DIVIDE                 = 267,
  GAME_KEY_KP_MULTIPLY               = 268,
  GAME_KEY_KP_MINUS                  = 269,
  GAME_KEY_KP_PLUS                   = 270,
  GAME_KEY_KP_ENTER                  = 271,
  GAME_KEY_KP_EQUALS                 = 272,

  GAME_KEY_UP                        = 273,
  GAME_KEY_DOWN                      = 274,
  GAME_KEY_RIGHT                     = 275,
  GAME_KEY_LEFT                      = 276,
  GAME_KEY_INSERT                    = 277,
  GAME_KEY_HOME                      = 278,
  GAME_KEY_END                       = 279,
  GAME_KEY_PAGEUP                    = 280,
  GAME_KEY_PAGEDOWN                  = 281,

  GAME_KEY_F1                        = 282,
  GAME_KEY_F2                        = 283,
  GAME_KEY_F3                        = 284,
  GAME_KEY_F4                        = 285,
  GAME_KEY_F5                        = 286,
  GAME_KEY_F6                        = 287,
  GAME_KEY_F7                        = 288,
  GAME_KEY_F8                        = 289,
  GAME_KEY_F9                        = 290,
  GAME_KEY_F10                       = 291,
  GAME_KEY_F11                       = 292,
  GAME_KEY_F12                       = 293,
  GAME_KEY_F13                       = 294,
  GAME_KEY_F14                       = 295,
  GAME_KEY_F15                       = 296,

  GAME_KEY_NUMLOCK                   = 300,
  GAME_KEY_CAPSLOCK                  = 301,
  GAME_KEY_SCROLLOCK                 = 302,
  GAME_KEY_RSHIFT                    = 303,
  GAME_KEY_LSHIFT                    = 304,
  GAME_KEY_RCTRL                     = 305,
  GAME_KEY_LCTRL                     = 306,
  GAME_KEY_RALT                      = 307,
  GAME_KEY_LALT                      = 308,
  GAME_KEY_RMETA                     = 309,
  GAME_KEY_LMETA                     = 310,
  GAME_KEY_LSUPER                    = 311,
  GAME_KEY_RSUPER                    = 312,
  GAME_KEY_MODE                      = 313,
  GAME_KEY_COMPOSE                   = 314,

  GAME_KEY_HELP                      = 315,
  GAME_KEY_PRINT                     = 316,
  GAME_KEY_SYSREQ                    = 317,
  GAME_KEY_BREAK                     = 318,
  GAME_KEY_MENU                      = 319,
  GAME_KEY_POWER                     = 320,
  GAME_KEY_EURO                      = 321,
  GAME_KEY_UNDO                      = 322,

  GAME_KEY_LAST
} GAME_KEY;

typedef enum GAME_KEY_MOD /* retro_mod in libretro */
{
  GAME_KEY_MOD_NONE                  = 0x0000,

  GAME_KEY_MOD_SHIFT                 = 0x01,
  GAME_KEY_MOD_CTRL                  = 0x02,
  GAME_KEY_MOD_ALT                   = 0x04,
  GAME_KEY_MOD_META                  = 0x08,

  GAME_KEY_MOD_NUMLOCK               = 0x10,
  GAME_KEY_MOD_CAPSLOCK              = 0x20,
  GAME_KEY_MOD_SCROLLOCK             = 0x40,
} GAME_KEY_MOD;

typedef enum GAME_LOG_LEVEL /* retro_log_level in libretro */
{
  GAME_LOG_DEBUG                     = 0,
  GAME_LOG_INFO                      = 1,
  GAME_LOG_WARN                      = 2,
  GAME_LOG_ERROR                     = 3
} GAME_LOG_LEVEL;

/*! ID values for SIMD CPU features */
typedef enum GAME_SIMD
{
  GAME_SIMD_SSE                      = (1 << 0),
  GAME_SIMD_SSE2                     = (1 << 1),
  GAME_SIMD_VMX                      = (1 << 2),
  GAME_SIMD_VMX128                   = (1 << 3),
  GAME_SIMD_AVX                      = (1 << 4),
  GAME_SIMD_NEON                     = (1 << 5),
  GAME_SIMD_SSE3                     = (1 << 6),
  GAME_SIMD_SSSE3                    = (1 << 7),
  GAME_SIMD_MMX                      = (1 << 8),
  GAME_SIMD_MMXEXT                   = (1 << 9),
  GAME_SIMD_SSE4                     = (1 << 10),
  GAME_SIMD_SSE42                    = (1 << 11),
  GAME_SIMD_AVX2                     = (1 << 12),
  GAME_SIMD_VFPU                     = (1 << 13),
} GAME_SIMD;

/*!
  * FIXME: Document the sensor API and work out behavior. It will be marked as
  * experimental until then.
  */
typedef enum GAME_SENSOR_ACTION /* retro_sensor_action in libretro */
{
  GAME_SENSOR_ACCELEROMETER_ENABLE   = 0,
  GAME_SENSOR_ACCELEROMETER_DISABLE  = 1
} GAME_SENSOR_ACTION;

/*! Id values for SENSOR types */
typedef enum GAME_SENSOR_ID
{
  GAME_SENSOR_ACCELEROMETER_X        = 0,
  GAME_SENSOR_ACCELEROMETER_Y        = 1,
  GAME_SENSOR_ACCELEROMETER_Z        = 2
} GAME_SENSOR_ID;

typedef enum GAME_CAMERA_BUFFER /* retro_camera_buffer in libretro */
{
  GAME_CAMERA_BUFFER_OPENGL_TEXTURE  = 0,
  GAME_CAMERA_BUFFER_RAW_FRAMEBUFFER = 1
} GAME_CAMERA_BUFFER;


typedef enum GAME_RUMBLE_EFFECT /* retro_rumble_effect in libretro */
{
  GAME_RUMBLE_STRONG                 = 0,
  GAME_RUMBLE_WEAK                   = 1
} GAME_RUMBLE_EFFECT;

// TODO
typedef enum GAME_HW_FRAME_BUFFER
{
  GAME_HW_FRAME_BUFFER_VALID         = -1, // Pass this to game_video_refresh if rendering to hardware
  GAME_HW_FRAME_BUFFER_DUPLICATE     = 0,  // Passing NULL to game_video_refresh is still a frame dupe as normal
  GAME_HW_FRAME_BUFFER_RENDER        = 1,
} GAME_HW_FRAME_BUFFER;

typedef enum GAME_HW_CONTEXT_TYPE /* retro_hw_context_type in libretro */
{
  GAME_HW_CONTEXT_NONE               = 0,
  GAME_HW_CONTEXT_OPENGL             = 1, // OpenGL 2.x. Latest version available before 3.x+. Driver can choose to use latest compatibility context
  GAME_HW_CONTEXT_OPENGLES2          = 2, // GLES 2.0
  GAME_HW_CONTEXT_OPENGL_CORE        = 3, // Modern desktop core GL context. Use major/minor fields to set GL version
  GAME_HW_CONTEXT_OPENGLES3          = 4, // GLES 3.0
} GAME_HW_CONTEXT_TYPE;

typedef enum GAME_PIXEL_FORMAT
{
  /*!
  * 0RGB1555, native endian. 0 bit must be set to 0. This pixel format is
  * default for compatibility concerns only. If a 15/16-bit pixel format is
  * desired, consider using RGB565.
  */
  GAME_PIXEL_FORMAT_0RGB1555         = 0,

  /*! XRGB8888, native endian. X bits are ignored. */
  GAME_PIXEL_FORMAT_XRGB8888         = 1,

  /*!
  * RGB565, native endian. This pixel format is the recommended format to use
  * if a 15/16-bit format is desired as it is the pixel format that is
  * typically available on a wide range of low-power devices. It is also
  * natively supported in APIs like OpenGL ES.
  */
  GAME_PIXEL_FORMAT_RGB565           = 2,
} GAME_PIXEL_FORMAT;

typedef enum GAME_ROTATION
{
  GAME_ROTATION_0_CW                 = 0,
  GAME_ROTATION_90_CW                = 1,
  GAME_ROTATION_180_CW               = 2,
  GAME_ROTATION_270_CW               = 3
} GAME_ROTATION;

typedef enum GAME_EJECT_STATE
{
  GAME_NOT_EJECTED,
  GAME_EJECTED
} GAME_EJECT_STATE;

/*!
  * Describes how the game client maps a game input bind to
  * its internal input system through a human readable string. This string
  * can be used to better let a user configure input.
  */
struct game_input_descriptor
{
  /*! Associates given parameters with a description. */
  unsigned    port;
  unsigned    device;
  unsigned    index;
  unsigned    id;
  const char *description;      // Human readable description for parameters.
                                // The pointer must remain valid until game_unload_game() is called.
};

struct game_geometry
{
  unsigned base_width;          // Nominal video width of game
  unsigned base_height;         // Nominal video height of game
  unsigned max_width;           // Maximum possible width of game
  unsigned max_height;          // Maximum possible height of game
  float    aspect_ratio;        // Nominal aspect ratio of game. If aspect_ratio is <= 0.0,
                                // an aspect ratio of base_width / base_height is assumed.
                                // A frontend could override this setting if desired.
};

struct game_system_timing
{
  double fps;                   // FPS of video content.
  double sample_rate;           // Sampling rate of audio.
};

struct game_system_av_info
{
  struct game_geometry geometry;
  struct game_system_timing timing;
};

typedef uint64_t  game_perf_tick_t;
typedef int64_t   game_time_t;
typedef int64_t   game_usec_t;
typedef void      (*game_proc_address_t)(void);

struct game_perf_counter
{
  const char       *ident;
  game_perf_tick_t start;
  game_perf_tick_t total;
  game_perf_tick_t call_cnt;
  bool              registered;
};

struct game_camera_info
{
  uint64_t caps;                // Set by game client. Example bitmask: caps = (1 << GAME_CAMERA_BUFFER_OPENGL_TEXTURE) | (1 << GAME_CAMERA_BUFFER_RAW_FRAMEBUFFER).
  unsigned width;               // Desired resolution for camera. Is only used as a hint.
  unsigned height;
};

struct game_hw_info
{
  enum GAME_HW_CONTEXT_TYPE context_type; // Which API to use. Set by game client.
  bool     depth;               // Set if render buffers should have depth component attached.
  bool     stencil;             // Set if stencil buffers should be attached.
                                // If depth and stencil are true, a packed 24/8 buffer will be added. Only attaching stencil is invalid and will be ignored.
  bool     bottom_left_origin;  // Use conventional bottom-left origin convention. Is false, standard top-left origin semantics are used.
  unsigned version_major;       // Major version number for core GL context.
  unsigned version_minor;       // Minor version number for core GL context.
  bool     cache_context;       // If this is true, the frontend will go very far to avoid resetting context in scenarios like toggling fullscreen, etc.
                                // The reset callback might still be called in extreme situations such as if the context is lost beyond recovery.
                                // For optimal stability, set this to false, and allow context to be reset at any time.
  bool     debug_context;       // Creates a debug context.
};

/*! Properties passed to the ADDON_Create() method of a game client */
typedef struct game_client_properties
{
  /*!
   * Path to the game client's shared library being loaded. Replaces
   * RETRO_ENVIRONMENT_GET_LIBRETRO_PATH.
   */
  const char* library_path;

  /*!
   * The "system" directory of the frontend. This directory can be used to
   * store system-specific ROMs such as BIOSes, configuration data, etc.
   * Replaces RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY.
   */
  const char* system_directory;

  /*!
    * The "content" directory of the frontend. This directory can be used to
    * store specific assets that the core relies upon, such as art assets, input
    * data, etc. Replaces RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY.
    */
  const char* content_directory;

  /*!
    * The "save" directory of the frontend. This directory can be used to store
    * SRAM, memory cards, high scores, etc, if the game client cannot use the
    * regular memory interface, GetMemoryData(). Replaces
    * RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY.
    */
  const char* save_directory;
} game_client_properties;

/*! Structure to transfer the methods from xbmc_game_dll.h to XBMC */
typedef struct GameClient
{
  const char* (__cdecl* GetGameAPIVersion)(void);
  const char* (__cdecl* GetMininumGameAPIVersion)(void);
  GAME_ERROR  (__cdecl* LoadGame)(const char* url);
  GAME_ERROR  (__cdecl* LoadGameSpecial)(GAME_TYPE type, const char** urls, size_t num_urls);
  GAME_ERROR  (__cdecl* UnloadGame)(void);
  GAME_ERROR  (__cdecl* Run)(void);
  GAME_ERROR  (__cdecl* Reset)(void);
  GAME_ERROR  (__cdecl* KeyboardEvent)(bool down, unsigned keycode, uint32_t character, uint16_t key_modifiers);
  GAME_ERROR  (__cdecl* GetSystemAVInfo)(struct game_system_av_info *info);
  GAME_ERROR  (__cdecl* SetControllerPortDevice)(unsigned port, unsigned device);
  size_t      (__cdecl* SerializeSize)(void);
  GAME_ERROR  (__cdecl* Serialize)(void *data, size_t size);
  GAME_ERROR  (__cdecl* Deserialize)(const void *data, size_t size);
  GAME_ERROR  (__cdecl* CheatReset)(void);
  GAME_ERROR  (__cdecl* CheatSet)(unsigned index, bool enabled, const char *code);
  GAME_REGION (__cdecl* GetRegion)(void);
  void*       (__cdecl* GetMemoryData)(GAME_MEMORY id);
  size_t      (__cdecl* GetMemorySize)(GAME_MEMORY id);
  GAME_ERROR  (__cdecl* DiskSetEjectState)(GAME_EJECT_STATE ejected);
  GAME_EJECT_STATE (__cdecl* DiskGetEjectState)(void);
  unsigned    (__cdecl* DiskGetImageIndex)(void);
  GAME_ERROR  (__cdecl* DiskSetImageIndex)(unsigned index);
  unsigned    (__cdecl* DiskGetNumImages)(void);
  GAME_ERROR  (__cdecl* DiskReplaceImageIndex)(unsigned index, const char* url);
  GAME_ERROR  (__cdecl* DiskAddImageIndex)(void);
  GAME_ERROR  (__cdecl* HwContextReset)(void);
  GAME_ERROR  (__cdecl* HwContextDestroy)(void);
  GAME_ERROR  (__cdecl* AudioAvailable)(void);
  GAME_ERROR  (__cdecl* AudioSetState)(bool enabled);
  GAME_ERROR  (__cdecl* FrameTimeNotify)(game_usec_t usec);
  GAME_ERROR  (__cdecl* CameraInitialized)(void);
  GAME_ERROR  (__cdecl* CameraDeinitialized)(void);
  GAME_ERROR  (__cdecl* CameraFrameRawBuffer)(const uint32_t *buffer, unsigned width, unsigned height, size_t pitch);
  GAME_ERROR  (__cdecl* CameraFrameOpenglTexture)(unsigned texture_id, unsigned texture_target, const float *affine);
} GameClient;

#ifdef __cplusplus
}
#endif

#endif // XBMC_GAME_TYPES_H_
