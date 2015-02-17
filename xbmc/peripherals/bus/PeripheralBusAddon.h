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

#include "guilib/IWindowManagerCallback.h"
#include "peripherals/addons/PeripheralAddon.h"
#include "peripherals/bus/PeripheralBus.h"
#include "threads/CriticalSection.h"

namespace PERIPHERALS
{
  class CPeripheralBusAddon : public CPeripheralBus
  {
  public:
    CPeripheralBusAddon(CPeripherals *manager);
    virtual ~CPeripheralBusAddon(void);

    bool GetAddon(const std::string &strId, ADDON::AddonPtr &addon) const;

    void ProcessEvents(void);

    // Inherited from CPeripheralBus
    virtual void         Register(CPeripheral *peripheral);
    virtual void         GetFeatures(std::vector<PeripheralFeature> &features) const;
    virtual bool         HasFeature(const PeripheralFeature feature) const;
    virtual CPeripheral* GetPeripheral(const std::string &strLocation) const;
    virtual CPeripheral* GetByPath(const std::string &strPath) const;
    virtual int          GetPeripheralsWithFeature(std::vector<CPeripheral *> &results, const PeripheralFeature feature) const;
    virtual size_t       GetNumberOfPeripherals(void) const;
    virtual size_t       GetNumberOfPeripheralsWithId(const int iVendorId, const int iProductId) const;
    virtual void         GetDirectory(const std::string &strPath, CFileItemList &items) const;

    bool SplitLocation(const std::string& strLocation, PeripheralAddonPtr& addon, unsigned int& peripheralIndex) const;

  protected:
    // Inherited from CPeripheralBus
    virtual bool PerformDeviceScan(PeripheralScanResults &results);
    virtual void UnregisterRemovedDevices(const PeripheralScanResults &results);

  private:
    PeripheralAddonVector m_addons;
    CCriticalSection      m_critSection;
  };
}
