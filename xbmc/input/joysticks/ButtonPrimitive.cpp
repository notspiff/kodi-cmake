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

#include "ButtonPrimitive.h"

CButtonPrimitive::CButtonPrimitive(void)
  : m_type(),
    m_index(0),
    m_hatDirection(),
    m_axisDirection()
{
}

CButtonPrimitive::CButtonPrimitive(unsigned int index)
  : m_type(ButtonPrimitiveTypeButton),
    m_index(index),
    m_hatDirection(),
    m_axisDirection()
{
}

CButtonPrimitive::CButtonPrimitive(unsigned int index, HatDirection direction)
  : m_type(ButtonPrimitiveTypeHatDirection),
    m_index(index),
    m_hatDirection(direction),
    m_axisDirection()
{
}

CButtonPrimitive::CButtonPrimitive(unsigned int index, SemiAxisDirection direction)
  : m_type(ButtonPrimitiveTypeSemiAxis),
    m_index(index),
    m_hatDirection(),
    m_axisDirection(direction)
{
}

bool CButtonPrimitive::operator==(const CButtonPrimitive& rhs) const
{
  if (m_type == rhs.m_type)
  {
    switch (m_type)
    {
    case ButtonPrimitiveTypeButton:
      return m_index == rhs.m_index;
    case ButtonPrimitiveTypeHatDirection:
      return m_index == rhs.m_index && m_hatDirection == rhs.m_hatDirection;
    case ButtonPrimitiveTypeSemiAxis:
      return m_index == rhs.m_index && m_axisDirection == rhs.m_axisDirection;
    case ButtonPrimitiveTypeUnknown:
    default:
      return true;
    }
  }
  return false;
}

bool CButtonPrimitive::operator<(const CButtonPrimitive& rhs) const
{
  if (m_type < rhs.m_type) return true;
  if (m_type > rhs.m_type) return false;

  if (m_type)
  {
    if (m_index < rhs.m_index) return true;
    if (m_index > rhs.m_index) return false;
  }

  if (m_type == ButtonPrimitiveTypeHatDirection)
  {
    if (m_hatDirection < rhs.m_hatDirection) return true;
    if (m_hatDirection > rhs.m_hatDirection) return false;
  }

  if (m_type == ButtonPrimitiveTypeSemiAxis)
  {
    if (m_axisDirection < rhs.m_axisDirection) return true;
    if (m_axisDirection > rhs.m_axisDirection) return false;
  }

  return false;
}
