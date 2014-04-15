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
#include "DllAvFormat.h"
#include "utils/StringUtils.h"
#include "music/tags/MusicInfoTag.h"

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
  m_av.Load();
  m_avu.Load();
  m_av.av_register_all();
}

CM4BFileDirectory::~CM4BFileDirectory(void)
{
  if (m_fctx)
    m_av.avformat_close_input(&m_fctx);
  if (m_ioctx)
  {
    m_avu.av_free(m_ioctx->buffer);
    m_avu.av_free(m_ioctx);
  }
}

bool CM4BFileDirectory::GetDirectory(const CStdString& strPath,
                                     CFileItemList &items)
{
  if (!m_fctx && !ContainsFiles(strPath))
    return true;

  std::string title;
  std::string author;

  AVDictionaryEntry* tag=NULL;
  while ((tag = m_avu.av_dict_get(m_fctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
  {
    if (strcmp(tag->key,"title") == 0)
      title = tag->value;
    if (strcmp(tag->key,"artist") == 0)
      author = tag->value;
  }

  for (size_t i=0;i<m_fctx->nb_chapters;++i)
  {
    tag=NULL;
    std::string chaptitle = "Unknown";
    while ((tag=m_avu.av_dict_get(m_fctx->chapters[i]->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
    {
      if (strcmp(tag->key,"title") == 0)
      {
        chaptitle = tag->value;
        break;
      }
    }
    CFileItemPtr item(new CFileItem(strPath.c_str(),false));
    item->GetMusicInfoTag()->SetTrackNumber(i+1);
    item->GetMusicInfoTag()->SetLoaded(true);
    item->GetMusicInfoTag()->SetTitle(chaptitle);
    item->GetMusicInfoTag()->SetAlbum(title);
    item->GetMusicInfoTag()->SetArtist(author);
    item->SetLabel(StringUtils::Format("%02i. %s - %s",i+1, title.c_str(), chaptitle.c_str()));
    item->m_lStartOffset = m_fctx->chapters[i]->start*av_q2d(m_fctx->chapters[i]->time_base)*75;
    item->m_lEndOffset = m_fctx->chapters[i]->end*av_q2d(m_fctx->chapters[i]->time_base)*75;
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

  uint8_t* buffer = (uint8_t*)m_avu.av_malloc(32768);
  m_ioctx = m_av.avio_alloc_context(buffer, 32768, 0, &file, m4b_file_read, NULL, m4b_file_seek);

  m_fctx = avformat_alloc_context();
  m_fctx->pb = m_ioctx;

  if (file.IoControl(IOCTRL_SEEK_POSSIBLE, NULL) == 0)
    m_ioctx->seekable = 0;

  m_ioctx->max_packet_size = 32768;

  AVInputFormat* iformat=NULL;
  m_av.av_probe_input_buffer(m_ioctx, &iformat, strPath.c_str(), NULL, 0, 0);

  bool contains = false;
  if (m_av.avformat_open_input(&m_fctx, strPath.c_str(), iformat, NULL) < 0)
  {
    if (m_fctx)
      m_av.avformat_close_input(&m_fctx);
    m_avu.av_free(m_ioctx->buffer);
    m_avu.av_free(m_ioctx);
    return false;
  }

  contains = m_fctx->nb_chapters > 1;

  return contains;
}
