/*
 *      Copyright (C) 2013 Arne Morten Kvarving
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

#include "AddonDll.h"
#include "include/xbmc_vfs_types.h"
#include "filesystem/IFile.h"
#include "filesystem/IDirectory.h"
#include "filesystem/IFileDirectory.h"
#include "filesystem/ILiveTV.h"

typedef DllAddon<VFSEntry, VFS_PROPS> DllVFSEntry;
namespace ADDON
{
  typedef CAddonDll<DllVFSEntry,
                    VFSEntry, VFS_PROPS> VFSEntryDll;

  class CVFSEntry : public VFSEntryDll
  {
  public:
    CVFSEntry(const AddonProps &props) : VFSEntryDll(props) {};
    CVFSEntry(const cp_extension_t *ext);
    virtual ~CVFSEntry() {}
    virtual AddonPtr Clone() const;

    // Things that MUST be supplied by the child classes
    void* Open(const CURL& url);
    void* OpenForWrite(const CURL& url, bool bOverWrite);
    bool Exists(const CURL& url);
    int Stat(const CURL& url, struct __stat64* buffer);
    unsigned int Read(void* ctx, void* lpBuf, int64_t uiBufSize);
    int Write(void* ctx, void* lpBuf, int64_t uiBufSize);
    int64_t Seek(void* ctx, int64_t iFilePosition, int iWhence = SEEK_SET);
    int Truncate(void* ctx, int64_t size);
    void Close(void* ctx);
    int64_t GetPosition(void* ctx);
    int64_t GetLength(void* ctx);
    int GetChunkSize(void* ctx);
    int IoControl(void* ctx, XFILE::EIoControl request, void* param);
    bool Delete(const CURL& url);
    bool Rename(const CURL& url, const CURL& url2);

    bool GetDirectory(const CStdString& strPath, CFileItemList& items);
    bool DirectoryExists(const CURL& url);
    bool RemoveDirectory(const CURL& url);
    bool CreateDirectory(const CURL& url);
    void ClearOutIdle();
    void DisconnectAll();

    bool ContainsFiles(const std::string& path, CFileItemList& items);

    bool NextChannel(void* ctx, bool preview);
    bool PrevChannel(void* ctx, bool preview);
    bool SelectChannel(void* ctx, unsigned int channel);
    int GetTotalTime(void* ctx);
    int GetStartTime(void* ctx);
    bool UpdateItem(void* ctx, CFileItem& item);

    const std::string& GetProtocols() const { return m_protocols; }
    const std::string& GetExtensions() const { return m_extensions; }
    bool HasFiles() const { return m_files; }
    bool HasDirectories() const { return m_directories; }
    bool HasFileDirectories() const { return m_filedirectories; }
    bool HasLiveTV() const { return m_livetv; }
  protected:
    std::string m_protocols;
    std::string m_extensions;
    bool m_files;
    bool m_directories;
    bool m_filedirectories;
    bool m_livetv;
  };

  typedef boost::shared_ptr<CVFSEntry> VFSEntryPtr;

  class CVFSEntryIFileWrapper : public XFILE::IFile
  {
  public:
    CVFSEntryIFileWrapper(VFSEntryPtr ptr);
    virtual ~CVFSEntryIFileWrapper();

    virtual bool Open(const CURL& url);
    virtual bool OpenForWrite(const CURL& url, bool bOverWrite);
    virtual bool Exists(const CURL& url);
    virtual int  Stat(const CURL& url, struct __stat64* buffer);
    virtual unsigned int Read(void* lpBuf, int64_t uiBufSize);
    virtual int Write(void* lpBuf, int64_t uiBufSize);
    virtual int64_t Seek(int64_t iFilePosition, int iWhence = SEEK_SET);
    virtual int Truncate(int64_t size);
    virtual void Close();
    virtual int64_t GetPosition();
    virtual int64_t GetLength();
    virtual int GetChunkSize();
    virtual int IoControl(XFILE::EIoControl request, void* param);
    virtual bool Delete(const CURL& url);
    virtual bool Rename(const CURL& url, const CURL& url2);
  protected:
    void* m_context;
    VFSEntryPtr m_addon;
  };

  class CVFSEntryILiveTVWrapper : public XFILE::ILiveTVInterface,
                                  public CVFSEntryIFileWrapper
  {
  public:
    CVFSEntryILiveTVWrapper(VFSEntryPtr ptr);
    virtual ~CVFSEntryILiveTVWrapper() {}

    bool NextChannel(bool preview=false);
    bool PrevChannel(bool preview=false);
    bool SelectChannel(unsigned int channel);

    int GetTotalTime();
    int GetStartTime();

    bool UpdateItem(CFileItem& item);
  };

  class CVFSEntryIDirectoryWrapper : public XFILE::IDirectory
  {
  public:
    CVFSEntryIDirectoryWrapper(VFSEntryPtr ptr);
    virtual ~CVFSEntryIDirectoryWrapper() {}

    virtual bool GetDirectory(const CStdString& strPath, CFileItemList& items);
    virtual bool Exists(const char* strPath);
    virtual bool Remove(const char* strPath);
    virtual bool Create(const char* strPath);
  protected:
    VFSEntryPtr m_addon;
  };

  class CVFSEntryIFileDirectoryWrapper : public XFILE::IFileDirectory,
                                         public CVFSEntryIDirectoryWrapper
  {
  public:
    CVFSEntryIFileDirectoryWrapper(VFSEntryPtr ptr) : CVFSEntryIDirectoryWrapper(ptr) {}

    bool ContainsFiles(const CStdString& strPath)
    {
      return m_addon->ContainsFiles(strPath, m_items);
    }

    bool GetDirectory(const CStdString& strPath, CFileItemList& items)
    {
      return CVFSEntryIDirectoryWrapper::GetDirectory(strPath, items);
    }

    bool Exists(const char* strPath)
    {
      return CVFSEntryIDirectoryWrapper::Exists(strPath);
    }

    bool Remove(const char* strPath)
    {
      return CVFSEntryIDirectoryWrapper::Remove(strPath);
    }

    bool Create(const char* strPath)
    {
      return CVFSEntryIDirectoryWrapper::Create(strPath);
    }

    CFileItemList m_items;
  };


  class CVFSEntryManager
  {
  public:
    static CVFSEntryManager& Get();

    VFSEntryPtr GetAddon(const std::string& id);

    void ClearOutIdle();
    void DisconnectAll();
  protected:
    CVFSEntryManager();

    std::map<std::string, VFSEntryPtr> m_addons;
  };

} /*namespace ADDON*/
