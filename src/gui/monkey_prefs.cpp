// SPDX-License-Identifier: GPL-3.0-or-later

#include "monkey_prefs.hpp"
#include "monkey_error.hpp"
#include "constants.hpp"

#include <tinyxml2.h>
#include <map>
#include <vector>

static wxString fromXml(const char *value) {
   return value ? wxString::FromUTF8(value) : wxString();
}

/**
* Fills Monkey-Moore properties with the default (factory) values.
*/
void MonkeyPrefs::setDefaultValues () {
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

void MonkeyPrefs::load (const wxString &config_file) {
   tinyxml2::XMLDocument doc;
   tinyxml2::XMLError err = doc.LoadFile(config_file.mb_str());

   if (err != tinyxml2::XML_SUCCESS) {
      throw monkeymoore_error(
         wxString::Format(
            _("An error prevented user preferences from being loaded. File: %s"), 
            config_file
         ),
         MMError_ConfigParseFailed
      );
   }

   tinyxml2::XMLElement *root = doc.FirstChildElement("monkey-moore-config");
   if (!root) {
      throw monkeymoore_error(
         _("Invalid configuration file: missing root node."),
         MMError_ConfigParseFailed
      );
   }

   const char *version = root->Attribute("version");
   if (!version || wxString(version) != MM_VERSION) {
      throw monkeymoore_error(
         _("Monkey-Moore version is different from the version used in the user preferences file."),
         MMError_ConfigVersionMismatch
      );
   }

   tinyxml2::XMLElement *category = root->FirstChildElement();
   while (category) {
      wxString cat_name = fromXml(category->Name());

      tinyxml2::XMLElement *property = category->FirstChildElement();
      while (property) {
         wxString prop_name = fromXml(property->Name());
         const char *value_attr = property->Attribute("value");

         if (value_attr) {
            values[cat_name + wxT("/") + prop_name] = fromXml(value_attr);
         }

         property = property->NextSiblingElement();
      }

      category = category->NextSiblingElement();
   }
}

void MonkeyPrefs::loadSequences (const wxString &sequences_file) {
   tinyxml2::XMLDocument doc;
   tinyxml2::XMLError err = doc.LoadFile(sequences_file.mb_str());

   if (err != tinyxml2::XML_SUCCESS) {
      throw monkeymoore_error(
         wxString::Format(
            _("Error loading custom character sequences. File: %s"), 
            sequences_file
         ),
         MMError_SequencesParseFailed
      );
   }

   tinyxml2::XMLElement *root = doc.FirstChildElement("monkey-moore-sequences");
   if (!root) {
      throw monkeymoore_error(
         wxString::Format(
            _("Invalid sequences file: missing root node. File: %s"), 
            sequences_file
         ),
         MMError_SequencesParseFailed
      );
   }

   tinyxml2::XMLElement *sequence = root->FirstChildElement("sequence");

   while (sequence) {
      wxString name = fromXml(sequence->Attribute("name"));
      wxString content = fromXml(sequence->GetText());

      common_charsets.push_back(std::make_pair(name, content));
      sequence = sequence->NextSiblingElement("sequence");
   }
}

void MonkeyPrefs::save (const wxString &config_file, bool recreate) {
   if (recreate) {
      setDefaultValues();
   }

   tinyxml2::XMLDocument doc;

   tinyxml2::XMLDeclaration *declaration = doc.NewDeclaration();
   doc.InsertEndChild(declaration);

   tinyxml2::XMLElement *root = doc.NewElement("monkey-moore-config");
   root->SetAttribute("version", wxString(MM_VERSION).ToUTF8());
   doc.InsertEndChild(root);

   std::map<wxString, tinyxml2::XMLElement *> category_nodes_map;

   for (auto const &pair : values) {
      wxString cat_name = pair.first.BeforeFirst(wxT('/'));
      wxString prop_name = pair.first.AfterFirst(wxT('/'));

      if (cat_name.IsEmpty() || prop_name.IsEmpty()) {
         continue;
      }

      tinyxml2::XMLElement *cat_elem = nullptr;

      if (category_nodes_map.find(cat_name) == category_nodes_map.end()) {
         cat_elem = doc.NewElement(cat_name.ToUTF8());
         root->InsertEndChild(cat_elem);

         category_nodes_map[cat_name] = cat_elem;
      }
      else {
         cat_elem = category_nodes_map[cat_name];
      }

      tinyxml2::XMLElement *prop_elem = doc.NewElement(prop_name.ToUTF8());
      prop_elem->SetAttribute("value", pair.second.ToUTF8());
      cat_elem->InsertEndChild(prop_elem);
   }

   tinyxml2::XMLError err = doc.SaveFile(config_file.mb_str());

   if (err != tinyxml2::XML_SUCCESS) {
      throw monkeymoore_error(
         wxString::Format(_("Failed to save user preferences. File: %s"), config_file),
         MMError_ConfigFileWriteError
      );
   }
}

void MonkeyPrefs::saveSequences(const wxString &file_name, bool recreate) {
   if (recreate) {
      common_charsets.clear();
      common_charsets.push_back(std::make_pair(wxT("Default Hiragana sequence"), MM_DEFAULT_HIRAGANA));
      common_charsets.push_back(std::make_pair(wxT("Default Katakana sequence"), MM_DEFAULT_KATAKANA));
   }

   tinyxml2::XMLDocument doc;
   tinyxml2::XMLDeclaration *declaration = doc.NewDeclaration();
   doc.InsertEndChild(declaration);

   tinyxml2::XMLElement *root = doc.NewElement("monkey-moore-sequences");
   doc.InsertEndChild(root);

   for (auto const &pair : common_charsets) {
      tinyxml2::XMLElement *seq_elem = doc.NewElement("sequence");
      
      seq_elem->SetAttribute("name", pair.first.ToUTF8());
      seq_elem->SetText(pair.second.ToUTF8());
      
      root->InsertEndChild(seq_elem);
   }

   tinyxml2::XMLError err = doc.SaveFile(file_name.mb_str());

   if (err != tinyxml2::XML_SUCCESS) {
      throw monkeymoore_error(
         wxString::Format(
            _("Failed to save custom character sequences. File: %s"),
            file_name
         ),
         MMError_SequencesFileWriteError
      );
   }
}