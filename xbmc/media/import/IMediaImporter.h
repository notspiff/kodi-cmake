#pragma once
/*
 *      Copyright (C) 2013 Team XBMC
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

#include <set>
#include <string>

#include "media/MediaType.h"
#include "media/import/MediaImport.h"
#include "media/import/MediaImportSource.h"

class CGUIDialogProgressBarHandle;
class CMediaImportRetrievalTask;
class CMediaImportUpdateTask;

/*!
 * \brief Interface of a media importer capable of importing media items from
 * a specific source into the local library.
 */
class IMediaImporter
{
public:
  virtual ~IMediaImporter() { }
  
  /*!
   * \brief Returns the path from where the media items are imported.
   */
  const CMediaImport& GetImport() const { return m_import; }
  /*!
   * \brief Returns the path from where the media items are imported.
   */
  const std::string& GetPath() const { return m_import.GetPath(); }
  /*!
   * \brief Returns the identification of the source from where the media
   * items are imported.
   *
   * This identification is used in the library to associate an imported
   * media item with where it came from for updating purposes.
   */
  const std::string& GetSourceIdentifier() const { return m_import.GetSource().GetIdentifier(); }
  /*!
   * \brief Returns the friendly name of the source from where the media
   * items are imported.
   */
  const std::string& GetFriendlySourceName() const { return m_import.GetSource().GetFriendlyName(); }
  /*!
   * \brief Returns the media types to be imported.
   */
  const MediaType& GetMediaType() const { return m_import.GetMediaType(); }

  /*!
   * \brief Gets a unique identification of the media importer
   */
  virtual const char* GetIdentification() const = 0;

  /*
   * \brief Checks if the implementation can import items from the given path.
   *
   * \param path Path to a source or import
   * \return True if the implementation can import items from the given path, false otherwise
   */
  virtual bool CanImport(const std::string& path) const = 0;

  /*!
   * \brief Checks if the implementation can update general metadata of an
   * imported item on the source with the given path.
   *
   * \param path Path to a source or import
   * \return True if the implementation can update general metadata of an imported item on the source with the given path, false otherwise
   */
  virtual bool CanUpdateMetadataOnSource(const std::string& path) const { return false; }

  /*!
   * \brief Checks if the implementation can update the playcount of an
   * imported item on the source with the given path.
   *
   * \param path Path to a source or import
   * \return True if the implementation can update the playcount of an imported item on the source with the given path, false otherwise
   */
  virtual bool CanUpdatePlaycountOnSource(const std::string& path) const { return false; }

  /*!
   * \brief Checks if the implementation can update the last played date of an
   * imported item on the source with the given path.
   *
   * \param path Path to a source or import
   * \return True if the implementation can update the last played date of an imported item on the source with the given path, false otherwise
   */
  virtual bool CanUpdateLastPlayedOnSource(const std::string& path) const { return false; }

  /*!
   * \brief Checks if the implementation can update the resume position of an
   * imported item on the source with the given path.
   *
   * \param path Path to a source or import
   * \return True if the implementation can update the resume position of an imported item on the source with the given path, false otherwise
   */
  virtual bool CanUpdateResumePositionOnSource(const std::string& path) const { return false; }

  /*!
   * \brief Creates a new IMediaImporter instance of the implementation.
   *
   * This method is responsible to only create a new instance of the
   * implementation if it can handle the given path..
   *
   * \param import Import with path and media type to be processed
   * \return New IMediaImporter instance of the implementation
   */
  virtual IMediaImporter* Create(const CMediaImport &import) const = 0;

  /*!
   * \brief Imports items from the source.
   *
   * The imported items are stored in the given CMediaImportRetrievalTask instance by
   * their respective media type.
   *
   * \param task Task performing the import (to check if we need to cancel import and to report progress)
   * \return True if the import succeeded, false otherwise
   */
  virtual bool Import(CMediaImportRetrievalTask* task) const = 0;

  /*!
   * \brief Updates an item's metadata on the source
   *
   * The given CMediaImportUpdateTask contains a media item whose metadata
   * should be updated on the source if it is supported by the importer.
   *
   * \param task Task performing the updating
   * \return True if updating the item's metadata succeeded, false otherwise
   */
  virtual bool UpdateOnSource(CMediaImportUpdateTask* task) const = 0;

protected:
  IMediaImporter(const CMediaImport &import)
    : m_import(import)
  { }

private:
  CMediaImport m_import;
};
