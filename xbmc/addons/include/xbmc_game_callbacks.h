/*
 *      Copyright (C) 2010-2014 The RetroArch team
 *      Copyright (C) 2014 Team XBMC
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this libretro API header.
 * ---------------------------------------------------------------------------------------
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef XBMC_GAME_CALLBACKS_H_
#define XBMC_GAME_CALLBACKS_H_

#include "xbmc_game_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CB_GameLib
{
  /*!
    * Requests the frontend to shutdown. Should only be used if game has a
    * specific way to shutdown the game from a menu item or similar.
    * Replaces RETRO_ENVIRONMENT_SHUTDOWN.
    */
  void (*ShutdownFrontend)(void* addonData);

  /*!
    * Sets screen rotation of graphics. Is only implemented if rotation can be
    * accelerated by hardware. Valid values are 0, 1, 2, 3, which rotates
    * screen by 0, 90, 180, 270 degrees counter-clockwise respectively.
    * Replaces RETRO_ENVIRONMENT_SET_ROTATION.
    */
  void (*EnvironmentSetRotation)(void* addonData, enum GAME_ROTATION rotation);

  /*!
    * Boolean value whether or not the implementation should use overscan, or
    * crop away overscan. Replaces RETRO_ENVIRONMENT_GET_OVERSCAN.
    */
  bool (*EnvironmentGetOverscan)(void* addonData);

  /*!
    * Boolean value whether or not frontend supports frame duping, passing
    * NULL to video frame callback. Replaces RETRO_ENVIRONMENT_GET_CAN_DUPE.
    */
  bool (*EnvironmentCanDupe)(void* addonData);

  /*!
    * Sets an array of game_input_descriptors. It is up to the frontend to
    * present this in a usable way. This function can be called at any time,
    * but it is recommended to call it as early as possible. Replaces
    * RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS.
    */
  void (*EnvironmentSetInputDescriptors)(void* addonData, const struct game_input_descriptor* descriptor, size_t count);

  /*!
    * Sets a new av_info structure. This can only be called from within
    * Run(). This should *only* be used if the core is completely
    * altering the internal resolutions, aspect ratios, timings, sampling rate,
    * etc. Calling this can require a full reinitialization of video/audio
    * drivers in the frontend, so it is important to call it very sparingly,
    * and usually only with the users explicit consent. An eventual driver
    * reinit will happen so that video and audio callbacks happening after
    * this call within the same Run() call will target the newly
    * initialized driver.
    *
    * This callback makes it possible to support configurable resolutions in
    * games, which can be useful to avoid setting the "worst case" in
    * max_width/max_height.
    *
    * ***HIGHLY RECOMMENDED*** Do not call this callback every time resolution
    * changes in an emulator core if it's expected to be a temporary change,
    * for the reasons of possible driver reinit. This call is not a free pass
    * for not trying to provide correct values in GetSystemAVInfo().
    *
    * If this returns false, the frontend does not acknowledge a changed
    * av_info struct.
    *
    * Replaces RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO.
    */
  bool (*EnvironmentSetSystemAvInfo)(void* addonData, const struct game_system_av_info* info);

  /*!
    * Render a frame. Pixel format is 15-bit 0RGB1555 native endian unless
    * changed (see EnvironmentSetPixelFormat()). Width and height
    * specify dimensions of buffer. Pitch specifices length in bytes between
    * two lines in buffer. For performance reasons, it is highly recommended
    * to have a frame that is packed in memory, i.e.
    * pitch == width * byte_per_pixel. Certain graphic APIs, such as OpenGL ES,
    * do not like textures that are not packed in memory.
    */
  void (*VideoRefresh)(void* addonData, const void* data, unsigned width, unsigned height, size_t pitch, enum GAME_PIXEL_FORMAT pixelFormat);

  /*!
    * Renders a single audio frame. Should only be used if implementation
    * generates a single sample at a time. Format is signed 16-bit native endian.
    */
  void (*AudioSample)(void* addonData, int16_t left, int16_t right);

  /*!
    * Renders multiple audio frames in one go. One frame is defined as a
    * sample of left and right channels, interleaved. I.e.
    * int16_t buf[4] = { l, r, l, r }; would be 2 frames. Only one of the
    * audio callbacks must ever be used.
    */
  size_t (*AudioSampleBatch)(void* addonData, const int16_t* data, size_t frames);

  /*!
    * Queries for input for player 'port'. device will be masked with
    * GAME_DEVICE_MASK. Specialization of devices such as
    * GAME_DEVICE_JOYPAD_MULTITAP that have been set with
    * SetControllerPortDevice() will still use the higher level
    * GAME_DEVICE_JOYPAD to request input.
    */
  int16_t (*InputState)(void* addonData, unsigned port, unsigned device, unsigned index, unsigned id);

  /*!
    * Gets a bitmask telling which device type are expected to be handled
    * properly in a call to InputState(). Devices which are not handled or
    * recognized always return 0 in input_state().
    *
    * Example bitmask: caps = (1 << GAME_DEVICE_JOYPAD) | (1 << GAME_DEVICE_ANALOG).
    * Should only be called in Run().
    *
    * Replaces RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES.
    */
  uint64_t (*InputGetDeviceCapabilities)(void* addonData);

  /*
    * Interface for performance counters. This is useful for performance
    * logging in a  cross-platform way and for detecting architecture-specific
    * features, such as SIMD support. Replaces RETRO_ENVIRONMENT_GET_PERF_INTERFACE.
    */

  /*!
    * Rumble interface for setting the state of rumble motors in controllers.
    * A strong and weak motor is supported, and they can be controlled
    * indepedently. Replaces RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE.
    *
    * Sets rumble state for joypad plugged in port 'port'. Rumble effects are
    * controlled independently, and setting e.g. strong rumble does not
    * override weak rumble. Strength has a range of [0, 0xffff].
    *
    * Returns true if rumble state request was honored. Calling this before
    * first Run() is likely to return false.
    */
  bool (*RumbleSetState)(void* addonData, unsigned port, enum GAME_RUMBLE_EFFECT effect, uint16_t strength);

  /*! Returns current time in microseconds. Tries to use the most accurate timer available. */
  game_time_t (*PerfGetTimeUsec)(void* addonData);

  /*!
    * A simple counter. Usually nanoseconds, but can also be CPU cycles. Can
    * be used directly if desired (when creating a more sophisticated
    * performance counter system).
    */
  game_perf_tick_t (*PerfGetCounter)(void* addonData);

  /*! Returns a bit-mask of detected CPU features (GAME_SIMD_*) */
  uint64_t (*PerfGetCpuFeatures)(void* addonData);

  /*!
    * Asks frontend to log and/or display the state of performance counters.
    * Performance counters can always be poked into manually as well.
    */
  void (*PerfLog)(void* addonData);

  /*!
    * Register a performance counter. ident field must be set with a discrete
    * value and other values in game_perf_counter must be 0. Registering can
    * be called multiple times. To avoid calling to frontend redundantly, you
    * can check registered field first.
    */
  void (*PerfRegister)(void* addonData, struct game_perf_counter* counter);

  /*! Starts a registered counter */
  void (*PerfStart)(void* addonData, struct game_perf_counter* counter);

  /*! Stops a registered counter */
  void (*PerfStop)(void* addonData, struct game_perf_counter* counter);

  /*!
    * For convenience, using the performance interface, it can be useful to
    * wrap register, start and stop in macros.
    *
    * E.g.:
    * #ifdef LOG_PERFORMANCE
    *   #define GAME_PERFORMANCE_INIT(perf_cb, name) static struct game_perf_counter name = {#name}; if (!name.registered) perf_cb.perf_register(&(name))
    *   #define GAME_PERFORMANCE_START(perf_cb, name) perf_cb.perf_start(&(name))
    *   #define GAME_PERFORMANCE_STOP(perf_cb, name) perf_cb.perf_stop(&(name))
    * #else
    *   ... Blank macros ...
    * #endif
    *
    * These can then be used mid-functions around code snippets.
    *
    * extern struct game_perf_callback perf_cb; // Somewhere in the core
    *
    * void do_some_heavy_work(void)
    * {
    *   GAME_PERFORMANCE_INIT(cb, work_1);
    *   GAME_PERFORMANCE_START(cb, work_1);
    *   heavy_work_1();
    *   GAME_PERFORMANCE_STOP(cb, work_1);
    *   GAME_PERFORMANCE_INIT(cb, work_2);
    *   GAME_PERFORMANCE_START(cb, work_2);
    *   heavy_work_2();
    *   GAME_PERFORMANCE_STOP(cb, work_2);
    * }
    *
    * void Unload(void)
    * {
    *   perf_cb.perf_log(); // Log all perf counters here for example
    * }
    */

  /*!
    * Sensor interface. The purpose of this interface is to allow setting
    * state related to sensors such as polling rate, enabling/disable it
    * entirely, etc. Replaces RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE.
    */
  bool (*SensorSetState)(void* addonData, unsigned port, enum GAME_SENSOR_ACTION action, unsigned rate);

  float (*SensorGetInput)(void* addonData, unsigned port, unsigned id);

  /*!
    * Interface to a video camera driver. A game client can use this
    * interface to get access to a video camera. New video frames are
    * delivered in a callback in same thread as Run().
    *
    * GET_CAMERA_INTERFACE should be called in LoadGame(). Depending
    * on the camera implementation used, camera frames will be delivered as a
    * raw framebuffer, or as an OpenGL texture directly.
    *
    * The core has to tell the frontend here which types of buffers can be
    * handled properly. An OpenGL texture can only be handled when using a
    * GL game client (SET_HW_RENDER). It is recommended to use a GL game client
    * when using camera interface.
    *
    * The camera is not started automatically. The retrieved start/stop
    * functions must be used to explicitly start and stop the camera driver.
    *
    * Replaces RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE.
    */
  void (*CameraSetInfo)(void* addonData, struct game_camera_info* camera_info);

  /*! Starts the camera driver. Can only be called in Run() */
  bool (*CameraStart)(void* addonData);

  /*! Stops the camera driver. Can only be called in Run() */
  void (*CameraStop)(void* addonData);

  /*!
    * Location interface. The purpose of this interface is to be able to
    * retrieve location-based information from the host device, such as
    * current latitude / longitude. Replaces RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE.
    */

  /*!
    * Start location services. The device will start listening for changes to
    * the current location at regular intervals (which are defined with
    * location_set_interval()).
    */
  bool (*LocationStart)(void* addonData);

  /*!
    * Stop location services. The device will stop listening for changes to
    * the current location.
    */
  void (*LocationStop)(void* addonData);

  /*!
    * Get the position of the current location. Will set parameters to 0 if no
    * new location update has happened since the last time.
    */
  bool (*LocationGetPosition)(void* addonData, double* lat, double* lon, double* horiz_accuracy, double* vert_accuracy);

  /*!
    * Sets the interval of time and/or distance at which to update/poll
    * location-based data. To ensure compatibility with all location-based
    * implementations, values for both interval_ms and interval_distance
    * should be provided. interval_ms is the interval expressed in
    * milliseconds. interval_distance is the distance interval expressed in
    * meters.
    */
  void (*LocationSetInterval)(void* addonData, unsigned interval_ms, unsigned interval_distance);

  /*!
    * Callback which signals when the location driver is initialized.
    * location_start() can be called in initialized callback.
    */
  void (*LocationInitialized)(void* addonData);

  /*! Callback which signals when the location driver is deinitialized */
  void (*LocationDeinitialized)(void* addonData);

  /*!
    * Frame time interface. Replaces RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK.
    *
    * Represents the time of one frame. It is computed as 1e+6 / fps, but
    * the implementation will resolve the rounding to ensure that
    * framestepping, etc is exact.
    */
  void (*FrameTimeSetReference)(void* addonData, game_usec_t usec);

  /*!
    * Hardware acceleration interface. Should be called in LoadGame().
    * If successful, game clients will be able to render to a frontend-
    * provided framebuffer. The size of framebuffer will be at least as large
    * as max_width/max_height provided in GetSystemAVInfo(). If HW
    * rendering is used, pass only GAME_HW_FRAME_BUFFER_VALID or NULL to
    * VideoRefresh(). Replaces RETRO_ENVIRONMENT_SET_HW_RENDER.
    */
  void (*HwSetInfo)(void* addonData, const struct game_hw_info* hw_info);

  /*!
    * Gets current framebuffer which is to be rendered to. Could change every
    * frame potentially.
    */
  uintptr_t (*HwGetCurrentFramebuffer)(void* addonData);

  /*! Get a symbol from HW context */
  game_proc_address_t (*HwGetProcAddress)(void* addonData, const char* sym);

} CB_GameLib;

#ifdef __cplusplus
}
#endif

#endif // XBMC_GAME_CALLBACKS_H_
