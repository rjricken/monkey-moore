// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#define MM_VERSION wxT("1.0a")
#define MM_BUILD   __TDATE__

// main frame outer border size in pixels
#define MM_BORDER 8

#define MM_CONFIG_FILE wxT("config.xml")
#define MM_CUSTOMSEQS_FILE wxT("customseqs.xml")
#define MM_GITHUB_REPO_URL wxT("https://github.com/rjricken/monkey-moore")

#define MM_ERRORCAPTION _("Monkey-Moore - Error")

// common dialog messages
#define MM_MSG_SAVETBL _("Choose where you want to save the table...")
#define MM_MSG_OPENFL  _("Choose the file you want to search on...")

// warnings
#define MM_WARNING_NOFILE            _("You must choose a file to perform the search on.")
#define MM_WARNING_FILENOTFOUND      _("Unable to open the specified file.\nCheck if it exists and if it's not being\nused by other applications.")
#define MM_WARNING_FILECANTACCESS    _("Unable to open the specified file for reading.\nCheck if it's not being used by other applications.")
#define MM_WARNING_KWORDSIZE         _("You must input a keyword with 3 or more characters.")
#define MM_WARNING_KWORDCAPLETTERS   _("The keyword must have at least\n3 upper or lower characters.")
#define MM_WARNING_KWORDLETTERS      _("The keyword must have at least\n3 letters, excluding wildcards.")
#define MM_WARNING_KWORDINVALIDCHARS _("Only letters and wildcards are supported.\nYou may not use any other characters.")
#define MM_WARNING_KWORDNONWILDCARD  _("You must input 3 or more non-wildcard characters.")
#define MM_WARNING_KWORDCPMISMATCH   _("You must input a keyword containg ONLY characters found in your defined charset.")
#define MM_WARNING_VSRINVALIDVAL     _("Invalid value found. You should input only\nnon-negative decimal numbers.")
#define MM_WARNING_CHARPATWILDCARD   _("You cannot use the defined wildcard character in your custom charset.")
#define MM_WARNING_CHARPATDUPLICATED _("The defined character set may not contain duplicated characters.")
#define MM_WARNING_NOWC              _("The wildcard option is enabled.\nYou must input the desired wildcard in the field.")
#define MM_WARNING_MANYWC            _("Only one character should be used as wildcard.\nRemove extra characters.")
#define MM_WARNING_NORESULTSELECTED  _("You must choose an item from the results\nlist in order to create a Thingy table.")
#define MM_WARNING_TABLENORESULTS    _("You cannot create a table while there are no results.")
#define MM_WARNING_THREADERROR       _("Unable to start a worker thread to perform the search.")

#define MM_DEFAULT_HIRAGANA wxT("\u3042\u3044\u3046\u3048\u304A\u304B\u304D\u304F\u3051\u3053\u3055\u3057\u3059\u305B\u305D\u305F\u3061\u3064\u3066\u3068\u306A\u306B\u306C\u306D\u306E\u306F\u3072\u3075\u3078\u307B\u307E\u307F\u3080\u3081\u3082\u3084\u3086\u3088\u3089\u308A\u308B\u308C\u308D\u308F\u3092\u3083\u3063\u3085\u3087")
#define MM_DEFAULT_KATAKANA wxT("\u30A2\u30A4\u30A6\u30A8\u30AA\u30AB\u30AD\u30AF\u30B1\u30B3\u30B5\u30B7\u30B9\u30BB\u30BD\u30BF\u30C1\u30C4\u30C6\u30C8\u30CA\u30CB\u30CC\u30CD\u30CE\u30CF\u30D2\u30D5\u30D8\u30DB\u30DE\u30DF\u30E0\u30E1\u30E2\u30E4\u30E6\u30E8\u30E9\u30EA\u30EB\u30EC\u30ED\u30EF\u30F2\u30E3\u30C3\u30E5\u30E7")

// identifiers
enum
{
   // main frame
   MonkeyMoore_FName = wxID_HIGHEST,
   MonkeyMoore_KWord,
   MonkeyMoore_Browse,
   MonkeyMoore_RelativeSearch,
   MonkeyMoore_ValueScanSearch,
   MonkeyMoore_Search,
   MonkeyMoore_UseWC,
   MonkeyMoore_Wildcard,
   MonkeyMoore_8bitMode,
   MonkeyMoore_16bitMode,
   MonkeyMoore_32bitMode,
   MonkeyMoore_Advanced,
   MonkeyMoore_EnableCP,
   MonkeyMoore_CharPattern,
   MonkeyMoore_CharsetList,
   MonkeyMoore_EnableByteOrder,
   MonkeyMoore_ByteOrderLE,
   MonkeyMoore_ByteOrderBE,
   MonkeyMoore_AllResults,
   MonkeyMoore_Results,
   MonkeyMoore_CreateTbl,
   MonkeyMoore_Clear,
   MonkeyMoore_Options,
   MonkeyMoore_About,
   MonkeyMoore_Cancel,
   MonkeyMoore_CopyAddress,
   MonkeyMoore_Counter,
   MonkeyMoore_Progress,
   MonkeyMoore_ElapsedTime,

   // options panel
   MonkeyOptions_AlwaysCenter,
   MonkeyOptions_RememberSize,
   MonkeyOptions_RememberPos,
   MonkeyOptions_RememberUI,
   MonkeyOptions_PreviewWidth,
   MonkeyOptions_OffsetHex,
   MonkeyOptions_OffsetDec,
   MonkeyOptions_MemoryPool,
   MonkeyOptions_MaxNumThreads,

   // table panel
   MonkeyTable_DataTable,
   MonkeyTable_FileName,
   MonkeyTable_SaveTable,
   MonkeyTable_FileFmt,
   MonkeyTable_FileEnc,

   // custom character sequences panel
   MonkeySeqs_DataTable,
   MonkeySeqs_AddNew,
   MonkeySeqs_Remove,
   MonkeySeqs_SaveChanges,
   MonkeySeqs_Cancel,
   
   // notifications
   MonkeyThread_JobFinished,
   MonkeyThread_JobAborted,

   // custom character sequences menu
   MonkeyMoore_ManageCharSeqs,
   MonkeyMoore_CharsetBase
};

// image indexes
enum
{
   MonkeyBmp_OpenFile,
   MonkeyBmp_OpenFileGrayed,
   MonkeyBmp_Search,
   MonkeyBmp_SearchGrayed,
   MonkeyBmp_Options,
   MonkeyBmp_OptionsGrayed,
   MonkeyBmp_About,
   MonkeyBmp_AboutGrayed,
   MonkeyBmp_Sequences,
   MonkeyBmp_SequencesGrayed,
   MonkeyBmp_MngSequences,
   MonkeyBmp_New,
   MonkeyBmp_TrashCan,
   MonkeyBmp_TrashCanGrayed,
   MonkeyBmp_SaveFile,
   MonkeyBmp_Copy,
   MonkeyBmp_Done,
   MonkeyBmp_Cancel,
   MonkeyBmp_CancelGrayed,
   MonkeyBmp_ShowAdv,
   MonkeyBmp_ShowAdvGrayed,
   MonkeyBmp_HideAdv,
   MonkeyBmp_HideAdvGrayed
};

enum
{
   ResultListCol_Offset = 80,
   ResultListCol_Values = 100,
   ResultListCol_Preview = 200
};

#endif //~CONSTANTS_HPP
