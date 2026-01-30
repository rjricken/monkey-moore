// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MONKEY_PREFS_HPP
#define MONKEY_PREFS_HPP

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <map>
#include <utility>
#include <vector>

/**
* Stores the application settings and preferences, allowing it to be
* saved/loaded from an external XML file.
*/
class MonkeyPrefs
{
public:
   MonkeyPrefs ()
   { setDefaultValues(); }

   void setDefaultValues ();
   void load (const wxString &configFile);
   void save (const wxString &configFile, bool recreate = false);
   void loadSequences (const wxString &sequencesFile);
   void saveSequences (const wxString &sequencesFile, bool recreate = false);

   /**
   * Gets the value of the specified property.
   * @param id property name
   * @return The property value (as wxString).
   */
   wxString get (const wxString &id) { return values[id]; }

   /**
   * Gets the boolean value of the specified property. Optionally,
   * you can change the default comparison string.
   * @param id property name
   * @param comp (optional) comparison string
   * @return The property value (as a bool).
   */
   bool getBool (const wxString &id, const wxChar *comp = wxT("true"))
   { return values[id] == comp; }

   /**
   * Gets the value of the specified property converted to integer.
   * @param id property name
   * @return The property value (as int).
   */
   int getInt (const wxString &id)
   { return wxAtoi(values[id]); }

   /**
   * Gets the value of the specified property pair as a wxSize instance.
   * @param id_w width property name
   * @param id_h height property name
   * @return The property pair value (as wxSize).
   */
   wxSize getSize (const wxString &id_w, const wxString &id_h)
   { return wxSize(getInt(id_w), getInt(id_h)); }

   /**
   * Gets the value of the specified property pair as a wxSize instance.
   * @param id_w width property name
   * @param id_h height property name
   * @return The property pair value (as wxSize).
   */
   wxPoint getPoint (const wxString &id_x, const wxString &id_y)
   { return wxPoint(getInt(id_x), getInt(id_y)); }

   /**
   * Returns a std::list containing all loaded charsets.
   * @return Charset list.
   */
   std::vector <std::pair <wxString, wxString>> &getCommonCharsetList()
   { return common_charsets; }

   /**
   * Sets a value to the specified property.
   * @param id property name
   * @param value new value
   */
   void set (const wxString &id, const wxString &value) { values[id] = value; }

   /**
   * Sets a boolean value to the specified property.
   * @param id property name
   * @param value new value
   */
   void setBool (const wxString &id, bool value)
   { values[id] = value ? wxT("true") : wxT("false"); }

   /**
   * Sets an integer value to the specified property.
   * @param id property name
   * @param value new value
   */
   void setInt (const wxString &id, int value)
   { values[id] = wxString::Format(wxT("%d"), value); }

   /**
   * Sets a wxSize value to the specified property pair (width and height).
   * @param id_w width property name
   * @param id_h height property name
   * @param value new value
   */
   void setSize (const wxString &id_w, const wxString &id_h, const wxSize &value)
   { setInt(id_w, value.x); setInt(id_h, value.y); }

   /**
   * Sets a wxPoint values to the specified property pair (x and y).
   * @param id_x x property name
   * @param id_y y property name
   * @param value new value
   */
   void setPoint (const wxString &id_x, const wxString &id_y, const wxPoint &value)
   { setInt(id_x, value.x); setInt(id_y, value.y); }

private:
   std::map<wxString, wxString> values;                         // preferences (name/value pair)
   std::vector<std::pair<wxString, wxString>> common_charsets;  // common-charsets
};

#endif //~MONKEY_PREFS_HPP
