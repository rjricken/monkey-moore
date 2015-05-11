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

#ifndef MONKEYERROR_HPP
#define MONKEYERROR_HPP

#include <wx/wxprec.h>

#ifdef __BORLANDC__
   #pragma hdrstop
#endif

#ifndef WX_PRECOMP
   #include <wx/wx.h>
#endif

#include <exception>

enum
{
   MMError_ConfigParseFailed,
   MMError_ConfigVersionMismatch,
   MMError_ConfigFileWriteError,
   MMError_SequencesParseFailed,
   MMError_SequencesFileWriteError
};

class monkeymoore_error : public std::exception
{
public:
   monkeymoore_error (const wxString &msg, int errorId) :
      std::exception(msg.c_str()), m_errorId(errorId) { }

   int code () const { return m_errorId; }

private:
   int m_errorId;
};


#endif //~MONKEYERROR_HPP