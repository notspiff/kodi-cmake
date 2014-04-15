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
#include "media/import/IMediaImportTask.h"
#include "media/import/MediaImportSource.h"
#include "threads/CriticalSection.h"
#include "utils/JobManager.h"

class IMediaImporter;
class IMediaImportHandler;
class IMediaImportRepository;

class CFileItem;
class CGUIDialogProgress;

class CMediaImportManager : public CJobQueue, public IMediaImportTaskCallback
{
public:
  ~CMediaImportManager();

  /*!
   * \brief Gets the singleton instance
   */
  static CMediaImportManager& Get();

  /*!
   * \brief TODO
   */
  void Uninitialize();

  /*!
   * \brief Register a media import repository implementation
   *
   * \param importRepository media import repository implementation to register
   */
  void RegisterSourceRepository(IMediaImportRepository* importRepository);
  /*!
   * \brief Unregister a media import repository implementation
   *
   * \param importRepository media import repository implementation to unregister
   * \return True if the media import repository implementation was unregistered, false otherwise
   */
  bool UnregisterSourceRepository(const IMediaImportRepository* importRepository);

  /*!
   * \brief Register a media importer implementation
   *
   * \param importer media importer implementation to register
   */
  void RegisterImporter(const IMediaImporter* importer);
  /*!
   * \brief Unregister a media importer implementation
   *
   * \param importer media importer implementation to unregister
   * \return True if the importer was unregistered, false otherwise
   */
  bool UnregisterImporter(const IMediaImporter* importer);

  /*!
   * \brief TODO
   */
  std::vector<const IMediaImporter*> GetImporter() const;

  /*!
   * \brief TODO
   */
  const IMediaImporter* GetImporter(const std::string &path) const;
  
  /*!
   * \brief Register a media import handler implementation
   *
   * \param importHandler media import handler implementation to register
   */
  void RegisterMediaImportHandler(IMediaImportHandler *importHandler);
  /*!
   * \brief Unregister a media import handler implementation
   *
   * \param importHandler media import handler implementation to unregister
   */
  void UnregisterMediaImportHandler(IMediaImportHandler *importHandler);

  std::vector<MediaType> GetSupportedMediaTypes() const;

  /*!
   * \brief TODO
   */
  std::vector< std::vector<MediaType> > GetSupportedMediaTypes(const std::set<MediaType> &mediaTypes) const;
  
  /*!
   * \brief TODO
   */
  std::set<MediaType> GetDependingMediaTypes(const MediaType &mediaType) const;
  
  /*!
   * \brief TODO
   */
  std::set<MediaType> GetGroupedMediaTypes(const MediaType &mediaType) const;
  
  /*!
   * \brief TODO
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
  void RemoveSource(const std::string& sourceID, CGUIDialogProgress *progress = NULL);

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
   * \brief TODO
   */
  bool HasImports(const std::string& sourceID) const;

  /*!
   * \brief TODO
   */
  bool HasImports(const CMediaImportSource &source) const;

  /*!
   * \brief TODO
   */
  bool AddImport(const std::string &sourceID, const std::string &path, const MediaType &mediaType, bool synchronise = true);

  /*!
   * \brief TODO
   */
  bool AddImports(const std::string &sourceID, const std::string &path, const std::set<MediaType> &mediaTypes, bool synchronise = true);

  /*!
   * \brief TODO
   */
  bool UpdateImport(const CMediaImport &import);

  /*!
   * \brief Completely removes the import with the given path and media type
   * Removes the import and all items imported from the import from the
   * libraries.
   *
   * \param path TODO
   * \param mediaType TODO
   */
  void RemoveImport(const std::string& path, const MediaType &mediaType, CGUIDialogProgress *progress = NULL);

  /*!
   * \brief TODO
   */
  std::vector<CMediaImport> GetImports() const;

  /*!
   * \brief TODO
   */
  std::vector<CMediaImport> GetImportsByMediaType(const MediaType &mediaType) const;

  /*!
   * \brief TODO
   */
  std::vector<CMediaImport> GetImportsBySource(const std::string &sourceID) const;

  /*!
   * \brief TODO
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
   * \brief TODO
   */
  bool UpdateImportedItem(const CFileItem &item);

  // implementation of IJobCallback
  virtual void OnJobComplete(unsigned int jobID, bool success, CJob *job);
  virtual void OnJobProgress(unsigned int jobID, unsigned int progress, unsigned int total, const CJob *job);

  // implementation of IMediaImportTaskCallback
  virtual std::vector<IMediaImportTask*> OnTaskComplete(bool success, IMediaImportTask *task);

private:
  CMediaImportManager();
  CMediaImportManager(const CMediaImportManager&);
  CMediaImportManager const& operator=(CMediaImportManager const&);

  bool AddSource(const CMediaImportSource &source);
  bool FindSource(const std::string &sourceID, CMediaImportSource &source) const;

  bool AddImport(const CMediaImport &import);
  bool FindImport(const std::string &path, const MediaType &mediaType, CMediaImport &import) const;

  void AddTaskProcessorJob(const std::string &path, CJob *job);
  void RemoveTaskProcessorJob(const std::string &path, const CJob *job);

  void Import(const CMediaImport &import);

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
  std::set<IMediaImportRepository*> m_importRepositories;

  CCriticalSection m_sourcesLock;
  std::map<std::string, bool> m_sources;

  CCriticalSection m_importersLock;
  std::map<const char*, const IMediaImporter*> m_importers;

  CCriticalSection m_importHandlersLock;
  std::map<MediaType, IMediaImportHandler*> m_importHandlersMap;
  std::vector<IMediaImportHandler*> m_importHandlers;

  CCriticalSection m_jobsLock;
  std::map<std::string, std::set<CJob*> > m_jobMap;
};
