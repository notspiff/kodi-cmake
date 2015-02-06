/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include <string>
#include <vector>
#include "URL.h"

template<class T>
struct URIUtilsGet
{
  std::string Get() const;
};

#define DEFINE_METHOD(x) \
  static bool (x)(const std::string& path); \
  bool (x)() const { return URIUtilsComp<std::string>::x(get.Get()); }

template<class T>
struct URIUtilsComp
{
  URIUtilsComp(const T& t) : get(t) {}

  DEFINE_METHOD(IsAddonsPath)
  DEFINE_METHOD(IsAfp)
  DEFINE_METHOD(IsInArchive)
  DEFINE_METHOD(IsAndroidApp)
  DEFINE_METHOD(IsAPK)
  DEFINE_METHOD(IsArchive)
  DEFINE_METHOD(IsInAPK)
  DEFINE_METHOD(IsInRAR)
  DEFINE_METHOD(IsInZIP)
  DEFINE_METHOD(IsBluray)
  DEFINE_METHOD(IsCDDA)
  DEFINE_METHOD(IsDAAP)
  DEFINE_METHOD(IsDAV)
  DEFINE_METHOD(IsDOSPath)
  DEFINE_METHOD(IsDVD)
  DEFINE_METHOD(IsFTP)
  DEFINE_METHOD(IsTCP)
  DEFINE_METHOD(IsUDP)
  DEFINE_METHOD(IsHD)
  DEFINE_METHOD(IsHDHomeRun)
  DEFINE_METHOD(IsHTSP)
  DEFINE_METHOD(IsISO9660)
  DEFINE_METHOD(IsLibraryFolder)
  DEFINE_METHOD(IsLibraryContent)
  DEFINE_METHOD(IsLiveTV)
  DEFINE_METHOD(IsMultiPath)
  DEFINE_METHOD(IsMusicDb)
  DEFINE_METHOD(IsMythTV)
  DEFINE_METHOD(IsNfs)
  DEFINE_METHOD(IsOnDVD)
  DEFINE_METHOD(IsOnLAN)
  DEFINE_METHOD(IsPVRRecording)
  DEFINE_METHOD(IsPlugin)
  DEFINE_METHOD(IsRAR)
  DEFINE_METHOD(IsRemote)
  DEFINE_METHOD(IsPVRChannel)
  DEFINE_METHOD(IsScript)
  DEFINE_METHOD(IsSlingbox)
  DEFINE_METHOD(IsSmb)
  DEFINE_METHOD(IsSpecial)
  DEFINE_METHOD(IsStack)
  DEFINE_METHOD(IsSourcesPath)
  DEFINE_METHOD(IsTuxBox)
  DEFINE_METHOD(IsUPnP)
  DEFINE_METHOD(IsURL)
  DEFINE_METHOD(IsUsingFastSwitch)
  DEFINE_METHOD(IsVideoDb)
  DEFINE_METHOD(IsVTP)
  DEFINE_METHOD(IsZIP)

  URIUtilsGet<T> get;
};

class URIUtils : public URIUtilsComp<std::string>
{
public:
  URIUtils(void);
  virtual ~URIUtils(void);
  static bool IsInPath(const std::string &uri, const std::string &baseURI);

  static std::string GetDirectory(const std::string &strFilePath);

  static const std::string GetFileName(const CURL& url);
  static const std::string GetFileName(const std::string& strFileNameAndPath);

  static std::string GetExtension(const CURL& url);
  static std::string GetExtension(const std::string& strFileName);

  /*!
   \brief Check if there is a file extension
   \param strFileName Path or URL to check
   \return \e true if strFileName have an extension.
   \note Returns false when strFileName is empty.
   \sa GetExtension
   */
  static bool HasExtension(const std::string& strFileName);

  /*!
   \brief Check if filename have any of the listed extensions
   \param strFileName Path or URL to check
   \param strExtensions List of '.' prefixed lowercase extensions seperated with '|'
   \return \e true if strFileName have any one of the extensions.
   \note The check is case insensitive for strFileName, but requires
         strExtensions to be lowercase. Returns false when strFileName or
         strExtensions is empty.
   \sa GetExtension
   */
  static bool HasExtension(const std::string& strFileName, const std::string& strExtensions);
  static bool HasExtension(const CURL& url, const std::string& strExtensions);

  static void RemoveExtension(std::string& strFileName);
  static std::string ReplaceExtension(const std::string& strFile,
                                     const std::string& strNewExtension);
  static void Split(const std::string& strFileNameAndPath, 
                    std::string& strPath, std::string& strFileName);
  static std::vector<std::string> SplitPath(const std::string& strPath);

  static void GetCommonPath(std::string& strPath, const std::string& strPath2);
  static std::string GetParentPath(const std::string& strPath);
  static bool GetParentPath(const std::string& strPath, std::string& strParent);

  /* \brief Change the base path of a URL: fromPath/fromFile -> toPath/toFile
    Handles changes in path separator and filename URL encoding if necessary to derive toFile.
    \param fromPath the base path of the original URL
    \param fromFile the filename portion of the original URL
    \param toPath the base path of the resulting URL
    \return the full path.
   */
  static std::string ChangeBasePath(const std::string &fromPath, const std::string &fromFile, const std::string &toPath, const bool &bAddPath = true);

  static CURL SubstitutePath(const CURL& url, bool reverse = false);
  static std::string SubstitutePath(const std::string& strPath, bool reverse = false);

  /*! \brief Check whether a URL is a given URL scheme.
   Comparison is case-insensitve as per RFC1738
   \param url a std::string path.
   \param type a lower-case scheme name, e.g. "smb".
   \return true if the url is of the given scheme, false otherwise.
   \sa PathStarts, PathEquals
   */
  static bool IsProtocol(const std::string& url, const std::string& type);

  /*! \brief Check whether a path starts with a given start.
   Comparison is case-sensitive.
   Use IsProtocol() to compare the protocol portion only.
   \param path a std::string path.
   \param start the string the start of the path should be compared against.
   \return true if the path starts with the given string, false otherwise.
   \sa IsProtocol, PathEquals
   */
  static bool PathStarts(const std::string& path, const char *start);

  /*! \brief Check whether a path equals another path.
   Comparison is case-sensitive.
   \param path1 a std::string path.
   \param path2 the second path the path should be compared against.
   \param ignoreTrailingSlash ignore any trailing slashes in both paths
   \return true if the paths are equal, false otherwise.
   \sa IsProtocol, PathStarts
   */
  static bool PathEquals(const std::string& path1, const std::string &path2, bool ignoreTrailingSlash = false);

  static bool IsInternetStream(const std::string& path, bool bStrictCheck = false);
  static bool IsInternetStream(const CURL& url, bool bStrictCheck = false);
  static bool IsHostOnLAN(const std::string& hostName, bool offLineCheck = false);

  static void AddSlashAtEnd(std::string& strFolder);
  static bool HasSlashAtEnd(const std::string& strFile, bool checkURL = false);
  static void RemoveSlashAtEnd(std::string& strFolder);
  static bool CompareWithoutSlashAtEnd(const std::string& strPath1, const std::string& strPath2);
  static std::string FixSlashesAndDups(const std::string& path, const char slashCharacter = '/', const size_t startFrom = 0);
  /**
   * Convert path to form without duplicated slashes and without relative directories
   * Strip duplicated slashes
   * Resolve and remove relative directories ("/../" and "/./")
   * Will ignore slashes with other direction than specified
   * Will not resolve path starting from relative directory
   * @warning Don't use with "protocol://path"-style URLs
   * @param path string to process
   * @param slashCharacter character to use as directory delimiter
   * @return transformed path
   */
  static std::string CanonicalizePath(const std::string& path, const char slashCharacter = '\\');

  static CURL CreateArchivePath(const std::string& type,
                                const CURL& archiveUrl,
                                const std::string& pathInArchive = "",
                                const std::string& password = "");

  static std::string AddFileToFolder(const std::string &strFolder, const std::string &strFile);

  static bool HasParentInHostname(const CURL& url);
  static bool HasEncodedHostname(const CURL& url);
  static bool HasEncodedFilename(const CURL& url);

  /*!
   \brief Cleans up the given path by resolving "." and ".."
   and returns it.

   This methods goes through the given path and removes any "."
   (as it states "this directory") and resolves any ".." by
   removing the previous directory from the path. This is done
   for file paths and host names (in case of VFS paths).

   \param path Path to be cleaned up
   \return Actual path without any "." or ".."
   */
  static std::string GetRealPath(const std::string &path);

  /*!
   \brief Updates the URL encoded hostname of the given path

   This method must only be used to update paths encoded with
   the old (Eden) URL encoding implementation to the new (Frodo)
   URL encoding implementation (which does not URL encode -_.!().

   \param strFilename Path to update
   \return True if the path has been updated/changed otherwise false
   */
  static bool UpdateUrlEncoding(std::string &strFilename);

private:
  static std::string resolvePath(const std::string &path);
};

