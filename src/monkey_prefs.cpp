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

#include "monkey_prefs.hpp"
#include "monkey_error.hpp"
#include "constants.hpp"

#include <wx/file.h>
#include <xmlparser.h>
#include <map>
#include <utility>
#include <vector>
#include <exception>
#include <regex>

using namespace std;

/**
* Fills Monkey-Moore properties with the default (factory) values.
*/
void MonkeyPrefs::setDefaultValues ()
{
   values[wxT("settings/ui-center-window")]      = wxT("true");
   values[wxT("settings/ui-remember-size")]      = wxT("true");
   values[wxT("settings/ui-remember-position")]  = wxT("false");
   values[wxT("settings/ui-remember-state")]     = wxT("true");
   values[wxT("settings/display-preview-width")] = wxT("50");
   values[wxT("settings/display-offset-mode")]   = wxT("hex");
   values[wxT("settings/perf-memory-pool")]      = wxT("8388608");
   values[wxT("settings/perf-search-threads")]   = wxT("4");

   values[wxT("window/position-x")]              = wxT("0");
   values[wxT("window/position-y")]              = wxT("0");
   values[wxT("window/size-width")]              = wxT("482");
   values[wxT("window/size-height")]             = wxT("608");
   values[wxT("window/maximized")]               = wxT("false");

   values[wxT("ui-state/search-type")]           = wxT("rs");
   values[wxT("ui-state/search-mode")]           = wxT("8-bit");
   values[wxT("ui-state/wildcard")]              = wxT("*");
   values[wxT("ui-state/advanced-shown")]        = wxT("true");
   values[wxT("ui-state/endianness-little")]     = wxT("true");
   values[wxT("ui-state/show-all-results")]      = wxT("true");

   values[wxT("directories/open-file")]          = wxT("");
   values[wxT("directories/save-table")]         = wxT("");
}

/**
* Loads the preferences from a proper XML file.
* @param configFile file name
* @return True on success, false on failure.
*/
void MonkeyPrefs::load (const wxString &configFile)
{
   XMLResults pResults;
   XMLNode root = XMLNode::parseFile(configFile, wxT("monkey-moore-config"), &pResults);

   if (pResults.error)
      throw monkeymoore_error(
         wxString::Format(_("An error prevented user preferences from being loaded.\n%s, line %d column %d"),
            configFile, pResults.nLine, pResults.nColumn),
         MMError_ConfigParseFailed
      );

   if (!root.getAttribute(wxT("version")) || wxString(root.getAttribute(wxT("version"))) != MM_VERSION)
      throw monkeymoore_error(
         _("Monkey-Moore version is different from the version used in the user preferences file."),
         MMError_ConfigVersionMismatch
      );
   
   for (int catIdx = 0; catIdx < root.nChildNode(); ++catIdx)
   {
      XMLNode catNode = root.getChildNode(catIdx);
      wxString catName(catNode.getName());

      for (int propIdx = 0; propIdx < catNode.nChildNode(); ++propIdx)
      {
         XMLNode curNode = catNode.getChildNode(propIdx);
         wxString name(curNode.getName());
         wxString value(curNode.getAttribute(wxT("value")));

         values[wxString::Format(wxT("%s/%s"), catName, name)] = value;
      }
   }
}

void MonkeyPrefs::loadSequences (const wxString &sequencesFile)
{
   XMLResults r;
   XMLNode root = XMLNode::parseFile(sequencesFile, wxT("monkey-moore-sequences"), &r);

   if (r.error)
      throw monkeymoore_error(wxEmptyString, MMError_SequencesParseFailed);

   int n_charsets = root.nChildNode(wxT("sequence"));

   for (int i = 0; i < n_charsets; i++)
   {
      XMLNode cnode = root.getChildNode(i);

      wxString name = cnode.getAttribute(wxT("name"));
      wxString content = cnode.getText();

      common_charsets.push_back(std::make_pair(name, content));
   }
}

/**
* Saves the current set of preferences into the config file.
* @param configFile file name
* @param recreate true recreates the config file
* @return True on success, false on failure.
*/
void MonkeyPrefs::save (const wxString &configFile, bool recreate)
{
   if (recreate)
      setDefaultValues();

   XMLNode root = XMLNode::createXMLTopNode(wxT("monkey-moore-config"), false);
   root.addAttribute(wxT("version"), MM_VERSION);

   map<wxString, XMLNode> nodes;
   wregex pattern(wxT("([\\w-]+)/([\\w-]+)"));

   for (auto i = values.rbegin(); i != values.rend(); ++i)
   {
      wsmatch match;

      if (regex_match(i->first.ToStdWstring(), match, pattern))
      {
         wxString name = match[1].str();

         if (!nodes.count(name))
            nodes[name] = root.addChild(name);

         nodes[name].addChild(wxString(match[2])).addAttribute(wxT("value"), i->second);
      }
   }

   auto error = root.writeToFile(configFile, "UTF-16", 1);

   if (error != XMLError::eXMLErrorNone)
      throw monkeymoore_error(
         wxString::Format(_("Unable to write to %s"), configFile),
         MMError_ConfigFileWriteError
      );
}

void MonkeyPrefs::saveSequences (const wxString &fileName, bool recreate)
{
   if (recreate)
   {
      common_charsets.clear();
      common_charsets.push_back(make_pair(wxT("Default Hiragana sequence"), MM_DEFAULT_HIRAGANA));
      common_charsets.push_back(make_pair(wxT("Default Katakana sequence"), MM_DEFAULT_KATAKANA));
   }

   XMLNode root = XMLNode::createXMLTopNode(wxT("monkey-moore-sequences"), false);

   for (auto i = common_charsets.begin(); i != common_charsets.end(); ++i)
   {
      XMLNode seq = root.addChild(wxT("sequence"));
      seq.addAttribute(wxT("name"), i->first);
      seq.addText(i->second);
   }

   auto error = root.writeToFile(fileName, "UTF-16", 1);

   if (error != XMLError::eXMLErrorNone)
      throw monkeymoore_error(
         wxString::Format(_("Unable to write to %s."), fileName),
         MMError_SequencesFileWriteError
      );
}