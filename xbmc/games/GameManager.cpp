/*
 *      Copyright (C) 2012-2014 Team XBMC
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

#include "GameManager.h"
#include "addons/AddonDatabase.h"
#include "addons/AddonInstaller.h"
#include "Application.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "filesystem/Directory.h"
#include "profiles/ProfilesManager.h"
#include "threads/SingleLock.h"
#include "URL.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "utils/Variant.h"

#include <algorithm>

using namespace ADDON;
using namespace GAME;
using namespace XFILE;
using namespace std;


/* TEMPORARY */
// Remove this struct when libretro has an API call to query the number of
// controller ports a game supports. If this code is still here in six months,
// Garrett will be very unhappy. I found a buffer overflow in SNES9x when
// trying to set controller ports 3-8, so this API call needs to happen.
/*
struct PortMapping
{
  GamePlatform platform;
  int          ports;
};

static const PortMapping ports[] =
{
  { PLATFORM_GAMEBOY,              1 },
  { PLATFORM_GAMEBOY_COLOR,        1 },
  { PLATFORM_GAMEBOY_ADVANCE,      1 },
  { PLATFORM_NEO_GEO_POCKET_COLOR, 1 },
  { PLATFORM_SEGA_MASTER_SYSTEM,   2 },
  { PLATFORM_SNES,                 2 },
};
*/


/* static */
CGameManager& CGameManager::Get()
{
  static CGameManager gameManagerInstance;
  return gameManagerInstance;
}

void CGameManager::Start()
{
  CAddonMgr::Get().RegisterObserver(this);
  CAddonInstaller::Get().RegisterObserver(this);
  // TODO: CAddonMgr::Get().RegisterAddonMgrCallback(ADDON_GAMEDLL, this);

  // Must call UpdateAddons(), as CAddonMgr::Init() is called before CGameManager::Start(),
  // so we won't receive the first ObservableMessageAddons message.
  UpdateAddons();
  UpdateExtensions();
}

void CGameManager::Stop()
{
  CAddonMgr::Get().UnregisterObserver(this);
  // TODO: Why does this crash?
  //CAddonInstaller::Get().UnregisterObserver(this);
}

bool CGameManager::UpdateAddons()
{
  VECADDONS gameClients;
  if (CAddonMgr::Get().GetAddons(ADDON_GAMEDLL, gameClients, true))
  {
    CSingleLock lock(m_critSection);

    for (VECADDONS::const_iterator it = gameClients.begin(); it != gameClients.end(); it++)
    {
      if (!RegisterAddon(boost::dynamic_pointer_cast<CGameClient>(*it)) && (*it)->Enabled())
        CAddonMgr::Get().DisableAddon((*it)->ID());
    }
  }
  return true;
}

void CGameManager::UpdateExtensions()
{
  CAddonDatabase database;
  if (database.Open())
  {
    VECADDONS addons;
    GetAllGameClients(addons);

    CSingleLock lock(m_critSection);

    for (VECADDONS::const_iterator it = addons.begin(); it != addons.end(); ++it)
    {
      const AddonPtr& addon = *it;
      if (!addon->IsType(ADDON_GAMEDLL))
        continue;

      GameClientPtr gc = boost::dynamic_pointer_cast<CGameClient>(addon);
      if (!gc)
        continue;

      const bool bIsBroken = !gc->Props().broken.empty();
      if (!bIsBroken && !gc->GetExtensions().empty())
        m_gameExtensions.insert(gc->GetExtensions().begin(), gc->GetExtensions().end());
    }
    CLog::Log(LOGDEBUG, "GameManager: tracking %d extensions", (int)(m_gameExtensions.size()));
  }
}

bool CGameManager::RegisterAddon(const GameClientPtr& client)
{
  if (!client)
    return false;

  if (!client->Enabled())
    return false;

  CSingleLock lock(m_critSection);

  if (!m_gameClients.empty())
  {
    GameClientMap::const_iterator it = m_gameClients.find(client->ID());
    if (it != m_gameClients.end())
      return true; // Already registered
  }

  m_gameClients[client->ID()] = client;
  CLog::Log(LOGDEBUG, "GameManager: Registered add-on %s", client->ID().c_str());

  // If a file was queued by RetroPlayer, try to launch the newly installed game client
  m_fileLauncher.Launch(client);

  return true;
}

void CGameManager::UnregisterAddonByID(const string& strClientId)
{
  CSingleLock lock(m_critSection);

  GameClientMap::iterator it = m_gameClients.find(strClientId);
  if (it != m_gameClients.end())
    m_gameClients.erase(it);
}

bool CGameManager::GetClient(const string& strClientId, GameClientPtr& addon) const
{
  CSingleLock lock(m_critSection);

  GameClientMap::const_iterator itr = m_gameClients.find(strClientId);
  if (itr != m_gameClients.end())
  {
    addon = itr->second;
    return true;
  }
  return false;
}

void CGameManager::GetGameClientIDs(const CFileItem& file, vector<string>& candidates) const
{
  CSingleLock lock(m_critSection);

  const string strRequestedClient = file.GetProperty("gameclient").asString();

  for (GameClientMap::const_iterator it = m_gameClients.begin(); it != m_gameClients.end(); it++)
  {
    if (!strRequestedClient.empty() && strRequestedClient != it->first)
      continue;

    CLog::Log(LOGDEBUG, "GameManager: To open or not to open using %s, that is the question", it->second->ID().c_str());
    if (it->second->CanOpen(file))
    {
      CLog::Log(LOGDEBUG, "GameManager: Adding client %s as a candidate", it->second->ID().c_str());
      candidates.push_back(it->second->ID());
    }

    // If the requested client isn't installed, there are no valid candidates
    if (!strRequestedClient.empty())
      break;
  }
}

void CGameManager::GetExtensions(vector<string> &exts) const
{
  CSingleLock lock(m_critSection);
  exts.insert(exts.end(), m_gameExtensions.begin(), m_gameExtensions.end());
}

const std::set<std::string>& CGameManager::GetExtensions() const
{
  CSingleLock lock(m_critSection);
  return m_gameExtensions;
}

bool CGameManager::IsGame(const std::string &path) const
{
  // Get the file extension (must use a CURL, if the string is top-level zip
  // directory it might not end in .zip)
  string extension(URIUtils::GetExtension(CURL(path).GetFileNameWithoutPath()));
  StringUtils::ToLower(extension);
  if (extension.empty())
    return false;

  CSingleLock lock(m_critSection);

  return m_gameExtensions.find(extension) != m_gameExtensions.end();
}

void CGameManager::Notify(const Observable& obs, const ObservableMessage msg)
{
  if (msg == ObservableMessageAddons)
    UpdateAddons();
  else if (msg == ObservableMessageRemoteAddons)
    UpdateExtensions();
}

void CGameManager::GetAllGameClients(ADDON::VECADDONS& addons)
{
  VECADDONS tempAddons;

  // Get add-ons in remote repositories
  CAddonDatabase database;
  if (database.Open())
  {
    database.GetAddons(tempAddons, ADDON_GAMEDLL);
    addons.insert(addons.end(), tempAddons.begin(), tempAddons.end());
    tempAddons.clear();
    database.Close();
  }

  // Get enabled add-ons installed locally
  CAddonMgr::Get().GetAddons(ADDON_GAMEDLL, tempAddons, true);
  addons.insert(addons.end(), tempAddons.begin(), tempAddons.end());
  tempAddons.clear();

  // Get disabled add-ons installed locally
  CAddonMgr::Get().GetAddons(ADDON_GAMEDLL, tempAddons, false);
  addons.insert(addons.end(), tempAddons.begin(), tempAddons.end());
  tempAddons.clear();

  if (!addons.empty())
  {
    // Sort by ID and remove duplicates
    AddonSortByIDFunctor AddonSortByID;
    std::sort(addons.begin(), addons.end(), AddonSortByID);
    for (VECADDONS::iterator it = addons.begin(); it + 1 != addons.end(); )
    {
      if ((*it)->ID() == (*(it + 1))->ID())
        addons.erase(it + 1);
      else
        ++it;
    }
  }
}
