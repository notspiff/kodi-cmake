#pragma once
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

#ifndef __XBMC_VFS_DLL_H__
#define __XBMC_VFS_DLL_H__

#include <stdint.h>
#include "xbmc_addon_dll.h"
#include "xbmc_vfs_types.h"

extern "C"
{
  //! \copydoc VFSEntry::Open
  void* Open(VFSURL* url);

  //! \copydoc VFSEntry::OpenForWrite
  void* OpenForWrite(VFSURL* url, bool bOverWrite);

  //! \copydoc VFSEntry::Read
  unsigned int Read(void* context, void* buffer, int64_t size);

  //! \copydoc VFSEntry::Write
  int Write(void* context, const void* buffer, int64_t size);

  //! \copydoc VFSEntry::Seek
  int64_t Seek(void* context, int64_t position, int whence);

  //! \copydoc VFSEntry::Truncate
  int Truncate(void* context, int64_t size);

  //! \copydoc VFSEntry::GetLength
  int64_t GetLength(void* context);

  //! \copydoc VFSEntry::GetPosition
  int64_t GetPosition(void* context);

  //! \copydoc VFSEntry::GetChunkSize
  int GetChunkSize(void* context);

  //! \copydoc VFSEntry::IoControl
  int IoControl(void* context, XFILE::EIoControl request, void* param);

  //! \copydoc VFSEntry::Stat
  int Stat(VFSURL* url, struct __stat64* buffer);

  //! \copydoc VFSEntry::Close
  bool Close(void* context);

  //! \copydoc VFSEntry::Exists
  bool Exists(VFSURL* url);

  //! \copydoc VFSEntry:;:ContainsFiles
  void* ContainsFiles(VFSURL* url, VFSDirEntry** entries, int* num_entries,
                      char* rootpath);

  //! \copydoc VFSEntry::ClearOutIdle
  void ClearOutIdle();

  //! \copydoc VFSEntry::DisconnectAll
  void DisconnectAll();

  //! \copydoc VFSEntry::DirectoryExists
  bool DirectoryExists(VFSURL* url);

  //! \copydoc VFSEntry::RemoveDirectory
  bool RemoveDirectory(VFSURL* url);

  //! \copydoc VFSEntry::CreateDirectory
  bool CreateDirectory(VFSURL* url);

  //! \copydoc VFSEntry::GetDirectory
  void* GetDirectory(VFSURL* url, VFSDirEntry** entries, int* num_entries);

  //! \copydoc VFSEntry::FreeDirectory
  void FreeDirectory(void* ctx);

  //! \copydoc VFSEntry::Delete
  bool Delete(VFSURL* url); 

  //! \copydoc VFSEntry::Rename
  bool Rename(VFSURL* url, VFSURL* url2);

  //! \copydoc VFSEntry::NextChannel
  bool NextChannel(void* ctx, bool preview);

  //! \copydoc VFSEntry::PrevChannel
  bool PrevChannel(void* ctx, bool preview);

  //! \copydoc VFSEntry::SelectChannel
  bool SelectChannel(void* ctx, unsigned int channel);

  //! \copydoc VFSEntry::GetTotalTime
  int GetTotalTime(void* ctx);

  //! \copydoc VFSEntry::GetStartTime
  int GetStartTime(void* ctx);

  //! \copydoc VFSEntry::UpdateItem
  bool UpdateItem(void* ctx);

  // function to export the above structure to XBMC
  void __declspec(dllexport) get_addon(struct VFSEntry* pScr)
  {
    pScr->Open = Open;
    pScr->OpenForWrite = OpenForWrite;
    pScr->Read = Read;
    pScr->Write = Write;
    pScr->Seek = Seek;
    pScr->GetLength = GetLength;
    pScr->GetPosition = GetPosition;
    pScr->IoControl = IoControl;
    pScr->Stat = Stat;
    pScr->Close = Close;
    pScr->Exists = Exists;
    pScr->ClearOutIdle = ClearOutIdle;
    pScr->DisconnectAll = DisconnectAll;
    pScr->DirectoryExists = DirectoryExists;
    pScr->GetDirectory = GetDirectory;
    pScr->FreeDirectory = FreeDirectory;
    pScr->Truncate = Truncate;
    pScr->Delete = Delete;
    pScr->Rename = Rename;
    pScr->RemoveDirectory = RemoveDirectory;
    pScr->CreateDirectory = CreateDirectory;
    pScr->ContainsFiles = ContainsFiles;
    pScr->NextChannel = NextChannel;
    pScr->PrevChannel = PrevChannel;
    pScr->SelectChannel = SelectChannel;
    pScr->GetTotalTime = GetTotalTime;
    pScr->GetStartTime = GetStartTime;
    pScr->UpdateItem = UpdateItem;
    pScr->GetChunkSize = GetChunkSize;
  };
};

#endif
