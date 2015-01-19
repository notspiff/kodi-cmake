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

#include "JoystickTypes.h"

#include <stdint.h>

enum ButtonPrimitiveType
{
  ButtonPrimitiveTypeUnknown = 0,
  ButtonPrimitiveTypeButton,
  ButtonPrimitiveTypeHatDirection,
  ButtonPrimitiveTypeSemiAxis,
};

class CButtonPrimitive
{
public:
  /*!
   * \brief Construct an invalid button primitive
   */
  CButtonPrimitive(void);

  /*!
   * \brief Construct an button primitive representing a button press
   */
  CButtonPrimitive(unsigned int buttonIndex);

  /*!
   * \brief Construct an button primitive representing a cardinal hat direction
   */
  CButtonPrimitive(unsigned int hatIndex, HatDirection direction);

  /*!
   * \brief Construct an button primitive representing a semi-axis
   */
  CButtonPrimitive(unsigned int axisIndex, SemiAxisDirection direction);

  bool operator==(const CButtonPrimitive& rhs) const;
  bool operator<(const CButtonPrimitive& rhs) const;

  bool operator!=(const CButtonPrimitive& rhs) const { return !operator==(rhs); }
  bool operator>(const CButtonPrimitive& rhs) const  { return !(operator<(rhs) || operator==(rhs)); }
  bool operator<=(const CButtonPrimitive& rhs) const { return   operator<(rhs) || operator==(rhs); }
  bool operator>=(const CButtonPrimitive& rhs) const { return  !operator<(rhs); }

  ButtonPrimitiveType Type(void) const        { return m_type; }
  unsigned int        Index(void) const       { return m_index; }
  HatDirection        HatDir(void) const      { return m_hatDirection; }
  SemiAxisDirection   SemiAxisDir(void) const { return m_semiAxisDirection; }

  /*!
   * \brief A button primitive is valid if has a known type and
   *        - for hats, is a cardinal direction
   *        - for semi-axes, is a positive or negative direction
   */
  bool IsValid(void) const;

private:
  ButtonPrimitiveType m_type;
  unsigned int        m_index;
  HatDirection        m_hatDirection;
  SemiAxisDirection   m_semiAxisDirection;
};
