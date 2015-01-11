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

#include "GenericJoystickActionHandler.h"

bool CGenericJoystickActionHandler::OnButtonPress(JoystickActionID id)
{
  return false; // TODO
}

bool CGenericJoystickActionHandler::OnButtonMotion(JoystickActionID id, float magnitude)
{
  return false; // TODO
}

bool CGenericJoystickActionHandler::OnButtonHold(JoystickActionID id)
{
  return false; // TODO
}

bool CGenericJoystickActionHandler::OnButtonDoublePress(JoystickActionID id)
{
  return false; // TODO
}

bool CGenericJoystickActionHandler::OnMultiPress(const std::vector<JoystickActionID>& ids)
{
  return false; // TODO
}

bool CGenericJoystickActionHandler::OnAnalogStickMotion(JoystickActionID id, float x, float y)
{
  return false; // TODO
}

bool CGenericJoystickActionHandler::OnAnalogStickThreshold(JoystickActionID, bool bPressed, HatDirection direction)
{
  return false; // TODO
}

bool CGenericJoystickActionHandler::OnAccelerometerMotion(JoystickActionID, float x, float y, float z)
{
  return false; // TODO
}
