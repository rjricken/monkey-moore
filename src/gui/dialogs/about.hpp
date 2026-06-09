// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABOUT_DIALOG_HPP
#define ABOUT_DIALOG_HPP

#include "../constants.hpp"

#include <wx/image.h>
#include <wx/dialog.h>
#include <wx/event.h>

/**
* Implements the about dialog box, which displays information about the program.
* ie: version, build, license, author(s).
*/
class AboutDialog : public wxDialog
{
public:
   AboutDialog (wxWindow* parent, const wxString& title, const wxSize& size = wxDefaultSize);

   ~AboutDialog ();

   void OnPaint(wxPaintEvent &WXUNUSED(event));

private:
   wxImage *doc_moore;

   DECLARE_EVENT_TABLE()
};

#endif
