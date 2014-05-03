/*
 *      Copyright (C) 2014 Arne Morten Kvarving
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

#include "M4BFileDirectory.h"
#include "filesystem/File.h"
#include "FileItem.h"
#include "utils/StringUtils.h"
#include "music/tags/MusicInfoTag.h"
#include "guilib/LocalizeStrings.h"

using namespace XFILE;

static int m4b_file_read(void *h, uint8_t* buf, int size)
{
  CFile* pFile = static_cast<CFile*>(h);
  return pFile->Read(buf, size);
}

static off_t m4b_file_seek(void *h, off_t pos, int whence)
{
  CFile* pFile = static_cast<CFile*>(h);
  if(whence == AVSEEK_SIZE)
    return pFile->GetLength();
  else
    return pFile->Seek(pos, whence & ~AVSEEK_FORCE);
}

CM4BFileDirectory::CM4BFileDirectory(void) : m_ioctx(NULL), m_fctx(NULL)
{
}

CM4BFileDirectory::~CM4BFileDirectory(void)
{
  if (m_fctx)
    avformat_close_input(&m_fctx);
  if (m_ioctx)
  {
    av_free(m_ioctx->buffer);
    av_free(m_ioctx);
  }
}

bool CM4BFileDirectory::GetDirectory(const CStdString& strPath,
                                     CFileItemList &items)
{
  if (!m_fctx && !ContainsFiles(strPath))
    return true;

  std::string title;
  std::string author;
  std::string album;

  AVDictionaryEntry* tag=NULL;
  while ((tag = av_dict_get(m_fctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
  {
    if (strcasecmp(tag->key,"title") == 0)
      title = tag->value;
    if (strcasecmp(tag->key,"album") == 0)
      album = tag->value;
    if (strcasecmp(tag->key,"artist") == 0)
      author = tag->value;
  }

  for (size_t i=0;i<m_fctx->nb_chapters;++i)
  {
    tag=NULL;
    std::string chaptitle = StringUtils::Format(g_localizeStrings.Get(25010), i+1);
    std::string chapauthor;
    std::string chapalbum;
    while ((tag=av_dict_get(m_fctx->chapters[i]->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
    {
      if (strcasecmp(tag->key,"title") == 0)
        chaptitle = tag->value;
      if (strcasecmp(tag->key,"artist") == 0)
        chapauthor = tag->value;
      if (strcasecmp(tag->key,"album") == 0)
        chapalbum = tag->value;
    }
    CFileItemPtr item(new CFileItem(strPath.c_str(),false));
    item->GetMusicInfoTag()->SetTrackNumber(i+1);
    item->GetMusicInfoTag()->SetLoaded(true);
    item->GetMusicInfoTag()->SetTitle(chaptitle);
    if (album.empty())
      item->GetMusicInfoTag()->SetAlbum(title);
    else if (chapalbum.empty())
      item->GetMusicInfoTag()->SetAlbum(album);
    else
      item->GetMusicInfoTag()->SetAlbum(chapalbum);
    if (chapauthor.empty())
      item->GetMusicInfoTag()->SetArtist(author);
    else
      item->GetMusicInfoTag()->SetArtist(chapauthor);

    item->SetLabel(StringUtils::Format("%02i. %s - %s",i+1,
                   item->GetMusicInfoTag()->GetAlbum().c_str(),
                   item->GetMusicInfoTag()->GetTitle().c_str()));
    item->m_lStartOffset = m_fctx->chapters[i]->start*av_q2d(m_fctx->chapters[i]->time_base)*75;
    item->m_lEndOffset = m_fctx->chapters[i]->end*av_q2d(m_fctx->chapters[i]->time_base);
    int compare = m_fctx->duration / (AV_TIME_BASE);
    if (item->m_lEndOffset < 0 || item->m_lEndOffset > compare)
    {
      if (i < m_fctx->nb_chapters-1)
        item->m_lEndOffset = m_fctx->chapters[i+1]->start*av_q2d(m_fctx->chapters[i+1]->time_base);
      else
        item->m_lEndOffset = compare;
    }
    item->m_lEndOffset *= 75;
    item->GetMusicInfoTag()->SetDuration((item->m_lEndOffset-item->m_lStartOffset)/75);
    item->SetProperty("item_start", item->m_lStartOffset);
    items.Add(item);
  }

  items.SetContent("chapters");

  return true;
}

bool CM4BFileDirectory::Exists(const char* strPath)
{
  return CFile::Exists(strPath) && ContainsFiles(strPath);
}

bool CM4BFileDirectory::ContainsFiles(const CStdString& strPath)
{
  CFile file;
  if (!file.Open(strPath))
    return false;

  uint8_t* buffer = (uint8_t*)av_malloc(32768);
  m_ioctx = avio_alloc_context(buffer, 32768, 0, &file, m4b_file_read, NULL, m4b_file_seek);

  m_fctx = avformat_alloc_context();
  m_fctx->pb = m_ioctx;

  if (file.IoControl(IOCTRL_SEEK_POSSIBLE, NULL) == 0)
    m_ioctx->seekable = 0;

  m_ioctx->max_packet_size = 32768;

  AVInputFormat* iformat=NULL;
  av_probe_input_buffer(m_ioctx, &iformat, strPath.c_str(), NULL, 0, 0);

  bool contains = false;
  if (avformat_open_input(&m_fctx, strPath.c_str(), iformat, NULL) < 0)
  {
    if (m_fctx)
      avformat_close_input(&m_fctx);
    av_free(m_ioctx->buffer);
    av_free(m_ioctx);
    return false;
  }

  contains = m_fctx->nb_chapters > 1;

  return contains;
}
