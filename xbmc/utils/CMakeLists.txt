set(SOURCES ActorProtocol.cpp 
            AlarmClock.cpp
            AliasShortcutUtils.cpp
            Archive.cpp
            AsyncFileCopy.cpp
            AutoPtrHandle.cpp
            Base64.cpp
            BitstreamConverter.cpp
            BitstreamStats.cpp
            BooleanLogic.cpp
            CharsetConverter.cpp 
            CharsetDetection.cpp
            CPUInfo.cpp
            Crc32.cpp
            CryptThreading.cpp
            DatabaseUtils.cpp
            EndianSwap.cpp
            EdenVideoArtUpdater.cpp
            Environment.cpp
            Fanart.cpp
            fastmemcpy.c
            FileOperationJob.cpp
            FileUtils.cpp
            fstrcmp.c
            fft.cpp
            GLUtils.cpp
            GroupUtils.cpp
            HTMLTable.cpp
            HTMLUtil.cpp
            HttpHeader.cpp
            HttpParser.cpp
            HttpResponse.cpp
            InfoLoader.cpp
            JobManager.cpp
            JSONVariantParser.cpp
            JSONVariantWriter.cpp
            LabelFormatter.cpp
            LangCodeExpander.cpp
            LegacyPathTranslation.cpp
            log.cpp
            md5.cpp
            Mime.cpp
            Observer.cpp
            PerformanceSample.cpp
            PerformanceStats.cpp
            POUtils.cpp
            RecentlyAddedJob.cpp
            RegExp.cpp
            RingBuffer.cpp
            RssManager.cpp
            RssReader.cpp
            ScraperParser.cpp
            ScraperUrl.cpp
            Screenshot.cpp
            SeekHandler.cpp
            SortUtils.cpp
            Splash.cpp
            Stopwatch.cpp
            StreamDetails.cpp
            StreamUtils.cpp
            StringUtils.cpp
            StringValidation.cpp
            SystemInfo.cpp
            TextSearch.cpp
            TimeSmoother.cpp
            TimeUtils.cpp
            TuxBoxUtil.cpp
            URIUtils.cpp
            UrlOptions.cpp
            Utf8Utils.cpp
            Variant.cpp
            Vector.cpp
            Weather.cpp
            XBMCTinyXML.cpp
            XMLUtils.cpp
            XSLTUtils.cpp)


if(ARM)
 enable_language(ASM)
 list(APPEND SOURCES fastmemcpy-arm.S)
endif()

# The large map trips the clang optimizer
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set_source_files_properties(Mime.cpp PROPERTIES COMPILE_FLAGS -O0)
endif()

core_add_library(utils)
add_dependencies(utils libcpluff ffmpeg)
