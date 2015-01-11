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

#include "input/joysticks/IJoystickInputHandler.h"
#include "threads/CriticalSection.h"

#include <vector>

class CGenericJoystickMultiPressDetector;
class CGenericRawButtonInputHandler;
class CGenericRawHatInputHandler;
class CGenericRawAxisInputHandler;
class IButtonMapper;

/*!
 * \ingroup joysticks_generic
 * \brief Generic implementation of IJoystickInputHandler to handle low level (raw)
 *        joystick events and translate them into joystick actions which are passed
 *        on to the registered IJoystickActionHandler implementation.
 *
 * The generic implementation supports single a double touch and hold
 * actions and basic gesture recognition for panning, swiping, pinching/zooming
 * and rotating.
 *
 * \sa IJoystickInputHandler
 */
class CGenericJoystickInputHandler : public IJoystickInputHandler
{
public:
  CGenericJoystickInputHandler(IButtonMapper* buttonMapper, unsigned int buttonCount, unsigned int hatCount, unsigned int axisCount);

  virtual ~CGenericJoystickInputHandler();

  // implementation of IJoystickInputHandler
  virtual bool HandleJoystickEvent(JoystickEvent event,
                                   unsigned int  index,
                                   int64_t       timeNs,
                                   bool          bPressed  = false,
                                   HatDirection  direction = HatDirectionNone,
                                   float         axisPos   = 0.0f);

private:
  CCriticalSection                            m_critical;
  std::vector<CGenericRawButtonInputHandler*> m_buttonHandlers;
  std::vector<CGenericRawHatInputHandler*>    m_hatHandlers;
  std::vector<CGenericRawAxisInputHandler*>   m_axisHandlers;
  CGenericJoystickMultiPressDetector          *m_inputHandler;
};
