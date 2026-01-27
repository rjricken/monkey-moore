// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MONKEY_OPTIONS_HPP
#define MONKEY_OPTIONS_HPP

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "constants.hpp"
#include "monkey_prefs.hpp"

#include <thread>
#include <wx/spinctrl.h>

/**
* Implements the about dialog box, which displays information about the program.
* ie: version, build, license, author(s).
*/
class MonkeyOptions : public wxDialog
{
public:
   MonkeyOptions (wxWindow *parent, const wxString &title, MonkeyPrefs &pref, const wxSize &size = wxDefaultSize) :
   wxDialog(parent, wxID_ANY, title, wxDefaultPosition, size), prefs(pref)
   {
      wxStaticBoxSizer *general_sz = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("General")), wxVERTICAL);
      wxCheckBox *always_center = new wxCheckBox(this, MonkeyOptions_AlwaysCenter, _(" Always center main window on screen"));
      wxCheckBox *remember_size = new wxCheckBox(this, MonkeyOptions_RememberSize, _(" Remember window size"));
      wxCheckBox *remember_pos = new wxCheckBox(this, MonkeyOptions_RememberPos, _(" Remember window position"));
      wxCheckBox *remember_ui = new wxCheckBox(this, MonkeyOptions_RememberUI, _(" Remember user interface options"));
      general_sz->Add(always_center, wxSizerFlags().Left().Border(wxALL, 2));
      general_sz->Add(remember_size, wxSizerFlags().Left().Border(wxALL, 2));
      general_sz->Add(remember_pos, wxSizerFlags().Left().Border(wxALL, 2));
      general_sz->Add(remember_ui, wxSizerFlags().Left().Border(wxALL, 2));

      wxBoxSizer *offset_sz = new wxBoxSizer(wxHORIZONTAL);
      wxStaticText *off_label = new wxStaticText(this, wxID_ANY, _("Offset mode: "));
      wxRadioButton *off_hex = new wxRadioButton(this, MonkeyOptions_OffsetHex, _(" hex"));
      wxRadioButton *off_dec = new wxRadioButton(this, MonkeyOptions_OffsetDec, _(" decimal"));
      offset_sz->Add(off_label, wxSizerFlags().Left().Border(wxTOP, 2));
      offset_sz->Add(off_hex, wxSizerFlags().Left().Border(wxALL, 2));
      offset_sz->Add(off_dec, wxSizerFlags().Left().Border(wxALL, 2));

      wxBoxSizer *preview_sz = new wxBoxSizer(wxHORIZONTAL);
      wxStaticText *prvw_label = new wxStaticText(this, wxID_ANY, _("Preview width: "));
      wxSpinCtrl *prvw_width = new wxSpinCtrl(this, MonkeyOptions_PreviewWidth, wxEmptyString, wxDefaultPosition, wxSize(50, 20));
      wxStaticText *prvw_unit = new wxStaticText(this, wxID_ANY, _(" characters"));
      preview_sz->Add(prvw_label, wxSizerFlags().Left().Border(wxTOP, 5));
      preview_sz->Add(prvw_width, wxSizerFlags().Left().Border(wxALL, 2));
      preview_sz->Add(prvw_unit, wxSizerFlags().Left().Border(wxTOP, 5));

      prvw_width->SetRange(20, 50);

      wxStaticBoxSizer *display_sz = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Display")), wxVERTICAL);
      display_sz->Add(preview_sz, wxSizerFlags().Border(wxLEFT, 2));
      display_sz->Add(offset_sz, wxSizerFlags().Border(wxLEFT, 2));

      wxBoxSizer *searchbuf_sz = new wxBoxSizer(wxHORIZONTAL);
      wxStaticText *sb_label = new wxStaticText(this, wxID_ANY, _("Memory pool: "));
      wxSpinCtrl *sb_size = new wxSpinCtrl(this, MonkeyOptions_MemoryPool, wxEmptyString, wxDefaultPosition, wxSize(50, 20));
      wxStaticText *sb_unit = new wxStaticText(this, wxID_ANY, _(" MB"));
      searchbuf_sz->Add(sb_label, wxSizerFlags().Left().Border(wxTOP, 5));
      searchbuf_sz->Add(sb_size, wxSizerFlags().Left().Border(wxALL, 2));
      searchbuf_sz->Add(sb_unit, wxSizerFlags().Left().Border(wxTOP, 5));

      sb_size->SetRange(8, 64);

      wxBoxSizer *smt_sz = new wxBoxSizer(wxHORIZONTAL);
      wxStaticText *smt_label = new wxStaticText(this, wxID_ANY, _("Search threads: "));
      wxSpinCtrl *smt_numthreads = new wxSpinCtrl(this, MonkeyOptions_MaxNumThreads, wxEmptyString, wxDefaultPosition, wxSize(50, 20));
      wxStaticText *smt_units = new wxStaticText(this, wxID_ANY, _(" threads"));
      smt_sz->Add(smt_label, wxSizerFlags().Left().Border(wxTOP, 5));
      smt_sz->Add(smt_numthreads, wxSizerFlags().Left().Border(wxALL, 2));
      smt_sz->Add(smt_units, wxSizerFlags().Left().Border(wxTOP, 5));

      smt_numthreads->SetRange(1, 16);
      smt_numthreads->SetValue(std::min<uint16_t>(std::thread::hardware_concurrency(), 16));

      wxStaticBoxSizer *perf_sz = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Performance")), wxVERTICAL);
      perf_sz->Add(searchbuf_sz, wxSizerFlags().Border(wxLEFT, 2));
      perf_sz->Add(smt_sz, wxSizerFlags().Border(wxLEFT, 2));

      wxBoxSizer *buttons_sz = new wxBoxSizer(wxHORIZONTAL);
      wxButton *ok = new wxButton(this, wxID_OK, _("Ok"));
      wxButton *cancel = new wxButton(this, wxID_CANCEL, _("Cancel"));
      buttons_sz->AddStretchSpacer(1);
      buttons_sz->Add(ok, wxSizerFlags().Border(wxALL, 2));
      buttons_sz->Add(cancel, wxSizerFlags().Border(wxALL, 2));

      wxBoxSizer *global_sz = new wxBoxSizer(wxVERTICAL);
      global_sz->Add(general_sz, wxSizerFlags().Expand().Border(wxTOP | wxLEFT | wxRIGHT, 6));
      global_sz->AddSpacer(5);
      global_sz->Add(display_sz, wxSizerFlags().Expand().Border(wxLEFT | wxRIGHT, 6));
      global_sz->AddSpacer(5);
      global_sz->Add(perf_sz, wxSizerFlags().Expand().Border(wxLEFT | wxRIGHT, 6));
      global_sz->AddSpacer(5);
      global_sz->AddStretchSpacer(1);
      global_sz->Add(buttons_sz, wxSizerFlags().Expand().Border(wxBOTTOM | wxLEFT | wxRIGHT, 6));

      SetSizer(global_sz);

      always_center->SetValue(prefs.getBool(wxT("settings/ui-center-window")));
      remember_size->SetValue(prefs.getBool(wxT("settings/ui-remember-size")));
      remember_pos->SetValue(prefs.getBool(wxT("settings/ui-remember-position")));
      remember_ui->SetValue(prefs.getBool(wxT("settings/ui-remember-state")));

      prvw_width->SetValue(prefs.get(wxT("settings/display-preview-width")));
      prefs.getBool(wxT("settings/display-offset-mode"), wxT("hex")) ? off_hex->SetValue(true) : off_dec->SetValue(true);

      sb_size->SetValue(wxString::Format(wxT("%d"), prefs.getInt(wxT("settings/perf-memory-pool")) / 1048576));
   }

   ~MonkeyOptions () {
   }

   void OnOk (wxCommandEvent &WXUNUSED(event))
   {
      prefs.setBool(wxT("settings/ui-center-window"), dynamic_cast <wxCheckBox *> (FindWindowById(MonkeyOptions_AlwaysCenter))->GetValue());
      prefs.setBool(wxT("settings/ui-remember-size"), dynamic_cast <wxCheckBox *> (FindWindowById(MonkeyOptions_RememberSize))->GetValue());
      prefs.setBool(wxT("settings/ui-remember-position"), dynamic_cast <wxCheckBox *> (FindWindowById(MonkeyOptions_RememberPos))->GetValue());
      prefs.setBool(wxT("settings/ui-remember-state"), dynamic_cast <wxCheckBox *> (FindWindowById(MonkeyOptions_RememberUI))->GetValue());

      wxRadioButton *hex_offset = dynamic_cast <wxRadioButton *> (FindWindowById(MonkeyOptions_OffsetHex));
      prefs.set(wxT("settings/display-offset-mode"), hex_offset->GetValue() ? wxT("hex") : wxT("dec"));

      wxSpinCtrl *prvw_width = dynamic_cast <wxSpinCtrl *> (FindWindowById(MonkeyOptions_PreviewWidth));
      prefs.setInt(wxT("settings/display-preview-width"), prvw_width->GetValue());

      wxSpinCtrl *sb_size = dynamic_cast <wxSpinCtrl *> (FindWindowById(MonkeyOptions_MemoryPool));
      prefs.setInt(wxT("settings/perf-memory-pool"), sb_size->GetValue() * 1048576);

      auto *numThreads = dynamic_cast<wxSpinCtrl *>(FindWindowById(MonkeyOptions_MaxNumThreads));
      prefs.setInt(wxT("settings/perf-search-threads"), numThreads->GetValue());

      Close();
   }

private:
   MonkeyPrefs &prefs;

   DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(MonkeyOptions, wxDialog)
   EVT_BUTTON(wxID_OK, MonkeyOptions::OnOk)
END_EVENT_TABLE()

#endif //~MONKEY_OPTIONS_HPP
