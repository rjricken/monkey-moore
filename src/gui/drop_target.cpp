#include "drop_target.hpp"

#include "monkey_frame.hpp"

SearchFileDropTarget::SearchFileDropTarget(
   MonkeyFrame *owner
) : m_owner(owner) {

}

bool SearchFileDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames) {
   if (filenames.IsEmpty()) {
      return false;
   }

   wxString droppedFilePath = filenames[0];

   m_owner->SetTargetFile(droppedFilePath);

   return true;
}