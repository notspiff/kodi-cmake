/*
 *      Copyright (C) 2007-2013 Team XBMC
 *      http://www.xbmc.org
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
 *  Parts of this file are Copyright (C) 2009 Stephen Kitt <steve@sk2.org>
 */

#pragma once

#include "input/Joystick.h"

#include <stdint.h>
#include <string>

class CLinuxJoystick : public CJoystick
{
public:
  static void Initialize(JoystickArray &joysticks);
  static void Deinitialize(JoystickArray &joysticks);

  virtual ~CLinuxJoystick();

  virtual void Update();

private:
  CLinuxJoystick(int fd, unsigned int id, const std::string &name, const std::string &filename, unsigned char buttons, unsigned char axes);

  static bool GetButtonMap(int fd, uint16_t *buttonMap);
  static bool GetAxisMap(int fd, uint8_t *axisMap);

  /**
   * Try a series of ioctls until one succeeds.
   * @param fd - The fd to perform ioctl on
   * @param ioctls - A zero-terminated list of ioctls
   * @param buttonMap - The discovered button map
   * @param ioctl_used - The ioctl that succeeded (untouched if false is returned)
   */
  static bool DetermineIoctl(int fd, const unsigned long *ioctls, uint16_t *buttonMap, unsigned long &ioctl_used);

  int            m_fd;
  std::string    m_filename; // For debugging purposes
  CJoystickState m_state; // Linux Joystick API only reports updates, need to track actual state between calls to Update()
};
