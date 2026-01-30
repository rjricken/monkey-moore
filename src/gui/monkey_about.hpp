// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MONKEY_ABOUT_HPP
#define MONKEY_ABOUT_HPP

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "constants.hpp"
#include "filesystem_utils.hpp"
#include <wx/hyperlink.h>

/**
* Implements the about dialog box, which displays information about the program.
* ie: version, build, license, author(s).
*/
class MonkeyAbout : public wxDialog
{
public:
   MonkeyAbout (wxWindow* parent, const wxString& title, const wxSize& size = wxDefaultSize) :
   wxDialog(parent, wxID_ANY, title, wxDefaultPosition, size)
   {
      wxSize clientSz = GetClientSize();
      new wxButton(this, wxID_OK, _("&Close"), wxPoint(clientSz.x - 135, clientSz.y - 30), wxSize(130, 25));

      wxHyperlinkCtrl *url = new wxHyperlinkCtrl(this, wxID_ANY, _("GitHub page"), MM_GITHUB_REPO_URL);
      url->SetPosition(wxPoint(200, 195));

      doc_moore = new wxImage(212, 269);
      doc_moore->LoadFile(getResourcePath(wxT("images/doctor_moore.png")), wxBITMAP_TYPE_PNG);
   }

   ~MonkeyAbout () {
      delete doc_moore;
   }

   /**
   * This method is called when the window needs to be repainted.
   * @param event not used
   */
   void OnPaint(wxPaintEvent &WXUNUSED(event))
   {
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

private:
   wxImage *doc_moore;

   DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(MonkeyAbout, wxDialog)
   EVT_PAINT(MonkeyAbout::OnPaint)
END_EVENT_TABLE()

#endif //~MONKEY_ABOUT_HPP
