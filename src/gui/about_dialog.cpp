// SPDX-License-Identifier: GPL-3.0-or-later

#include "monkey_about.hpp"

#include <wx/hyperlink.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dcbuffer.h>

BEGIN_EVENT_TABLE(MonkeyAbout, wxDialog)
EVT_PAINT(MonkeyAbout::OnPaint)
END_EVENT_TABLE()

MonkeyAbout::MonkeyAbout(
   wxWindow *parent, 
   const wxString &title, 
   const wxSize &size
): wxDialog(parent, wxID_ANY, title, wxDefaultPosition, size) {
   wxSize clientSz = GetClientSize();
   new wxButton(this, wxID_OK, _("&Close"), wxPoint(clientSz.x - 135, clientSz.y - 30), wxSize(130, 25));

   wxHyperlinkCtrl *url = new wxHyperlinkCtrl(this, wxID_ANY, _("GitHub page"), MM_GITHUB_REPO_URL);
   url->SetPosition(wxPoint(200, 195));

   doc_moore = new wxImage(212, 269);
   doc_moore->LoadFile(getResourcePath(wxT("ui/doctor_moore.png")), wxBITMAP_TYPE_PNG);
}

MonkeyAbout::~MonkeyAbout () {
   delete doc_moore;
}

void MonkeyAbout::OnPaint(
   wxPaintEvent &WXUNUSED(event)
) {
   wxPaintDC dc(this);

   wxSize clientsz = GetClientSize();
   dc.DrawBitmap(wxBitmap(*doc_moore), 0, clientsz.y - 269, true);

   dc.SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
   dc.DrawText(wxT("MONKEY-MOORE"), 198, 50);

   dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
   dc.DrawText(wxString::Format(_("Version %s (%s)"), MM_VERSION, MM_BUILD), 200, 65);
   dc.DrawText(_("2007-2026, writen by Darkl0rd."), 200, 90);
   dc.DrawText(_("All rights reserved."), 200, 105);
   dc.DrawText(_("This is a free software and it's"), 200, 130);
   dc.DrawText(_("under GNU's General Public"), 200, 145);
   dc.DrawText(_("License (GPL)."), 200, 160);
}