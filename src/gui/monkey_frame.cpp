// SPDX-License-Identifier: GPL-3.0-or-later

#include "mmoore/text_utils.hpp"
#include "monkey_frame.hpp"
#include "monkey_about.hpp"
#include "monkey_options.hpp"
#include "monkey_table.hpp"
#include "monkey_seqs.hpp"
#include "monkey_thread.hpp"
#include "filesystem_utils.hpp"

#include <wx/file.h>
#include <wx/tokenzr.h>
#include <wx/clipbrd.h> 
#include <algorithm>
#include <numeric>
#include <utility>
#include <string>
#include <memory>
#include <array>

#if !defined(__WXMSW__) && !defined(__WXPM__)
   #include "../resources/mmoore.xpm"
#endif

wxBEGIN_EVENT_TABLE(MonkeyFrame, wxFrame)
   EVT_BUTTON(MonkeyMoore_About, MonkeyFrame::OnAbout)
   EVT_BUTTON(MonkeyMoore_Advanced, MonkeyFrame::OnAdvanced)
   EVT_BUTTON(MonkeyMoore_Browse, MonkeyFrame::OnBrowse)
   EVT_BUTTON(MonkeyMoore_Cancel, MonkeyFrame::OnCancel)
   EVT_BUTTON(MonkeyMoore_CharsetList, MonkeyFrame::OnCharsetList)
   EVT_BUTTON(MonkeyMoore_Options, MonkeyFrame::OnOptions)
   EVT_BUTTON(MonkeyMoore_Search, MonkeyFrame::OnSearch)
   EVT_CLOSE(MonkeyFrame::OnClose)
   EVT_LIST_COL_BEGIN_DRAG(MonkeyMoore_Results, MonkeyFrame::OnResizeResultsListColumn)
   EVT_LIST_COL_DRAGGING(MonkeyMoore_Results, MonkeyFrame::OnResizeResultsListColumn)
   EVT_LIST_COL_END_DRAG(MonkeyMoore_Results, MonkeyFrame::OnResizeResultsListColumn)
   EVT_LIST_ITEM_ACTIVATED(MonkeyMoore_Results, MonkeyFrame::OnListItemActivated)
   EVT_LIST_ITEM_RIGHT_CLICK(MonkeyMoore_Results, MonkeyFrame::OnListItemRightClick)
   EVT_LIST_DELETE_ALL_ITEMS(MonkeyMoore_Results, MonkeyFrame::OnListDeleteAll)
   EVT_MENU(MonkeyMoore_CopyAddress, MonkeyFrame::OnCopyAddress)
   EVT_MENU(MonkeyMoore_ManageCharSeqs, MonkeyFrame::OnManageCharSeqs)
   EVT_RADIOBUTTON(MonkeyMoore_RelativeSearch, MonkeyFrame::OnSearchType)
   EVT_RADIOBUTTON(MonkeyMoore_ValueScanSearch, MonkeyFrame::OnSearchType)
   EVT_RADIOBUTTON(MonkeyMoore_8bitMode, MonkeyFrame::OnSearchMode)
   EVT_RADIOBUTTON(MonkeyMoore_16bitMode, MonkeyFrame::OnSearchMode)
   EVT_RADIOBUTTON(MonkeyMoore_ByteOrderBE, MonkeyFrame::OnByteOrder)
   EVT_RADIOBUTTON(MonkeyMoore_ByteOrderLE, MonkeyFrame::OnByteOrder)
   EVT_SIZE(MonkeyFrame::OnSize)
   EVT_SHOW(MonkeyFrame::OnShow)
   EVT_TEXT_ENTER(MonkeyMoore_KWord, MonkeyFrame::OnTextEnter)
   EVT_UPDATE_UI_RANGE(MonkeyMoore_Browse, MonkeyMoore_Cancel, MonkeyFrame::OnUpdateUI)
wxEND_EVENT_TABLE()

wxDEFINE_EVENT(mmEVT_SEARCHTHREAD_COMPLETED, wxThreadEvent);
wxDEFINE_EVENT(mmEVT_SEARCHTHREAD_UPDATE, wxThreadEvent);
wxDEFINE_EVENT(mmEVT_SEARCHTHREAD_ABORTED, wxThreadEvent);

MonkeyFrame::MonkeyFrame (const wxString &title, MonkeyPrefs &mprefs, const wxPoint &pos, const wxSize &size) :
wxFrame(0, wxID_ANY, title, pos, size, wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL), prefs(mprefs),
search_done(false), search_in_progress(false), search_was_aborted(false), advanced_shown(false),
searchmode_8bits(true), byteorder_little(true)
{
   SetIcon(wxICON(mmoore));
   wxValidator::SuppressBellOnError();

   wxImage buttonset(getResourcePath(wxT("images/buttons.png")), wxBITMAP_TYPE_PNG);
   const int num_images = buttonset.GetWidth() / 18;

   images.Create(18, 15, true, num_images);

   for (int i = 0; i < num_images; i++)
      images.Add(buttonset.GetSubImage(wxRect(i * 18, 0, 18, 15)));

   main_panel = new wxPanel(this, wxID_ANY);
   main_panel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));

   Bind(wxEVT_SYS_COLOUR_CHANGED, [this](wxSysColourChangedEvent &event) {
      main_panel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
      main_panel->Refresh();
      event.Skip();
   });

   // _________________________________________________________________________
   // File
   wxTextCtrl *fname = new wxTextCtrl(main_panel, MonkeyMoore_FName, wxEmptyString, wxDefaultPosition, wxSize(-1, 28), wxBORDER_DEFAULT);
   wxButton *browse = new wxButton(main_panel, MonkeyMoore_Browse, _("Browse"), wxDefaultPosition, wxSize(-1, 28), wxBU_NOTEXT | wxBU_EXACTFIT);

   browse->SetBitmap(images.GetBitmap(MonkeyBmp_OpenFile));
   browse->SetBitmapDisabled(images.GetBitmap(MonkeyBmp_OpenFileGrayed));

   wxStaticBoxSizer *filebox_sz = new wxStaticBoxSizer(new wxStaticBox(main_panel, wxID_ANY, _("File")), wxHORIZONTAL);
   filebox_sz->Add(fname, wxSizerFlags(1).Expand().Border().FixedMinSize());
   filebox_sz->Add(browse, wxSizerFlags().Border(wxALL ^ wxLEFT).Expand());
    
   // _________________________________________________________________________
   // Search Parameters 

   // -- search type
   wxRadioButton *searchtype_rs = new wxRadioButton(main_panel, MonkeyMoore_RelativeSearch, _(" Relative search"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
   wxRadioButton *searchtype_vsr = new wxRadioButton(main_panel, MonkeyMoore_ValueScanSearch, _(" Value scan relative"));
   
   searchtype_rs->SetValue(true);

   wxBoxSizer *searchtype_sz = new wxBoxSizer(wxHORIZONTAL);
   searchtype_sz->Add(searchtype_rs, wxSizerFlags().Border(wxRIGHT));
   searchtype_sz->Add(searchtype_vsr);

   // -- search keyword textinput
   wxTextCtrl *kword = new wxTextCtrl(main_panel, MonkeyMoore_KWord, wxT(""), wxDefaultPosition, wxSize(-1, 28), wxTE_PROCESS_ENTER);
   wxButton *search = new wxButton(main_panel, MonkeyMoore_Search, _("Search"), wxDefaultPosition, wxSize(-1, 28), wxBU_NOTEXT | wxBU_EXACTFIT);

   search->SetBitmap(images.GetBitmap(MonkeyBmp_Search));
   search->SetBitmapDisabled(images.GetBitmap(MonkeyBmp_SearchGrayed));

   wxBoxSizer *searchinput_sz = new wxBoxSizer(wxHORIZONTAL);
   searchinput_sz->Add(kword, wxSizerFlags(1).Border(wxRIGHT).Expand().FixedMinSize());
   searchinput_sz->Add(search, wxSizerFlags().Expand());

   // -- search options (controls below the keyword input)
   wxButton *advanced = new wxButton(main_panel, MonkeyMoore_Advanced, _("Advanced"), wxDefaultPosition, wxSize(-1, 28), wxBU_NOTEXT | wxBU_EXACTFIT);
   wxCheckBox *use_wc = new wxCheckBox(main_panel, MonkeyMoore_UseWC, _(" Enable Wildcards"));
   wxTextCtrl *wildcard = new wxTextCtrl(main_panel, MonkeyMoore_Wildcard, wxEmptyString, wxDefaultPosition, wxSize(30, 28));
   wxRadioButton *searchmode_8bit = new wxRadioButton(main_panel, MonkeyMoore_8bitMode, _(" 8-bit"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
   wxRadioButton *searchmode_16bit = new wxRadioButton(main_panel, MonkeyMoore_16bitMode, _(" 16-bit"));

   advanced->SetBitmap(images.GetBitmap(MonkeyBmp_ShowAdv));
   searchmode_8bit->SetValue(true);
   wildcard->WriteText(wxT("*"));
   wildcard->Disable();

   wxBoxSizer *searchopt_sz = new wxBoxSizer(wxHORIZONTAL);
   searchopt_sz->Add(use_wc, wxSizerFlags().Border(wxRIGHT).Center());
   searchopt_sz->Add(wildcard, wxSizerFlags().Border(wxRIGHT).Center());
   searchopt_sz->AddStretchSpacer(1);
   searchopt_sz->Add(searchmode_8bit, wxSizerFlags().Border(wxRIGHT).Center());
   searchopt_sz->Add(searchmode_16bit, wxSizerFlags().Border(wxRIGHT).Center());
   searchopt_sz->Add(advanced, wxSizerFlags().Expand());

   // -- search box
   wxStaticBoxSizer *searchbox_sz = new wxStaticBoxSizer(new wxStaticBox(main_panel, wxID_ANY, _("Search Parameters")), wxVERTICAL);
   searchbox_sz->Add(searchtype_sz, wxSizerFlags().Border(wxLEFT | wxTOP | wxRIGHT));
   searchbox_sz->Add(searchinput_sz, wxSizerFlags().Border().Expand());
   searchbox_sz->Add(searchopt_sz, wxSizerFlags(1).Border(wxLEFT | wxBOTTOM | wxRIGHT).Expand());
   searchbox_sz->AddSpacer(4);

   // _________________________________________________________________________
   // Advanced

   // -- custom character pattern row
   wxCheckBox *adv_enablepat = new wxCheckBox(main_panel, MonkeyMoore_EnableCP, _(" Character sequence:"));
   wxTextCtrl *char_pattern = new wxTextCtrl(main_panel, MonkeyMoore_CharPattern, wxEmptyString, wxDefaultPosition, wxSize(-1, 28));
   wxButton *charset_list = new wxButton(main_panel, MonkeyMoore_CharsetList, _("Sequences List"), wxDefaultPosition, wxSize(-1, 28), wxBU_NOTEXT | wxBU_EXACTFIT);

   // disabled at startup
   char_pattern->Disable();
   charset_list->Disable();
   charset_list->SetBitmap(images.GetBitmap(MonkeyBmp_Sequences));
   charset_list->SetBitmapDisabled(images.GetBitmap(MonkeyBmp_SequencesGrayed));

   wxBoxSizer *advancedopt_sz = new wxBoxSizer(wxHORIZONTAL);

   advancedopt_sz->Add(adv_enablepat, wxSizerFlags().Border(wxRIGHT).Center());
   advancedopt_sz->Add(char_pattern, wxSizerFlags(1).Border(wxRIGHT).Expand());
   advancedopt_sz->Add(charset_list, wxSizerFlags().Expand());

   // -- byte order row
   wxCheckBox *byteorder_enable = new wxCheckBox(main_panel, MonkeyMoore_EnableByteOrder, _(" Byte order:"));
   wxRadioButton *byteorder_le = new wxRadioButton(main_panel, MonkeyMoore_ByteOrderLE, _(" Little Endian"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
   wxRadioButton *byteorder_be = new wxRadioButton(main_panel, MonkeyMoore_ByteOrderBE, _(" Big Endian"));

   byteorder_enable->SetValue(true);

   wxBoxSizer *advbyteorder_sz = new wxBoxSizer(wxHORIZONTAL);
   advbyteorder_sz->Add(byteorder_enable, wxSizerFlags().Border(wxRIGHT));
   advbyteorder_sz->Add(byteorder_le, wxSizerFlags().Border(wxRIGHT));
   advbyteorder_sz->Add(byteorder_be, wxSizerFlags().Border(wxRIGHT));

   // -- advanced box
   wxStaticBoxSizer *advancedbox_sz = new wxStaticBoxSizer(new wxStaticBox(main_panel, wxID_ANY, _("Advanced")), wxVERTICAL);
   advancedbox_sz->Add(advancedopt_sz, wxSizerFlags().Border(wxLEFT | wxTOP | wxRIGHT).Expand());
   advancedbox_sz->Add(advbyteorder_sz, wxSizerFlags().Border());

   // _________________________________________________________________________
   // Results

   // -- result list options

   wxButton *createtbl = new wxButton(main_panel, MonkeyMoore_CreateTbl, _("Create Table"));
   wxButton *clear = new wxButton(main_panel, MonkeyMoore_Clear, _("Clear List"));
   wxButton *options = new wxButton(main_panel, MonkeyMoore_Options, _("Options"), wxDefaultPosition, wxSize(24, 23), wxBU_NOTEXT);
   wxButton *about = new wxButton(main_panel, MonkeyMoore_About, _("About"), wxDefaultPosition, wxSize(24, 23), wxBU_NOTEXT);
   wxStaticText *count_txt = new wxStaticText(main_panel, wxID_ANY, _("Results: "));
   wxStaticText *counter = new wxStaticText(main_panel, MonkeyMoore_Counter, wxT("0"), wxDefaultPosition, wxSize(30, -1), wxALIGN_RIGHT | wxST_NO_AUTORESIZE);

   options->SetBitmap(images.GetBitmap(MonkeyBmp_Options));
   options->SetBitmapDisabled(images.GetBitmap(MonkeyBmp_OptionsGrayed));
   about->SetBitmap(images.GetBitmap(MonkeyBmp_About));
   about->SetBitmapDisabled(images.GetBitmap(MonkeyBmp_AboutGrayed));

   wxBoxSizer *resultopt_sz = new wxBoxSizer(wxHORIZONTAL);
   resultopt_sz->Add(createtbl, wxSizerFlags().Border(wxRIGHT).Center());
   resultopt_sz->Add(clear, wxSizerFlags().Border(wxALL, 4).Left());
   resultopt_sz->Add(options, wxSizerFlags().Expand().Shaped().Border(wxALL, 4).FixedMinSize());
   resultopt_sz->Add(about, wxSizerFlags().Expand().Shaped().Border(wxALL, 4).FixedMinSize());
   resultopt_sz->AddStretchSpacer(1);
   resultopt_sz->Add(count_txt, wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
   resultopt_sz->Add(counter, wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
   resultopt_sz->AddSpacer(3);

   // result box
   wxListCtrl *results = new wxListCtrl(main_panel, MonkeyMoore_Results, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
   wxCheckBox *show_all = new wxCheckBox(main_panel, MonkeyMoore_AllResults, _(" Show repeated results"));

   results->InsertColumn(0, _("Offset"), wxLIST_FORMAT_LEFT, ResultListCol_Offset);
   results->InsertColumn(1, _("Values"), wxLIST_FORMAT_LEFT, ResultListCol_Values);
   results->InsertColumn(2, _("Preview"), wxLIST_FORMAT_LEFT, ResultListCol_Preview);

   wxStaticBoxSizer *resultbox_sz = new wxStaticBoxSizer(new wxStaticBox(main_panel, wxID_ANY, _("Results")), wxVERTICAL);
   resultbox_sz->Add(show_all, wxSizerFlags().Left().Border());
   resultbox_sz->Add(results, wxSizerFlags(1).Border(wxLEFT | wxRIGHT).Expand());
   resultbox_sz->Add(resultopt_sz, wxSizerFlags().Border().Expand());

   // _________________________________________________________________________
   // Search Progress
   
   // -- widgets shown below the progress bar
   wxStaticText *elapsed_time = new wxStaticText(main_panel, MonkeyMoore_ElapsedTime, wxT(""));
   wxBitmapButton *cancel_search = new wxBitmapButton(main_panel, MonkeyMoore_Cancel, images.GetBitmap(MonkeyBmp_Cancel));

   cancel_search->SetBitmapDisabled(images.GetBitmap(MonkeyBmp_CancelGrayed));
   
   wxBoxSizer * progressinfo_sz = new wxBoxSizer(wxHORIZONTAL);
   progressinfo_sz->Add(elapsed_time, wxSizerFlags().Center());
   progressinfo_sz->AddStretchSpacer(1);
   progressinfo_sz->Add(cancel_search, wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));

   wxGauge *progress = new wxGauge(main_panel, MonkeyMoore_Progress, 100);
   
   wxStaticBoxSizer *progress_sz = new wxStaticBoxSizer(new wxStaticBox(main_panel, wxID_ANY, _("Search Progress")), wxVERTICAL);
   progress_sz->Add(progress, wxSizerFlags().Border().Expand());
   progress_sz->Add(progressinfo_sz, wxSizerFlags().Border().Expand());

   // _________________________________________________________________________
   // Final layout 
   wxBoxSizer *global_sizer = new wxBoxSizer(wxVERTICAL);
   global_sizer->Add(filebox_sz, wxSizerFlags().Border(wxLEFT | wxRIGHT | wxTOP).Expand());
   global_sizer->AddSpacer(5);
   global_sizer->Add(searchbox_sz, wxSizerFlags().Border(wxLEFT | wxRIGHT).Expand());
   global_sizer->AddSpacer(5);
   global_sizer->Add(advancedbox_sz, wxSizerFlags().Border(wxLEFT | wxRIGHT).Expand());
   global_sizer->AddSpacer(5);
   global_sizer->Add(resultbox_sz, wxSizerFlags(1).Border(wxLEFT | wxRIGHT).Expand());
   global_sizer->AddSpacer(5);
   global_sizer->Add(progress_sz, wxSizerFlags().Border(wxLEFT | wxRIGHT | wxBOTTOM).Expand());

   main_panel->SetSizer(global_sizer);
   wxBoxSizer *frame_sizer = new wxBoxSizer(wxVERTICAL);
   frame_sizer->Add(main_panel, 1, wxEXPAND);

   SetSizer(frame_sizer);
   Layout();

   progressBoxHeight = progress_sz->GetSize().y + 5;
   
   // hide progress box/advanced box
   global_sizer->Show(progress_sz, false);
   global_sizer->Show(advancedbox_sz, false);

   // tooltips for widgets whose meaning may not be obvious.
   advanced->SetToolTip(_("Advanced options"));
   options->SetToolTip(_("Settings panel"));
   about->SetToolTip(_("About Monkey-Moore"));
   charset_list->SetToolTip(_("List of predefined character sequences"));

   byteorder_enable->SetToolTip(_("Endianness format\n"
                                  "(used only with multi-byte searches)"));

   adv_enablepat->SetToolTip(_("Specify a custom sequence of characters\n"
                               "which the relative search will be based upon"));

   browse->SetFocus();

   wxCommandEvent evt(wxEVT_COMMAND_RADIOBUTTON_SELECTED, MonkeyMoore_8bitMode);
   OnSearchMode(evt);

   // restores previous UI state
   if (prefs.getBool(wxT("settings/ui-remember-state")))
   {
      int stypeid = prefs.getBool(wxT("ui-state/search-type"), wxT("rs")) ? MonkeyMoore_RelativeSearch : MonkeyMoore_ValueScanSearch;
      GetWindow<wxRadioButton>(stypeid)->SetValue(true);

      wxCommandEvent evt(wxEVT_COMMAND_RADIOBUTTON_SELECTED, stypeid);
      OnSearchType(evt);

      wildcard->SetValue(wxString::Format(wxT("%c"), prefs.get(wxT("ui-state/wildcard")).GetChar(0)));
      show_all->SetValue(prefs.getBool(wxT("ui-state/show-all-results")));

      if (prefs.getBool(wxT("ui-state/advanced-shown"))) {
         auto dummyEvent = wxCommandEvent();
         OnAdvanced(dummyEvent);
      }     
   }

   std::vector<std::pair<wxString, wxString>> &charsets = prefs.getCommonCharsetList();
   const int start = MonkeyMoore_CharsetBase;
   const int end = MonkeyMoore_CharsetBase + charsets.size();

   // binds the common charsets menu entries dynamically to the correct event handler
   Bind(wxEVT_COMMAND_MENU_SELECTED, &MonkeyFrame::OnCharsetChosen, this, start, end);
}

/**
* Destructor. Frees allocated memory and resources.
*/
MonkeyFrame::~MonkeyFrame ()
{
}

// template specializations to return a reference to the correct results vector
template <> std::vector<mmoore::SearchResult<uint8_t>> &MonkeyFrame::lastResults<uint8_t> () { return last_results8; }
template <> std::vector<mmoore::SearchResult<uint16_t>> &MonkeyFrame::lastResults<uint16_t> () { return last_results16; }

/**
* Method called when the browse button is pressed.
* @param event not used
*/
void MonkeyFrame::OnBrowse (wxCommandEvent &WXUNUSED(event))
{
   const long flags = wxFD_OPEN | wxFD_FILE_MUST_EXIST;
   wxFileDialog dialog(this, MM_MSG_OPENFL, prefs.get(wxT("directories/open-file")), wxT(""), _("All files (*.*)|*.*"), flags);

   if (dialog.ShowModal() == wxID_OK)
   {
      wxTextCtrl *fname = GetWindow<wxTextCtrl>(MonkeyMoore_FName);
      
      if (!fname->IsEmpty()) fname->Clear();
      fname->WriteText(dialog.GetPath());

      prefs.set(wxT("directories/open-file"), dialog.GetDirectory());
      FindWindowById(MonkeyMoore_KWord)->SetFocus();
   }
}

/**
* Method called when a search type is chosen. Sets the correct validator in the keyword textbox.
* @param event information regarding the event
*/
void MonkeyFrame::OnSearchType (wxCommandEvent &event)
{
   if (event.GetId() == MonkeyMoore_RelativeSearch)
      GetWindow<wxTextCtrl>(MonkeyMoore_KWord)->SetValidator(wxDefaultValidator);
   else
   {
      wxTextValidator v(wxFILTER_INCLUDE_CHAR_LIST);
      v.SetCharIncludes(wxT("0123456789 "));

      GetWindow<wxTextCtrl>(MonkeyMoore_KWord)->SetValidator(v);
   }

   wxPostEvent(this, wxCommandEvent(wxEVT_BUTTON, MonkeyMoore_Clear));
   GetWindow<wxTextCtrl>(MonkeyMoore_KWord)->Clear();
}

void MonkeyFrame::OnSearchMode (wxCommandEvent &event)
{
   wxPostEvent(this, wxCommandEvent(wxEVT_BUTTON, MonkeyMoore_Clear));

   searchmode_8bits = event.GetId() == MonkeyMoore_8bitMode;

   if (searchmode_8bits)
   {
      Bind(wxEVT_BUTTON, &MonkeyFrame::OnCreateTbl<uint8_t>, this, MonkeyMoore_CreateTbl);
      Bind(wxEVT_BUTTON, &MonkeyFrame::OnClear<uint8_t>, this, MonkeyMoore_Clear);
      Bind(wxEVT_CHECKBOX, &MonkeyFrame::OnAllResults<uint8_t>, this, MonkeyMoore_AllResults);
   }
   else
   {
      Bind(wxEVT_BUTTON, &MonkeyFrame::OnCreateTbl<uint16_t>, this, MonkeyMoore_CreateTbl);
      Bind(wxEVT_BUTTON, &MonkeyFrame::OnClear<uint16_t>, this, MonkeyMoore_Clear);
      Bind(wxEVT_CHECKBOX, &MonkeyFrame::OnAllResults<uint16_t>, this, MonkeyMoore_AllResults);
   }
}

void MonkeyFrame::OnByteOrder (wxCommandEvent &event)
{
   wxPostEvent(this, wxCommandEvent(wxEVT_BUTTON, MonkeyMoore_Clear));

   byteorder_little = event.GetId() == MonkeyMoore_ByteOrderLE;
}

static std::vector<CharType> convert_to_vector_array(const wxString &from) {
   std::vector<CharType> result;
   result.reserve(from.length());

   for (const auto &ch : from) {
      result.push_back(static_cast<CharType>(ch.GetValue()));
   }

   return result;
}

/**
* Method called when the search button is pressed.
* Validates the user input and init the search thread.
* @param event not used
*/
void MonkeyFrame::OnSearch (wxCommandEvent &WXUNUSED(event))
{
   wxString filename = GetValue<wxString, wxTextCtrl>(MonkeyMoore_FName);
   wxString keyword = GetValue<wxString, wxTextCtrl>(MonkeyMoore_KWord);

   if (!filename)
      return ShowWarning(MM_WARNING_NOFILE);

   wxChar card = 0;
   wxString charpattern = wxT("");
   std::vector <short> values;

   bool relative_search = GetValue<bool, wxRadioButton>(MonkeyMoore_RelativeSearch);

   if (relative_search)
   {
      bool use_wildcards = IsChecked(MonkeyMoore_UseWC);

      if (use_wildcards)
      {
         wxTextCtrl *wc = GetWindow<wxTextCtrl>(MonkeyMoore_Wildcard);
         wxString wildcard = wc->GetValue();

         if (!wildcard)
            return ShowWarning(MM_WARNING_NOWC);

         if (wildcard.size() > 1)
            return ShowWarning(MM_WARNING_MANYWC);

          card = wildcard[0];
      }

      bool enable_cp = IsChecked(MonkeyMoore_EnableCP);

      if (enable_cp)
      {
         wxTextCtrl *cp = GetWindow<wxTextCtrl>(MonkeyMoore_CharPattern);
         charpattern = cp->GetValue();
      }

      // we need a valid keyword (when we use ascii)
      if (!CheckKeyword(keyword, card, charpattern)) return;
   }
   else
   {
      // search type is value scan relative

      wxStringTokenizer tkz(keyword, wxT(" "));

      for (int i = 0; tkz.HasMoreTokens(); i++)
      {
         wxString tk = tkz.GetNextToken();
         long value;
         
         if (!tk.ToLong(&value, 10))
            return ShowWarning(MM_WARNING_VSRINVALIDVAL);

         values.push_back(static_cast <short> (value));
      }
   }

   if (!wxFile::Exists(filename))
      return ShowWarning(MM_WARNING_FILENOTFOUND);

   if (!wxFile::Access(filename, wxFile::read))
      return ShowWarning(MM_WARNING_FILECANTACCESS);

   std::shared_ptr<wxFile> file(new wxFile(filename, wxFile::read));

   if (!file->IsOpened())
      return ShowWarning(MM_WARNING_FILENOTFOUND);

   mmoore::SearchConfig config;
   config.is_relative_search = relative_search;
   config.endianness = byteorder_little ? mmoore::Endianness::Little : mmoore::Endianness::Big;
   config.file_path = filename.ToStdString();
   config.wildcard = card;
   config.keyword = convert_to_vector_array(keyword);
   config.custom_char_seq = convert_to_vector_array(charpattern);
   config.reference_values = values;
   config.preferred_num_threads = prefs.getInt("settings/perf-search-threads");
   config.preferred_search_block_size = prefs.getInt("settings/perf-memory-pool");
   config.preferred_preview_width = prefs.getInt("settings/display-preview-width");

   if (searchmode_8bits)
      StartSearchThread<uint8_t>(config);
   else {
      StartSearchThread<uint16_t>(config);
   }
}

/**
* Method called when the advanced options button is pressed.
* Shows/hides the advanced options box.
* @param event not used
*/
void MonkeyFrame::OnAdvanced(wxCommandEvent &WXUNUSED(event))
{
   wxBitmapButton *advanced = GetWindow<wxBitmapButton>(MonkeyMoore_Advanced);
   advanced->SetBitmapLabel(images.GetBitmap(advanced_shown ? MonkeyBmp_ShowAdv : MonkeyBmp_HideAdv));
   advanced->SetBitmapDisabled(images.GetBitmap(advanced_shown ? MonkeyBmp_ShowAdvGrayed : MonkeyBmp_HideAdvGrayed));

   wxSizer *advancedbox_sz = main_panel->GetSizer()->GetItem(4)->GetSizer();
   wxASSERT(advancedbox_sz);

   advanced_shown = !advanced_shown;

   main_panel->GetSizer()->Show(advancedbox_sz, advanced_shown);
   Layout(), Update();
}

/**
* Method called when the common charsets buttons is pressed.
* Shows a popup menu containing the charsets found in the config file.
* @param event not used
*/
void MonkeyFrame::OnCharsetList (wxCommandEvent &WXUNUSED(event))
{
   bool enableCP = IsChecked(MonkeyMoore_EnableCP);

   std::vector<std::pair<wxString, wxString>> &seqs = prefs.getCommonCharsetList();
   std::vector<std::pair<wxString, wxString>>::iterator i;

   wxMenu charset_menu(wxT(""));
   int index = 0;

   for (i = seqs.begin(); i != seqs.end(); i++)
   {
      wxMenuItem *mi = new wxMenuItem(&charset_menu, MonkeyMoore_CharsetBase + index++, i->first);
      charset_menu.Append(mi);
      
      // enable only if the character sequence checkbox is checked
      mi->Enable(enableCP);
   }

   wxMenuItem *mng = new wxMenuItem(&charset_menu, MonkeyMoore_ManageCharSeqs, _("Manage character sequences..."));
   mng->SetBitmap(images.GetBitmap(MonkeyBmp_MngSequences));

   charset_menu.AppendSeparator();
   charset_menu.Append(mng);

   PopupMenu(&charset_menu);
}

/**
* Method called when an item from the common charset popup menu is chosen.
* Replaces the contents of the character set text control with the chosen charset.
* @param event not used
*/
void MonkeyFrame::OnCharsetChosen (wxCommandEvent &event)
{
   auto &seqs = prefs.getCommonCharsetList();
   wxString val = seqs[event.GetId() - MonkeyMoore_CharsetBase].second;

   GetWindow<wxTextCtrl>(MonkeyMoore_CharPattern)->SetValue(val);
}

/**
* Method called when the checkbox which toggles the results display mode is pressed.
* @param event information regarding the event
*/
template <typename _DataType>
void MonkeyFrame::OnAllResults (wxCommandEvent &event)
{
   // if there's a search in progress, the change will only take effect when it's done
   if (!search_in_progress)
      ShowResults<_DataType>(event.IsChecked());
}

/**
* This method is called when the create table button is pressed.
* @param event not used
*/
template <typename _DataType>
void MonkeyFrame::OnCreateTbl (wxCommandEvent &WXUNUSED(event))
{
   wxListCtrl *result_box = GetWindow<wxListCtrl>(MonkeyMoore_Results);
   int target = result_box->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

   auto results = lastResults<_DataType>();

   if (results.empty())
      return ShowWarning(MM_WARNING_TABLENORESULTS);

   if (target != wxNOT_FOUND)
   {
      wxListItem sel_item;
      sel_item.SetId(target);
      sel_item.SetMask(wxLIST_MASK_DATA);

      result_box->GetItem(sel_item);
      wxASSERT(sel_item.GetData() < results.size());

      MonkeyTable tbldiag(this, _("Create table file"), prefs, images, wxSize(500, 440));

      auto values_map = results.at(sel_item.GetData()).values_map;

      tbldiag.InitTableData<_DataType>(values_map, byteorder_little);
      tbldiag.CenterOnParent();
      tbldiag.ShowModal();
   }
   else ShowWarning(MM_WARNING_NORESULTSELECTED);
}

void MonkeyFrame::OnResizeResultsListColumn(wxListEvent &event)
{
   wxListCtrl *list = static_cast<wxListCtrl *>(event.GetEventObject());

   int numCols = list->GetColumnCount();
   int evtCol = event.GetColumn();
   
   // preview column can't be modified
   if (event.GetEventType() == wxEVT_LIST_COL_BEGIN_DRAG && evtCol == numCols - 1)
      event.Veto();
   else if (event.GetEventType() == wxEVT_LIST_COL_END_DRAG)
      AdjustResultColumns();

   // wxEVT_LIST_COL_DRAGGING is ignored so we don't get temporary scrollbars
}

/**
* This method is called when the user double clicks or press ENTER on one
* of the items shown in the result box.
* @param event not used
*/
void MonkeyFrame::OnListItemActivated (wxListEvent &WXUNUSED(event))
{
   // redirects to the appropriate method
   wxPostEvent(this, wxCommandEvent(wxEVT_BUTTON, MonkeyMoore_CreateTbl));
}

/**
* This method is called when the user right clicks one of the items
* shown in the result box.
* @param event not used
*/
void MonkeyFrame::OnListItemRightClick (wxListEvent &WXUNUSED(event))
{
   wxMenu list_menu(wxT(""));

   wxMenuItem *copy = new wxMenuItem(&list_menu, MonkeyMoore_CopyAddress, _("Copy address"));
   copy->SetBitmap(images.GetBitmap(MonkeyBmp_Copy));

   list_menu.Append(copy);

   PopupMenu(&list_menu);
}

void MonkeyFrame::OnListDeleteAll (wxListEvent &event)
{
   wxListCtrl *list = static_cast<wxListCtrl *>(event.GetEventObject());

   list->SetColumnWidth(0, ResultListCol_Offset);
   AdjustResultColumns();
}

/**
* This method is called when the user chooses want to copy an address
* from the result box items.
* @param event not used
*/
void MonkeyFrame::OnCopyAddress (wxCommandEvent &WXUNUSED(event))
{
   wxListCtrl *result_box = GetWindow<wxListCtrl>(MonkeyMoore_Results);
   int target = result_box->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

   wxASSERT(result_box->GetItemCount() != 0);

   if (target != wxNOT_FOUND)
   {
      wxListItem sel_item;
      sel_item.SetId(target);
      sel_item.SetColumn(0);
      sel_item.SetMask(wxLIST_MASK_TEXT);
    
      result_box->GetItem(sel_item);

      wxString address = sel_item.GetText();

      if (wxTheClipboard->Open())
      {
         wxTheClipboard->SetData(new wxTextDataObject(address.substr(2)));
         wxTheClipboard->Close();
      }
   }
}

void MonkeyFrame::OnManageCharSeqs (wxCommandEvent &WXUNUSED(event))
{
   MonkeySeqs seqs(this, _("Manage character sequences"), prefs, images, wxSize(450, 340));
   seqs.CenterOnParent();
   seqs.ShowModal();

   auto end = MonkeyMoore_CharsetBase + prefs.getCommonCharsetList().size();
   Bind(wxEVT_COMMAND_MENU_SELECTED, &MonkeyFrame::OnCharsetChosen, this, MonkeyMoore_CharsetBase, end);
}

/**
* This method is called when the clear button is pressed.
* @param event not used
*/
template <typename _DataType>
void MonkeyFrame::OnClear (wxCommandEvent &WXUNUSED(event))
{
   GetWindow<wxListCtrl>(MonkeyMoore_Results)->DeleteAllItems();
   GetWindow<wxStaticText>(MonkeyMoore_Counter)->SetLabel(wxT("0"));

   ShowProgressBar(false);
   SetCurrentProgress(0);

   search_done = false;
   lastResults<_DataType>().clear();
}

void MonkeyFrame::OnOptions (wxCommandEvent &WXUNUSED(event))
{
   MonkeyOptions options(this, _("Settings"), prefs);
   options.Fit();
   options.CenterOnParent();
   options.ShowModal();
}

/**
* This method is called when the about button is pressed.
* @param event not used
*/
void MonkeyFrame::OnAbout (wxCommandEvent &WXUNUSED(event))
{
   MonkeyAbout about(this, _("About Monkey-Moore"), wxSize(480, 400));
   about.ShowModal();
}

/**
* Method is called when the cancel button of the progress bar is pressed.
* @param event not used
*/
void MonkeyFrame::OnCancel (wxCommandEvent &WXUNUSED(event))
{
   if (search_in_progress)
   {
      {
         std::lock_guard<std::mutex> lock(abortMutex);
         search_was_aborted = true;
      }
      
      GetWindow<wxStaticText>(MonkeyMoore_ElapsedTime)->SetLabel(_("Aborting..."));
   }
   else ShowProgressBar(false);
}

/**
* This method is called when the ENTER key is pressed in the keyword
* input box, triggering the search button.
* @param event not used
*/
void MonkeyFrame::OnTextEnter (wxCommandEvent &WXUNUSED(event))
{
   if (!search_in_progress)
   {
      wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, MonkeyMoore_Search);
      AddPendingEvent(evt);
   }
}


/**
* This method is called when the windows is resized.
* It basically adjusts the result list column header's width.
* @param information regarding the event
*/
void MonkeyFrame::OnSize (wxSizeEvent &event)
{
   AdjustResultColumns();
   event.Skip();
}

void MonkeyFrame::OnShow (wxShowEvent &event)
{
   if (event.IsShown())
      AdjustResultColumns();

   event.Skip();
}

/**
* Method called when the window is about to be closed.
* @param event not used
*/
void MonkeyFrame::OnClose (wxCloseEvent &WXUNUSED(event))
{
   Show(false);
   ShowProgressBar(false);

   if (prefs.getBool(wxT("settings/ui-remember-state")))
   {
      prefs.set(wxT("ui-state/search-type"), GetValue<bool, wxRadioButton>(MonkeyMoore_RelativeSearch) ? wxT("rs") : wxT("vsr"));
      prefs.set(wxT("ui-state/wildcard"), GetValue<wxString, wxTextCtrl>(MonkeyMoore_Wildcard).substr(0, 1));
      prefs.setBool(wxT("ui-state/advanced-shown"), advanced_shown);
      prefs.setBool(wxT("ui-state/show-all-results"), IsChecked(MonkeyMoore_AllResults));

      prefs.setBool(wxT("window/maximized"), IsMaximized());
   }

   if (!IsMaximized() && prefs.getBool(wxT("settings/ui-remember-size")))
      prefs.setSize(wxT("window/size-width"), wxT("window/size-height"), GetSize());

   if (!prefs.getBool(wxT("settings/ui-center-window")) && prefs.getBool(wxT("settings/ui-remember-position")))
      prefs.setPoint(wxT("window/position-x"), wxT("window/position-y"), GetPosition());

   Destroy();
}

/**
* This method is called periodically to keep the UI controls in a consistent
* state, enabling and disabling according to the application state.
* @param event information regarding the event
*/
void MonkeyFrame::OnUpdateUI (wxUpdateUIEvent &event)
{
   bool search_relative = GetValue<bool, wxRadioButton>(MonkeyMoore_RelativeSearch);
   bool haveResults = searchmode_8bits ? !last_results8.empty() : !last_results16.empty();

   switch (event.GetId())
   {
      case MonkeyMoore_Browse:
      case MonkeyMoore_Options:
      case MonkeyMoore_RelativeSearch:
      case MonkeyMoore_ValueScanSearch:
      case MonkeyMoore_8bitMode:
      case MonkeyMoore_16bitMode:
         event.Enable(!search_in_progress);
         break;

      case MonkeyMoore_Cancel:
      case MonkeyMoore_About:
      case MonkeyMoore_AllResults:
      case MonkeyMoore_Advanced:
         event.Enable(!search_was_aborted);
         break;

      case MonkeyMoore_Wildcard:
         event.Enable(!search_was_aborted && search_relative && IsChecked(MonkeyMoore_UseWC));
         break;

      case MonkeyMoore_CharsetList:
         event.Enable(!search_was_aborted && search_relative);
         break;

      case MonkeyMoore_CharPattern:
         event.Enable(!search_was_aborted && search_relative && IsChecked(MonkeyMoore_EnableCP));
         break;

      case MonkeyMoore_UseWC:
      case MonkeyMoore_EnableCP:
         event.Enable(!search_was_aborted && search_relative);
         break;

      case MonkeyMoore_CreateTbl:
         event.Enable(
            haveResults &&
            !search_in_progress &&
            GetWindow<wxListCtrl>(MonkeyMoore_Results)->GetSelectedItemCount()
         );
         break;

      case MonkeyMoore_Clear:
         event.Enable(!search_in_progress && haveResults);
         break;

      case MonkeyMoore_Search:
         event.Enable(
            !search_in_progress &&
            !GetValue<wxString, wxTextCtrl>(MonkeyMoore_FName).empty() &&
            !GetValue<wxString, wxTextCtrl>(MonkeyMoore_KWord).empty()
         );
         break;

      case MonkeyMoore_Results:
         {
            wxListCtrl *l = static_cast<wxListCtrl *>(event.GetEventObject());
            
            if (search_relative && l->GetColumnCount() != 3)
            {
               l->InsertColumn(1, _("Values"), wxLIST_FORMAT_LEFT, 100);
               AdjustResultColumns();
            }
            else if (!search_relative && l->GetColumnCount() != 2)
            {
               l->DeleteColumn(1);
               AdjustResultColumns();
            }
         }
         break;

      case MonkeyMoore_EnableByteOrder:
         event.Enable(!search_in_progress && GetValue<bool, wxRadioButton>(MonkeyMoore_16bitMode));
         break;

      case MonkeyMoore_ByteOrderLE:
      case MonkeyMoore_ByteOrderBE:
         event.Enable(
            !search_in_progress &&
            GetValue<bool, wxRadioButton>(MonkeyMoore_16bitMode) &&
            IsChecked(MonkeyMoore_EnableByteOrder)
         );
         break;

   }
}

/**
* Shows/hides the progress bar. Also adjusts the window size appropriately.
* @param show true to show, false to hide
*/
void MonkeyFrame::ShowProgressBar (const bool show)
{
   wxSizer *progress_sz = main_panel->GetSizer()->GetItem(8)->GetSizer();
   wxASSERT(progress_sz);

   bool is_shown = main_panel->GetSizer()->IsShown(progress_sz);
   main_panel->GetSizer()->Show(progress_sz, show);

   bool is_being_shown = !is_shown && show;
   bool is_being_hidden = is_shown && !show;

   // screen size is changed only if:
   // 1) progress box was hidden and now it's being shown
   // 2) progress box was shown and now it's being hidden
   if (is_being_shown || is_being_hidden)
   {
      if (!IsMaximized())
      {
         SetMinSize(wxSize(GetMinSize().x, GetMinSize().y + (show ? progressBoxHeight : -progressBoxHeight)));
         SetSize(wxSize(GetSize().x, GetSize().y + (show ? progressBoxHeight : -progressBoxHeight)));
      }
   }
}

/**
* Checks whether the keyword is valid or not.
* @param kw keyword
* @param wc user-defined wildcard
* @return True if it's valid, false otherwise.
*/
bool MonkeyFrame::CheckKeyword (const wxString &kw, const wxChar wc, const wxString &cp)
{
   // custom character sequence being used?
   bool custom_cp = cp.length() != 0;

   // ASCII printable characters are in the 0x20 - 0x7A range.
   bool ascii_input =
      std::count_if(kw.begin(), kw.end(), [](wxChar c) { return c < 0x20; }) == 0 &&
      std::count_if(kw.begin(), kw.end(), [](wxChar c) { return c > 0x7A; }) == 0;
      
   int n_wildcards = std::count(kw.begin(), kw.end(), wc);

   // we need 3 or more characters
   if (!kw || kw.size() < 3)
   {
      ShowWarning(MM_WARNING_KWORDSIZE);
      return false;
   }

   if (!custom_cp && ascii_input)
   {
      int n_lower = static_cast<int>(std::count_if(kw.begin(), kw.end(), [](const wxUniChar &c) {
         return is_ascii_lower(static_cast<char32_t>(c.GetValue()));
      }));

      int n_upper = static_cast <int> (std::count_if(kw.begin(), kw.end(), [](const wxUniChar &c) {
         return is_ascii_upper(static_cast<char32_t>(c.GetValue()));
      }));

      // we need 3 or more characters with the SAME capitalization
      if (n_lower && n_upper)
      {
         if (n_lower < 3 && n_upper < 3)
         {
            ShowWarning(MM_WARNING_KWORDCAPLETTERS);
            return false;
         }
      }
      else
      {
         int n_letters = static_cast <int>(std::count_if(
            kw.begin(), 
            kw.end(), [](wxChar c) { 
               return std::isalpha(c); 
            }));

         // we still need 3 or more characters, not counting wildcards
         if (n_letters < 3)
         {
            ShowWarning(MM_WARNING_KWORDLETTERS);
            return false;
         }
      }

      // checks if we have only valid characters (letters and wildcards)
      auto count_non_alpha_chars = static_cast<int>(std::count_if(
         kw.begin(), 
         kw.end(), 
         [](wxChar c) { 
            return !(std::isalpha(c) ? true : false); 
         }));

      if (count_non_alpha_chars > n_wildcards)
      {
         ShowWarning(MM_WARNING_KWORDINVALIDCHARS);
         return false;
      }
   }
   else
   {
      // when not working with ASCII, we still have some restrictions

      if (kw.size() - n_wildcards < 3)
      {
         ShowWarning(MM_WARNING_KWORDNONWILDCARD);
         return false;
      }

      if (custom_cp)
      {
         if (n_wildcards && std::count(cp.begin(), cp.end(), wc))
         {
            ShowWarning(MM_WARNING_CHARPATWILDCARD);
            return false;
         }

         // For some reason, wxString don't work with some STL
         // algorithms (like sort), so plain std::wstring is used instead.
         std::wstring sorted_kw(kw.c_str()), sorted_cp(cp.c_str());

         std::sort(sorted_kw.begin(), sorted_kw.end());
         std::sort(sorted_cp.begin(), sorted_cp.end());

         if (std::unique(sorted_cp.begin(), sorted_cp.end()) != sorted_cp.end())
         {
            ShowWarning(MM_WARNING_CHARPATDUPLICATED);
            return false;
         }

         sorted_kw.erase(std::remove(sorted_kw.begin(), sorted_kw.end(), wc), sorted_kw.end());
         sorted_kw.erase(std::unique(sorted_kw.begin(), sorted_kw.end()), sorted_kw.end());

         if (!std::includes(sorted_cp.begin(), sorted_cp.end(), sorted_kw.begin(), sorted_kw.end()))
         {
            ShowWarning(MM_WARNING_KWORDCPMISMATCH);
            return false;
         }
      }
   }

   return true;
}

void MonkeyFrame::AdjustResultColumns (bool sizeToContents)
{
   bool rs = GetValue<bool, wxRadioButton>(MonkeyMoore_RelativeSearch);
   wxListCtrl *list = GetWindow<wxListCtrl>(MonkeyMoore_Results);

   list->Freeze();

   if (sizeToContents && list->GetItemCount())
      list->SetColumnWidth(0, wxLIST_AUTOSIZE);

   std::array<int, 3> cols = {0};
   cols[0] = list->GetColumnWidth(0);
   cols[1] = list->GetColumnWidth(1);

   int bestSize = list->GetClientSize().x - cols[0] - (rs ? cols[1] : 0);
   list->SetColumnWidth(rs ? 2 : 1, bestSize);
   
   // get rid of annoying scrollbar showing up when it is not needed.
   list->Thaw();
}

/**
* Creates a detached SearchThread to run in the background.
* @tparam _ResultType Type defining the expected result of the search
* @tparam _DataType Basic underlying type used to represent the data
*/
template <typename _DataType>
bool MonkeyFrame::StartSearchThread (mmoore::SearchConfig &config)
{
   SearchThread<_DataType> *worker =
      new SearchThread<_DataType>(config, lastResults<_DataType>(), this);

   if (worker->Create() == wxTHREAD_NO_ERROR)
   {
      wxBitmapButton *cancel_search = GetWindow<wxBitmapButton>(MonkeyMoore_Cancel);
      cancel_search->SetBitmapLabel(images.GetBitmap(MonkeyBmp_Cancel));

      wxListCtrl *result_box = GetWindow<wxListCtrl>(MonkeyMoore_Results);
      if (result_box->GetItemCount() != 0) result_box->DeleteAllItems();

      GetWindow<wxStaticText>(MonkeyMoore_Counter)->SetLabel(wxT("0"));
      GetWindow<wxStaticText>(MonkeyMoore_ElapsedTime)->SetLabel(_("Waiting..."));

      // bind thread notification events to the proper function template
      Bind(mmEVT_SEARCHTHREAD_UPDATE, &MonkeyFrame::OnThreadUpdate<_DataType>, this);
      Bind(mmEVT_SEARCHTHREAD_COMPLETED, &MonkeyFrame::OnThreadCompleted<_DataType>, this);
      Bind(mmEVT_SEARCHTHREAD_ABORTED, &MonkeyFrame::OnThreadAborted<_DataType>, this);

      SetCurrentProgress(0);
      ShowProgressBar();
      chronometer.Start();
      
      lastResults<_DataType>().clear();
      
      search_in_progress = true;
      worker->SetPriority(25);
      worker->Run();

      return true;
   }
   else
   {
      wxMessageBox(MM_WARNING_THREADERROR);

      delete worker;
      return false;
   }
}

/**
* Display the search results.
* @param r search results
*/
template <typename _DataType>
void MonkeyFrame::ShowResults (bool showAll)
{
   wxListCtrl *result_box = GetWindow<wxListCtrl>(MonkeyMoore_Results);
   bool search_relative = GetValue<bool, wxRadioButton>(MonkeyMoore_RelativeSearch);

   uint32_t numBytes = static_cast<uint32_t>(sizeof(_DataType)) * 2;
   wxString hexValueFmt = wxString::Format(wxT("%%c=%%0%uX "), numBytes);

   std::vector<typename MonkeyMoore<_DataType>::equivalency_map> unique;
   auto results = lastResults<_DataType>();

   // index of the element being inserted in the wxListCtrl
   long curListIndex = 0;

   if (!results.empty())
   {
      if (result_box->GetItemCount() != 0)
         result_box->DeleteAllItems();

      result_box->Freeze();

      for (uint32_t i = 0; i < results.size(); i++)
      {
         const auto &[result_offset, result_map, result_preview] = results[i];

         // if another result with the same values was already inserted in the list, don't insert
         if (!std::count(unique.begin(), unique.end(), result_map))
         {
            if (!showAll)
               unique.push_back(result_map);

            bool hex_offset = prefs.getBool(wxT("settings/display-offset-mode"), wxT("hex"));
            wxString offset = wxString::Format(hex_offset ? wxT("0x%llX") : wxT("%lld"), result_offset);

            result_box->InsertItem(curListIndex, offset);
            result_box->SetItemData(curListIndex, i);

            wxString values;

            for (auto j = result_map.cbegin(); j != result_map.cend(); j++)
            {
               const auto &[character, hex_value] = *j;
               // swap bytes acording to the endianness the search was performed on
               _DataType value_swapped = byteorder_little ?
                  swap_on_le<_DataType>(hex_value) :
                  swap_on_be<_DataType>(hex_value);

               values += wxString::Format(hexValueFmt, static_cast<int>(character), value_swapped);
            }

            result_box->SetItem(curListIndex, 1, values);
            result_box->SetItem(curListIndex, search_relative ? 2 : 1, result_preview);

            curListIndex++;
         }
      }

      result_box->Thaw();
      AdjustResultColumns(true);

      wxString counterLabel = wxString::Format(
         wxT("%d"), 
         static_cast<int>(showAll ? results.size() : unique.size())
      );

      GetWindow<wxStaticText>(MonkeyMoore_Counter)->SetLabel(counterLabel);
   }
}

template <typename _DataType>
void MonkeyFrame::OnThreadUpdate (wxThreadEvent &event)
{
   if (!search_was_aborted)
   {
      GetWindow<wxStaticText>(MonkeyMoore_ElapsedTime)->SetLabel(event.GetString());
      SetCurrentProgress(event.GetInt());
   }
}

template <typename _DataType>
void MonkeyFrame::OnThreadCompleted (wxThreadEvent &WXUNUSED(event))
{
   search_done = true;
   search_in_progress = false;

   wxTimeSpan elapsed(0, 0, 0, chronometer.Time());
   wxString format = _("Elapsed time: ");
   
   format += elapsed.GetSeconds().ToLong() ?
      elapsed.GetMinutes() ? elapsed.Format(_("%M minute(s) and %S second(s).")) : elapsed.Format(_("%S second(s).")) :
      elapsed.Format(_("less than one second."));

   size_t resultsCount = lastResults<_DataType>().size();

   resultsCount ?
      GetWindow<wxStaticText>(MonkeyMoore_ElapsedTime)->SetLabel(format) :
      GetWindow<wxStaticText>(MonkeyMoore_ElapsedTime)->SetLabel(_("No results found."));

   bool showAll = IsChecked(MonkeyMoore_AllResults);
   
   ShowResults<_DataType>(showAll);

   wxBitmapButton *cancel_search = GetWindow<wxBitmapButton>(MonkeyMoore_Cancel);
   cancel_search->SetBitmapLabel(images.GetBitmap(MonkeyBmp_Done));
}

template <typename _DataType>
void MonkeyFrame::OnThreadAborted (wxThreadEvent &WXUNUSED(event))
{
   std::lock_guard<std::mutex> lock(abortMutex);

   search_in_progress = false;
   search_was_aborted = false;

   GetWindow<wxStaticText>(MonkeyMoore_ElapsedTime)->SetLabel(_("Search was aborted."));

   lastResults<_DataType>().clear();

   wxBitmapButton *cancel_search = GetWindow<wxBitmapButton>(MonkeyMoore_Cancel);
   cancel_search->SetBitmapLabel(images.GetBitmap(MonkeyBmp_Done));

   SetCurrentProgress(0);
}