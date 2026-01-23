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

#ifndef MONKEY_SEQS_HPP
#define MONKEY_SEQS_HPP

#include <wx/wxprec.h>

#ifdef __BORLANDC__
   #pragma hdrstop
#endif

#ifndef WX_PRECOMP
   #include <wx/wx.h>
#endif

#include "constants.hpp"
#include "monkey_app.hpp"
#include "monkey_prefs.hpp"

#include <wx/dataview.h>
#include <wx/file.h>
#include <vector>
#include <utility>

/**
* -
*/
class MonkeySeqs : public wxDialog
{
public:
   MonkeySeqs (wxWindow *parent, const wxString &title, MonkeyPrefs &pref, wxImageList &imgs, const wxSize& size = wxDefaultSize) :
   wxDialog(parent, wxID_ANY, title, wxDefaultPosition, size), prefs(pref)
   {
      wxBoxSizer *global_sz = new wxBoxSizer(wxVERTICAL);

      wxStaticText *header = new wxStaticText(this, wxID_ANY, _("Slowly double-click or press SPACE on a column to edit:"));
      wxDataViewListCtrl *datavw = new wxDataViewListCtrl(this, MonkeySeqs_DataTable);

      datavw->AppendTextColumn(wxEmptyString, wxDATAVIEW_CELL_INERT, 20, wxALIGN_CENTER);
      datavw->AppendTextColumn(_("Name"), wxDATAVIEW_CELL_EDITABLE, 150);
      datavw->AppendTextColumn(_("Sequence"), wxDATAVIEW_CELL_EDITABLE, 240);

      wxBoxSizer *buttons_sz = new wxBoxSizer(wxHORIZONTAL);
      wxButton *addBtn = new wxButton(this, MonkeySeqs_AddNew, _("Add new item"), wxDefaultPosition, wxSize(20, 15), wxBU_NOTEXT);
      wxButton *removeBtn = new wxButton(this, MonkeySeqs_Remove, _("Remove item"), wxDefaultPosition, wxSize(20, 15), wxBU_NOTEXT | wxBU_EXACTFIT);
      wxButton *saveBtn = new wxButton(this, MonkeySeqs_SaveChanges, _("Save changes"), wxDefaultPosition, wxSize(20, 15), wxBU_NOTEXT | wxBU_EXACTFIT);
      wxButton *cancelBtn = new wxButton(this, MonkeySeqs_Cancel, _("Cancel"));

      addBtn->SetBitmap(imgs.GetBitmap(MonkeyBmp_New));
      removeBtn->SetBitmap(imgs.GetBitmap(MonkeyBmp_TrashCan));
      removeBtn->SetBitmapDisabled(imgs.GetBitmap(MonkeyBmp_TrashCanGrayed));
      saveBtn->SetBitmap(imgs.GetBitmap(MonkeyBmp_SaveFile));

      buttons_sz->Add(addBtn, wxSizerFlags().Left().Expand().Shaped());
      buttons_sz->AddSpacer(5);
      buttons_sz->Add(removeBtn, wxSizerFlags().Left().Expand().Shaped());
      buttons_sz->AddStretchSpacer(1);
      buttons_sz->Add(saveBtn, wxSizerFlags().Expand().Shaped());
      buttons_sz->AddSpacer(5);
      buttons_sz->Add(cancelBtn, wxSizerFlags());

      global_sz->Add(header, wxSizerFlags().Border(wxTOP | wxLEFT | wxRIGHT, 10));
      global_sz->Add(datavw, wxSizerFlags(1).Expand().Border(wxALL, 10));
      global_sz->Add(buttons_sz, wxSizerFlags().Expand().Border(wxLEFT | wxBOTTOM | wxRIGHT, 10));
      SetSizer(global_sz);

      addBtn->SetToolTip(_("Add a new sequence"));
      removeBtn->SetToolTip(_("Remove an item from the list"));
      saveBtn->SetToolTip(_("Save changes"));

      const auto &seqs = prefs.getCommonCharsetList();
      InitTableData(seqs);
   }

   ~MonkeySeqs () {
   }

   void OnRemove (wxCommandEvent &WXUNUSED(event))
   {
      wxDataViewListCtrl *dataVw = static_cast<wxDataViewListCtrl *>(FindWindowById(MonkeySeqs_DataTable));

      int selRow = dataVw->GetSelectedRow();

      if (dataVw->GetSelectedItemsCount())
         dataVw->DeleteItem(selRow);

      // correct row numbers
      if (selRow < dataVw->GetItemCount())
      {
         wxDataViewListStore *store = dataVw->GetStore();

         std::vector<std::pair<wxString, wxString>> temp;

         for (uint32_t i = selRow; i < store->GetCount(); ++i)
            store->SetValueByRow(wxVariant(static_cast<int>(i + 1)), i, 0);
      }
   }

   void OnAddItem (wxCommandEvent &WXUNUSED(event))
   {
      wxDataViewListCtrl *dataVw = static_cast<wxDataViewListCtrl *>(FindWindowById(MonkeySeqs_DataTable));

      wxVector<wxVariant> newItem(3, wxVariant(wxT("")));
      newItem[0] = wxVariant(dataVw->GetItemCount() + 1);

      dataVw->AppendItem(newItem);

      auto item = dataVw->RowToItem(dataVw->GetItemCount() - 1);
      auto *col = dataVw->GetColumn(1);

      dataVw->EditItem(item, col);
   }

   void OnUpdateUI (wxUpdateUIEvent &event)
   {
      wxDataViewListCtrl *dataVw = static_cast<wxDataViewListCtrl *>(FindWindowById(MonkeySeqs_DataTable));

      if (event.GetId() == MonkeySeqs_Remove)
         event.Enable(dataVw->GetSelectedItemsCount() ? true : false);
   }

   void OnCancel (wxCommandEvent &WXUNUSED(event))
   {
      Close();
   }

   void OnSave (wxCommandEvent &WXUNUSED(event))
   {
      wxDataViewListCtrl *dataVw = static_cast<wxDataViewListCtrl *>(FindWindowById(MonkeySeqs_DataTable));
      wxDataViewListStore *store = dataVw->GetStore();

      auto &seqs = prefs.getCommonCharsetList();
      std::vector<std::pair<wxString, wxString>> temp;

      for (uint32_t i = 0; i < store->GetCount(); ++i)
      {
         wxVariant name, seq;

         store->GetValueByRow(name, i, 1);
         store->GetValueByRow(seq, i, 2);

         if (!name.GetString().IsEmpty())
         {
            if (seq.GetString().IsEmpty())
            {
               wxString msg(_("\"%s\" - incomplete item\nEnter a valid character sequence to proceed."));
               ShowWarning(wxString::Format(msg, name.GetString()));

               return;
            }

            temp.push_back(std::make_pair(name.GetString(), seq.GetString()));
         }
      }

      seqs.clear();
      seqs.insert(seqs.begin(), temp.begin(), temp.end());

      prefs.saveSequences(wxGetApp().getSequencesFileName());

      Close();
   }

private:
   void InitTableData (const std::vector<std::pair<wxString, wxString>> &items)
   {
      wxDataViewListCtrl *datavw = static_cast<wxDataViewListCtrl *>(FindWindowById(MonkeySeqs_DataTable));
      wxVector<wxVariant> data;

      int counter = 1;

      for (auto i = items.begin(); i != items.end(); ++i)
      {
         data.push_back(wxVariant(counter++));
         data.push_back(wxVariant(i->first));
         data.push_back(wxVariant(i->second));

         datavw->AppendItem(data);
         data.clear();
      }
   }

   /**
   * Display info in a simple message box.
   * @param msg the message
   * @param capt an optional caption
   */
   void ShowInfo (const wxString &msg, const wxString &c = wxT("Monkey-Moore")) {
      wxMessageBox(msg, c, wxOK | wxICON_INFORMATION, this);
   }

   /**
   * Display a warning message.
   * @param msg the message
   * @param capt an optional caption
   */
   void ShowWarning (const wxString &msg, const wxString &c = _("Monkey-Moore - Error")) {
      wxMessageBox(msg, c, wxOK | wxICON_WARNING, this);
   }

   MonkeyPrefs &prefs;               /**< user preferences */

   DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(MonkeySeqs, wxDialog)
   EVT_BUTTON(MonkeySeqs_AddNew, MonkeySeqs::OnAddItem)
   EVT_BUTTON(MonkeySeqs_Remove, MonkeySeqs::OnRemove)
   EVT_BUTTON(MonkeySeqs_Cancel, MonkeySeqs::OnCancel)
   EVT_BUTTON(MonkeySeqs_SaveChanges, MonkeySeqs::OnSave)
   EVT_UPDATE_UI(MonkeySeqs_Remove, MonkeySeqs::OnUpdateUI)
END_EVENT_TABLE()

#endif //~MONKEY_SEQS_HPP
