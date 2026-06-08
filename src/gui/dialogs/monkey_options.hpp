// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MONKEY_OPTIONS_HPP
#define MONKEY_OPTIONS_HPP

#include "../constants.hpp"
#include "../monkey_prefs.hpp"

#include <wx/event.h>
#include <wx/dialog.h>

/**
* Implements the about dialog box, which displays information about the program.
* ie: version, build, license, author(s).
*/
class MonkeyOptions : public wxDialog
{
public:
   MonkeyOptions (wxWindow *parent, const wxString &title, MonkeyPrefs &pref, const wxSize &size = wxDefaultSize);

   ~MonkeyOptions ();

   void OnOk (wxCommandEvent &WXUNUSED(event));

private:
   MonkeyPrefs &prefs;

   DECLARE_EVENT_TABLE()
};

#endif //~MONKEY_OPTIONS_HPP
