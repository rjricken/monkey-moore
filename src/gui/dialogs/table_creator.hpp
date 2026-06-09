// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABLE_CREATOR_HPP
#define TABLE_CREATOR_HPP

#include "../constants.hpp"
#include "../monkey_prefs.hpp"
#include "mmoore/monkey_moore.hpp"
#include <vector>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/imaglist.h>
#include <wx/msgdlg.h>

/**
* Implements the table creation dialog box, which displays information
* about the created table file and makes it possible to the user to edit
* its contents before saving in the specified format and encoding.
*/
class TableCreatorDialog : public wxDialog
{
public:
   TableCreatorDialog(
      wxWindow *parent, 
      const wxString &title, 
      MonkeyPrefs &pref, 
      wxImageList &imgs, 
      const wxSize& size = wxDefaultSize);

   ~TableCreatorDialog();

   /**
   * Method called when the save button is pressed.
   * Gets the (possibly) edited values from the data table and saves it to the specified file.
   * @param event not used
   */
   void OnSaveTable (wxCommandEvent &WXUNUSED(event));

   /**
   * Fill the data table with the results obtained from the search.
   * @param d search result
   */
   template <typename _Type>
   void InitTableData (const typename MonkeyMoore<_Type>::equivalency_map &d, bool isLittleEndian);

private:
   /**
   * Appends a sorted list of values to the data table.
   * @param items sorted values
   */
   void AppendItemsToTable(const std::map<wxString, wxString> &items);

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

#endif
