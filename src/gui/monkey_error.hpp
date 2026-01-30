// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MONKEYERROR_HPP
#define MONKEYERROR_HPP

#include <wx/wxprec.h>

#ifdef __BORLANDC__
   #pragma hdrstop
#endif

#ifndef WX_PRECOMP
   #include <wx/wx.h>
#endif

#include <stdexcept>

enum
{
   MMError_ConfigParseFailed,
   MMError_ConfigVersionMismatch,
   MMError_ConfigFileWriteError,
   MMError_SequencesParseFailed,
   MMError_SequencesFileWriteError
};

class monkeymoore_error : public std::runtime_error
{
public:
   monkeymoore_error (const wxString &msg, int errorId) :
      std::runtime_error(msg.c_str()), m_errorId(errorId) { }

   int code () const { return m_errorId; }

private:
   int m_errorId;
};


#endif //~MONKEYERROR_HPP