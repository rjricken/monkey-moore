// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MONKEY_TABLE_HPP
#define MONKEY_TABLE_HPP

#include <wx/wxprec.h>

#ifdef __BORLANDC__
   #pragma hdrstop
#endif

#ifndef WX_PRECOMP
   #include <wx/wx.h>
#endif

#include "constants.hpp"
#include "byteswap.hpp"
#include "monkey_prefs.hpp"

#include <vector>
#include <limits>
#include <wx/dataview.h>
#include <wx/file.h>

/**
* Implements the table creation dialog box, which displays information
* about the created table file and makes it possible to the user to edit
* its contents before saving in the specified format and encoding.
*/
class MonkeyTable : public wxDialog
{
public:
   MonkeyTable (wxWindow *parent, const wxString &title, MonkeyPrefs &pref, wxImageList &imgs, const wxSize& size = wxDefaultSize) :
   wxDialog(parent, wxID_ANY, title, wxDefaultPosition, size), prefs(pref)
   {
      filters.push_back(_("Thingy table file (*.tbl)|*.tbl"));

      wxBoxSizer *global_sz = new wxBoxSizer(wxVERTICAL);

      wxStaticText *header = new wxStaticText(this, wxID_ANY, _("You can edit the table values below before saving:"));
      wxDataViewListCtrl *datavw = new wxDataViewListCtrl(this, MonkeyTable_DataTable);

      datavw->AppendTextColumn(_("Byte"), wxDATAVIEW_CELL_EDITABLE);
      datavw->AppendTextColumn(_("Value"), wxDATAVIEW_CELL_EDITABLE);

      wxBoxSizer *tblnm_sz = new wxBoxSizer(wxHORIZONTAL);
      wxStaticText *flnmhd = new wxStaticText(this, wxID_ANY, _("Name "));
      wxTextCtrl *flnm = new wxTextCtrl(this, MonkeyTable_FileName);
      wxBitmapButton *browse = new wxBitmapButton(this, MonkeyTable_SaveTable, imgs.GetBitmap(MonkeyBmp_SaveFile));

      browse->SetDefault();

      tblnm_sz->Add(flnmhd, wxSizerFlags().Border(wxRIGHT, 5).Border(wxTOP, 4));
      tblnm_sz->Add(flnm, wxSizerFlags(1).Expand().Border(wxRIGHT, 5));
      tblnm_sz->Add(browse);

      wxBoxSizer *tblopt_sz = new wxBoxSizer(wxHORIZONTAL);
      wxStaticText *flfmthd = new wxStaticText(this, wxID_ANY, _("Format "));
      wxChoice *flfmt = new wxChoice(this, MonkeyTable_FileFmt);
      wxStaticText *flenchd = new wxStaticText(this, wxID_ANY, _("Encoding "));
      wxChoice *flenc = new wxChoice(this, MonkeyTable_FileEnc);

      flfmt->AppendString(_("Thingy table file (*.tbl)"));
      flfmt->SetSelection(0);

      flenc->AppendString(_("ANSI (ISO-8859-1)"));
      flenc->AppendString(_("Unicode (UTF-8)"));
      flenc->AppendString(_("Unicode (UTF-16)"));
      //flenc->AppendString(_("Shift-JIS (SJIS)"));
      flenc->SetSelection(1);

      tblopt_sz->Add(flfmthd, wxSizerFlags().Border(wxRIGHT, 5).Border(wxTOP, 4));
      tblopt_sz->Add(flfmt, wxSizerFlags().Border(wxRIGHT, 5));
      tblopt_sz->AddStretchSpacer(1);
      tblopt_sz->Add(flenchd, wxSizerFlags().Border(wxRIGHT, 5).Border(wxTOP, 4).Right());
      tblopt_sz->Add(flenc, wxSizerFlags().Right());

      global_sz->Add(header, wxSizerFlags().Border(wxTOP | wxLEFT | wxRIGHT, 10));
      global_sz->Add(datavw, wxSizerFlags(1).Expand().Border(wxALL, 10));
      global_sz->Add(tblnm_sz, wxSizerFlags().Expand().Border(wxRIGHT | wxLEFT, 10));
      global_sz->Add(tblopt_sz, wxSizerFlags().Expand().Border(wxALL, 10));
      SetSizer(global_sz);
   }

   ~MonkeyTable () {
   }

   /**
   * Method called when the save button is pressed.
   * Gets the (possibly) edited values from the data table and saves it to the specified file.
   * @param event not used
   */
   void OnSaveTable (wxCommandEvent &WXUNUSED(event))
   {
      const wxString fn = dynamic_cast <wxTextCtrl *>(FindWindowById(MonkeyTable_FileName))->GetValue();
      const int filter = dynamic_cast <wxChoice *>(FindWindowById(MonkeyTable_FileFmt))->GetSelection();
      const int enc = dynamic_cast <wxChoice *>(FindWindowById(MonkeyTable_FileEnc))->GetSelection();

      const long flags = wxFD_SAVE | wxFD_OVERWRITE_PROMPT;
      wxFileDialog dialog(this, MM_MSG_SAVETBL, prefs.get(wxT("directories/save-table")), fn, filters[filter], flags);

      wxDataViewListCtrl *datavw = dynamic_cast <wxDataViewListCtrl *> (FindWindowById(MonkeyTable_DataTable));
      wxDataViewListStore *store = datavw->GetStore();
      wxString buf;

      // iterates through the data table rows
      for (int i = 0; i < static_cast<int>(store->GetCount()); i++)
      {
         wxVariant byte, value;

         store->GetValueByRow(byte, i, 0);
         store->GetValueByRow(value, i, 1);

         buf += byte.GetString() + wxT("=") + value.GetString() + wxT("\r\n");
      }

      // checks if the table contains non-ANSI characters
      if (enc == 0 && !buf.IsAscii())
         return ShowWarning(_("Can't save in the specified encoding:\nThe table contains non-ANSI characters."));

      if (dialog.ShowModal() == wxID_CANCEL) return;

      prefs.set(wxT("directories/save-table"), dialog.GetDirectory());
      wxFile tbl(dialog.GetPath(), wxFile::write);

      if (!tbl.IsOpened())
         return ShowWarning(_("The table file couldn't be created. Try again."));

      switch (enc)
      {
         // encodes the file in ANSI (ISO-8859-1)
         case 0: tbl.Write(buf, wxCSConv(wxT("iso-8859-1")));
                 break;

         // encodes the file in Unicode (UTF-8)
         case 1: tbl.Write(buf, wxConvUTF8);
                 break;

         // encodes the file in Unicode (UTF-16)
         case 2: tbl.Write(buf.data().AsWChar(), buf.size() * sizeof(wxChar));
                 break;

         // Does not work. Delved deep in the wx implementation and didn't find shit yet.
         // So... WIP.
         /*case 3: {
            wxCSConv enc(wxFONTENCODING_SHIFT_JIS);
            
            if (enc.IsOk())
               tbl.Write(buf, enc);
            else
               ShowWarning(_("Your system does not support conversion to Shift JIS."));
         }
         break;*/
      }

      tbl.Close();
      ShowInfo(_("Table saved successfully."), wxT("Monkey-Moore"));

      Close();
   }

   /**
   * Fill the data table with the results obtained from the search.
   * @param d search result
   */
   template <typename _Type>
   void InitTableData (const typename MonkeyMoore<_Type>::equivalency_map &d, bool isLittleEndian)
   {
      std::map<wxString, wxString> tbldata;

      uint32_t numBytes = static_cast<uint32_t>(sizeof(_Type)) * 2;
      wxString bytefmt = wxString::Format(wxT("%%0%dX"), numBytes);

      for (auto i = d.begin(); i != d.end(); i++)
      {
         // when dealing with ASCII searches, we must generate the missing characters
         if (i->first == wxT('A') || i->first == wxT('a'))
         {
            for (int j = 0, counter = i->second; j < 26; j++, counter++)
            {
               if (counter == std::numeric_limits<_Type>::max() + 1)
                  counter = 0;

               // swap the bytes according to the endianness of the results
               _Type value = static_cast<_Type>(counter);
               value = isLittleEndian ? swap_on_le<_Type>(value) : swap_on_be<_Type>(value);

               tbldata[wxString::Format(bytefmt, value)] = wxString::Format(wxT("%c"), i->first + j);
            }
         }
         else
         {
            _Type value = isLittleEndian ? swap_on_le<_Type>(i->second) : swap_on_be<_Type>(i->second);
            tbldata[wxString::Format(bytefmt, value)] = wxString::Format(wxT("%c"), static_cast<int>(i->first));
         }
      }

      AppendItemsToTable(tbldata);
   }

private:
   /**
   * Appends a sorted list of values to the data table.
   * @param items sorted values
   */
   void AppendItemsToTable(const std::map<wxString, wxString> &items)
   {
      wxDataViewListCtrl *datavw = dynamic_cast <wxDataViewListCtrl *> (FindWindowById(MonkeyTable_DataTable));
      wxVector<wxVariant> data;

      for (std::map<wxString, wxString>::const_iterator i = items.begin(); i != items.end(); i++)
      {
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
   void ShowWarning (const wxString &msg, const wxString &c = MM_ERRORCAPTION) {
      wxMessageBox(msg, c, wxOK | wxICON_WARNING, this);
   }

   MonkeyPrefs &prefs;               /**< user preferences */
   std::vector <wxString> filters;   /**< file format filters */

   DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(MonkeyTable, wxDialog)
   EVT_BUTTON(MonkeyTable_SaveTable, MonkeyTable::OnSaveTable)
END_EVENT_TABLE()

#endif //~MONKEY_TABLE_HPP
