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

#include <memory>
#include <vector>

#include "media/MediaType.h"
#include "media/import/MediaImport.h"
#include "media/import/MediaImportSource.h"

class CFileItem;

/*!
 * \brief Interface defining a repository capable of storing media items imported from sources and their imports.
 */
class IMediaImportRepository
{
public:
  virtual ~IMediaImportRepository() { }

  /*
   * \brief Initializes the repository.
   *
   * \return True if the initialization was successful, false otherwise
   */
  virtual bool Initialize() = 0;
  
  /*
   * \brief Gets the media type of the given item.
   *
   * \param item Item
   * \param mediaType[out] Media type of the item
   * \return True if the media type was determined, false otherwise
   */
  virtual bool GetMediaType(const CFileItem &item, MediaType &mediaType) const = 0;
  
  /*
   * \brief Gets all imports stored in the repository.
   *
   * \return List of imports
   */
  virtual std::vector<CMediaImport> GetImports() const = 0;

  /*
   * \brief Gets all imports from the source with the given identifier stored in the repository.
   *
   * \param sourceIdentifier Source identifier
   * \return List of imports
   */
  virtual std::vector<CMediaImport> GetImportsBySource(const std::string &sourceIdentifier) const = 0;

  /*
   * \brief Gets all imports for the given media type stored in the repository.
   *
   * \param mediaType Media type
   * \return List of imports
   */
  virtual std::vector<CMediaImport> GetImportsByMediaType(const MediaType &mediaType) const = 0;

  /*
   * \brief Gets the import for the given path and media type.
   *
   * \param path Path of the import
   * \param mediaType Media type of the items imported from the import
   * \param import[out] Import
   * \return True if a matching import was found, false otherwise
   */
  virtual bool GetImport(const std::string &path, const MediaType &mediaType, CMediaImport &import) const = 0;

  /*
   * \brief Adds the given import to the repository.
   *
   * \param import Import to be added
   * \return True if the import was successfully added, false otherwise
   */
  virtual bool AddImport(const CMediaImport &import) = 0;

  /*
   * \brief Updates the given import in the repository.
   *
   * \param import Import to be updated
   * \return True if the import was successfully updated, false otherwise
   */
  virtual bool UpdateImport(const CMediaImport &import) = 0;

  /*
   * \brief Removes the given import from the repository.
   *
   * \param import Import to be removed
   * \return True if the import was successfully removed, false otherwise
   */
  virtual bool RemoveImport(const CMediaImport &import) = 0;
 
  /*
   * \brief Updates the last synchronisation timestamp of the given import in the repository.
   *
   * \param import Import to be updated
   * \return True if the import was successfully updated, false otherwise
   */
  virtual bool UpdateLastSync(CMediaImport &import) = 0;

  /*
   * \brief Gets all sources supporting the given media type stored in the repository.
   *
   * \param sourceIdentifier Source identifier
   * \return List of sources
   */
  virtual std::vector<CMediaImportSource> GetSources(const MediaType &mediaType = MediaTypeNone) const = 0;

  /*
   * \brief Gets the source with the given identifier.
   *
   * \param identifier Source identifier
   * \param source[out] Source
   * \return True if a matching source was found, false otherwise
   */
  virtual bool GetSource(const std::string &identifier, CMediaImportSource &source) const = 0;

  /*
   * \brief Adds the given source to the repository.
   *
   * \param source Source to be added
   * \return True if the source was successfully added, false otherwise
   */
  virtual bool AddSource(const CMediaImportSource &source) = 0;

  /*
   * \brief Updates the given source in the repository.
   *
   * \param source Source to be updated
   * \return True if the source was successfully updated, false otherwise
   */
  virtual bool UpdateSource(const CMediaImportSource &source) = 0;

  /*
   * \brief Removes the source with the given identifier from the repository.
   *
   * \param identifier Source identifier
   * \return True if the source was successfully removed, false otherwise
   */
  virtual bool RemoveSource(const std::string &identifier) = 0;
};

typedef std::shared_ptr<IMediaImportRepository> MediaImportRepositoryPtr;
