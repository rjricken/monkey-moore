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

#ifndef MONKEY_THREAD_HPP
#define MONKEY_THREAD_HPP

#include <wx/wxprec.h>

#ifdef __BORLANDC__
   #pragma hdrstop
#endif

#ifndef WX_PRECOMP
   #include <wx/wx.h>
#endif

#include <wx/file.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <memory>
#include <future>
#include <thread>
#include <tuple>

#include "constants.hpp"
#include "byteswap.hpp"
#include "monkey_moore.hpp"

using namespace std;

wxDECLARE_EVENT(mmEVT_SEARCHTHREAD_UPDATE, wxThreadEvent);
wxDECLARE_EVENT(mmEVT_SEARCHTHREAD_COMPLETED, wxThreadEvent);
wxDECLARE_EVENT(mmEVT_SEARCHTHREAD_ABORTED, wxThreadEvent);

/**
* Structure to keep track of the parameters used in the search.
* It works for both types of searches - relative and value.
*/
struct SearchParameters
{
   /**
   * Constructor, relative search version.
   * @param[in] file Pointer to a previously allocated wxFile object.
   * @param[in] keyw,pattern,wcard Parameters needed to perform the search.
   */
   SearchParameters (shared_ptr<wxFile> &file, const wxString &keyw, const wxString &pattern, const wxChar wcard) :
      m_file(move(file)), keyword(keyw), pattern(pattern), wildcard(wcard),
      search_type(relative), endianness(little_endian) { }

   /**
   * Constructor, value scan version.
   * @param[in] file Pointer to a previously allocated wxFile object.
   * @param[in] vals Vector of values needed for a value scan search.
   */
   SearchParameters (shared_ptr<wxFile> &file, vector <short> vals) :
      m_file(move(file)), values(vals), search_type(value_scan), endianness(little_endian) { }

   /**
   * Returns the number of characters in the keyword.
   * Character in this context may be letters or numeric values.
   * @return Keyword length.
   */
   uint32_t keylen () const {
      return static_cast<uint32_t>(search_type == relative ? keyword.length() : values.size());
   }

   /**
   * Sets the endianness to be used in multi-byte searches
   * @param byteorder The desired endianness, possible values are: little_endian or big_endian
   */
   void setEndianness (int byteorder) {
      endianness = static_cast<decltype(endianness)>(byteorder);
   }

   enum { relative, value_scan } search_type;
   enum { little_endian, big_endian } endianness;

   shared_ptr<wxFile> m_file;

   wxString keyword;   /**< Keyword, only valid for relative searches */
   wxString pattern;   /**< Custom character sequence, valid for relative searches */
   wxChar wildcard;    /**< Character used as wildcard on relative searches */

   vector <short> values;  /**< Values used on value scan searches */
};

/**
* Represents a full-fledged detached thread of execution used to manage the search process.
* @tparam _Type Basic underlying type used to represent the data.
*/
template <typename _Type>
class SearchThread : public wxThread
{
public:
   typedef tuple<wxFileOffset, typename MonkeyMoore<_Type>::equivalency_type, wxString> result_type;
   typedef pair<wxFileOffset, unsigned int> datablock_type;

   SearchThread (SearchParameters p, vector<result_type> &results, MonkeyPrefs &mp, MonkeyFrame *mf) :
   wxThread(), m_info(p), m_results(results), m_prefs(mp), m_frame(mf)
   {
      wxASSERT(m_frame != 0);
      m_multiByteSearch = sizeof(_Type) > 1;
   }

   /**
   * Method called when the thread is executed.
   * @return Thread-defined return code.
   */
   virtual void *Entry ()
   {
      NotifyMainThread(mmEVT_SEARCHTHREAD_UPDATE, _("Initializing..."));

      // creates a monkey-moore instance based on which type of search will be performed
      unique_ptr<MonkeyMoore<_Type>> moore(
         m_info.search_type == SearchParameters::relative ?
            new MonkeyMoore<_Type>(m_info.keyword, m_info.wildcard, m_info.pattern) :
            new MonkeyMoore<_Type>(m_info.values)
      );

      const wxFileOffset fileSize = m_info.m_file->Length();
      const uint32_t blockBaseSize = 524288;

      const auto dataTypeSize = sizeof(_Type);
      const uint32_t kwOverlapSize = (m_info.keylen() - 1) * dataTypeSize;
      const uint32_t blockSize = blockBaseSize + kwOverlapSize + dataTypeSize - 1;

      // number of blocks
      const uint32_t numBlocks = static_cast<uint32_t>(ceil(double(fileSize) / blockBaseSize));

      vector<datablock_type> blocks;


      wxLogDebug("fileSize: %I64d", fileSize);
      wxLogDebug("kwOverlapSize: %u", kwOverlapSize);
      wxLogDebug("dataTypeSize: %u", dataTypeSize);
      wxLogDebug("blockSize: %u", blockSize);
      wxLogDebug("numBlocks: %u\n", numBlocks);

      for (uint32_t i = 0; i < numBlocks; ++i)
      {
         // each block has some extra overlapping bytes so we don't miss
         // a possible match split between two different blocks.
         wxFileOffset thisBlockOffset = i * blockBaseSize;
         uint32_t thisBlockSize = min(blockSize, static_cast<uint32_t>(fileSize - thisBlockOffset));

         wxLogDebug("block #%u: offset(%I64d) size(%u)", i, thisBlockOffset, thisBlockSize);

         blocks.push_back(make_pair(thisBlockOffset, thisBlockSize));
      }



      






      // keeps track of progress
      const float progressInc = 100.0f / numBlocks;
      float totalProgress = 0.0f;

      int maxThreads = thread::hardware_concurrency();
      int threadsRunning = 0;

      // data access synchronization objects
      mutex threadCountMutex;
      mutex resultsMutex;
      mutex progressMutex;

      auto nextBlock = blocks.begin();

      // loops until the last block of data has been passed through to a new thread
      while (nextBlock != blocks.end())
      {
         unique_lock<mutex> threadCountLock(threadCountMutex);

         // create a new thread if the max number of concurrent threads has not been reached
         if (threadsRunning < maxThreads)
         {
            threadCountLock.unlock();

            shared_ptr<u8> blockData(new u8[nextBlock->second], default_delete<u8[]>());

            m_info.m_file->Seek(nextBlock->first, wxFromStart);
            m_info.m_file->Read(blockData.get(), nextBlock->second);

            // _______________________________________________________________________________________
            // this lambda is responsible for running the appropriate search algorithm,
            // adjusting the offset of each result and appending them to the results pool.
            auto search = [&, this] (shared_ptr<u8> data, wxFileOffset offset, uint32_t size, uint32_t blockNumber)
            {
               wxString dbgOutput =
                  wxString::Format("  thread launched for #%u block: [%I64d-%I64d]\n",
                     blockNumber, offset, offset + size);

               for (uint32_t padding = 0; padding < dataTypeSize; ++padding)
               {
                  _Type *dataPtr = reinterpret_cast<_Type *>(data.get() + padding);
                  uint32_t dataSize = static_cast<uint32_t>(floor(double(size) / dataTypeSize));

                  if (reinterpret_cast<uint8_t *>(dataPtr + dataSize) > data.get() + size)
                     dataSize--;

                  dbgOutput +=
                     wxString::Format("    searching block #%u: padding=%u, [%I64d-%I64d]\n",
                        blockNumber, padding, offset + padding, offset + padding + dataSize * dataTypeSize);

                  // swap bytes when needed
                  if (m_multiByteSearch)
                     HandleEndianness(dataPtr, dataSize, m_info.endianness == SearchParameters::little_endian);

                  auto localResults = moore->search(dataPtr, dataSize);

                  {
                     // prevent other threads from modifying the results while we're using it
                     lock_guard<mutex> lock(resultsMutex);
                  
                     for (auto elem = localResults.begin(); elem != localResults.end(); ++elem)
                     {
                        // correct the offset for multibyte searches
                        wxFileOffset off = offset + elem->first * dataTypeSize + padding;
                        m_results.push_back(make_tuple(off, elem->second, wxT("")));
                     }
                  }
               }

               {
                  lock_guard<mutex> lock(progressMutex);
                  totalProgress += progressInc;

                  NotifyMainThread(mmEVT_SEARCHTHREAD_UPDATE,
                     _("Searching..."), static_cast<int>(ceil(totalProgress)));
               }
               {
                  // frees a slot for a new thread to be spawned
                  lock_guard<mutex> lock(threadCountMutex);
                  --threadsRunning;
               }

               wxLogDebug(dbgOutput);
            };
            // _______________________________________________________________________________________

            uint32_t curBlockNum = distance(blocks.begin(), nextBlock);
            wxLogDebug("Launching thread for #%u block", curBlockNum);
            
            //                                                                           v remove  v
            async(launch::async, search, blockData, nextBlock->first, nextBlock->second, curBlockNum);

            {
               lock_guard<mutex> lock(threadCountMutex);
               ++threadsRunning;
            }
            ++nextBlock;
         }
         else
         {
            threadCountLock.unlock();

            // we can't spawn more threads, so wee hint the OS scheduler to free the
            // remaining time on our timeslice to allow the other threads to execute.
            wxThread::Yield();
            wxThread::Sleep(100);
         }

         // checks if the search was aborted in the main thread
         if (m_frame->IsSearchAborted())
         {
            WaitRunningThreads(threadCountMutex, threadsRunning);

            NotifyMainThread(mmEVT_SEARCHTHREAD_ABORTED);
            return NULL;
         }
      }

      // we need to wait until all threads have finished
      WaitRunningThreads(threadCountMutex, threadsRunning);

      NotifyMainThread(mmEVT_SEARCHTHREAD_UPDATE, _("Generating previews..."), 100);

      sort(m_results.begin(), m_results.end());

      // generates previews
      for (auto i = m_results.begin(); i != m_results.end(); i++)
         get<2>(*i) = GeneratePreview(get<0>(*i), get<1>(*i));

      NotifyMainThread(mmEVT_SEARCHTHREAD_COMPLETED);

      return NULL;
   }

private:
   /**
   * Check the endianness of the system against the desired endianness in the search
   * and swap byte positions when _Type is a multibyte type.
   * @param data Target block of data
   * @param size Size of the block of data in bytes
   * @param littleEndian If the search is little endian or not
   */
   void HandleEndianness (_Type *dataPtr, uint32_t dataSize, bool littleEndian)
   {
      bool sysLittleEndian = m_sysinfo.GetEndianness() == wxEndianness::wxENDIAN_LITTLE;

      // swap bytes only when the endianness of the system is different
      // from the the endianness defined in the search options
      if ((sysLittleEndian && !littleEndian) || (!sysLittleEndian && littleEndian))
      {
         transform(dataPtr, dataPtr + dataSize, dataPtr, [](_Type elem) -> _Type {
            return swap_always<_Type>(elem);
         });
      }
   }

   /**
   * Wait until all threads have finished execution.
   * Each thread will decrement a counter variable upon finishing, so  we wait
   * until this variable reaches 0, which means we have no threads executing.
   * @param[in] threadCountMutex Synchronization object to the trhead count variable
   * @param[in] threadsRunning Number of threads running
   */
   void WaitRunningThreads (mutex &threadCountMutex, const int &threadsRunning)
   {
      while (true)
      {
         {
            lock_guard<mutex> lock(threadCountMutex);
            
            if (!threadsRunning)
               return;
         }

         wxThread::Yield();
         wxThread::Sleep(100);
      }
   }

   void NotifyMainThread (wxEventType evtType, wxString msg = wxEmptyString, int progress = 0)
   {
      wxThreadEvent *evt = new wxThreadEvent(evtType);

      if (evtType == mmEVT_SEARCHTHREAD_UPDATE)
      {
         evt->SetString(msg);
         evt->SetInt(progress);
      }

      wxQueueEvent(m_frame, evt);
   }

   /**
   * Generates a preview for each search result.
   * @param offset result offset in the file
   * @param table equivalency table
   * @return Result preview.
   */
   wxString GeneratePreview (const wxFileOffset offset, const typename MonkeyMoore<_Type>::equivalency_type &table)
   {
      const int width = m_prefs.getInt(wxT("settings/display-preview-width"));

      const uint32_t kwAlignWidth = floor(double(m_info.keyword.size()) / 2);

      int64_t offsetDelta = sizeof(_Type) * roundUp((width / 2) - kwAlignWidth, sizeof(_Type));

      if (m_info.keyword.size() > width)
         offsetDelta = 0;

      // changes the offset so we can put the keyword in the center of the preview
      wxFileOffset nice_pos = offset - offsetDelta;
      //const wxFileOffset read_offset = nice_pos >= 0 ? roundUp(nice_pos) : 0;
      wxFileOffset read_offset = nice_pos >= 0 ? nice_pos : 0;

      //read_offset += offset % sizeof(_Type) ? 1 : 0

      wxLogDebug("Generating preview at 0x%I64X: width(%i) kwAlignWdth(%u) offsetDelta(%I64d) readOffset(0x%I64X)",
         offset, width, kwAlignWidth, offsetDelta, read_offset);




      
      bool matchOffsetAligned = offset % sizeof(_Type) ? false : true;
      bool niceOffsetAligned = read_offset % sizeof(_Type) ? false : true;

      if ((matchOffsetAligned && !niceOffsetAligned) || (!matchOffsetAligned && niceOffsetAligned)) {
         wxLogDebug("Preview generation offset alignment mismatch");
      }



      unique_ptr<_Type[]> raw_data(new _Type[width]);

      m_info.m_file->Seek(read_offset, wxFromStart);
      m_info.m_file->Read(raw_data.get(), width * sizeof(_Type));

      _Type *rawDataPtr = raw_data.get();

      // swap bytes when needed
      if (m_multiByteSearch)
         HandleEndianness(rawDataPtr, width, m_info.endianness == SearchParameters::little_endian);

      wxString result;

      if (m_info.search_type == SearchParameters::relative)
      {
         // maps the table entries
         map <_Type, wxChar> cur_table;

         // generates the table
         for (MonkeyMoore<_Type>::equivalency_type::const_iterator i = table.begin(); i != table.end(); i++)
         {
            if (!m_info.pattern.length() && (i->first == wxT('A') || i->first == wxT('a')))
               for (int j = 0; j < 26; j++)
                  cur_table[i->second + static_cast <_Type> (j)] = i->first + static_cast <wxChar> (j);
            else
               cur_table[i->second] = i->first;
         }

         // replace the available characters
         for (_Type *start = rawDataPtr; start != rawDataPtr + width; start++)
            result += cur_table.count(*start) ? cur_table[*start] : wxT('#');
      }
      else
      {
         for (_Type *start = rawDataPtr; start != rawDataPtr + width; start++)
            result += wxString::Format(wxT("%02X "), *start);

         // erase trailing whitespace
         result.erase(result.length() - 1);
      }

      return result;
   }

   /**
   * Rounds a number up to the next multiple that is a power of 2.
   * @param num Number to be rounded.
   * @param multiple a power of 2 multiple
   * @return Rounded number
   */
   inline int64_t roundUp (int64_t num, int32_t multiple) {
      return (num + multiple - 1) & ~(multiple - 1);
   }

   bool m_multiByteSearch;
   SearchParameters m_info;
   wxPlatformInfo m_sysinfo;
   MonkeyFrame *m_frame;
   MonkeyPrefs &m_prefs;

   vector <result_type> &m_results;
};

#endif //~MONKEY_THREAD_HPP
