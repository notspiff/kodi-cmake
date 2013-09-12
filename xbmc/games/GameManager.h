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
#pragma once

#include "GameClient.h"
#include "GameFileAutoLauncher.h"
#include "FileItem.h"
#include "threads/CriticalSection.h"
#include "threads/Thread.h"
#include "utils/Observer.h"

#include <map>
#include <set>
#include <string>

namespace GAME
{
  /**
   * The main function of CGameManager is resolving file items into CGameClients.
   *
   * A manager is needed for resolving game clients as they are selected by the
   * file extensions they support. This is determined by loading the DLL and
   * querying it directly, so it is desirable to only do this once and cache the
   * information.
   */
  class CGameManager : public Observer
  {
  protected:
    CGameManager() { }

  public:
    static CGameManager& Get();
    virtual ~CGameManager() { Stop(); }

    virtual void Start();
    virtual void Stop();

    /**
     * Create and maintain a cache of game client add-on information. If a file
     * has been placed in the queue via SetAutolaunch(), it will be launched if a
     * compatible emulator is registered.
     */
    bool RegisterAddon(const GameClientPtr& client);
    void UnregisterAddonByID(const std::string& strClientId);

    virtual bool GetClient(const std::string& strClientId, GameClientPtr& addon) const;

    /**
     * Resolve a file item to a list of game client IDs.
     *
     *   # If the file forces a particular game client via file.SetProperty("gameclient", id),
     *     the result will contain no more than one possible candidate.
     *   # If the file's game info tag provides a "platform", the available game
     *     clients will be filtered by this platform (given the <platform> tag
     *     in their addon.xml).
     *   # If file is a zip file, the contents of that zip will be used to find
     *     suitable candidates (which may yield multiple if there are several
     *     different kinds of ROMs inside).
     */
    void GetGameClientIDs(const CFileItem& file, std::vector<std::string>& candidates) const;

    /**
     * Get a list of valid game client extensions (as determined by the tag in
     * addon.xml). Includes game clients in remote repositories.
     */
    void GetExtensions(std::vector<std::string>& exts) const;
    const std::set<std::string>& GetExtensions() const;

    /**
     * Returns true if the file extension is supported by an add-on in an enabled
     * repository.
     */
    bool IsGame(const std::string& path) const;

    // Queue a file to be launched when the next game client is installed.
    void SetAutoLaunch(const CFileItem& file) { m_fileLauncher.SetAutoLaunch(file); }
    void ClearAutoLaunch()                    { m_fileLauncher.ClearAutoLaunch(); }

    // Inherited from Observer
    virtual void Notify(const Observable& obs, const ObservableMessage msg);

    // Get all known add-ons, both local and remote
    static void GetAllGameClients(ADDON::VECADDONS& addons);

  private:
    // Initialize m_gameClients with enabled game clients
    virtual bool UpdateAddons();
    void UpdateExtensions();

    typedef std::map<std::string, GameClientPtr> GameClientMap;

    GameClientMap         m_gameClients;
    std::set<std::string> m_gameExtensions;
    CGameFileAutoLauncher m_fileLauncher;
    CCriticalSection      m_critSection;

    struct AddonSortByIDFunctor
    {
      bool operator() (ADDON::AddonPtr i, ADDON::AddonPtr j) { return i->ID() < j->ID(); }
    };

  };
} // namespace GAME
