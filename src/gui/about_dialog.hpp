// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABOUT_DIALOG_HPP
#define ABOUT_DIALOG_HPP

#include "constants.hpp"
#include "filesystem_utils.hpp"

#include <wx/image.h>
#include <wx/dialog.h>
#include <wx/event.h>

/**
* Implements the about dialog box, which displays information about the program.
* ie: version, build, license, author(s).
*/
class MonkeyAbout : public wxDialog
{
public:
   MonkeyAbout (wxWindow* parent, const wxString& title, const wxSize& size = wxDefaultSize);

   ~MonkeyAbout ();

   void OnPaint(wxPaintEvent &WXUNUSED(event));

private:
   wxImage *doc_moore;

   DECLARE_EVENT_TABLE()
};

#endif
