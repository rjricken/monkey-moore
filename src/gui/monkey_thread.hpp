// SPDX-License-Identifier: GPL-3.0-or-later

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
#include <atomic>

#include "constants.hpp"
#include "byteswap.hpp"
#include "mmoore/search_engine.hpp"

wxDECLARE_EVENT(mmEVT_SEARCHTHREAD_UPDATE, wxThreadEvent);
wxDECLARE_EVENT(mmEVT_SEARCHTHREAD_COMPLETED, wxThreadEvent);
wxDECLARE_EVENT(mmEVT_SEARCHTHREAD_ABORTED, wxThreadEvent);
wxDECLARE_EVENT(mmEVT_SEARCHTHREAD_FAILED, wxThreadEvent);

/**
* Represents a full-fledged detached thread of execution used to manage the search process.
* @tparam DataType Basic underlying type used to represent the data.
*/
template <typename DataType>
class SearchThread : public wxThread
{
public:
   using result_type = mmoore::SearchResult<DataType>;

   SearchThread (
      mmoore::SearchConfig config, 
      std::vector<result_type> &results, 
      MonkeyFrame *parent
   ) : wxThread(), m_config(config), m_results(results), m_frame(parent)
   {
      wxASSERT(parent != 0);
   }

   /**
   * Method called when the thread is executed.
   * @return Thread-defined return code.
   */
   virtual void *Entry () {
      try {
         auto progress_callback = [this](int percent, const mmoore::SearchStep step) {
            if (m_frame->IsSearchAborted()) {
               m_abort_flag = true;
            }

            std::string message;

            switch(step) {
               case mmoore::SearchStep::Initializing:
                  message = _("Initializing...");
                  break;
               case mmoore::SearchStep::Searching:
                  message = _("Searching...");
                  break;
               case mmoore::SearchStep::GeneratingPreviews:
                  message = _("Generating previews...");
                  break;
               case mmoore::SearchStep::Aborting:
                  message = _("Aborting...");
                  break;
            }

            if (message.empty()) {
               throw std::runtime_error("Unreachable - missing SearchStep enum value");
            }
            
            NotifyMainThread(mmEVT_SEARCHTHREAD_UPDATE, wxString(message), percent);
         };

         mmoore::SearchEngine<DataType> engine(m_config);
         auto results = engine.run(progress_callback, m_abort_flag, true);

         if (m_abort_flag) {
            NotifyMainThread(mmEVT_SEARCHTHREAD_ABORTED);
            return NULL;
         }

         m_results = std::move(results);
         NotifyMainThread(mmEVT_SEARCHTHREAD_COMPLETED);
      }
      catch(const std::exception &e) {
         NotifyMainThread(mmEVT_SEARCHTHREAD_FAILED, e.what());
      }
      catch(...) {
         NotifyMainThread(
            mmEVT_SEARCHTHREAD_FAILED,
            _("Unknown fatal error occurred in search thread."));
      }

      return NULL;
   }

private:
   void NotifyMainThread (wxEventType evtType, wxString msg = wxEmptyString, int progress = 0) {
      wxThreadEvent *evt = new wxThreadEvent(evtType);
      evt->SetString(msg);

      if (evtType == mmEVT_SEARCHTHREAD_UPDATE) {
         evt->SetInt(progress);
      }

      wxQueueEvent(m_frame, evt);
   }

   void CancelSearch() {
      m_abort_flag = true;
   }

   mmoore::SearchConfig m_config;
   MonkeyFrame *m_frame;
   std::atomic<bool> m_abort_flag;
   std::vector<result_type> &m_results;
};

#endif //~MONKEY_THREAD_HPP
