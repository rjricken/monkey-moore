// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MONKEY_APP_HPP
#define MONKEY_APP_HPP

#include <wx/wxprec.h>

#ifdef __BORLANDC__
   #pragma hdrstop
#endif

#ifndef WX_PRECOMP
   #include <wx/wx.h>
#endif

#include "monkey_prefs.hpp"

/**
* Starts Monkey-Moore's user interface execution.
*/
class MonkeyApp : public wxApp
{
public:
   /**
   * This method is called during program initialization. It chooses the
   * correct language based on the current operational system's language,
   * picking it from a catalog, when available. It also creates and shows
   * the main windows frame.
   * @return True if initializated with success, false otherwise.
   */
   virtual bool OnInit();
   virtual int OnExit();

   inline wxString getSequencesFileName() const
   { return m_seqsFile; }

private:
   bool LoadConfiguration ();

   wxLocale m_loc;          // defines our current locale
   wxString m_cfgFile;      // complete path to configuration file
   wxString m_seqsFile; // custom character sequences file path
   MonkeyPrefs m_prefs;     // application settings and preferences
};

DECLARE_APP(MonkeyApp)

#endif //~MONKEY_APP_HPP
