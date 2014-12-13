/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include "system.h"
#include "input/Key.h"

CKey::CKey(void)
{
  Reset();
}

CKey::~CKey(void)
{}

CKey::CKey(uint32_t buttonCode, unsigned int held /* = 0 */)
{
  Reset();
  m_buttonCode = buttonCode;
  m_held = held;
}

CKey::CKey(uint8_t vkey, wchar_t unicode, char ascii, uint32_t modifiers, unsigned int held)
{
  Reset();
  if (vkey) // FIXME: This needs cleaning up - should we always use the unicode key where available?
    m_buttonCode = vkey | KEY_VKEY;
  else
    m_buttonCode = KEY_UNICODE;
  m_buttonCode |= modifiers;
  m_vkey = vkey;
  m_unicode = unicode;
  m_ascii = ascii;
  m_modifiers = modifiers;
  m_held = held;
}

CKey::CKey(const CKey& key)
{
  *this = key;
}

void CKey::Reset()
{
  m_fromService = false;
  m_buttonCode = KEY_INVALID;
  m_vkey = 0;
  m_unicode = 0;
  m_ascii = 0;
  m_modifiers = 0;
  m_held = 0;
}

CKey& CKey::operator=(const CKey& key)
{
  if (&key == this) return * this;
  m_fromService  = key.m_fromService;
  m_buttonCode   = key.m_buttonCode;
  m_vkey         = key.m_vkey;
  m_unicode     = key.m_unicode;
  m_ascii       = key.m_ascii;
  m_modifiers    = key.m_modifiers;
  m_held         = key.m_held;
  return *this;
}

bool CKey::FromKeyboard() const
{
  return (m_buttonCode >= KEY_VKEY && m_buttonCode != KEY_INVALID);
}

bool CKey::IsIRRemote() const
{
  if (GetButtonCode() < 256)
    return true;
  return false;
}

void CKey::SetFromService(bool fromService)
{
  if (fromService && (m_buttonCode & KEY_ASCII))
    m_unicode = m_buttonCode - KEY_ASCII;
    
  m_fromService = fromService;
}

CAction::CAction(int actionID, float amount1 /* = 1.0f */, float amount2 /* = 0.0f */, const std::string &name /* = "" */, unsigned int holdTime /*= 0*/)
{
  m_id = actionID;
  m_amount[0] = amount1;
  m_amount[1] = amount2;
  for (unsigned int i = 2; i < max_amounts; i++)
    m_amount[i] = 0;  
  m_name = name;
  m_repeat = 0;
  m_buttonCode = 0;
  m_unicode = 0;
  m_holdTime = holdTime;
}

CAction::CAction(int actionID, unsigned int state, float posX, float posY, float offsetX, float offsetY, const std::string &name):
  m_name(name)
{
  m_id = actionID;
  m_amount[0] = posX;
  m_amount[1] = posY;
  m_amount[2] = offsetX;
  m_amount[3] = offsetY;
  for (unsigned int i = 4; i < max_amounts; i++)
    m_amount[i] = 0;
  m_repeat = 0;
  m_buttonCode = 0;
  m_unicode = 0;
  m_holdTime = state;
}

CAction::CAction(int actionID, wchar_t unicode)
{
  m_id = actionID;
  for (unsigned int i = 0; i < max_amounts; i++)
    m_amount[i] = 0;  
  m_repeat = 0;
  m_buttonCode = 0;
  m_unicode = unicode;
  m_holdTime = 0;
}

CAction::CAction(int actionID, const std::string &name, const CKey &key):
  m_name(name)
{
  m_id = actionID;
  m_amount[0] = 1; // digital button (could change this for repeat acceleration)
  for (unsigned int i = 1; i < max_amounts; i++)
    m_amount[i] = 0;
  m_buttonCode = key.GetButtonCode();
  m_unicode = 0;
  m_holdTime = key.GetHeld();
}

CAction::CAction(int actionID, const std::string &name):
  m_name(name)
{
  m_id = actionID;
  for (unsigned int i = 0; i < max_amounts; i++)
    m_amount[i] = 0;
  m_repeat = 0;
  m_buttonCode = 0;
  m_unicode = 0;
  m_holdTime = 0;
}
