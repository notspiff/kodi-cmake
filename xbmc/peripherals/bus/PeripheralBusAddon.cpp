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

#include "PeripheralBusAddon.h"
#include "addons/AddonManager.h"
#include "peripherals/Peripherals.h"
#include "threads/SingleLock.h"
#include "utils/log.h"

#include <algorithm>

using namespace ADDON;
using namespace PERIPHERALS;

CPeripheralBusAddon::CPeripheralBusAddon(CPeripherals *manager) :
    CPeripheralBus("PeripBusAddon", manager, PERIPHERAL_BUS_ADDON)
{
}

CPeripheralBusAddon::~CPeripheralBusAddon(void)
{
  CSingleLock lock(m_critSection);

  for (PeripheralAddonVector::const_iterator itAddon = m_addons.begin(); itAddon != m_addons.end(); ++itAddon)
    (*itAddon)->Destroy();
}

bool CPeripheralBusAddon::GetAddon(const std::string &strId, AddonPtr &addon) const
{
  CSingleLock lock(m_critSection);
  for (PeripheralAddonVector::const_iterator it = m_addons.begin(); it != m_addons.end(); ++it)
  {
    if ((*it)->ID() == strId)
    {
      addon = *it;
      return true;
    }
  }
  return false;
}

bool CPeripheralBusAddon::PerformDeviceScan(PeripheralScanResults &results)
{
  VECADDONS addons;
  CAddonMgr::Get().GetAddons(ADDON_PERIPHERALDLL, addons, true);

  {
    CSingleLock lock(m_critSection);

    PeripheralAddonVector createdAddons;
    createdAddons.swap(m_addons);

    for (VECADDONS::const_iterator it = addons.begin(); it != addons.end(); ++it)
    {
      PeripheralAddonPtr addon = boost::dynamic_pointer_cast<CPeripheralAddon>(*it);
      if (!addon)
        continue;

      // If add-on hasn't been created, try to create it now
      if (std::find(createdAddons.begin(), createdAddons.end(), addon) == createdAddons.end())
      {
        if (addon->Create() != ADDON_STATUS_OK)
        {
          addon->MarkAsDisabled();
          continue;
        }
      }

      m_addons.push_back(addon);
    }

    for (PeripheralAddonVector::const_iterator itAddon = m_addons.begin(); itAddon != m_addons.end(); ++itAddon)
      (*itAddon)->PerformDeviceScan(results);
  }

  // Scan during bus initialization must return true or bus gets deleted
  return true;
}

void CPeripheralBusAddon::ProcessEvents(void)
{
  CSingleLock lock(m_critSection);

  for (PeripheralAddonVector::const_iterator itAddon = m_addons.begin(); itAddon != m_addons.end(); ++itAddon)
    (*itAddon)->ProcessEvents();
}

void CPeripheralBusAddon::UnregisterRemovedDevices(const PeripheralScanResults &results)
{
  CSingleLock lock(m_critSection);

  std::vector<CPeripheral*> removedPeripherals;

  for (PeripheralAddonVector::const_iterator itAddon = m_addons.begin(); itAddon != m_addons.end(); ++itAddon)
    (*itAddon)->UnregisterRemovedDevices(results, removedPeripherals);

  for (unsigned int iDevicePtr = 0; iDevicePtr < removedPeripherals.size(); iDevicePtr++)
  {
    CPeripheral *peripheral = removedPeripherals.at(iDevicePtr);
    m_manager->OnDeviceDeleted(*this, *peripheral);
    delete peripheral;
  }
}

void CPeripheralBusAddon::Register(CPeripheral* peripheral)
{
  if (!peripheral)
    return;

  PeripheralAddonPtr addon;
  unsigned int       peripheralIndex;

  CSingleLock lock(m_critSection);

  if (SplitLocation(peripheral->Location(), addon, peripheralIndex))
  {
    if (addon->Register(peripheralIndex, peripheral))
      m_manager->OnDeviceAdded(*this, *peripheral);
  }
}

void CPeripheralBusAddon::GetFeatures(std::vector<PeripheralFeature> &features) const
{
  CSingleLock lock(m_critSection);
  for (PeripheralAddonVector::const_iterator itAddon = m_addons.begin(); itAddon != m_addons.end(); ++itAddon)
    (*itAddon)->GetFeatures(features);
}

bool CPeripheralBusAddon::HasFeature(const PeripheralFeature feature) const
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);
  for (PeripheralAddonVector::const_iterator itAddon = m_addons.begin(); itAddon != m_addons.end(); ++itAddon)
    bReturn = bReturn || (*itAddon)->HasFeature(feature);
  return bReturn;
}

CPeripheral *CPeripheralBusAddon::GetPeripheral(const CStdString &strLocation) const
{
  CPeripheral*       peripheral(NULL);
  PeripheralAddonPtr addon;
  unsigned int       peripheralIndex;

  CSingleLock lock(m_critSection);

  if (SplitLocation(strLocation, addon, peripheralIndex))
    peripheral = addon->GetPeripheral(peripheralIndex);

  return peripheral;
}

CPeripheral *CPeripheralBusAddon::GetByPath(const CStdString &strPath) const
{
  CSingleLock lock(m_critSection);

  for (PeripheralAddonVector::const_iterator itAddon = m_addons.begin(); itAddon != m_addons.end(); ++itAddon)
  {
    CPeripheral* peripheral = (*itAddon)->GetByPath(strPath);
    if (peripheral)
      return peripheral;
  }

  return NULL;
}

int CPeripheralBusAddon::GetPeripheralsWithFeature(std::vector<CPeripheral *> &results, const PeripheralFeature feature) const
{
  int iReturn(0);
  CSingleLock lock(m_critSection);
  for (PeripheralAddonVector::const_iterator itAddon = m_addons.begin(); itAddon != m_addons.end(); ++itAddon)
    iReturn += (*itAddon)->GetPeripheralsWithFeature(results, feature);
  return iReturn;
}

size_t CPeripheralBusAddon::GetNumberOfPeripherals(void) const
{
  size_t iReturn(0);
  CSingleLock lock(m_critSection);
  for (PeripheralAddonVector::const_iterator itAddon = m_addons.begin(); itAddon != m_addons.end(); ++itAddon)
    iReturn += (*itAddon)->GetNumberOfPeripherals();
  return iReturn;
}

size_t CPeripheralBusAddon::GetNumberOfPeripheralsWithId(const int iVendorId, const int iProductId) const
{
  size_t iReturn(0);
  CSingleLock lock(m_critSection);
  for (PeripheralAddonVector::const_iterator itAddon = m_addons.begin(); itAddon != m_addons.end(); ++itAddon)
    iReturn += (*itAddon)->GetNumberOfPeripheralsWithId(iVendorId, iProductId);
  return iReturn;
}

void CPeripheralBusAddon::GetDirectory(const CStdString &strPath, CFileItemList &items) const
{
  CStdString strDevPath;
  CSingleLock lock(m_critSection);
  for (PeripheralAddonVector::const_iterator itAddon = m_addons.begin(); itAddon != m_addons.end(); ++itAddon)
    (*itAddon)->GetDirectory(strPath, items);
}

bool CPeripheralBusAddon::SplitLocation(const std::string& strLocation, PeripheralAddonPtr& addon, unsigned int& peripheralIndex) const
{
  std::vector<std::string> parts = StringUtils::Split(strLocation, "/");
  if (parts.size() == 2)
  {
    addon.reset();

    CSingleLock lock(m_critSection);

    const std::string& strAddonId = parts[0];
    for (PeripheralAddonVector::const_iterator itAddon = m_addons.begin(); itAddon != m_addons.end(); ++itAddon)
    {
      if ((*itAddon)->ID() == strAddonId)
      {
        addon = *itAddon;
        break;
      }
    }

    if (addon)
    {
      const char* strJoystickIndex = parts[1].c_str();
      char* p = NULL;
      peripheralIndex = strtol(strJoystickIndex, &p, 10);
      if (strJoystickIndex != p)
        return true;
    }
  }
  return false;
}
