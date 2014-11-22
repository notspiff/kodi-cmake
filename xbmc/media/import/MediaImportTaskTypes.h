#pragma once
/*
*      Copyright (C) 2014 Team XBMC
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

enum class MediaImportTaskType
{
  SourceRegistration,
  Retrieval,
  Changeset,
  Scraping,
  Synchronisation,
  Cleanup,
  Update,
  Removal
};

class MediaImportTaskTypes
{
public:
  static std::string ToString(MediaImportTaskType taskType)
  {
    switch (taskType)
    {
    case MediaImportTaskType::SourceRegistration:
      return "source registration";

    case MediaImportTaskType::Retrieval:
      return "retrieval";

    case MediaImportTaskType::Changeset:
      return "changeset";

    case MediaImportTaskType::Scraping:
      return "scraping";

    case MediaImportTaskType::Synchronisation:
      return "synchronisation";

    case MediaImportTaskType::Cleanup:
      return "cleanup";

    case MediaImportTaskType::Update:
      return "update";

    case MediaImportTaskType::Removal:
      return "removal";

    default:
      break;
    }

    return "unknown";
  }

private:
  MediaImportTaskTypes();
};
