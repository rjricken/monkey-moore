#ifndef DROP_TARGET_HPP
#define DROP_TARGET_HPP

#include <wx/dnd.h>

class MonkeyFrame;

class SearchFileDropTarget : public wxFileDropTarget {
public:
    SearchFileDropTarget(MonkeyFrame *owner);

    virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames) override;

private:
    MonkeyFrame *m_owner;
};

#endif