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

/*
 * Adding new functions and callbacks to the XBMC Game API
 *
 * The Game API is spread out across various files. Adding new functions and
 * callbacks is an adventure that spans many layers of abstraction. Necessary
 * steps can easily be omitted, so the process of adding functions and
 * callbacks is documented here.
 *
 * The Game API is layed out in three files:
 *   - xbmc_game_dll.h        (API function declarations)
 *   - xbmc_game_callbacks.h  (API callback pointers)
 *   - xbmc_game_types.h      (API enums and structs)
 *
 * To add a new API function:
 *   1.  Declare the function in xbmc_game_dll.h with some helpful documentation
 *   2.  Assign the function pointer in get_addon() of the same file. get_addon()
 *       (aliased to GetAddon()) is called in AddonDll.h immediately after
 *       loading the shared library.
 *   3.  Add the function to the GameClient struct in xbmc_game_types.h. This
 *       struct contains pointers to all the API functions. It is populated in
 *       get_addon(). CGameClient invokes API functions through this struct.
 *   4.  Define the function in the cpp file of the game client project
 *
 * To add a new API callback:
 *   1.  Declare the callback as a function pointer in the CB_GameLib struct of
 *       xbmc_game_callbacks.h with some helpful documentation. The first
 *       parameter, addonData, is the CAddonCallbacksGame object associated with
 *       the game client instance.
 *   2.  Declare the callback as a static member function of CAddonCallbacksGame
 *   3.  Define the function in AddonCallbacksGame.cpp and assign the callback
 *       to the callbacks table in the constructor.
 *   4.  Expose the function to the game client in libXBMC_game.cpp. This shared
 *       library allows for ABI compatibility if the API is unchanged across
 *       releases.
 *   5.  Add the callback to the helper class in libXBMC_game.h. Requires three
 *       modifications: register the symbol exported from the shared library,
 *       expose the callback using a member function wrapper, and declare the
 *       function pointer as a protected member variable.
 */

#include "GameClientProperties.h"
#include "SerialState.h"
//#include "tags/GameInfoTagLoader.h"
#include "addons/Addon.h"
#include "addons/AddonDll.h"
#include "addons/DllGameClient.h"
#include "addons/include/xbmc_addon_types.h"
#include "threads/CriticalSection.h"

#include <memory>
#include <set>
#include <string>
#include <vector>

#define LIBRETRO_WRAPPER_LIBRARY   "library.xbmc.libretro"
#define GAMECLIENT_MAX_PLAYERS     8

class CFileItem;
class IPlayer;

namespace GAME
{

class CGameClient;
typedef std::shared_ptr<CGameClient> GameClientPtr;
typedef std::vector<GameClientPtr>   GameClientVector;

class CGameClient : public ADDON::CAddonDll<DllGameClient, GameClient, game_client_properties>
{
public:
  CGameClient(const ADDON::AddonProps& props);
  CGameClient(const cp_extension_t* props);
  virtual ~CGameClient(void);

  virtual ADDON::AddonPtr GetRunningInstance() const;

  // Initialise the instance of this add-on
  ADDON_STATUS Create();

  // Destroy the instance of this add-on
  void Destroy(void);

  virtual void OnEnabled();
  virtual void OnDisabled();

  // Path to the game client library
  const std::string& GameClientPath() const { return m_strGameClientPath; }

  // Override LibPath() to return a helper library for v1 clients
  // TODO: Don't make this function virtual, find another way
  virtual const std::string LibPath() const;

  // Return true if this instance is initialised, false otherwise
  bool ReadyToUse(void) const { return m_bReadyToUse; }

  // Query properties of the game client
  const std::set<std::string>& GetExtensions() const    { return m_extensions; }
  bool                         SupportsVFS() const      { return m_bSupportsVFS; }
  bool                         SupportsNoGame() const   { return m_bSupportsNoGame; }
  //const GamePlatforms&         GetPlatforms() const     { return m_platforms; }

  // Query properties of the running game
  bool               IsPlaying() const     { return m_bIsPlaying; }
  const std::string& GetFilePath() const   { return m_filePath; }
  IPlayer*           GetPlayer() const     { return m_player; }
  int                GetRegion() const     { return m_region; }
  double             GetFrameRate() const  { return m_frameRate * m_frameRateCorrection; }
  double             GetSampleRate() const { return m_sampleRate; }

  // Modify the value returned by GetFrameRate(), used to sync gameplay to audio
  void SetFrameRateCorrection(double correctionFactor);

  /**
   * Perform the gamut of checks on the file: "gameclient" property, platform,
   * extension, and a positive match on at least one of the CGameFileLoader
   * strategies.
   */
  bool CanOpen(const CFileItem& file) const;

  bool OpenFile(const CFileItem& file, IPlayer* player);

  void CloseFile();

  /**
   * Each port (or player, if you will) must be associated with a device. The
   * default device is GAME_DEVICE_JOYPAD.
   *
   * TODO: Do not exceed the number of devices that the game client supports.
   * A quick analysis of SNES9x Next v2 showed that a third port will overflow
   * a buffer. Currently, there is no way to determine the number of ports a
   * client will support, so stick with 1.
   *
   * Precondition: OpenFile() must return true.
   */
  void SetDevice(unsigned int port, unsigned int device);

  /**
   * Allow the game to run and produce a video frame.
   * Precondition: OpenFile() returned true.
   * Returns false if an exception is thrown in retro_run().
   */
  bool RunFrame();

  /**
   * Rewind gameplay 'frames' frames.
   * As there is a fixed size buffer backing
   * save state deltas, it might not be possible to rewind as many
   * frames as desired. The function returns number of frames actually rewound.
   */
  unsigned int RewindFrames(unsigned int frames);

  // Returns how many frames it is possible to rewind with a call to RewindFrames()
  size_t GetAvailableFrames() const { return m_bRewindEnabled ? m_serialState.GetFramesAvailable() : 0; }

  // Returns the maximum amount of frames that can ever be rewound
  size_t GetMaxFrames() const { return m_bRewindEnabled ? m_serialState.GetMaxFrames() : 0; }

  // Reset the game, if running
  void Reset();

  // If the game client provided no extensions, this will optimistically return true
  bool IsExtensionValid(const std::string& strExtension) const;

private:
  // Called by the constructors
  void InitializeProperties(void);

  void LogAddonProperties(void);

  bool OpenInternal(const CFileItem& file);

  bool LoadGameInfo();

  /**
   * Initialize the game client serialization subsystem. If successful,
   * m_bRewindEnabled and m_serializeSize are set appropriately.
   */
  bool InitSerialization();

  // Parse a pipe-separated list returned from the game client
  static void SetExtensions(const std::string& strExtensionList, std::set<std::string>& extensions);
  //void SetPlatforms(const std::string& strPlatformList);

  bool LogError(GAME_ERROR error, const char* strMethod) const;
  void LogException(const char* strFunctionName) const;
  static const char* ToString(GAME_ERROR error);

  ADDON::AddonVersion   m_apiVersion;
  CGameClientProperties m_libraryProps;        // Properties to pass to the DLL
  const std::string     m_strGameClientPath;   // Path to the game client library
  bool                  m_bReadyToUse;         // True if this add-on is connected to the backend, false otherwise */

  // Game API xml parameters
  std::set<std::string> m_extensions;
  bool                  m_bSupportsVFS;
  bool                  m_bSupportsNoGame;
  //GamePlatforms         m_platforms;

  // Properties of the current playing file
  bool                  m_bIsPlaying;          // This is true between OpenFile() and CloseFile()
  std::string           m_filePath;            // The current playing file
  IPlayer*              m_player;              // The player core that called OpenFile()
  GAME_REGION           m_region;              // Region of the loaded game
  double                m_frameRate;           // Video framerate
  double                m_frameRateCorrection; // Framerate correction factor (to sync to audio)
  double                m_sampleRate;          // Audio frequency

  // Save/rewind functionality
  unsigned int          m_serializeSize;
  bool                  m_bRewindEnabled;
  CSerialState          m_serialState;

  CCriticalSection      m_critSection;
};

} // namespace GAME
