// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOM_SEQUENCES_HPP
#define CUSTOM_SEQUENCES_HPP

#include "../constants.hpp"
#include "../monkey_app.hpp"
#include "../monkey_prefs.hpp"

#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/imaglist.h>
#include <wx/msgdlg.h>
#include <vector>
#include <utility>

class CustomSequencesDialog : public wxDialog {
public:
   CustomSequencesDialog (wxWindow *parent, const wxString &title, MonkeyPrefs &pref, wxImageList &imgs, const wxSize& size = wxDefaultSize);
   ~CustomSequencesDialog ();

   void OnRemove (wxCommandEvent &WXUNUSED(event));
   void OnAddItem (wxCommandEvent &WXUNUSED(event));
   void OnUpdateUI (wxUpdateUIEvent &event);
   void OnCancel (wxCommandEvent &WXUNUSED(event));
   void OnSave (wxCommandEvent &WXUNUSED(event));

private:
   void InitTableData (const std::vector<std::pair<wxString, wxString>> &items);

   void ShowInfo (const wxString &msg, const wxString &c = wxT("Monkey-Moore")) {
      wxMessageBox(msg, c, wxOK | wxICON_INFORMATION, this);
   }

   void ShowWarning (const wxString &msg, const wxString &c = _("Monkey-Moore - Error")) {
      wxMessageBox(msg, c, wxOK | wxICON_WARNING, this);
   }

   MonkeyPrefs &prefs;

   DECLARE_EVENT_TABLE()
};

#endif
