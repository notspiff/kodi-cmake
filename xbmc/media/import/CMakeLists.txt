set(SOURCES MediaImport.cpp
            MediaImportManager.cpp
            MediaImportTaskProcessorJob.cpp
            dialogs/GUIDialogMediaImportInfo.cpp
            handler/EpisodeImportHandler.cpp
            handler/MovieImportHandler.cpp
            handler/MusicVideoImportHandler.cpp
            handler/SeasonImportHandler.cpp
            handler/TvShowImportHandler.cpp
            handler/VideoImportHandler.cpp
            repositories/VideoImportRepository.cpp
            task/MediaImportRetrievalTask.cpp
            task/MediaImportSourceRegistrationTask.cpp
            task/MediaImportSynchronisationTask.cpp
            task/MediaImportUpdateTask.cpp
            windows/GUIViewStateMediaSourceBrowser.cpp
            windows/GUIWindowMediaSourceBrowser.cpp)

core_add_library(media_import)
