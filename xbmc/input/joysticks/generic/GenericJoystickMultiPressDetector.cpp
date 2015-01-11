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

#include "GenericJoystickMultiPressDetector.h"
#include "GenericJoystickButtonMapper.h"
#include "threads/SystemClock.h"

#include <algorithm>

using namespace XbmcThreads;

#define BUTTON_COMBO_TIMEOUT_MS  80 // TODO

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

// --- CGenericJoystickMultiPressDetector --------------------------------------

CGenericJoystickMultiPressDetector::CGenericJoystickMultiPressDetector(IButtonMapper* buttonMapper)
  : m_lastPressMs(0)
{
  m_buttonMapper = new CGenericJoystickButtonMapper(buttonMapper);
  RegisterHandler(m_buttonMapper);
}

CGenericJoystickMultiPressDetector::~CGenericJoystickMultiPressDetector()
{
  delete m_buttonMapper;
}

bool CGenericJoystickMultiPressDetector::OnRawButtonPress(unsigned int index)
{
  ClearButton(index);
  OnPress(ButtonPrimitive(index));
  return IJoystickInputHandling::OnRawButtonPress(index);
}

bool CGenericJoystickMultiPressDetector::OnRawButtonDoublePress(unsigned int index)
{
  ClearButton(index);
  OnPress(ButtonPrimitive(index));
  return IJoystickInputHandling::OnRawButtonDoublePress(index);
}

bool CGenericJoystickMultiPressDetector::OnRawButtonRelease(unsigned int index)
{
  ClearButton(index);
  return IJoystickInputHandling::OnRawButtonRelease(index);
}

bool CGenericJoystickMultiPressDetector::OnRawHatPress(unsigned int index, HatDirection direction)
{
  ClearHat(index);
  OnPress(ButtonPrimitive(index, direction));
  return IJoystickInputHandling::OnRawHatPress(index, direction);
}

bool CGenericJoystickMultiPressDetector::OnRawHatMotion(unsigned int index, HatDirection direction)
{
  ClearHat(index);
  OnPress(ButtonPrimitive(index, direction));
  return IJoystickInputHandling::OnRawHatMotion(index, direction);
}

bool CGenericJoystickMultiPressDetector::OnRawHatDoublePress(unsigned int index, HatDirection direction)
{
  ClearHat(index);
  OnPress(ButtonPrimitive(index, direction));
  return IJoystickInputHandling::OnRawHatDoublePress(index, direction);
}

bool CGenericJoystickMultiPressDetector::OnRawHatRelease(unsigned int index)
{
  ClearHat(index);
  return IJoystickInputHandling::OnRawHatRelease(index);
}

bool CGenericJoystickMultiPressDetector::OnRawAxisPress(unsigned int index, SemiAxisDirection direction)
{
  ClearAxis(index);
  OnPress(ButtonPrimitive(index, direction));
  return IJoystickInputHandling::OnRawAxisPress(index, direction);
}

bool CGenericJoystickMultiPressDetector::OnRawAxisDoublePress(unsigned int index, SemiAxisDirection direction)
{
  ClearAxis(index);
  OnPress(ButtonPrimitive(index, direction));
  return IJoystickInputHandling::OnRawAxisDoublePress(index, direction);
}

bool CGenericJoystickMultiPressDetector::OnRawAxisRelease(unsigned int index)
{
  ClearAxis(index);
  return IJoystickInputHandling::OnRawAxisRelease(index);
}

bool CGenericJoystickMultiPressDetector::OnPress(const ButtonPrimitive& button)
{
  const unsigned int pressTimeMs = SystemClockMillis();

  // Check if a previous multi-press timed out
  if (pressTimeMs - m_lastPressMs > BUTTON_COMBO_TIMEOUT_MS)
    m_currentCombo.clear();

  // Append the new press to the current multi-press
  m_currentCombo.push_back(button);

  // Dispatch >= 2 simultaneous presses
  if (m_currentCombo.size() >= 2)
    OnRawMultiPress(m_currentCombo);

  m_lastPressMs = pressTimeMs;
  return true;
}

void CGenericJoystickMultiPressDetector::ClearButton(unsigned int index)
{
  m_currentCombo.erase(std::remove_if(m_currentCombo.begin(), m_currentCombo.end(), ButtonIndexEquals(index)), m_currentCombo.end());
}

void CGenericJoystickMultiPressDetector::ClearHat(unsigned int index)
{
  m_currentCombo.erase(std::remove_if(m_currentCombo.begin(), m_currentCombo.end(), HatIndexEquals(index)), m_currentCombo.end());
}

void CGenericJoystickMultiPressDetector::ClearAxis(unsigned int index)
{
  m_currentCombo.erase(std::remove_if(m_currentCombo.begin(), m_currentCombo.end(), AxisIndexEquals(index)), m_currentCombo.end());
}
