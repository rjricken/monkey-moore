// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include "../constants.hpp"
#include "../monkey_prefs.hpp"

#include <wx/event.h>
#include <wx/dialog.h>

/**
* Implements the about dialog box, which displays information about the program.
* ie: version, build, license, author(s).
*/
class SettingsDialog : public wxDialog
{
public:
   SettingsDialog (wxWindow *parent, const wxString &title, MonkeyPrefs &pref, const wxSize &size = wxDefaultSize);

   ~SettingsDialog ();

   void OnOk (wxCommandEvent &WXUNUSED(event));

private:
   MonkeyPrefs &prefs;

   DECLARE_EVENT_TABLE()
};

#endif
