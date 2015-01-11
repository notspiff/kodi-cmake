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
#ifndef __PERIPHERAL_DLL_H__
#define __PERIPHERAL_DLL_H__

#include "xbmc_peripheral_types.h"

#define PERIPHERAL_ADDON_JOYSTICKS // TODO

#ifdef __cplusplus
extern "C"
{
#endif

  /// @name Peripheral operations
  ///{
  /*!
   * @brief  Get the PERIPHERAL_API_VERSION used to compile this peripheral add-on
   * @return XBMC_PERIPHERAL_API_VERSION from xbmc_peripheral_types
   * @remarks Valid implementation required
   *
   * Used to check if the implementation is compatible with the frontend.
   */
  const char* GetPeripheralAPIVersion(void);

  /*!
   * @brief Get the XBMC_PERIPHERAL_MIN_API_VERSION used to compile this peripheral add-on
   * @return XBMC_PERIPHERAL_MIN_API_VERSION from xbmc_peripheral_types
   * @remarks Valid implementation required
   *
   * Used to check if the implementation is compatible with the frontend.
   */
  const char* GetMinimumPeripheralAPIVersion(void);

  /*!
   * @brief Get the list of features that this add-on provides
   * @param pCapabilities The add-on's capabilities.
   * @return PERIPHERAL_NO_ERROR if the properties were fetched successfully.
   * @remarks Valid implementation required.
   *
   * Called by the frontend to query the add-on's capabilities and supported
   * peripherals. All capabilities that the add-on supports should be set to true.
   *
   */
  PERIPHERAL_ERROR GetAddonCapabilities(PERIPHERAL_CAPABILITIES *pCapabilities);

  /*!
   * @brief Perform a scan for joysticks
   * @return PERIPHERAL_NO_ERROR if successful; joysticks must be freed using
   * FreeJoysticks() in this case
   *
   * The frontend calls this when a hardware change is detected. If an add-on
   * detects a hardware change, it can trigger this function using the
   * TriggerScan() callback.
   */
  PERIPHERAL_ERROR PerformDeviceScan(unsigned int* peripheral_count, PERIPHERAL_INFO** scan_results);
  void FreeScanResults(unsigned int peripheral_count, PERIPHERAL_INFO* scan_results);
  ///}

  /// @name Joystick operations
  /*!
   * @note #define PERIPHERAL_ADDON_JOYSTICKS before including xbmc_peripheral_dll.h
   * in the add-on if the add-on provides joysticks and add provides_joysticks="true"
   * to the xbmc.peripheral extension point node in addon.xml.
   */
  ///{
#ifdef PERIPHERAL_ADDON_JOYSTICKS
  PERIPHERAL_ERROR GetJoystickInfo(unsigned int index, JOYSTICK_INFO* info);
  void             FreeJoystickInfo(JOYSTICK_INFO* info);

  /*!
   * @brief Get all events that have occurred since the last call to GetEvents()
   * @return PERIPHERAL_NO_ERROR if successful; events must be freed using
   * FreeEvents() in this case
   */
  PERIPHERAL_ERROR GetEvents(unsigned int* event_count, PERIPHERAL_EVENT** events);
  void FreeEvents(unsigned int event_count, PERIPHERAL_EVENT* events);

  /*!
   * @brief Look up joystick elements in the button map
   */
  JOYSTICK_ID GetAction(JOYSTICK_BUTTON_PRIMITIVE* source);
  JOYSTICK_ID GetAnalogStick(unsigned int axis_index, unsigned int* horiz_index, unsigned int* vert_index);
  JOYSTICK_ID GetAccelerometer(unsigned int axis_index, unsigned int* x_index, unsigned int* y_index, unsigned int* z_index);
#endif
  ///}

  /*!
   * Called by the frontend to assign the function pointers of this add-on to
   * pClient. Note that get_addon() is defined here, so it will be available in
   * all compiled peripheral add-ons.
   */
  void __declspec(dllexport) get_addon(struct PeripheralAddon* pClient)
  {
    pClient->GetPeripheralAPIVersion        = GetPeripheralAPIVersion;
    pClient->GetMinimumPeripheralAPIVersion = GetMinimumPeripheralAPIVersion;
    pClient->GetAddonCapabilities           = GetAddonCapabilities;
    pClient->PerformDeviceScan              = PerformDeviceScan;
    pClient->FreeScanResults                = FreeScanResults;

#ifdef PERIPHERAL_ADDON_JOYSTICKS
    pClient->GetJoystickInfo                = GetJoystickInfo;
    pClient->FreeJoystickInfo               = FreeJoystickInfo;
    pClient->GetEvents                      = GetEvents;
    pClient->FreeEvents                     = FreeEvents;
    pClient->GetAction                      = GetAction;
    pClient->GetAnalogStick                 = GetAnalogStick;
    pClient->GetAccelerometer               = GetAccelerometer;
#endif
  };

#ifdef __cplusplus
};
#endif

#endif // __PERIPHERAL_DLL_H__
