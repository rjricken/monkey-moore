/*
 * Monkey-Moore - A simple and powerful relative search tool
 * Copyright (C) 2007 Ricardo J. Ricken (Darkl0rd)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

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
      url->SetPosition(wxPoint(192, 195));

      doc_moore = new wxImage(212, 269);
      doc_moore->LoadFile(wxT("images/doctor_moore.png"), wxBITMAP_TYPE_PNG);
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
      dc.DrawText(wxT("MONKEY-MOORE"), 190, 50);

      dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
      dc.DrawText(wxString::Format(_("Version %s (%s)"), MM_VERSION, MM_BUILD), 192, 65);
      dc.DrawText(_("2007-2014, writen by Darkl0rd."), 192, 90);
      dc.DrawText(_("All rights reserved."), 192, 105);
      dc.DrawText(_("This is a free software and it's"), 192, 130);
      dc.DrawText(_("under GNU's General Public"), 192, 145);
      dc.DrawText(_("License (GPL)."), 192, 160);
   }

private:
   wxImage *doc_moore;

   DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(MonkeyAbout, wxDialog)
   EVT_PAINT(MonkeyAbout::OnPaint)
END_EVENT_TABLE()

#endif //~MONKEY_ABOUT_HPP
