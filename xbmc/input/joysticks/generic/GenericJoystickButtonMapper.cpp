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
#include "GenericJoystickButtonMapper.h"

#include <algorithm>

// --- ButtonIndexEquals -------------------------------------------------------

struct ButtonIndexEquals
{
  ButtonIndexEquals(unsigned int targetIndex) : m_targetIndex(targetIndex) { }
  bool operator()(const ButtonPrimitive& button) { return button.Type() == ButtonPrimitiveTypeButton && button.Index() == m_targetIndex; }
private:
  const unsigned int m_targetIndex;
};

// --- HatIndexEquals ----------------------------------------------------------

struct HatIndexEquals
{
  HatIndexEquals(unsigned int targetIndex) : m_targetIndex(targetIndex) { }
  bool operator()(const ButtonPrimitive& button) { return button.Type() == ButtonPrimitiveTypeHatDirection && button.Index() == m_targetIndex; }
private:
  const unsigned int m_targetIndex;
};

// --- AxisIndexEquals ---------------------------------------------------------

struct AxisIndexEquals
{
  AxisIndexEquals(unsigned int targetIndex) : m_targetIndex(targetIndex) { }
  bool operator()(const ButtonPrimitive& button) { return button.Type() == ButtonPrimitiveTypeSemiAxis && button.Index() == m_targetIndex; }
private:
  const unsigned int m_targetIndex;
};

// --- CGenericJoystickButtonMapper --------------------------------------------

CGenericJoystickButtonMapper::CGenericJoystickButtonMapper(IButtonMapper *buttonMapper)
  : m_buttonMapper(buttonMapper)
{
  m_handler = new CGenericJoystickActionHandler();
  RegisterHandler(m_handler);

  ButtonMap buttonMap;
  m_buttonMapper->LoadButtonMap(JoystickIdentifier(), buttonMap); // TODO

  for (ButtonMap::const_iterator it = buttonMap.begin(); it != buttonMap.end(); ++it)
  {
    const ButtonRecipe& buttonRecipe = *it;
    for (std::vector<ButtonPrimitive>::const_iterator it2 = buttonRecipe.buttons.begin(); it2 != buttonRecipe.buttons.end(); ++it2)
    {
      const ButtonPrimitive& button = *it2;
      m_buttonMap[button].push_back(buttonRecipe);
    }
  }
}

CGenericJoystickButtonMapper::~CGenericJoystickButtonMapper()
{
  delete m_handler;
}

bool CGenericJoystickButtonMapper::OnRawButtonPress(unsigned int index)
{
  /* TODO
  bool bMapped = false;
  bool bOverloaded = false;

  ButtonMap::const_iterator it = m_buttonMap.find(ButtonPrimitive(index));
  if (it != m_buttonMap.end())
  {
    bMapped = true;
    bOverloaded = (it->buttons.size() >= 2);
  }
  */

  return true;
}

bool CGenericJoystickButtonMapper::OnRawButtonHold(unsigned int index)
{
  return true;
}

bool CGenericJoystickButtonMapper::OnRawButtonDoublePress(unsigned int index)
{
  return true;
}

bool CGenericJoystickButtonMapper::OnRawButtonDoublePressHold(unsigned int index)
{
  return true;
}

bool CGenericJoystickButtonMapper::OnRawButtonRelease(unsigned int index)
{
  return true;
}

bool CGenericJoystickButtonMapper::OnRawHatPress(unsigned int index, HatDirection direction)
{
  return true;
}

bool CGenericJoystickButtonMapper::OnRawHatMotion(unsigned int index, HatDirection direction)
{
  return true;
}

bool CGenericJoystickButtonMapper::OnRawHatHold(unsigned int index, HatDirection direction)
{
  return true;
}

bool CGenericJoystickButtonMapper::OnRawHatDoublePress(unsigned int index, HatDirection direction)
{
  return true;
}

bool CGenericJoystickButtonMapper::OnRawHatDoublePressHold(unsigned int index, HatDirection direction)
{
  return true;
}

bool CGenericJoystickButtonMapper::OnRawHatRelease(unsigned int index)
{
  return true;
}

bool CGenericJoystickButtonMapper::OnRawAxisPress(unsigned int index, SemiAxisDirection direction)
{
  return true;
}

bool CGenericJoystickButtonMapper::OnRawAxisMotion(unsigned int index, float position)
{
  return true;
}

bool CGenericJoystickButtonMapper::OnRawAxisHold(unsigned int index, SemiAxisDirection direction)
{
  return true;
}

bool CGenericJoystickButtonMapper::OnRawAxisDoublePress(unsigned int index, SemiAxisDirection direction)
{
  return true;
}

bool CGenericJoystickButtonMapper::OnRawAxisDoublePressHold(unsigned int index, SemiAxisDirection direction)
{
  return true;
}

bool CGenericJoystickButtonMapper::OnRawAxisRelease(unsigned int index)
{
  return true;
}

bool CGenericJoystickButtonMapper::OnRawMultiPress(const std::vector<ButtonPrimitive>& buttons)
{
  return true;
}
