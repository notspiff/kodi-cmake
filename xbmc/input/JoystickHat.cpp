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

#include "JoystickHat.h"

#define HAT_MAKE_DIRECTION(n, e, s, w) ((n ? 1 : 0) << 3 | (e ? 1 : 0) << 2 | (s ? 1 : 0) << 1 | (w ? 1 : 0))

void CJoystickHat::Center()
{
  up = right = down = left = false;
}

bool &CJoystickHat::operator[](Direction dir)
{
  static bool INVALID = false;
  switch (dir)
  {
  case UP:    return up;
  case RIGHT: return right;
  case DOWN:  return down;
  case LEFT:  return left;
  default:    return INVALID;
  }
}

bool CJoystickHat::operator==(const CJoystickHat &rhs) const
{
  return up == rhs.up && right == rhs.right && down == rhs.down && left == rhs.left;
}

const char *CJoystickHat::GetDirection() const
{
  switch (HAT_MAKE_DIRECTION(up, right, down, left))
  {
  case HAT_MAKE_DIRECTION(1, 0, 0, 0): return "N";
  case HAT_MAKE_DIRECTION(1, 1, 0, 0): return "NE";
  case HAT_MAKE_DIRECTION(0, 1, 0, 0): return "E";
  case HAT_MAKE_DIRECTION(0, 1, 1, 0): return "SE";
  case HAT_MAKE_DIRECTION(0, 0, 1, 0): return "S";
  case HAT_MAKE_DIRECTION(0, 0, 1, 1): return "SW";
  case HAT_MAKE_DIRECTION(0, 0, 0, 1): return "W";
  case HAT_MAKE_DIRECTION(1, 0, 0, 1): return "NW";
  case HAT_MAKE_DIRECTION(0, 0, 0, 0): return "centered";
  default:                             return "invalid";
  }
}
