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

#include "GameClient.h"
#include "GameManager.h"
#include "addons/AddonManager.h"
#include "cores/IPlayer.h"
#include "FileItem.h"
#include "filesystem/Directory.h"
#include "filesystem/SpecialProtocol.h"
#include "settings/Settings.h"
#include "threads/SingleLock.h"
#include "URL.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "utils/Variant.h"

using namespace ADDON;
using namespace GAME;
using namespace XFILE;
using namespace std;

#define EXTENSION_SEPARATOR          "|"
#define GAME_REGION_NTSC_STRING      "NTSC"
#define GAME_REGION_PAL_STRING       "PAL"

#ifdef TARGET_WINDOWS
  #pragma warning (push)
  #pragma warning (disable : 4355) // "this" : used in base member initializer list
#endif

CGameClient::CGameClient(const AddonProps& props)
  : CAddonDll<DllGameClient, GameClient, game_client_properties>(props),
    m_apiVersion("0.0.0"),
    m_libraryProps(this),
    m_strGameClientPath(CAddon::LibPath())
{
  InitializeProperties();

  InfoMap::const_iterator it;
  /*
  if ((it = props.extrainfo.find("platforms")) != props.extrainfo.end())
    SetPlatforms(it->second);
  */
  if ((it = props.extrainfo.find("extensions")) != props.extrainfo.end())
    SetExtensions(it->second, m_extensions);
  if ((it = props.extrainfo.find("supports_vfs")) != props.extrainfo.end())
    m_bSupportsVFS = (it->second == "true" || it->second == "yes");
  if ((it = props.extrainfo.find("supports_no_game")) != props.extrainfo.end())
    m_bSupportsNoGame = (it->second == "true" || it->second == "yes");
}

CGameClient::CGameClient(const cp_extension_t* ext)
  : CAddonDll<DllGameClient, GameClient, game_client_properties>(ext),
    m_apiVersion("0.0.0"),
    m_libraryProps(this),
    m_strGameClientPath(CAddon::LibPath())
{
  InitializeProperties();

  if (ext)
  {
    /*
    string strPlatforms = CAddonMgr::Get().GetExtValue(ext->configuration, "platforms");
    if (!strPlatforms.empty())
    {
      Props().extrainfo.insert(make_pair("platforms", strPlatforms));
      SetPlatforms(strPlatforms);
    }
    */
    string strExtensions = CAddonMgr::Get().GetExtValue(ext->configuration, "extensions");
    if (!strExtensions.empty())
    {
      Props().extrainfo.insert(make_pair("extensions", strExtensions));
      SetExtensions(strExtensions, m_extensions);
    }
    string strSupportsVFS = CAddonMgr::Get().GetExtValue(ext->configuration, "supports_vfs");
    if (!strSupportsVFS.empty())
    {
      Props().extrainfo.insert(make_pair("supports_vfs", strSupportsVFS));
      m_bSupportsVFS = (strSupportsVFS == "true" || strSupportsVFS == "yes");
    }
    string strSupportsNoGame = CAddonMgr::Get().GetExtValue(ext->configuration, "supports_no_game");
    if (!strSupportsNoGame.empty())
    {
      Props().extrainfo.insert(make_pair("supports_no_game", strSupportsNoGame));
      m_bSupportsNoGame = (strSupportsNoGame == "true" || strSupportsNoGame == "yes");
    }
  }
}

void CGameClient::InitializeProperties(void)
{
  m_bReadyToUse = false;
  m_bIsPlaying = false;
  m_player = NULL;
  m_region = GAME_REGION_NTSC;
  m_frameRate = 0.0;
  m_frameRateCorrection = 1.0;
  m_sampleRate = 0.0;
  m_serializeSize = 0;
  m_bRewindEnabled = false;
  m_pInfo = m_libraryProps.CreateProps();
}

CGameClient::~CGameClient(void)
{
  Destroy();
  SAFE_DELETE(m_pInfo);
}

AddonPtr CGameClient::GetRunningInstance() const
{
  GameClientPtr gameAddon;
  if (CGameManager::Get().GetClient(ID(), gameAddon))
    return std::dynamic_pointer_cast<CAddon>(gameAddon);

  return CAddon::GetRunningInstance();
}

ADDON_STATUS CGameClient::Create(void)
{
  ADDON_STATUS status = ADDON_STATUS_UNKNOWN;

  // Ensure that a previous instance is destroyed
  Destroy();

  // Initialise the add-on
  bool bReadyToUse = false;
  CLog::Log(LOGDEBUG, "GAME - %s - creating game add-on instance '%s'", __FUNCTION__, Name().c_str());
  try
  {
    status = CAddonDll<DllGameClient, GameClient, game_client_properties>::Create();
    if (status == ADDON_STATUS_OK)
    {
      bReadyToUse = true;
      LogAddonProperties();
    }
  }
  catch (...) { LogException(__FUNCTION__); }

  m_bReadyToUse = bReadyToUse;

  return status;
}

void CGameClient::Destroy(void)
{
  if (m_bIsPlaying && m_player)
    m_player->CloseFile();

  // Reset 'ready to use' to false
  if (!m_bReadyToUse)
    return;
  m_bReadyToUse = false;

  CLog::Log(LOGDEBUG, "GAME: %s - destroying game add-on %s", __FUNCTION__, ID().c_str());

  // Destroy the add-on
  try { CAddonDll<DllGameClient, GameClient, game_client_properties>::Destroy(); }
  catch (...) { LogException(__FUNCTION__); }
}

void CGameClient::OnEnabled()
{
  CGameManager::Get().RegisterAddon(std::dynamic_pointer_cast<CGameClient>(GetRunningInstance()));
}

void CGameClient::OnDisabled()
{
  CGameManager::Get().UnregisterAddonByID(ID());
}

void CGameClient::LogAddonProperties(void)
{
  vector<string> vecExtensions(m_extensions.begin(), m_extensions.end());

  CLog::Log(LOGINFO, "GAME: ------------------------------------");
  CLog::Log(LOGINFO, "GAME: Loaded DLL for %s", ID().c_str());
  CLog::Log(LOGINFO, "GAME: Client: %s at version %s", Name().c_str(), Version().asString().c_str());
  CLog::Log(LOGINFO, "GAME: Valid extensions: %s", StringUtils::Join(vecExtensions, " ").c_str());
  CLog::Log(LOGINFO, "GAME: Supports VFS: %s", m_bSupportsVFS ? "yes" : "no");
  CLog::Log(LOGINFO, "GAME: Supports no game: %s", m_bSupportsNoGame ? "yes" : "no");
  CLog::Log(LOGINFO, "GAME: ------------------------------------");
}

const std::string CGameClient::LibPath() const
{
  // Use helper library add-on to load libretro cores
  const ADDONDEPS& dependencies = GetDeps();
  ADDONDEPS::const_iterator it = dependencies.find(LIBRETRO_WRAPPER_LIBRARY);
  if (it != dependencies.end())
  {
    AddonPtr addon;
    if (CAddonMgr::Get().GetAddon(LIBRETRO_WRAPPER_LIBRARY, addon, ADDON_SHARED_LIBRARY) && addon)
      return addon->LibPath();
  }

  return CAddon::LibPath();
}

bool HasLocalParentZip(const string& path, string& strParentZip)
{
  // Can't use parent zip if path isn't a child file of a zip folder
  if (!URIUtils::IsInZIP(path))
    return false;

  // Make sure we're in the root folder of the zip (no parent folder)
  CURL parentURL(URIUtils::GetParentPath(path));
  if (!parentURL.GetFileName().empty())
    return false;

  // Make sure the container zip is on the local hard disk
  string parentZip = parentURL.GetHostName();
  if (!CURL(parentZip).GetProtocol().empty())
    return false;

  strParentZip = parentZip;
  return true;
}

bool CGameClient::CanOpen(const CFileItem& file) const
{
  // Game clients not supporting files can't open files
  if (m_bSupportsNoGame)
    return false;

  // Filter by gameclient property
  if (file.HasProperty("gameclient") && file.GetProperty("gameclient").asString() != ID())
    return false;

  CURL translatedUrl(CSpecialProtocol::TranslatePath(file.GetPath()));
  if (translatedUrl.GetProtocol() == "file")
    translatedUrl.SetProtocol("");

  // Filter by extension
  string strExtension = URIUtils::GetExtension(file.GetPath());
  StringUtils::ToLower(strExtension);
  if (!IsExtensionValid(strExtension))
  {
    // If the extension is .zip, try looking inside for files with valid extensions
    if (strExtension == ".zip" && SupportsVFS())
    {
      // Enumerate the zip and look for a file inside the zip with a valid extension
      std::string strZipUrl;

      const CURL pathToUrl(translatedUrl.Get());
      CURL url(URIUtils::CreateArchivePath("zip", pathToUrl, "", ""));
      strZipUrl = url.Get();

      string strValidExts;
      for (set<string>::const_iterator it = m_extensions.begin(); it != m_extensions.end(); it++)
        strValidExts += *it + "|";

      CFileItemList itemList;
      if (CDirectory::GetDirectory(strZipUrl, itemList, strValidExts, DIR_FLAG_READ_CACHE | DIR_FLAG_NO_FILE_INFO) && itemList.Size())
      {
        // Return true if any files matches the directory's extension filter
        return itemList.Size() > 0;
      }
    }
    return false;
  }

  // If the client supports VFS, it can load all URIs
  if (SupportsVFS())
    return true;

  // If file is on the VFS, check if it is in the top-level directory of a local zip
  if (!translatedUrl.GetProtocol().empty())
  {
    string strParentZip;
    if (IsExtensionValid(".zip") && HasLocalParentZip(translatedUrl.Get(), strParentZip))
      return true;
    return false;
  }

  return true;
}

bool CGameClient::OpenFile(const CFileItem& file, IPlayer* player)
{
  CSingleLock lock(m_critSection);

  if (!ReadyToUse() || !CanOpen(file))
    return false;

  CloseFile();

  if (OpenInternal(file))
  {
    m_player = player;
    InitSerialization();

    // TODO: Need an API call in libretro that lets us know the number of ports
    SetDevice(0, GAME_DEVICE_JOYPAD);

    return true;
  }
  return false;
}

bool CGameClient::OpenInternal(const CFileItem& file)
{
  // Try to resolve path to a local file, as not all game clients support VFS
  CURL translatedUrl(CSpecialProtocol::TranslatePath(file.GetPath()));
  if (translatedUrl.GetProtocol() == "file")
    translatedUrl.SetProtocol("");

  string strTranslatedUrl = translatedUrl.Get();

  // If the game client doesn't support VFS, we'll need a backup plan
  if (!SupportsVFS() && !translatedUrl.GetProtocol().empty())
  {
    // Maybe the file is in a local zip, and the game client supports zips
    string strParentZip;
    if (IsExtensionValid(".zip") && HasLocalParentZip(translatedUrl.Get(), strParentZip))
      strTranslatedUrl = strParentZip;
  }

  // If the game client doesn't support zips, we can try to load the file via the zip:// VFS protocol
  if (SupportsVFS() && StringUtils::EqualsNoCase(URIUtils::GetExtension(strTranslatedUrl), ".zip") && !IsExtensionValid(".zip"))
  {
    // Enumerate the zip and look for a file inside the zip with a valid extension
    std::string strZipUrl;

    const CURL pathToUrl(translatedUrl.Get());
    CURL url(URIUtils::CreateArchivePath("zip", pathToUrl, "", ""));
    strZipUrl = url.Get();

    string strValidExts;
    for (set<string>::const_iterator it = m_extensions.begin(); it != m_extensions.end(); it++)
      strValidExts += *it + "|";

    CFileItemList itemList;
    if (CDirectory::GetDirectory(strZipUrl, itemList, strValidExts, DIR_FLAG_READ_CACHE | DIR_FLAG_NO_FILE_INFO) && itemList.Size())
    {
      // Use the first file discovered
      strTranslatedUrl = itemList[0]->GetPath();
    }
  }

  GAME_ERROR error = GAME_ERROR_FAILED;
  try { LogError(error = m_pStruct->LoadGame(strTranslatedUrl.c_str()), "LoadGame()"); }
  catch (...) { LogException("LoadGame()"); }

  if (error != GAME_ERROR_NO_ERROR)
    return false;

  if (LoadGameInfo())
  {
    m_filePath = strTranslatedUrl;
    m_bIsPlaying = true;
    return true;
  }
  return false;
}

bool CGameClient::LoadGameInfo()
{
  // Get information about system audio/video timings and geometry
  // Can be called only after retro_load_game()
  game_system_av_info av_info = { };

  GAME_ERROR error = GAME_ERROR_FAILED;
  try { LogError(error = m_pStruct->GetSystemAVInfo(&av_info), "GetSystemAVInfo()"); }
  catch (...) { LogException("GetSystemAVInfo()"); }

  if (error != GAME_ERROR_NO_ERROR)
    return false;

  GAME_REGION region;
  try { region = m_pStruct->GetRegion(); }
  catch (...) { LogException("GetRegion()"); return false; }

  CLog::Log(LOGINFO, "GAME: ---------------------------------------");
  CLog::Log(LOGINFO, "GAME: Opened file %s",   m_filePath.c_str());
  CLog::Log(LOGINFO, "GAME: Base Width:   %u", av_info.geometry.base_width);
  CLog::Log(LOGINFO, "GAME: Base Height:  %u", av_info.geometry.base_height);
  CLog::Log(LOGINFO, "GAME: Max Width:    %u", av_info.geometry.max_width);
  CLog::Log(LOGINFO, "GAME: Max Height:   %u", av_info.geometry.max_height);
  CLog::Log(LOGINFO, "GAME: Aspect Ratio: %f", av_info.geometry.aspect_ratio);
  CLog::Log(LOGINFO, "GAME: FPS:          %f", av_info.timing.fps);
  CLog::Log(LOGINFO, "GAME: Sample Rate:  %f", av_info.timing.sample_rate);
  CLog::Log(LOGINFO, "GAME: Region:       %s", region == GAME_REGION_NTSC ? GAME_REGION_NTSC_STRING : GAME_REGION_PAL_STRING);
  CLog::Log(LOGINFO, "GAME: ---------------------------------------");

  m_frameRate  = av_info.timing.fps;
  m_sampleRate = av_info.timing.sample_rate;
  m_region     = region;

  return true;
}

bool CGameClient::InitSerialization()
{
  // Check if serialization is supported so savestates and rewind can be used
  unsigned int serializeSize;
  try { serializeSize = m_pStruct->SerializeSize(); }
  catch (...) { LogException("SerializeSize()"); return false; }

  if (serializeSize == 0)
  {
    CLog::Log(LOGINFO, "GAME: Serialization not supported, continuing without save or rewind");
    return false;
  }

  m_serializeSize = serializeSize;
  m_bRewindEnabled = CSettings::Get().GetBool("gamesgeneral.enablerewind");

  // Set up rewind functionality
  if (m_bRewindEnabled)
  {
    m_serialState.Init(m_serializeSize, (size_t)(CSettings::Get().GetInt("gamesgeneral.rewindtime") * GetFrameRate()));

    GAME_ERROR error = GAME_ERROR_FAILED;
    try { LogError(error = m_pStruct->Serialize(m_serialState.GetState(), m_serialState.GetFrameSize()), "Serialize()"); }
    catch (...) { LogException("Serialize()"); }

    if (error != GAME_ERROR_NO_ERROR)
    {
      m_serializeSize = 0;
      m_bRewindEnabled = false;
      m_serialState.Reset();
      CLog::Log(LOGERROR, "GAME: Unable to serialize state, proceeding without save or rewind");
      return false;
    }
  }

  return true;
}

void CGameClient::SetDevice(unsigned int port, unsigned int device)
{
  if (m_bIsPlaying)
  {
    // Validate port (TODO: Check if port is less that players that individual game client supports)
    if (port < GAMECLIENT_MAX_PLAYERS)
    {
      // Validate device
      if (device <= GAME_DEVICE_ANALOG ||
          device == GAME_DEVICE_JOYPAD_MULTITAP ||
          device == GAME_DEVICE_LIGHTGUN_SUPER_SCOPE ||
          device == GAME_DEVICE_LIGHTGUN_JUSTIFIER ||
          device == GAME_DEVICE_LIGHTGUN_JUSTIFIERS)
      {
        try { LogError(m_pStruct->SetControllerPortDevice(port, device), "SetControllerPortDevice()"); }
        catch (...) { LogException("SetControllerPortDevice()"); }
      }
    }
  }
}

void CGameClient::CloseFile()
{
  CSingleLock lock(m_critSection);

  if (m_bReadyToUse && m_bIsPlaying)
  {
    try { LogError(m_pStruct->UnloadGame(), "UnloadGame()"); }
    catch (...) { LogException("UnloadGame()"); }
  }

  m_bIsPlaying = false;
  m_filePath.clear();
  m_player = NULL;
}

bool CGameClient::RunFrame()
{
  CSingleLock lock(m_critSection);

  if (!m_bIsPlaying)
    return false;

  GAME_ERROR error = GAME_ERROR_FAILED;
  try { LogError(error = m_pStruct->Run(), "Run()"); }
  catch (...) { LogException("Run()"); }

  if (error != GAME_ERROR_NO_ERROR)
    return false;

  // Append a new state delta to the rewind buffer
  if (m_bRewindEnabled)
  {
    error = GAME_ERROR_FAILED;
    try { LogError(error = m_pStruct->Serialize(m_serialState.GetNextState(), m_serialState.GetFrameSize()), "Serialize()"); }
    catch (...) { LogException("Serialize()"); }

    if (error != GAME_ERROR_NO_ERROR)
    {
      m_bRewindEnabled = false;
      return false;
    }

    m_serialState.AdvanceFrame();
  }

  return true;
}

unsigned int CGameClient::RewindFrames(unsigned int frames)
{
  CSingleLock lock(m_critSection);

  unsigned int rewound = 0;
  if (m_bIsPlaying && m_bRewindEnabled)
  {
    rewound = m_serialState.RewindFrames(frames);
    if (rewound != 0)
    {
      try { LogError(m_pStruct->Deserialize(m_serialState.GetState(), m_serialState.GetFrameSize()), "Deserialize()"); }
      catch (...) { LogException("Deserialize()"); }
    }
  }
  return rewound;
}

void CGameClient::Reset()
{
  if (m_bIsPlaying)
  {
    // TODO: Reset all controller ports to their same value. bSNES since v073r01
    // resets controllers to JOYPAD after a reset, so guard against this.
    try { LogError(m_pStruct->Reset(), "Reset()"); }
    catch (...) { LogException("Reset()"); }

    if (m_bRewindEnabled)
    {
      m_serialState.ReInit();

      GAME_ERROR error = GAME_ERROR_FAILED;
      try { LogError(error = m_pStruct->Serialize(m_serialState.GetNextState(), m_serialState.GetFrameSize()), "Serialize()"); }
      catch (...) { LogException("Serialize()"); }

      if (error != GAME_ERROR_NO_ERROR)
        m_bRewindEnabled = false;
    }
  }
}

void CGameClient::SetFrameRateCorrection(double correctionFactor)
{
  if (correctionFactor != 0.0)
    m_frameRateCorrection = correctionFactor;
  if (m_bRewindEnabled)
    m_serialState.SetMaxFrames((size_t)(CSettings::Get().GetInt("gamesgeneral.rewindtime") * GetFrameRate()));
}

void CGameClient::SetExtensions(const string &strExtensionList, std::set<std::string>& extensions)
{
  extensions.clear();

  vector<string> vecExtensions = StringUtils::Split(strExtensionList, EXTENSION_SEPARATOR);
  for (vector<string>::iterator it = vecExtensions.begin(); it != vecExtensions.end(); it++)
  {
    string& ext = *it;
    if (ext.empty())
      continue;

    StringUtils::ToLower(ext);

    // Make sure extension starts with "."
    if (ext[0] != '.')
      ext.insert(0, ".");

    extensions.insert(ext);
  }
}

/*
void CGameClient::SetPlatforms(const string& strPlatformList)
{
  m_platforms.clear();

  vector<string> platforms = StringUtils::Split(strPlatformList, EXTENSION_SEPARATOR);
  for (vector<string>::iterator it = platforms.begin(); it != platforms.end(); it++)
  {
    StringUtils::Trim(*it);
    GamePlatform id = CGameInfoTagLoader::GetPlatformInfoByName(*it).id;
    if (id != PLATFORM_UNKNOWN)
      m_platforms.insert(id);
  }
}
*/

bool CGameClient::IsExtensionValid(const string& strExtension) const
{
  if (m_extensions.empty())
    return true; // Be optimistic :)
  if (strExtension.empty())
    return false;

  // Convert to lower case and canonicalize with a leading "."
  string strExtension2(strExtension);
  StringUtils::ToLower(strExtension2);
  if (strExtension2[0] != '.')
    strExtension2.insert(0, ".");
  return m_extensions.find(strExtension2) != m_extensions.end();
}

bool CGameClient::LogError(GAME_ERROR error, const char* strMethod) const
{
  if (error != GAME_ERROR_NO_ERROR)
  {
    CLog::Log(LOGERROR, "GAME - %s - addon '%s' returned an error: %s",
        strMethod, ID().c_str(), ToString(error));
    return false;
  }
  return true;
}

void CGameClient::LogException(const char* strFunctionName) const
{
  CLog::Log(LOGERROR, "GAME: exception caught while trying to call '%s' on add-on %s",
      strFunctionName, ID().c_str());
  CLog::Log(LOGERROR, "Please contact the developer of this add-on: %s", Author().c_str());
}

const char* CGameClient::ToString(GAME_ERROR error)
{
  switch (error)
  {
  case GAME_ERROR_NO_ERROR:
    return "no error";
  case GAME_ERROR_NOT_IMPLEMENTED:
    return "not implemented";
  case GAME_ERROR_REJECTED:
    return "rejected by the client";
  case GAME_ERROR_INVALID_PARAMETERS:
    return "invalid parameters for this method";
  case GAME_ERROR_FAILED:
    return "the command failed";
  case GAME_ERROR_UNKNOWN:
  default:
    return "unknown error";
  }
}

#ifdef TARGET_WINDOWS
  #pragma warning (pop)
#endif
