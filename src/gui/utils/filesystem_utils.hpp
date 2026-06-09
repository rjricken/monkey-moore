// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILESYSTEM_UTILS_HPP
#define FILESYSTEM_UTILS_HPP

#include <wx/stdpaths.h>
#include <wx/filename.h>

inline wxString getResourcePath(const wxString &relativePath) {
   wxString dataDir = wxStandardPaths::Get().GetDataDir();
   wxString fullPath = dataDir + wxFileName::GetPathSeparator() + relativePath;

   if (wxFileName::Exists(fullPath)) {
      return fullPath;
   }

   wxFileName devPath(wxStandardPaths::Get().GetExecutablePath());

   devPath.RemoveLastDir();
   devPath.AppendDir(wxT("assets"));

   return devPath.GetPath() + wxFileName::GetPathSeparator() + relativePath;
}

#endif // FILESYSTEM_UTILS_HPP