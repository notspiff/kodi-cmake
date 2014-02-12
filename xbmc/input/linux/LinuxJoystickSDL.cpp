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

#include "system.h"
#if defined(HAS_SDL_JOYSTICK)

#include "LinuxJoystickSDL.h"
#include "input/JoystickManager.h"
#include "utils/log.h"
#include "utils/StringUtils.h"

#include <SDL/SDL.h>
#include <SDL/SDL_joystick.h>

#define MAX_AXES          64
#define MAX_AXISAMOUNT    32768

CLinuxJoystickSDL::CLinuxJoystickSDL(const std::string& name, SDL_Joystick *pJoystick, unsigned int id)
 : CJoystick(name, id, SDL_JoystickNumButtons(pJoystick), SDL_JoystickNumButtons(pJoystick), SDL_JoystickNumButtons(pJoystick)),
   m_pJoystick(pJoystick)
{
  CLog::Log(LOGNOTICE, "Enabled Joystick: \"%s\" (SDL)", name.c_str());
  CLog::Log(LOGNOTICE, "Details: Total Axes: %d Total Hats: %d Total Buttons: %d",
    SDL_JoystickNumButtons(pJoystick), SDL_JoystickNumButtons(pJoystick), SDL_JoystickNumButtons(pJoystick));
}

/* static */
void CLinuxJoystickSDL::Initialize(JoystickArray &joysticks)
{
  Deinitialize(joysticks);

  if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0)
  {
    CLog::Log(LOGERROR, "(Re)start joystick subsystem failed : %s", SDL_GetError());
    return;
  }

  // Any joysticks connected?
  if (SDL_NumJoysticks() > 0)
  {
    // Load joystick names and open all connected joysticks
    for (int i = 0 ; i < SDL_NumJoysticks(); i++)
    {
      SDL_Joystick *joy = SDL_JoystickOpen(i);
#if defined(TARGET_DARWIN)
      // On OS X, the 360 controllers are handled externally, since the SDL code is
      // really buggy and doesn't handle disconnects.
      if (std::string(SDL_JoystickName(i)).find("360") != std::string::npos)
      {
        CLog::Log(LOGNOTICE, "Ignoring joystick: %s", SDL_JoystickName(i));
        continue;
      }
#endif
      if (joy)
      {
	    // Some (Microsoft) Keyboards are recognized as Joysticks by modern kernels
		// Don't enumerate them
        // https://bugs.launchpad.net/ubuntu/+source/linux/+bug/390959
        // NOTICE: Enabled Joystick: Microsoft Wired Keyboard 600
        // Details: Total Axis: 37 Total Hats: 0 Total Buttons: 57
        // NOTICE: Enabled Joystick: Microsoft Microsoft® 2.4GHz Transceiver v6.0
        // Details: Total Axis: 37 Total Hats: 0 Total Buttons: 57
        int num_axis = SDL_JoystickNumAxes(joy);
        int num_buttons = SDL_JoystickNumButtons(joy);
        if (num_axis > 20 && num_buttons > 50)
          CLog::Log(LOGNOTICE, "Your Joystick seems to be a Keyboard, ignoring it: %s Axis: %d Buttons: %d", 
            SDL_JoystickName(i), num_axis, num_buttons);
        else
          joysticks.push_back(JoystickPtr(new CLinuxJoystickSDL(SDL_JoystickName(i), joy, CJoystickManager::Get().NextID())));
      }
    }
  }

  // Disable joystick events, since we'll be polling them
  SDL_JoystickEventState(SDL_DISABLE);
}

/* static */
void CLinuxJoystickSDL::Deinitialize(JoystickArray &joysticks)
{
  for (int i = 0; i < (int)joysticks.size(); i++)
  {
    if (boost::dynamic_pointer_cast<CLinuxJoystickSDL>(joysticks[i]))
      joysticks.erase(joysticks.begin() + i--);
  }
  // Restart SDL joystick subsystem
  SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
  if (SDL_WasInit(SDL_INIT_JOYSTICK) !=  0)
    CLog::Log(LOGERROR, "Stopping joystick SDL subsystem failed");
}

void CLinuxJoystickSDL::Update()
{
  CJoystickState &state = InitialState();

  // Update the state of all opened joysticks
  SDL_JoystickUpdate();

  // Gamepad buttons
  for (unsigned int b = 0; b < state.buttons.size(); b++)
    state.buttons[b] = (SDL_JoystickGetButton(m_pJoystick, b) ? true : false);

  // Gamepad hats
  for (unsigned int h = 0; h < state.hats.size(); h++)
  {
    state.hats[h].Center();
    uint8_t hat = SDL_JoystickGetHat(m_pJoystick, h);
    if      (hat & SDL_HAT_UP)    state.hats[h][CJoystickHat::UP] = true;
    else if (hat & SDL_HAT_DOWN)  state.hats[h][CJoystickHat::DOWN] = true;
    if      (hat & SDL_HAT_RIGHT) state.hats[h][CJoystickHat::RIGHT] = true;
    else if (hat & SDL_HAT_LEFT)  state.hats[h][CJoystickHat::LEFT] = true;
  }

  // Gamepad axes
  for (unsigned int a = 0; a < state.axes.size(); a++)
    state.SetAxis(a, (long)SDL_JoystickGetAxis(m_pJoystick, a), MAX_AXISAMOUNT);
}

#endif // HAS_SDL_JOYSTICK
