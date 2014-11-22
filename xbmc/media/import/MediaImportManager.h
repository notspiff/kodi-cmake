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

#include <map>
#include <set>
#include <vector>

#include "media/MediaType.h"
#include "media/import/IMediaImporter.h"
#include "media/import/IMediaImportHandler.h"
#include "media/import/IMediaImportRepository.h"
#include "media/import/IMediaImportTask.h"
#include "media/import/MediaImportSource.h"
#include "threads/CriticalSection.h"
#include "utils/Job.h"

class CFileItem;
class CGUIDialogProgress;
class CMediaImportTaskProcessorJob;

class CMediaImportManager : public IJobCallback, public IMediaImportTaskCallback
{
public:
  ~CMediaImportManager();

  /*!
   * \brief Gets the singleton instance
   */
  static CMediaImportManager& Get();

  /*!
   * \brief Uninitializes the media manager by releasing all importers, import handlers and repositories.
   */
  void Uninitialize();

  /*!
   * \brief Register a media import repository implementation
   *
   * \param importRepository media import repository implementation to register
   */
  void RegisterImportRepository(MediaImportRepositoryPtr importRepository);
  /*!
   * \brief Unregister a media import repository implementation
   *
   * \param importRepository media import repository implementation to unregister
   * \return True if the media import repository implementation was unregistered, false otherwise
   */
  bool UnregisterImportRepository(const MediaImportRepositoryPtr importRepository);

  /*!
   * \brief Register a media importer implementation
   *
   * \param importer media importer implementation to register
   */
  void RegisterImporter(MediaImporterPtr importer);
  /*!
   * \brief Unregister a media importer implementation
   *
   * \param importer media importer implementation to unregister
   * \return True if the importer was unregistered, false otherwise
   */
  bool UnregisterImporter(MediaImporterPtr importer);

  /*!
   * \brief Returns a list of registered media import implementations.
   */
  std::vector<MediaImporterConstPtr> GetImporters() const;

  /*!
   * \brief Returns the media importer implementation capable of importing
   * media items from the given path.
   *
   * \param path Path to import items from
   * \return Media importer implementation
   */
  const MediaImporterConstPtr GetImporter(const std::string &path) const;
  
  /*!
   * \brief Register a media import handler implementation
   *
   * \param importHandler media import handler implementation to register
   */
  void RegisterMediaImportHandler(MediaImportHandlerPtr importHandler);
  /*!
   * \brief Unregister a media import handler implementation
   *
   * \param importHandler media import handler implementation to unregister
   */
  void UnregisterMediaImportHandler(MediaImportHandlerPtr importHandler);

  /*!
   * \brief Returns the media import handler implementation capable of handling
   * imported media items of the given media type.
   *
   * \param mediaType Media type of an imported item
   * \pram Media import handler implementation
   */
  const MediaImportHandlerConstPtr GetImportHandler(const MediaType& mediaType) const;

  /*!
   * \brief Returns a list of registered media import handler implementations.
   */
  std::vector<MediaImportHandlerConstPtr> GetImportHandlers() const;

  /*!
   * \brief Returns a list of registered media import handler implementations
   * capable of handling imported media items of one of the given media types.
   *
   * \param mediaTypes Media types of imported items
   * \pram List of media import handler implementations
   */
  std::vector<MediaImportHandlerConstPtr> GetImportHandlers(const std::set<MediaType>& mediaTypes) const;

  /*!
   * \brief Returns a list of all supported media types.
   */
  std::set<MediaType> GetSupportedMediaTypes() const;

  /*!
   * \brief Returns a list of all supported media types in the order they need to be synchronised.
   *
   * \param reversed Whether the list should be reversed or not
   */
  std::vector<MediaType> GetSupportedMediaTypesOrdered(bool reversed = false) const;

  /*!
   * \brief Returns a list of the given media types in the order they need to be synchronised.
   *
   * \param mediaTypes List of media types to be ordered
   * \param reversed Whether the list should be reversed or not
   */
  std::vector<MediaType> GetSupportedMediaTypesOrdered(const std::set<MediaType>& mediaTypes, bool reversed = false) const;

  /*!
   * \brief Returns a list of the given media types with depending media types grouped together.
   *
   * \param mediaTypes List of media types to be grouped
   */
  std::vector< std::vector<MediaType> > GetSupportedMediaTypesGrouped(const std::set<MediaType> &mediaTypes) const;
  
  /*!
   * \brief Returns a set of all media types depending on the given media type.
   */
  std::set<MediaType> GetDependingMediaTypes(const MediaType &mediaType) const;
  
  /*!
   * \brief Returns a set of all media types grouped together with the given media type.
   */
  std::set<MediaType> GetGroupedMediaTypes(const MediaType &mediaType) const;
  
  /*!
   * \brief Returns a set of all media types connected to the given media type.
   */
  std::set<MediaType> GetConnectedMediaTypes(const MediaType &mediaType) const;
  
  /*!
   * \brief Registers a discovered source
   * If the source is new/unknown the user will be prompted with a dialog with
   * the option to import media items from the source or to ignore it.
   * If the source is already known all imports from that source are being
   * triggered.
   *
   * \param sourceID Unique identifier (VFS path) of the source
   * \param friendlyName Friendly name of the source
   * \param mediaTypes Media types supported by the source
   */
  void RegisterSource(const std::string& sourceID, const std::string& friendlyName, const std::string& iconUrl = "", const std::set<MediaType>& mediaTypes = std::set<MediaType>());

  /*!
   * \brief Unregisters the source with the given identifier
   * All items imported from the source are being marked as disabled.
   *
   * \param sourceID Unique identifier of the source
   */
  void UnregisterSource(const std::string& sourceID);

  /*!
   * \brief Completely removes the source with the given identifier
   * Removes the source, all its imports and all items imported from the
   * source from the libraries.
   *
   * \param sourceID Unique identifier of the source
   */
  void RemoveSource(const std::string& sourceID);

  /*!
   * \brief Whether any sources have been registered.
   *
   * \return True if at least one source has been registered, false otherwise.
   */
  bool HasSources() const;

  /*!
   * \brief Whether any active/inactive sources have been registered.
   *
   * \return True if at least one active/inactive source has been registered, false otherwise.
   */
  bool HasSources(bool active) const;

  /*!
   * \brief Gets all registered sources
   *
   * \return Set of all registered sources
   */
  std::vector<CMediaImportSource> GetSources() const;

  /*!
   * \brief Gets all active or inactive registered sources
   *
   * \param active Whether to get active or inactive sources
   * \return Set of all active or inactive registered sources
   */
  std::vector<CMediaImportSource> GetSources(bool active) const;

  /*!
   * \brief Gets the source with the given identifier
   *
   * \param sourceID Unique identifier of the source
   * \param source Source with the given identifier
   * \return True if the source with the given identifier was found, false otherwise
   */
  bool GetSource(const std::string& sourceID, CMediaImportSource& source) const;

  /*!
   * \brief Checks whether the given source is active or not
   *
   * \param sourceID Unique identifier of the source
   * \return True if the given source is active, otherwise false
   */
  bool IsSourceActive(const std::string& sourceID) const;

  /*!
   * \brief Checks whether the given source is active or not
   *
   * \param source Source object
   * \return True if the given source is active, otherwise false
   */
  bool IsSourceActive(const CMediaImportSource &source) const;

  /*!
   * \brief Whether any imports have been defined.
   *
   * \return True if at least one import has been defined, false otherwise.
   */
  bool HasImports() const;

  /*!
   * \brief Whether the source with the given identifier has imports defined.
   *
   * \param sourceID Identifier of a source
   * \return True if the source with the given identifier has imports defined, false otherwise.
   */
  bool HasImports(const std::string& sourceID) const;

  /*!
   * \brief Whether the given source has imports defined.
   *
   * \param source Source
   * \return True if the given source has imports defined, false otherwise.
   */
  bool HasImports(const CMediaImportSource &source) const;

  /*!
   * \brief Adds a new import to the given source for the given path and media type.
   *
   * \param sourceID Source identifier
   * \param path Path from where to import media items
   * \param mediaType Type of the media items to import
   * \return True if the import was successfully added, false otherwise
   */
  bool AddImport(const std::string &sourceID, const std::string &path, const MediaType &mediaType);

  /*!
   * \brief Adds new imports to the given source for the given path and media types.
   *
   * \param sourceID Source identifier
   * \param path Path from where to import media items
   * \param mediaTypes Set of types of the media items to import
   * \return True if the imports were successfully added, false otherwise
   */
  bool AddImports(const std::string &sourceID, const std::string &path, const std::set<MediaType> &mediaTypes);

  /*!
   * \brief Updates the details and settings of the given import.
   *
   * \param import Updated import
   * \return True if the import was successfully updated, false otherwise
   */
  bool UpdateImport(const CMediaImport &import);

  /*!
   * \brief Completely removes the import with the given path and media type.
   * 
   * \details Removes the import and all items imported from the import from the libraries.
   *
   * \param path Path of the import
   * \param mediaType Media type of the import
   */
  void RemoveImport(const std::string& path, const MediaType &mediaType);

  /*!
   * \brief Returns a list of all registered media imports.
   */
  std::vector<CMediaImport> GetImports() const;

  /*!
   * \brief Returns a list of all registered media imports for the given media type.
   *
   * \param mediaType Media type of the imports
   */
  std::vector<CMediaImport> GetImportsByMediaType(const MediaType &mediaType) const;

  /*!
   * \brief Returns a list of media imports belonging to the source with the given identifier.
   *
   * \param sourceID Source identifier
   */
  std::vector<CMediaImport> GetImportsBySource(const std::string &sourceID) const;

  /*!
   * \brief Gets the import for the given path and media type.
   *
   * \param path Path of the import
   * \param mediaType Media type of the import
   * \return True if the import for the given path and media type was found, false otherwise
   */
  bool GetImport(const std::string &path, const MediaType &mediaType, CMediaImport &import) const;

  /*!
   * \brief Import media items from all registered sources and imports
   *
   * \return True if the import of media items has been started, false otherwise
   */
  bool Import();

  /*!
   * \brief Import media items from the given source
   *
   * \param sourceID Unique identifier of the source
   * \return True if the import of media items has been started, false otherwise
   */
  bool Import(const std::string& sourceID);

  /*!
   * \brief Import media items of the given media type from the given path
   *
   * \param path Path from where media items will be imported
   * \param mediaType media type to import
   * \return True if the import of media items has been started, false otherwise
   */
  bool Import(const std::string& path, const MediaType& mediaType);
  
  /*!
   * \brief Updates the details of the imported media item on the source from where it was imported.
   *
   * \param item Imported media item to update on the source
   * \return True if the imported media item was successfully updated on the source, false otherwise
   */
  bool UpdateImportedItem(const CFileItem &item);

  // implementation of IJobCallback
  virtual void OnJobComplete(unsigned int jobID, bool success, CJob *job);
  virtual void OnJobProgress(unsigned int jobID, unsigned int progress, unsigned int total, const CJob *job);

  // implementation of IMediaImportTaskCallback
  virtual bool OnTaskComplete(bool success, IMediaImportTask *task);

private:
  CMediaImportManager();
  CMediaImportManager(const CMediaImportManager&);
  CMediaImportManager const& operator=(CMediaImportManager const&);

  bool AddSource(const CMediaImportSource &source);
  bool FindSource(const std::string &sourceID, CMediaImportSource &source) const;

  bool AddImport(const CMediaImport &import);
  bool FindImport(const std::string &path, const MediaType &mediaType, CMediaImport &import) const;

  void AddTaskProcessorJob(const std::string &path, CMediaImportTaskProcessorJob *job);
  void RemoveTaskProcessorJob(const std::string &path, const CMediaImportTaskProcessorJob *job);
  void CancelTaskProcessorJobs(const std::string &sourceID);
  void CancelAllTaskProcessorJobs();

  void Import(const CMediaImportSource &source, bool automatically = false);
  void Import(const CMediaImport &import, bool automatically = false);

  void OnSourceAdded(const CMediaImportSource &source);
  void OnSourceUpdated(const CMediaImportSource &source);
  void OnSourceRemoved(const CMediaImportSource &source);
  void OnSourceActivated(const CMediaImportSource &source);
  void OnSourceDeactivated(const CMediaImportSource &source);
  void OnImportAdded(const CMediaImport &import);
  void OnImportUpdated(const CMediaImport &import);
  void OnImportRemoved(const CMediaImport &import);
  void SendSourceMessage(const CMediaImportSource &source, int message, int param = 0);
  void SendImportMessage(const CMediaImport &import, int message);

  CCriticalSection m_importRepositoriesLock;
  std::set<MediaImportRepositoryPtr> m_importRepositories;

  typedef struct MediaImportSource {
    bool active;
    bool removing;
  } MediaImportSource;

  CCriticalSection m_sourcesLock;
  std::map<std::string, MediaImportSource> m_sources;

  CCriticalSection m_importersLock;
  std::map<const char*, MediaImporterConstPtr> m_importers;

  CCriticalSection m_importHandlersLock;
  std::map<MediaType, MediaImportHandlerConstPtr> m_importHandlersMap;
  std::vector<MediaImportHandlerConstPtr> m_importHandlers;

  CCriticalSection m_jobsLock;
  typedef std::set<CMediaImportTaskProcessorJob*> MediaImportTaskProcessorJobs;
  typedef std::map<std::string, MediaImportTaskProcessorJobs> MediaImportTaskProcessorJobsMap;
  MediaImportTaskProcessorJobsMap m_jobMap;
};
