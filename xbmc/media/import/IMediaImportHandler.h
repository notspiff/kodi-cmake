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

#include "media/MediaType.h"
#include "media/import/IMediaImporter.h"
#include "media/import/MediaImport.h"

class CFileItemList;
class IMediaImportTask;

/*!
 * \brief Interface of a handler capable of handling imported media items of a
 * specific media type.
 */
class IMediaImportHandler
{
public:
  virtual ~IMediaImportHandler() { }

  virtual IMediaImportHandler* Create() const = 0;

  /*!
   * \brief Returns the media type the implementation is capable of handling.
   */
  virtual MediaType GetMediaType() const = 0;
  /*!
   * \brief Returns a list of media types which need to be handled before
   * using this implementation.
   */
  virtual std::set<MediaType> GetDependencies() const { return std::set<MediaType>(); }
  /*!
   * \brief Returns a list of media types which must be importable for
   * this implementation to be usable.
   */
  virtual std::set<MediaType> GetRequiredMediaTypes() const { return std::set<MediaType>(); }
  /*!
   * \brief Returns a list of media types which can be grouped together
   * with the media type of this implementation.
   */
  virtual std::vector<MediaType> GetGroupedMediaTypes() const { return std::vector<MediaType>(); }

  /*!
   * \brief Handles the given list of media items (must all be of the supported
   * media type) previously imported by the given media importer.
   *
   * \param import place from which the given items were imported
   * \param items list of media items of the supported media type
   * \param progressBar progress bar handler
   */
  virtual void HandleImportedItems(const CMediaImport &import, const CFileItemList &items, IMediaImportTask *task) = 0;

  /*!
   * \brief Enable/disable imported items
   *
   * \param importPath Path of the import
   * \param enable Whether to enable or disable imported items
   */
  virtual void SetImportedItemsEnabled(const CMediaImport &import, bool enable) = 0;
};
