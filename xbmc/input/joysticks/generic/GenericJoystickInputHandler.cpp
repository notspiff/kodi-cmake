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

#include <math.h>

#include "input/joysticks/generic/GenericJoystickInputHandler.h"
#include "input/joysticks/generic/GenericJoystickMultiPressDetector.h"
#include "input/joysticks/generic/GenericRawButtonInputHandler.h"
#include "input/joysticks/generic/GenericRawHatInputHandler.h"
#include "input/joysticks/generic/GenericRawAxisInputHandler.h"
#include "threads/SingleLock.h"
#include "utils/log.h"

CGenericJoystickInputHandler::CGenericJoystickInputHandler(IButtonMapper* buttonMapper, unsigned int buttonCount, unsigned int hatCount, unsigned int axisCount)
{
  m_buttonHandlers.reserve(buttonCount);
  for (unsigned int i = 0; i < buttonCount; i++)
    m_buttonHandlers.push_back(new CGenericRawButtonInputHandler(i, this));

  m_hatHandlers.reserve(hatCount);
  for (unsigned int i = 0; i < hatCount; i++)
    m_hatHandlers.push_back(new CGenericRawHatInputHandler(i, this));

  m_axisHandlers.reserve(axisCount);
  for (unsigned int i = 0; i < axisCount; i++)
    m_axisHandlers.push_back(new CGenericRawAxisInputHandler(i, this));

  m_inputHandler = new CGenericJoystickMultiPressDetector(buttonMapper);
  RegisterHandler(m_inputHandler);
}

CGenericJoystickInputHandler::~CGenericJoystickInputHandler()
{
  for (unsigned int i = 0; i < m_buttonHandlers.size(); i++)
    delete m_buttonHandlers[i];

  for (unsigned int i = 0; i < m_hatHandlers.size(); i++)
    delete m_hatHandlers[i];

  for (unsigned int i = 0; i < m_axisHandlers.size(); i++)
    delete m_axisHandlers[i];

  delete m_inputHandler;
}

bool CGenericJoystickInputHandler::HandleJoystickEvent(JoystickEvent event,
                                                       unsigned int  index,
                                                       int64_t       timeNs,
                                                       bool          bPressed  /* = false */,
                                                       HatDirection  direction /* = HatDirectionNone */,
                                                       float         axisPos   /* = 0.0f */)
{
  if (timeNs < 0)
    return false;

  CSingleLock lock(m_critical);

  bool result = false;

  switch (event)
  {
    case JoystickEventRawButton:
      if (index < m_buttonHandlers.size())
        result = m_buttonHandlers[index]->OnMotion(bPressed);
      break;

    case JoystickEventRawHat:
      if (index < m_hatHandlers.size())
        result = m_hatHandlers[index]->OnMotion(direction);
      break;

    case JoystickEventRawAxis:
      if (index < m_axisHandlers.size())
        result = m_axisHandlers[index]->OnMotion(axisPos);
      break;

    default:
      CLog::Log(LOGDEBUG, "CGenericJoystickInputHandler: unknown JoystickEvent: %u", event);
      break;
  }

  return result;
}
