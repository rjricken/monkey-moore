// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILESYSTEM_UTILS_HPP
#define FILESYSTEM_UTILS_HPP

#include <wx/stdpaths.h>
#include <wx/filename.h>

inline wxString getResourcePath(const wxString &relativePath) {
   wxFileName executablePath(wxStandardPaths::Get().GetExecutablePath());
   wxString resourcePath = executablePath.GetPath();
   
   wxString fullPath = resourcePath + wxFileName::GetPathSeparator() + relativePath ;

   return fullPath;
}

#endif // FILESYSTEM_UTILS_HPP