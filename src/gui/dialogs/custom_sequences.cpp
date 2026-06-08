// SPDX-License-Identifier: GPL-3.0-or-later

#include "custom_sequences.hpp"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/dataview.h>
#include <wx/file.h>

wxBEGIN_EVENT_TABLE(CustomSequencesDialog, wxDialog)
   EVT_BUTTON(MonkeySeqs_AddNew, CustomSequencesDialog::OnAddItem)
   EVT_BUTTON(MonkeySeqs_Remove, CustomSequencesDialog::OnRemove)
   EVT_BUTTON(MonkeySeqs_Cancel, CustomSequencesDialog::OnCancel)
   EVT_BUTTON(MonkeySeqs_SaveChanges, CustomSequencesDialog::OnSave)
   EVT_UPDATE_UI(MonkeySeqs_Remove, CustomSequencesDialog::OnUpdateUI)
wxEND_EVENT_TABLE()

CustomSequencesDialog::CustomSequencesDialog(
   wxWindow *parent, 
   const wxString &title, 
   MonkeyPrefs &pref, 
   wxImageList &imgs, 
   const wxSize &size
) : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, size), prefs(pref) {
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

CustomSequencesDialog::~CustomSequencesDialog()
{
}

void CustomSequencesDialog::OnRemove(wxCommandEvent &WXUNUSED(event))
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

void CustomSequencesDialog::OnAddItem(wxCommandEvent &WXUNUSED(event))
{
   wxDataViewListCtrl *dataVw = static_cast<wxDataViewListCtrl *>(FindWindowById(MonkeySeqs_DataTable));

   wxVector<wxVariant> newItem(3, wxVariant(wxT("")));
   newItem[0] = wxVariant(dataVw->GetItemCount() + 1);

   dataVw->AppendItem(newItem);

   auto item = dataVw->RowToItem(dataVw->GetItemCount() - 1);
   auto *col = dataVw->GetColumn(1);

   dataVw->EditItem(item, col);
}

void CustomSequencesDialog::OnUpdateUI(wxUpdateUIEvent &event)
{
   wxDataViewListCtrl *dataVw = static_cast<wxDataViewListCtrl *>(FindWindowById(MonkeySeqs_DataTable));

   if (event.GetId() == MonkeySeqs_Remove)
      event.Enable(dataVw->GetSelectedItemsCount() ? true : false);
}

void CustomSequencesDialog::OnCancel(wxCommandEvent &WXUNUSED(event))
{
   Close();
}

void CustomSequencesDialog::OnSave(wxCommandEvent &WXUNUSED(event))
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

void CustomSequencesDialog::InitTableData(
   const std::vector<std::pair<wxString, wxString>> &items
) {
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