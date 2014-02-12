#pragma once
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
 */

/**
 * An arrow-based device on a gamepad. If no buttons are pressed, the hat is
 * considered centered. If two non-adjacent buttons (such as up and down) are
 * pressed, the hat is in an invalid state.
 */
class CJoystickHat
{
public:
  enum Direction
  {
    UP    = 0,
    RIGHT = 1,
    DOWN  = 2,
    LEFT  = 3
  };

  /**
   * Create a new centered hat
   */
  CJoystickHat() { Center(); }
  void Center();
  
  /**
   * Access direction states
   */
  bool       &operator[](Direction dir);
  const bool &operator[](Direction dir) const { return const_cast<CJoystickHat&>(*this)[dir]; }
  
  /**
   * Comparison operators
   */
  bool operator==(const CJoystickHat &rhs) const;
  bool operator!=(const CJoystickHat &rhs) const { return !(*this == rhs); }
  
  /**
   * Helper function to translate this hat into a cardinal direction
   * ("N", "NE", "E", ...), "centered", or "invalid".
   */
  const char *GetDirection() const;

private:
  bool up;
  bool right;
  bool down;
  bool left;
};
