// SPDX-License-Identifier: GPL-3.0-or-later

#include "monkey_app.hpp"
#include "monkey_frame.hpp"
#include "monkey_error.hpp"
#include "filesystem_utils.hpp"
#include "constants.hpp"

#include <wx/file.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>

IMPLEMENT_APP(MonkeyApp)

bool MonkeyApp::OnInit ()
{
   if (!wxApp::OnInit())
      return false;

   wxImage::AddHandler(new wxPNGHandler());

   // set up language selection system
   m_loc.Init(wxLANGUAGE_DEFAULT, wxLOCALE_LOAD_DEFAULT);
   wxLocale::AddCatalogLookupPathPrefix(getResourcePath(wxT("./language")));
   m_loc.AddCatalog(wxT("monkeymoore"));

   // retrieve the OS specific data directory for our application
   wxStandardPaths stdPaths = wxStandardPaths::Get();
   wxString dataDir = stdPaths.GetUserDataDir();

   if (!wxDirExists(dataDir))
      wxFileName::Mkdir(dataDir, 0777, wxPATH_MKDIR_FULL);

   // generate an OS-native file path for the config file
   wxFileName configFile(dataDir, MM_CONFIG_FILE);
   m_cfgFile = configFile.GetFullPath();

   wxFileName customCharSeqsFile(dataDir, MM_CUSTOMSEQS_FILE);
   m_seqsFile = customCharSeqsFile.GetFullPath();

   if (!LoadConfiguration())
      return false;

   MonkeyFrame *f = new MonkeyFrame(wxT("Monkey-Moore"), m_prefs, wxDefaultPosition, wxSize(450, 460));
   f->SetBackgroundColour(wxColour(240, 240, 240));
   f->SetMinSize(wxSize(440, 440));

   // restores the saved window properties
   if (!m_prefs.getBool(wxT("settings/ui-center-window")) && m_prefs.getBool(wxT("settings/ui-remember-position")))
      f->SetPosition(m_prefs.getPoint(wxT("window/position-x"), wxT("window/position-y")));

   if (m_prefs.getBool(wxT("settings/ui-remember-size")))
      f->SetSize(m_prefs.getSize(wxT("window/size-width"), wxT("window/size-height")));

   if (m_prefs.getBool(wxT("settings/ui-center-window"))) f->Center();
   if (m_prefs.getBool(wxT("window/maximized"))) f->Maximize();

   f->Show();
   SetTopWindow(f);

   return true;
}

int MonkeyApp::OnExit()
{
   m_prefs.save(m_cfgFile);

   return wxApp::OnExit();
}

bool MonkeyApp::LoadConfiguration ()
{
   try
   {
      // tries to load the config file, creating a fresh one if it doesn't exist
      wxFile::Exists(m_cfgFile) && wxFile::Access(m_cfgFile, wxFile::read) ?
         m_prefs.load(m_cfgFile) :
         m_prefs.save(m_cfgFile, true);

      wxFile::Exists(m_seqsFile) && wxFile::Access(m_seqsFile, wxFile::read) ?
         m_prefs.loadSequences(m_seqsFile) :
         m_prefs.saveSequences(m_seqsFile, true);
   }
   catch (monkeymoore_error &e)
   {
      switch (e.code())
      {
         case MMError_ConfigParseFailed:
         case MMError_ConfigVersionMismatch:
         {
            wxString message(e.what());
            message += _("\n\n[OK] to restore default settings, [Cancel] to quit Monkey-Moore.");

            if (wxMessageBox(message, MM_ERRORCAPTION, wxOK | wxCANCEL | wxICON_WARNING) != wxOK)
               return false;

            m_prefs.save(m_cfgFile, true);
         }
         break;

         case MMError_SequencesParseFailed:
            m_prefs.saveSequences(m_seqsFile, true);
            break;

         case MMError_SequencesFileWriteError:
         case MMError_ConfigFileWriteError:
            wxMessageBox(e.what(), MM_ERRORCAPTION, wxOK | wxICON_ERROR);
            break;
      }
   }

   return true;
}