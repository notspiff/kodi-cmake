#pragma once
/*
 * DAAP Support for XBMC
 *      Copyright (c) 2004 Forza (Chris Barnett)
 *      Portions Copyright (c) by the authors of libOpenDAAP
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

#include "IDirectory.h"

extern "C"
{
#include "libXDAAP/client.h"
#include "libXDAAP/private.h"
}

namespace XFILE
{
class CDAAPDirectory :

      public IDirectory
{
public:
  CDAAPDirectory(void);
  virtual ~CDAAPDirectory(void);
  virtual bool AllowAll() const { return true; }
  virtual bool GetDirectory(const CURL& url, CFileItemList &items);
  //virtual void CloseDAAP(void);
  int GetCurrLevel(const std::string &strPath);

private:
  void free_albums(albumPTR *alb);
  void free_artists();
  void AddToArtistAlbum(char *artist_s, char *album_s);

  DAAP_ClientHost_DatabaseItem *m_currentSongItems;
  int m_currentSongItemCount;

  DAAP_SClientHost *m_thisHost;
  int m_currLevel;

  artistPTR *m_artisthead;
  std::string m_selectedPlaylist;
  std::string m_selectedArtist;
  std::string m_selectedAlbum;
};
}
