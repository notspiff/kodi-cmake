#pragma once
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

#include "AddonCallbacks.h"
#include "include/xbmc_adsp_types.h"

namespace ActiveAE
{
  class CActiveAEDSPAddon;
}

namespace ADDON
{

/*!
 * Callbacks for a audio DSP add-on to XBMC.
 *
 * Also translates the addon's C structures to XBMC's C++ structures.
 */
class CAddonCallbacksADSP
{
public:
  CAddonCallbacksADSP(CAddon* addon);
  ~CAddonCallbacksADSP(void);

  /*!
   * @return The callback table.
   */
  CB_ADSPLib *GetCallbacks() { return m_callbacks; }

  /*!
   * @brief Add or replace a menu hook for the menu for this add-on
   * @param addonData A pointer to the add-on.
   * @param hook The hook to add.
   */
  static void ADSPAddMenuHook(void* addonData, AE_DSP_MENUHOOK* hook);

  /*!
   * @brief Remove a menu hook for the menu for this add-on
   * @param addonData A pointer to the add-on.
   * @param hook The hook to remove.
   */
  static void ADSPRemoveMenuHook(void* addonData, AE_DSP_MENUHOOK* hook);

  /*!
   * @brief Add or replace master mode information inside audio dsp database.
   * Becomes identifier written inside mode to iModeID if it was 0 (undefined)
   * @param mode The master mode to add or update inside database
   */
  static void ADSPRegisterMode(void* addonData, AE_DSP_MODES::AE_DSP_MODE* mode);

  /*!
   * @brief Remove a master mode from audio dsp database
   * @param mode The Mode to remove
   */
  static void ADSPUnregisterMode(void* addonData, AE_DSP_MODES::AE_DSP_MODE* mode);

private:
  static ActiveAE::CActiveAEDSPAddon* GetAudioDSPAddon(void* addonData);

  CB_ADSPLib   *m_callbacks; /*!< callback addresses */
  CAddon       *m_addon;     /*!< the addon */
};

}; /* namespace ADDON */
