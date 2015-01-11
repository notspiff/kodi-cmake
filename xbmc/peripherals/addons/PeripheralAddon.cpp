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

#include "PeripheralAddon.h"
#include "addons/AddonManager.h"
#include "filesystem/SpecialProtocol.h"
#include "peripherals/Peripherals.h"
#include "peripherals/bus/PeripheralBusAddon.h"
#include "peripherals/devices/PeripheralJoystick.h"
#include "utils/log.h"
#include "utils/StringUtils.h"

#include <algorithm>
#include <string.h>

using namespace PERIPHERALS;

#ifndef SAFE_DELETE
  #define SAFE_DELETE(p)  do { delete (p); (p) = NULL; } while (0)
#endif

CPeripheralAddon::CPeripheralAddon(const ADDON::AddonProps& props)
 : CAddonDll<DllPeripheral, PeripheralAddon, PERIPHERAL_PROPERTIES>(props),
   m_apiVersion("0.0.0"),
   m_bProvidesJoysticks(false)
{
  ResetProperties();
}

CPeripheralAddon::CPeripheralAddon(const cp_extension_t *ext)
 : CAddonDll<DllPeripheral, PeripheralAddon, PERIPHERAL_PROPERTIES>(ext),
   m_apiVersion("0.0.0")
{
  ResetProperties();

  std::string strProvidesJoysticks = ADDON::CAddonMgr::Get().GetExtValue(ext->configuration, "@provides_joysticks");

  m_bProvidesJoysticks = StringUtils::EqualsNoCase(strProvidesJoysticks, "true");
}

CPeripheralAddon::~CPeripheralAddon(void)
{
  Destroy();
  SAFE_DELETE(m_pInfo);
}

void CPeripheralAddon::ResetProperties(void)
{
  /* initialise members */
  SAFE_DELETE(m_pInfo);
  m_pInfo = new PERIPHERAL_PROPERTIES;
  m_strUserPath        = CSpecialProtocol::TranslatePath(Profile());
  m_pInfo->user_path   = m_strUserPath.c_str();
  m_strClientPath      = CSpecialProtocol::TranslatePath(Path());
  m_pInfo->addon_path  = m_strClientPath.c_str();
  m_apiVersion = ADDON::AddonVersion("0.0.0");
  // TODO
}

ADDON::AddonPtr CPeripheralAddon::GetRunningInstance(void) const
{
  CPeripheralBusAddon* addonBus = static_cast<CPeripheralBusAddon*>(g_peripherals.GetBusByType(PERIPHERAL_BUS_ADDON));
  if (addonBus)
  {
    ADDON::AddonPtr peripheralAddon;
    if (addonBus->GetAddon(ID(), peripheralAddon))
      return peripheralAddon;
  }
  return CAddon::GetRunningInstance();
}

ADDON_STATUS CPeripheralAddon::Create(void)
{
  ADDON_STATUS status(ADDON_STATUS_UNKNOWN);

  /* ensure that a previous instance is destroyed */
  Destroy();

  /* reset all properties to defaults */
  ResetProperties();

  /* initialise the add-on */
  CLog::Log(LOGDEBUG, "PERIPHERAL - %s - creating peripheral add-on instance '%s'", __FUNCTION__, Name().c_str());
  try { status = CAddonDll<DllPeripheral, PeripheralAddon, PERIPHERAL_PROPERTIES>::Create(); }
  catch (const std::exception &e) { LogException(e, __FUNCTION__); }

  if (status == ADDON_STATUS_OK)
  {
    if (!GetAddonProperties())
    {
      Destroy();
      status = ADDON_STATUS_PERMANENT_FAILURE;
    }
  }

  return status;
}

void CPeripheralAddon::Destroy(void)
{
  /* reset 'ready to use' to false */
  CLog::Log(LOGDEBUG, "PERIPHERAL - %s - destroying peripheral add-on '%s'", __FUNCTION__, Name().c_str());

  /* destroy the add-on */
  try { CAddonDll<DllPeripheral, PeripheralAddon, PERIPHERAL_PROPERTIES>::Destroy(); }
  catch (const std::exception &e) { LogException(e, __FUNCTION__); }

  /* reset all properties to defaults */
  ResetProperties();

  CAddonDll<DllPeripheral, PeripheralAddon, PERIPHERAL_PROPERTIES>::Destroy();
}

bool CPeripheralAddon::GetAddonProperties(void)
{
  PERIPHERAL_CAPABILITIES addonCapabilities = { };

  /* get the capabilities */
  try
  {
    PERIPHERAL_ERROR retVal = m_pStruct->GetAddonCapabilities(&addonCapabilities);
    if (retVal != PERIPHERAL_NO_ERROR)
    {
      CLog::Log(LOGERROR, "PERIPHERAL - couldn't get the capabilities for add-on '%s'. Please contact the developer of this add-on: %s",
          Name().c_str(), Author().c_str());
      return false;
    }
  }
  catch (std::exception &e) { LogException(e, "GetAddonCapabilities()"); return false; }

  // Verify capabilities against addon.xml
  if (m_bProvidesJoysticks != addonCapabilities.provides_joysticks)
  {
    CLog::Log(LOGERROR, "PERIPHERAL - Add-on '%s': provides_joysticks'(%s) in add-on DLL  doesn't match 'provides_joysticks'(%s) in addon.xml. Please contact the developer of this add-on: %s",
        Name().c_str(), addonCapabilities.provides_joysticks ? "true" : "false",
        m_bProvidesJoysticks ? "true" : "false", Author().c_str());
    return false;
  }

  return true;
}

bool CPeripheralAddon::CheckAPIVersion(void)
{
  /* check the API version */
  ADDON::AddonVersion minVersion = ADDON::AddonVersion(PERIPHERAL_MIN_API_VERSION);
  try { m_apiVersion = ADDON::AddonVersion(m_pStruct->GetPeripheralAPIVersion()); }
  catch (std::exception &e) { LogException(e, "GetPeripheralAPIVersion()"); return false; }

  if (!IsCompatibleAPIVersion(minVersion, m_apiVersion))
  {
    CLog::Log(LOGERROR, "PERIPHERAL - Add-on '%s' is using an incompatible API version. XBMC minimum API version = '%s', add-on API version '%s'", Name().c_str(), minVersion.asString().c_str(), m_apiVersion.asString().c_str());
    return false;
  }

  return true;
}

bool CPeripheralAddon::IsCompatibleAPIVersion(const ADDON::AddonVersion &minVersion, const ADDON::AddonVersion &version)
{
  ADDON::AddonVersion myMinVersion = ADDON::AddonVersion(PERIPHERAL_MIN_API_VERSION);
  ADDON::AddonVersion myVersion = ADDON::AddonVersion(PERIPHERAL_API_VERSION);
  return (version >= myMinVersion && minVersion <= myVersion);
}

bool CPeripheralAddon::Register(unsigned int peripheralIndex, CPeripheral *peripheral)
{
  if (!peripheral)
    return false;

  CSingleLock lock(m_critSection);

  if (m_peripherals.find(peripheralIndex) == m_peripherals.end())
  {
    m_peripherals[peripheralIndex] = peripheral;
    CLog::Log(LOGNOTICE, "%s - new %s device registered on %s->%s: %s",
        __FUNCTION__, PeripheralTypeTranslator::TypeToString(peripheral->Type()),
        PeripheralTypeTranslator::BusTypeToString(PERIPHERAL_BUS_ADDON),
        peripheral->Location().c_str(), peripheral->DeviceName().c_str());
    return true;
  }
  return false;
}

void CPeripheralAddon::UnregisterRemovedDevices(const PeripheralScanResults &results, std::vector<CPeripheral*>& removedPeripherals)
{
  CSingleLock lock(m_critSection);
  std::vector<unsigned int> removedIndexes;
  for (std::map<unsigned int, CPeripheral*>::const_iterator it = m_peripherals.begin(); it != m_peripherals.end(); ++it)
  {
    CPeripheral *peripheral = it->second;
    PeripheralScanResult updatedDevice(PERIPHERAL_BUS_ADDON);
    if (!results.GetDeviceOnLocation(peripheral->Location(), &updatedDevice) ||
      *peripheral != updatedDevice)
    {
      /* device removed */
      removedIndexes.push_back(it->first);
      //m_peripherals.erase(m_peripherals.begin() + iDevicePtr);
    }
  }
  lock.Leave();

  for (std::vector<unsigned int>::const_iterator it = removedIndexes.begin(); it != removedIndexes.end(); ++it)
  {
    CPeripheral *peripheral = m_peripherals[*it];
    CLog::Log(LOGNOTICE, "%s - device removed from %s/%s: %s (%s:%s)", __FUNCTION__, PeripheralTypeTranslator::TypeToString(peripheral->Type()), peripheral->Location().c_str(), peripheral->DeviceName().c_str(), peripheral->VendorIdAsString(), peripheral->ProductIdAsString());
    peripheral->OnDeviceRemoved();
    removedPeripherals.push_back(peripheral);
    m_peripherals.erase(*it);
  }
}

bool CPeripheralAddon::HasFeature(const PeripheralFeature feature) const
{
  if (feature == FEATURE_JOYSTICK)
    return m_bProvidesJoysticks;

  return false;
}

void CPeripheralAddon::GetFeatures(std::vector<PeripheralFeature> &features) const
{
  if (m_bProvidesJoysticks && std::find(features.begin(), features.end(), FEATURE_JOYSTICK) == features.end())
    features.push_back(FEATURE_JOYSTICK);
}

CPeripheral* CPeripheralAddon::GetPeripheral(unsigned int index) const
{
  CPeripheral* peripheral(NULL);
  CSingleLock lock(m_critSection);
  std::map<unsigned int, CPeripheral*>::const_iterator it = m_peripherals.find(index);
  if (it != m_peripherals.end())
    peripheral = it->second;
  return peripheral;
}

CPeripheral *CPeripheralAddon::GetByPath(const std::string &strPath) const
{
  CSingleLock lock(m_critSection);
  for (std::map<unsigned int, CPeripheral*>::const_iterator it = m_peripherals.begin(); it != m_peripherals.end(); ++it)
  {
    if (StringUtils::EqualsNoCase(strPath, it->second->FileLocation()))
      return it->second;
  }

  return NULL;
}

int CPeripheralAddon::GetPeripheralsWithFeature(std::vector<CPeripheral*> &results, const PeripheralFeature feature) const
{
  int iReturn(0);
  CSingleLock lock(m_critSection);
  for (std::map<unsigned int, CPeripheral*>::const_iterator it = m_peripherals.begin(); it != m_peripherals.end(); ++it)
  {
    if (it->second->HasFeature(feature))
    {
      results.push_back(it->second);
      ++iReturn;
    }
  }
  return iReturn;
}

size_t CPeripheralAddon::GetNumberOfPeripherals(void) const
{
  CSingleLock lock(m_critSection);
  return m_peripherals.size();
}

size_t CPeripheralAddon::GetNumberOfPeripheralsWithId(const int iVendorId, const int iProductId) const
{
  int iReturn(0);
  CSingleLock lock(m_critSection);
  for (std::map<unsigned int, CPeripheral*>::const_iterator it = m_peripherals.begin(); it != m_peripherals.end(); ++it)
  {
    if (it->second->VendorId() == iVendorId &&
        it->second->ProductId() == iProductId)
      iReturn++;
  }

  return iReturn;
}

void CPeripheralAddon::GetDirectory(const CStdString &strPath, CFileItemList &items) const
{
  CStdString strDevPath;
  CSingleLock lock(m_critSection);
  for (std::map<unsigned int, CPeripheral*>::const_iterator it = m_peripherals.begin(); it != m_peripherals.end(); ++it)
  {
    const CPeripheral *peripheral = it->second;
    if (peripheral->IsHidden())
      continue;

    CFileItemPtr peripheralFile(new CFileItem(peripheral->DeviceName()));
    peripheralFile->SetPath(peripheral->FileLocation());
    peripheralFile->SetProperty("vendor", peripheral->VendorIdAsString());
    peripheralFile->SetProperty("product", peripheral->ProductIdAsString());
    peripheralFile->SetProperty("bus", PeripheralTypeTranslator::BusTypeToString(peripheral->GetBusType()));
    peripheralFile->SetProperty("location", peripheral->Location());
    peripheralFile->SetProperty("class", PeripheralTypeTranslator::TypeToString(peripheral->Type()));
    peripheralFile->SetProperty("version", peripheral->GetVersionInfo());
    items.Add(peripheralFile);
  }
}

bool CPeripheralAddon::PerformDeviceScan(PeripheralScanResults &results)
{
  unsigned int            peripheralCount;
  PERIPHERAL_INFO*        pScanResults;
  PERIPHERAL_ERROR        retVal;

  try { LogError(retVal = m_pStruct->PerformDeviceScan(&peripheralCount, &pScanResults), "PerformDeviceScan()"); }
  catch (std::exception &e) { LogException(e, "PerformDeviceScan()"); return false;  }

  if (retVal == PERIPHERAL_NO_ERROR)
  {
    for (unsigned int i = 0; i < peripheralCount; i++)
    {
      ADDON::Peripheral peripheral(pScanResults[i]);

      PeripheralScanResult result(PERIPHERAL_BUS_ADDON);
      switch (peripheral.Type())
      {
      case PERIPHERAL_TYPE_JOYSTICK:
        result.m_type = PERIPHERAL_JOYSTICK;
        break;
      default:
        continue;
      }

      result.m_strDeviceName = peripheral.Name();
      result.m_strLocation   = StringUtils::Format("%s/%d", ID().c_str(), peripheral.Index());
      result.m_iVendorId     = peripheral.VendorID();
      result.m_iProductId    = peripheral.ProductID();
      result.m_mappedType    = PERIPHERAL_JOYSTICK;
      result.m_mappedBusType = PERIPHERAL_BUS_ADDON;
      result.m_iSequence     = GetNumberOfPeripheralsWithId(result.m_iVendorId, result.m_iProductId);

      if (!results.ContainsResult(result))
        results.m_results.push_back(result);
    }

    try { m_pStruct->FreeScanResults(peripheralCount, pScanResults); }
    catch (std::exception &e) { LogException(e, "FreeJoysticks()"); }

    return true;
  }

  return false;
}
/*
bool CPeripheralAddon::GetJoystickInfo(unsigned int index, ADDON::Joystick& info)
{
  if (!HasFeature(FEATURE_JOYSTICK))
    return false;

  PERIPHERAL_ERROR retVal;

  JOYSTICK_INFO infoStruct;

  try { LogError(retVal = m_pStruct->GetJoystickInfo(index, &infoStruct), "GetJoystickInfo()"); }
  catch (std::exception &e) { LogException(e, "GetJoystickInfo()"); return false; }

  if (retVal == PERIPHERAL_NO_ERROR)
  {
    info = infoStruct;

    try { m_pStruct->FreeJoystickInfo(&infoStruct); }
    catch (std::exception &e) { LogException(e, "FreeJoystickInfo()"); }

    return true;
  }

  return false;
}
*/


bool CPeripheralAddon::ProcessEvents(void)
{
  if (!HasFeature(FEATURE_JOYSTICK))
    return false;

  PERIPHERAL_ERROR retVal;

  unsigned int      eventCount = 0;
  PERIPHERAL_EVENT* pEvents = NULL;

  try { LogError(retVal = m_pStruct->GetEvents(&eventCount, &pEvents), "GetEvents()"); }
  catch (std::exception &e) { LogException(e, "GetEvents()"); return false;  }

  if (retVal == PERIPHERAL_NO_ERROR && pEvents != NULL)
  {
    for (unsigned int i = 0; i < eventCount; i++)
    {
      CPeripheral* device = GetPeripheral(pEvents[i].peripheral_index);
      if (!device)
        continue;

      switch (device->Type())
      {
      case PERIPHERAL_JOYSTICK:
      {
        CPeripheralJoystick* joystickDevice = static_cast<CPeripheralJoystick*>(device);

        switch (pEvents[i].type)
        {
          case JOYSTICK_EVENT_TYPE_RAW_BUTTON:
          {
            const bool bPressed = (pEvents[i].button_state == JOYSTICK_STATE_BUTTON_PRESSED);
            joystickDevice->OnButtonMotion(pEvents[i].raw_index, bPressed);
            break;
          }
          case JOYSTICK_EVENT_TYPE_RAW_HAT:
          {
            HatDirection dir(HatDirectionNone);
            switch (pEvents[i].hat_state)
            {
              case JOYSTICK_STATE_HAT_LEFT:       dir = HatDirectionLeft;      break;
              case JOYSTICK_STATE_HAT_RIGHT:      dir = HatDirectionRight;     break;
              case JOYSTICK_STATE_HAT_UP:         dir = HatDirectionUp;        break;
              case JOYSTICK_STATE_HAT_DOWN:       dir = HatDirectionDown;      break;
              case JOYSTICK_STATE_HAT_LEFT_UP:    dir = HatDirectionLeftUp;    break;
              case JOYSTICK_STATE_HAT_LEFT_DOWN:  dir = HatDirectionLeftDown;  break;
              case JOYSTICK_STATE_HAT_RIGHT_UP:   dir = HatDirectionRightUp;   break;
              case JOYSTICK_STATE_HAT_RIGHT_DOWN: dir = HatDirectionRightDown; break;
              case JOYSTICK_STATE_HAT_UNPRESSED:
              default:
                break;
            }
            joystickDevice->OnHatMotion(pEvents[i].raw_index, dir);
            break;
          }
          case JOYSTICK_EVENT_TYPE_RAW_AXIS:
          {
            joystickDevice->OnAxisMotion(pEvents[i].raw_index, pEvents[i].axis_state);
            break;
          }
          default:
            break;
        }
        break;
      }
      default:
        break;
      }
    }

    try { m_pStruct->FreeEvents(eventCount, pEvents); }
    catch (std::exception &e) { LogException(e, "FreeJoysticks()"); }

    return true;
  }

  return false;
}

const char *CPeripheralAddon::ToString(const PERIPHERAL_ERROR error)
{
  switch (error)
  {
  case PERIPHERAL_NO_ERROR:
    return "no error";
  case PERIPHERAL_ERROR_FAILED:
    return "command failed";
  case PERIPHERAL_ERROR_INVALID_PARAMETERS:
    return "invalid parameters";
  case PERIPHERAL_ERROR_NOT_IMPLEMENTED:
    return "not implemented";
  case PERIPHERAL_ERROR_NOT_CONNECTED:
    return "not connected";
  case PERIPHERAL_ERROR_CONNECTION_FAILED:
    return "connection failed";
  case PERIPHERAL_ERROR_UNKNOWN:
  default:
    return "unknown error";
  }
}

bool CPeripheralAddon::LogError(const PERIPHERAL_ERROR error, const char *strMethod) const
{
  if (error != PERIPHERAL_NO_ERROR)
  {
    CLog::Log(LOGERROR, "PERIPHERAL - %s - addon '%s' returned an error: %s",
        strMethod, Name().c_str(), ToString(error));
    return false;
  }
  return true;
}

void CPeripheralAddon::LogException(const std::exception &e, const char *strFunctionName) const
{
  CLog::Log(LOGERROR, "PERIPHERAL - exception '%s' caught while trying to call '%s' on add-on '%s'. Please contact the developer of this add-on: %s",
            e.what(), strFunctionName, Name().c_str(), Author().c_str());
}
