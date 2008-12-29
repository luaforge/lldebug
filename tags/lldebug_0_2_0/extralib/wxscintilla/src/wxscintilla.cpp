////////////////////////////////////////////////////////////////////////////
// Name:        wxscintilla.cpp
// Purpose:     A wxWidgets implementation of Scintilla.  This class is the
//              one meant to be used directly by wx applications.  It does not
//              derive directly from the Scintilla classes, but instead
//              delegates most things to the real Scintilla class.
//              This allows the use of Scintilla without polluting the
//              namespace with all the classes and identifiers from Scintilla.
//
// Author:      Robin Dunn
//
// Created:     13-Jan-2000
// RCS-ID:      $Id: wxscintilla.cpp,v 1.1 2008-12-29 02:24:46 cielacanth Exp $
// Copyright:   (c) 2004 wxCode
// Licence:     wxWindows
/////////////////////////////////////////////////////////////////////////////

// modified
// wxscintilla.cpp,v 1.34 2007/12/19 07:37:29 cielacanth Exp $

#include <ctype.h>

#include "ScintillaWX.h"
#include "wx/wxscintilla.h"

#include "wx/wx.h"
#include "wx/tokenzr.h"
#include "wx/mstream.h"
#include "wx/image.h"
#include "wx/file.h"


//----------------------------------------------------------------------

const wxChar* wxSCINameStr = _T("SCIwindow");

#ifdef MAKELONG
#undef MAKELONG
#endif

#define MAKELONG(a, b) ((a) | ((b) << 16))


static long wxColourAsLong(const wxColour& co) {
    return (((long)co.Blue()  << 16) |
            ((long)co.Green() <<  8) |
            ((long)co.Red()));
}

static wxColour wxColourFromLong(long c) {
    wxColour clr;
    clr.Set((unsigned char)(c & 0xff),
            (unsigned char)((c >> 8) & 0xff),
            (unsigned char)((c >> 16) & 0xff));
    return clr;
}


static wxColour wxColourFromSpec(const wxString& spec) {
    // spec should be a colour name or "#RRGGBB"
    if (spec.GetChar(0) == _T('#')) {

        long red, green, blue;
        red = green = blue = 0;
        spec.Mid(1,2).ToLong(&red,   16);
        spec.Mid(3,2).ToLong(&green, 16);
        spec.Mid(5,2).ToLong(&blue,  16);
        return wxColour((unsigned char)red, (unsigned char)green, (unsigned char)blue);
    }else{
        return wxColour(spec);
    }
}

//----------------------------------------------------------------------

DEFINE_EVENT_TYPE( wxEVT_SCI_CHANGE )
DEFINE_EVENT_TYPE( wxEVT_SCI_STYLENEEDED )
DEFINE_EVENT_TYPE( wxEVT_SCI_CHARADDED )
DEFINE_EVENT_TYPE( wxEVT_SCI_SAVEPOINTREACHED )
DEFINE_EVENT_TYPE( wxEVT_SCI_SAVEPOINTLEFT )
DEFINE_EVENT_TYPE( wxEVT_SCI_ROMODIFYATTEMPT )
DEFINE_EVENT_TYPE( wxEVT_SCI_KEY )
DEFINE_EVENT_TYPE( wxEVT_SCI_DOUBLECLICK )
DEFINE_EVENT_TYPE( wxEVT_SCI_UPDATEUI )
DEFINE_EVENT_TYPE( wxEVT_SCI_MODIFIED )
DEFINE_EVENT_TYPE( wxEVT_SCI_MACRORECORD )
DEFINE_EVENT_TYPE( wxEVT_SCI_MARGINCLICK )
DEFINE_EVENT_TYPE( wxEVT_SCI_NEEDSHOWN )
DEFINE_EVENT_TYPE( wxEVT_SCI_PAINTED )
DEFINE_EVENT_TYPE( wxEVT_SCI_USERLISTSELECTION )
DEFINE_EVENT_TYPE( wxEVT_SCI_URIDROPPED )
DEFINE_EVENT_TYPE( wxEVT_SCI_DWELLSTART )
DEFINE_EVENT_TYPE( wxEVT_SCI_DWELLEND )
DEFINE_EVENT_TYPE( wxEVT_SCI_START_DRAG )
DEFINE_EVENT_TYPE( wxEVT_SCI_DRAG_OVER )
DEFINE_EVENT_TYPE( wxEVT_SCI_DO_DROP )
DEFINE_EVENT_TYPE( wxEVT_SCI_ZOOM )
DEFINE_EVENT_TYPE( wxEVT_SCI_HOTSPOT_CLICK )
DEFINE_EVENT_TYPE( wxEVT_SCI_HOTSPOT_DCLICK )
DEFINE_EVENT_TYPE( wxEVT_SCI_CALLTIP_CLICK )
DEFINE_EVENT_TYPE( wxEVT_SCI_AUTOCOMP_SELECTION )



BEGIN_EVENT_TABLE(wxScintilla, wxControl)
    EVT_PAINT                   (wxScintilla::OnPaint)
    EVT_SCROLLWIN               (wxScintilla::OnScrollWin)
    EVT_SCROLL                  (wxScintilla::OnScroll)
    EVT_SIZE                    (wxScintilla::OnSize)
    EVT_LEFT_DOWN               (wxScintilla::OnMouseLeftDown)
    // Let Scintilla see the double click as a second click
    EVT_LEFT_DCLICK             (wxScintilla::OnMouseLeftDown)
    EVT_MOTION                  (wxScintilla::OnMouseMove)
    EVT_LEFT_UP                 (wxScintilla::OnMouseLeftUp)
#if defined(__WXGTK__) || defined(__WXMAC__)
    EVT_RIGHT_UP                (wxScintilla::OnMouseRightUp)
#else
    EVT_CONTEXT_MENU            (wxScintilla::OnContextMenu)
#endif
    EVT_MOUSEWHEEL              (wxScintilla::OnMouseWheel)
    EVT_MIDDLE_UP               (wxScintilla::OnMouseMiddleUp)
    EVT_CHAR                    (wxScintilla::OnChar)
    EVT_KEY_DOWN                (wxScintilla::OnKeyDown)
    EVT_KILL_FOCUS              (wxScintilla::OnLoseFocus)
    EVT_SET_FOCUS               (wxScintilla::OnGainFocus)
    EVT_SYS_COLOUR_CHANGED      (wxScintilla::OnSysColourChanged)
    EVT_ERASE_BACKGROUND        (wxScintilla::OnEraseBackground)
    EVT_MENU_RANGE              (10, 16, wxScintilla::OnMenu)
    EVT_LISTBOX_DCLICK          (wxID_ANY, wxScintilla::OnListBox)
END_EVENT_TABLE()


IMPLEMENT_CLASS(wxScintilla, wxControl)
IMPLEMENT_DYNAMIC_CLASS(wxScintillaEvent, wxCommandEvent)

#ifdef LINK_LEXERS
// forces the linking of the lexer modules
int Scintilla_LinkLexers();
#endif

//----------------------------------------------------------------------
// Constructor and Destructor

wxScintilla::wxScintilla (wxWindow *parent,
                          wxWindowID id,
                          const wxPoint& pos,
                          const wxSize& size,
                          long style,
                          const wxString& name) {
    m_swx = NULL;
    Create (parent, id, pos, size, style, name);
}


bool wxScintilla::Create (wxWindow *parent,
                          wxWindowID id,
                          const wxPoint& pos,
                          const wxSize& size,
                          long style,
                          const wxString& name) {
#ifdef __WXMAC__
    style |= wxVSCROLL | wxHSCROLL;
#endif
    if (!wxControl::Create (parent, id, pos, size,
                            style | wxWANTS_CHARS | wxCLIP_CHILDREN,
                            wxDefaultValidator, name)) {
        return false;
    }

#ifdef LINK_LEXERS
    Scintilla_LinkLexers();
#endif
    m_swx = new ScintillaWX(this);
    m_stopWatch.Start();
    m_lastKeyDownConsumed = FALSE;
    m_vScrollBar = NULL;
    m_hScrollBar = NULL;
#if wxUSE_UNICODE
    // Put Scintilla into unicode (UTF-8) mode
    SetCodePage(wxSCI_CP_UTF8);
#endif

#if wxCHECK_VERSION(2, 5, 0)
    // Reduces flicker on GTK+/X11
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    SetBestFittingSize(size);
#endif
    return true;
}

wxScintilla::~wxScintilla() {
    delete m_swx;
}


//----------------------------------------------------------------------
// Send message to Scintilla
sptr_t wxScintilla::SendMsg (unsigned int msg, uptr_t wp, sptr_t lp) {
    return m_swx->WndProc (msg, wp, lp);
}

//----------------------------------------------------------------------

// Set the vertical scrollbar to use instead of the ont that's built-in.
void wxScintilla::SetVScrollBar (wxScrollBar* bar) {
    m_vScrollBar = bar;
    if (bar != NULL) {
        // ensure that the built-in scrollbar is not visible
        SetScrollbar(wxVERTICAL, 0, 0, 0);
    }
}

// Set the horizontal scrollbar to use instead of the ont that's built-in.
void wxScintilla::SetHScrollBar (wxScrollBar* bar) {
    m_hScrollBar = bar;
    if (bar != NULL) {
        // ensure that the built-in scrollbar is not visible
        SetScrollbar(wxHORIZONTAL, 0, 0, 0);
    }
}

//----------------------------------------------------------------------
// BEGIN generated section.  The following code is automatically generated
//       by gen_iface.py from the contents of Scintilla.iface.  Do not edit
//       this file.  Edit wxscintilla.cpp.in or gen_iface.py instead and regenerate.


// Add text to the document at current position.
void wxScintilla::AddText(const wxString& text) {
                    wxWX2MBbuf buf = (wxWX2MBbuf)wx2sci(text);
                    SendMsg(2001, (uptr_t)strlen(buf), (sptr_t)(const char*)buf);
}

// Add array of cells to document.
void wxScintilla::AddStyledText(const wxMemoryBuffer& data) {
                          SendMsg(2002, data.GetDataLen(), (sptr_t)data.GetData());
}

// Insert string at a position.
void wxScintilla::InsertText(int pos, const wxString& text) {
    SendMsg(2003, pos, (sptr_t)(const char*)wx2sci(text));
}

// Delete all text in the document.
void wxScintilla::ClearAll() {
    SendMsg(2004, 0, 0);
}

// Set all style bytes to 0, remove all folding information.
void wxScintilla::ClearDocumentStyle() {
    SendMsg(2005, 0, 0);
}

// Returns the number of characters in the document.
int wxScintilla::GetLength() {
    return SendMsg(2006, 0, 0);
}

// Returns the character byte at the position.
int wxScintilla::GetCharAt(int pos) {
         return (unsigned char)SendMsg(2007, pos, 0);
}

// Returns the position of the caret.
int wxScintilla::GetCurrentPos() {
    return SendMsg(2008, 0, 0);
}

// Returns the position of the opposite end of the selection to the caret.
int wxScintilla::GetAnchor() {
    return SendMsg(2009, 0, 0);
}

// Returns the style byte at the position.
int wxScintilla::GetStyleAt(int pos) {
         return (unsigned char)SendMsg(2010, pos, 0);
}

// Redoes the next action on the undo history.
void wxScintilla::Redo() {
    SendMsg(2011, 0, 0);
}

// Choose between collecting actions into the undo
// history and discarding them.
void wxScintilla::SetUndoCollection(bool collectUndo) {
    SendMsg(2012, collectUndo, 0);
}

// Select all the text in the document.
void wxScintilla::SelectAll() {
    SendMsg(2013, 0, 0);
}

// Remember the current position in the undo history as the position
// at which the document was saved.
void wxScintilla::SetSavePoint() {
    SendMsg(2014, 0, 0);
}

// Retrieve a buffer of cells.
wxMemoryBuffer wxScintilla::GetStyledText(int startPos, int endPos) {
        wxMemoryBuffer buf;
        if (endPos < startPos) {
            int temp = startPos;
            startPos = endPos;
            endPos = temp;
        }
        int len = endPos - startPos;
        if (!len) return buf;
        TextRange tr;
        tr.lpstrText = (char*)buf.GetWriteBuf(len*2+1);
        tr.chrg.cpMin = startPos;
        tr.chrg.cpMax = endPos;
        len = SendMsg(2015, 0, (sptr_t)&tr);
        buf.UngetWriteBuf(len);
        return buf;
}

// Are there any redoable actions in the undo history?
bool wxScintilla::CanRedo() {
    return SendMsg(2016, 0, 0) != 0;
}

// Retrieve the line number at which a particular marker is located.
int wxScintilla::MarkerLineFromHandle(int handle) {
    return SendMsg(2017, handle, 0);
}

// Delete a marker.
void wxScintilla::MarkerDeleteHandle(int handle) {
    SendMsg(2018, handle, 0);
}

// Is undo history being collected?
bool wxScintilla::GetUndoCollection() {
    return SendMsg(2019, 0, 0) != 0;
}

// Are white space characters currently visible?
// Returns one of SCWS_* constants.
int wxScintilla::GetViewWhiteSpace() {
    return SendMsg(2020, 0, 0);
}

// Make white space characters invisible, always visible or visible outside indentation.
void wxScintilla::SetViewWhiteSpace(int viewWS) {
    SendMsg(2021, viewWS, 0);
}

// Find the position from a point within the window.
int wxScintilla::PositionFromPoint(wxPoint pt) {
        return SendMsg(2022, pt.x, pt.y);
}

// Find the position from a point within the window but return
// INVALID_POSITION if not close to text.
int wxScintilla::PositionFromPointClose(int x, int y) {
    return SendMsg(2023, x, y);
}

// Set caret to start of a line and ensure it is visible.
void wxScintilla::GotoLine(int line) {
    SendMsg(2024, line, 0);
}

// Set caret to a position and ensure it is visible.
void wxScintilla::GotoPos(int pos) {
    SendMsg(2025, pos, 0);
}

// Set the selection anchor to a position. The anchor is the opposite
// end of the selection from the caret.
void wxScintilla::SetAnchor(int posAnchor) {
    SendMsg(2026, posAnchor, 0);
}

// Retrieve the text of the line containing the caret.
// Returns the index of the caret on the line.
wxString wxScintilla::GetCurLine(int* linePos) {
        int len = LineLength(GetCurrentLine());
        if (!len) {
            if (linePos)  *linePos = 0;
            return wxEmptyString;
        }

        wxMemoryBuffer mbuf(len+1);
        char* buf = (char*)mbuf.GetWriteBuf(len+1);

        int pos = SendMsg(2027, len+1, (sptr_t)buf);
        mbuf.UngetWriteBuf(len);
        mbuf.AppendByte(0);
        if (linePos)  *linePos = pos;
        return sci2wx(buf);
}

// Retrieve the position of the last correctly styled character.
int wxScintilla::GetEndStyled() {
    return SendMsg(2028, 0, 0);
}

// Convert all line endings in the document to one mode.
void wxScintilla::ConvertEOLs(int eolMode) {
    SendMsg(2029, eolMode, 0);
}

// Retrieve the current end of line mode - one of CRLF, CR, or LF.
int wxScintilla::GetEOLMode() {
    return SendMsg(2030, 0, 0);
}

// Set the current end of line mode.
void wxScintilla::SetEOLMode(int eolMode) {
    SendMsg(2031, eolMode, 0);
}

// Set the current styling position to pos and the styling mask to mask.
// The styling mask can be used to protect some bits in each styling byte from modification.
void wxScintilla::StartStyling(int pos, int mask) {
    SendMsg(2032, pos, mask);
}

// Change style from current styling position for length characters to a style
// and move the current styling position to after this newly styled segment.
void wxScintilla::SetStyling(int length, int style) {
    SendMsg(2033, length, style);
}

// Is drawing done first into a buffer or direct to the screen?
bool wxScintilla::GetBufferedDraw() {
    return SendMsg(2034, 0, 0) != 0;
}

// If drawing is buffered then each line of text is drawn into a bitmap buffer
// before drawing it to the screen to avoid flicker.
void wxScintilla::SetBufferedDraw(bool buffered) {
    SendMsg(2035, buffered, 0);
}

// Change the visible size of a tab to be a multiple of the width of a space character.
void wxScintilla::SetTabWidth(int tabWidth) {
    SendMsg(2036, tabWidth, 0);
}

// Retrieve the visible size of a tab.
int wxScintilla::GetTabWidth() {
    return SendMsg(2121, 0, 0);
}

// Set the code page used to interpret the bytes of the document as characters.
void wxScintilla::SetCodePage(int codePage) {
#if wxUSE_UNICODE
    wxASSERT_MSG(codePage == wxSCI_CP_UTF8,
                 wxT("Only wxSTC_CP_UTF8 may be used when wxUSE_UNICODE is on."));
#else
    wxASSERT_MSG(codePage != wxSCI_CP_UTF8,
                 wxT("wxSTC_CP_UTF8 may not be used when wxUSE_UNICODE is off."));
#endif
    SendMsg(2037, codePage);
}

// Set the symbol used for a particular marker number,
// and optionally the fore and background colours.
void wxScintilla::MarkerDefine(int markerNumber, int markerSymbol,
                const wxColour& foreground,
                const wxColour& background) {

                SendMsg(2040, markerNumber, markerSymbol);
                if (foreground.Ok())
                    MarkerSetForeground(markerNumber, foreground);
                if (background.Ok())
                    MarkerSetBackground(markerNumber, background);
}

// Set the foreground colour used for a particular marker number.
void wxScintilla::MarkerSetForeground(int markerNumber, const wxColour& fore) {
    SendMsg(2041, markerNumber, wxColourAsLong(fore));
}

// Set the background colour used for a particular marker number.
void wxScintilla::MarkerSetBackground(int markerNumber, const wxColour& back) {
    SendMsg(2042, markerNumber, wxColourAsLong(back));
}

// Add a marker to a line, returning an ID which can be used to find or delete the marker.
int wxScintilla::MarkerAdd(int line, int markerNumber) {
    return SendMsg(2043, line, markerNumber);
}

// Delete a marker from a line.
void wxScintilla::MarkerDelete(int line, int markerNumber) {
    SendMsg(2044, line, markerNumber);
}

// Delete all markers with a particular number from all lines.
void wxScintilla::MarkerDeleteAll(int markerNumber) {
    SendMsg(2045, markerNumber, 0);
}

// Get a bit mask of all the markers set on a line.
int wxScintilla::MarkerGet(int line) {
    return SendMsg(2046, line, 0);
}

// Find the next line after lineStart that includes a marker in mask.
int wxScintilla::MarkerNext(int lineStart, int markerMask) {
    return SendMsg(2047, lineStart, markerMask);
}

// Find the previous line before lineStart that includes a marker in mask.
int wxScintilla::MarkerPrevious(int lineStart, int markerMask) {
    return SendMsg(2048, lineStart, markerMask);
}

// Define a marker from a bitmap
void wxScintilla::MarkerDefineBitmap(int markerNumber, const wxBitmap& bmp) {
        // convert bmp to a xpm in a string
        wxMemoryOutputStream strm;
        wxImage img = bmp.ConvertToImage();
        if (img.HasAlpha())
            img.ConvertAlphaToMask();
        img.SaveFile(strm, wxBITMAP_TYPE_XPM);
        size_t len = strm.GetSize();
        char* buff = new char[len+1];
        strm.CopyTo(buff, len);
        buff[len] = 0;
        SendMsg(2049, markerNumber, (sptr_t)buff);
        delete [] buff;
        
}

// Add a set of markers to a line.
void wxScintilla::MarkerAddSet(int line, int set) {
    SendMsg(2466, line, set);
}

// Set the alpha used for a marker that is drawn in the text area, not the margin.
void wxScintilla::MarkerSetAlpha(int markerNumber, int alpha) {
    SendMsg(2476, markerNumber, alpha);
}

// Set a margin to be either numeric or symbolic.
void wxScintilla::SetMarginType(int margin, int marginType) {
    SendMsg(2240, margin, marginType);
}

// Retrieve the type of a margin.
int wxScintilla::GetMarginType(int margin) {
    return SendMsg(2241, margin, 0);
}

// Set the width of a margin to a width expressed in pixels.
void wxScintilla::SetMarginWidth(int margin, int pixelWidth) {
    SendMsg(2242, margin, pixelWidth);
}

// Retrieve the width of a margin in pixels.
int wxScintilla::GetMarginWidth(int margin) {
    return SendMsg(2243, margin, 0);
}

// Set a mask that determines which markers are displayed in a margin.
void wxScintilla::SetMarginMask(int margin, int mask) {
    SendMsg(2244, margin, mask);
}

// Retrieve the marker mask of a margin.
int wxScintilla::GetMarginMask(int margin) {
    return SendMsg(2245, margin, 0);
}

// Make a margin sensitive or insensitive to mouse clicks.
void wxScintilla::SetMarginSensitive(int margin, bool sensitive) {
    SendMsg(2246, margin, sensitive);
}

// Retrieve the mouse click sensitivity of a margin.
bool wxScintilla::GetMarginSensitive(int margin) {
    return SendMsg(2247, margin, 0) != 0;
}

// Clear all the styles and make equivalent to the global default style.
void wxScintilla::StyleClearAll() {
    SendMsg(2050, 0, 0);
}

// Set the foreground colour of a style.
void wxScintilla::StyleSetForeground(int style, const wxColour& fore) {
    SendMsg(2051, style, wxColourAsLong(fore));
}

// Set the background colour of a style.
void wxScintilla::StyleSetBackground(int style, const wxColour& back) {
    SendMsg(2052, style, wxColourAsLong(back));
}

// Set a style to be bold or not.
void wxScintilla::StyleSetBold(int style, bool bold) {
    SendMsg(2053, style, bold);
}

// Set a style to be italic or not.
void wxScintilla::StyleSetItalic(int style, bool italic) {
    SendMsg(2054, style, italic);
}

// Set the size of characters of a style.
void wxScintilla::StyleSetSize(int style, int sizePoints) {
    SendMsg(2055, style, sizePoints);
}

// Set the font of a style.
void wxScintilla::StyleSetFaceName(int style, const wxString& fontName) {
    SendMsg(2056, style, (sptr_t)(const char*)wx2sci(fontName));
}

// Set a style to have its end of line filled or not.
void wxScintilla::StyleSetEOLFilled(int style, bool filled) {
    SendMsg(2057, style, filled);
}

// Reset the default style to its state at startup
void wxScintilla::StyleResetDefault() {
    SendMsg(2058, 0, 0);
}

// Set a style to be underlined or not.
void wxScintilla::StyleSetUnderline(int style, bool underline) {
    SendMsg(2059, style, underline);
}

// Get the foreground colour of a style.
wxColour wxScintilla::StyleGetForeground(int style) {
    long c = SendMsg(2481, style, 0);
    return wxColourFromLong(c);
}

// Get the background colour of a style.
wxColour wxScintilla::StyleGetBackground(int style) {
    long c = SendMsg(2482, style, 0);
    return wxColourFromLong(c);
}

// Get is a style bold or not.
bool wxScintilla::StyleGetBold(int style) {
    return SendMsg(2483, style, 0) != 0;
}

// Get is a style italic or not.
bool wxScintilla::StyleGetItalic(int style) {
    return SendMsg(2484, style, 0) != 0;
}

// Get the size of characters of a style.
int wxScintilla::StyleGetSize(int style) {
    return SendMsg(2485, style, 0);
}

// Get the font facename of a style
wxString wxScintilla::StyleGetFaceName(int style) {
         long msg = 2486;
         long len = SendMsg(msg, style, 0);
         wxMemoryBuffer mbuf(len+1);
         char* buf = (char*)mbuf.GetWriteBuf(len+1);
         SendMsg(msg, style, (sptr_t)buf);
         mbuf.UngetWriteBuf(len);
         mbuf.AppendByte(0);
         return sci2wx(buf);
}

// Get is a style to have its end of line filled or not.
bool wxScintilla::StyleGetEOLFilled(int style) {
    return SendMsg(2487, style, 0) != 0;
}

// Get is a style underlined or not.
bool wxScintilla::StyleGetUnderline(int style) {
    return SendMsg(2488, style, 0) != 0;
}

// Get is a style mixed case, or to force upper or lower case.
int wxScintilla::StyleGetCase(int style) {
    return SendMsg(2489, style, 0);
}

// Get the character get of the font in a style.
int wxScintilla::StyleGetCharacterSet(int style) {
    return SendMsg(2490, style, 0);
}

// Get is a style visible or not.
bool wxScintilla::StyleGetVisible(int style) {
    return SendMsg(2491, style, 0) != 0;
}

// Get is a style changeable or not (read only).
// Experimental feature, currently buggy.
bool wxScintilla::StyleGetChangeable(int style) {
    return SendMsg(2492, style, 0) != 0;
}

// Get is a style a hotspot or not.
bool wxScintilla::StyleGetHotSpot(int style) {
    return SendMsg(2493, style, 0) != 0;
}

// Set a style to be mixed case, or to force upper or lower case.
void wxScintilla::StyleSetCase(int style, int caseForce) {
    SendMsg(2060, style, caseForce);
}

// Set a style to be a hotspot or not.
void wxScintilla::StyleSetHotSpot(int style, bool hotspot) {
    SendMsg(2409, style, hotspot);
}

// Set the foreground colour of the selection and whether to use this setting.
void wxScintilla::SetSelForeground(bool useSetting, const wxColour& fore) {
    SendMsg(2067, useSetting, wxColourAsLong(fore));
}

// Set the background colour of the selection and whether to use this setting.
void wxScintilla::SetSelBackground(bool useSetting, const wxColour& back) {
    SendMsg(2068, useSetting, wxColourAsLong(back));
}

// Get the alpha of the selection.
int wxScintilla::GetSelAlpha() {
    return SendMsg(2477, 0, 0);
}

// Set the alpha of the selection.
void wxScintilla::SetSelAlpha(int alpha) {
    SendMsg(2478, alpha, 0);
}

// Is the selection end of line filled?
bool wxScintilla::GetSelEOLFilled() {
    return SendMsg(2479, 0, 0) != 0;
}

// Set the selection to have its end of line filled or not.
void wxScintilla::SetSelEOLFilled(bool filled) {
    SendMsg(2480, filled, 0);
}

// Set the foreground colour of the caret.
void wxScintilla::SetCaretForeground(const wxColour& fore) {
    SendMsg(2069, wxColourAsLong(fore), 0);
}

// When key+modifier combination km is pressed perform msg.
void wxScintilla::CmdKeyAssign(int key, int modifiers, int cmd) {
         SendMsg(2070, MAKELONG(key, modifiers), cmd);
}

// When key+modifier combination km is pressed do nothing.
void wxScintilla::CmdKeyClear(int key, int modifiers) {
         SendMsg(2071, MAKELONG(key, modifiers));
}

// Drop all key mappings.
void wxScintilla::CmdKeyClearAll() {
    SendMsg(2072, 0, 0);
}

// Set the styles for a segment of the document.
void wxScintilla::SetStyleBytes(int length, char* styleBytes) {
        SendMsg(2073, length, (sptr_t)styleBytes);
}

// Set a style to be visible or not.
void wxScintilla::StyleSetVisible(int style, bool visible) {
    SendMsg(2074, style, visible);
}

// Get the time in milliseconds that the caret is on and off.
int wxScintilla::GetCaretPeriod() {
    return SendMsg(2075, 0, 0);
}

// Get the time in milliseconds that the caret is on and off. 0 = steady on.
void wxScintilla::SetCaretPeriod(int periodMilliseconds) {
    SendMsg(2076, periodMilliseconds, 0);
}

// Set the set of characters making up words for when moving or selecting by word.
// First sets deaults like SetCharsDefault.
void wxScintilla::SetWordChars(const wxString& characters) {
    SendMsg(2077, 0, (sptr_t)(const char*)wx2sci(characters));
}

// Start a sequence of actions that is undone and redone as a unit.
// May be nested.
void wxScintilla::BeginUndoAction() {
    SendMsg(2078, 0, 0);
}

// End a sequence of actions that is undone and redone as a unit.
void wxScintilla::EndUndoAction() {
    SendMsg(2079, 0, 0);
}

// Set an indicator to plain, squiggle or TT.
void wxScintilla::IndicatorSetStyle(int indic, int style) {
    SendMsg(2080, indic, style);
}

// Retrieve the style of an indicator.
int wxScintilla::IndicatorGetStyle(int indic) {
    return SendMsg(2081, indic, 0);
}

// Set the foreground colour of an indicator.
void wxScintilla::IndicatorSetForeground(int indic, const wxColour& fore) {
    SendMsg(2082, indic, wxColourAsLong(fore));
}

// Retrieve the foreground colour of an indicator.
wxColour wxScintilla::IndicatorGetForeground(int indic) {
    long c = SendMsg(2083, indic, 0);
    return wxColourFromLong(c);
}

// Set an indicator to draw under text or over(default).
void wxScintilla::IndicatorSetUnder(int indic, bool under) {
    SendMsg(2510, indic, under);
}

// Retrieve whether indicator drawn under or over text.
bool wxScintilla::IndicatorGetUnder(int indic) {
    return SendMsg(2511, indic, 0) != 0;
}

// Set the foreground colour of all whitespace and whether to use this setting.
void wxScintilla::SetWhitespaceForeground(bool useSetting, const wxColour& fore) {
    SendMsg(2084, useSetting, wxColourAsLong(fore));
}

// Set the background colour of all whitespace and whether to use this setting.
void wxScintilla::SetWhitespaceBackground(bool useSetting, const wxColour& back) {
    SendMsg(2085, useSetting, wxColourAsLong(back));
}

// Divide each styling byte into lexical class bits (default: 5) and indicator
// bits (default: 3). If a lexer requires more than 32 lexical states, then this
// is used to expand the possible states.
void wxScintilla::SetStyleBits(int bits) {
    SendMsg(2090, bits, 0);
}

// Retrieve number of bits in style bytes used to hold the lexical state.
int wxScintilla::GetStyleBits() {
    return SendMsg(2091, 0, 0);
}

// Used to hold extra styling information for each line.
void wxScintilla::SetLineState(int line, int state) {
    SendMsg(2092, line, state);
}

// Retrieve the extra styling information for a line.
int wxScintilla::GetLineState(int line) {
    return SendMsg(2093, line, 0);
}

// Retrieve the last line number that has line state.
int wxScintilla::GetMaxLineState() {
    return SendMsg(2094, 0, 0);
}

// Is the background of the line containing the caret in a different colour?
bool wxScintilla::GetCaretLineVisible() {
    return SendMsg(2095, 0, 0) != 0;
}

// Display the background of the line containing the caret in a different colour.
void wxScintilla::SetCaretLineVisible(bool show) {
    SendMsg(2096, show, 0);
}

// Get the colour of the background of the line containing the caret.
wxColour wxScintilla::GetCaretLineBackground() {
    long c = SendMsg(2097, 0, 0);
    return wxColourFromLong(c);
}

// Set the colour of the background of the line containing the caret.
void wxScintilla::SetCaretLineBackground(const wxColour& back) {
    SendMsg(2098, wxColourAsLong(back), 0);
}

// Set a style to be changeable or not (read only).
// Experimental feature, currently buggy.
void wxScintilla::StyleSetChangeable(int style, bool changeable) {
    SendMsg(2099, style, changeable);
}

// Display a auto-completion list.
// The lenEntered parameter indicates how many characters before
// the caret should be used to provide context.
void wxScintilla::AutoCompShow(int lenEntered, const wxString& itemList) {
    SendMsg(2100, lenEntered, (sptr_t)(const char*)wx2sci(itemList));
}

// Remove the auto-completion list from the screen.
void wxScintilla::AutoCompCancel() {
    SendMsg(2101, 0, 0);
}

// Is there an auto-completion list visible?
bool wxScintilla::AutoCompActive() {
    return SendMsg(2102, 0, 0) != 0;
}

// Retrieve the position of the caret when the auto-completion list was displayed.
int wxScintilla::AutoCompPosStart() {
    return SendMsg(2103, 0, 0);
}

// User has selected an item so remove the list and insert the selection.
void wxScintilla::AutoCompComplete() {
    SendMsg(2104, 0, 0);
}

// Define a set of character that when typed cancel the auto-completion list.
void wxScintilla::AutoCompStops(const wxString& characterSet) {
    SendMsg(2105, 0, (sptr_t)(const char*)wx2sci(characterSet));
}

// Change the separator character in the string setting up an auto-completion list.
// Default is space but can be changed if items contain space.
void wxScintilla::AutoCompSetSeparator(int separatorCharacter) {
    SendMsg(2106, separatorCharacter, 0);
}

// Retrieve the auto-completion list separator character.
int wxScintilla::AutoCompGetSeparator() {
    return SendMsg(2107, 0, 0);
}

// Select the item in the auto-completion list that starts with a string.
void wxScintilla::AutoCompSelect(const wxString& text) {
    SendMsg(2108, 0, (sptr_t)(const char*)wx2sci(text));
}

// Should the auto-completion list be cancelled if the user backspaces to a
// position before where the box was created.
void wxScintilla::AutoCompSetCancelAtStart(bool cancel) {
    SendMsg(2110, cancel, 0);
}

// Retrieve whether auto-completion cancelled by backspacing before start.
bool wxScintilla::AutoCompGetCancelAtStart() {
    return SendMsg(2111, 0, 0) != 0;
}

// Define a set of characters that when typed will cause the autocompletion to
// choose the selected item.
void wxScintilla::AutoCompSetFillUps(const wxString& characterSet) {
    SendMsg(2112, 0, (sptr_t)(const char*)wx2sci(characterSet));
}

// Should a single item auto-completion list automatically choose the item.
void wxScintilla::AutoCompSetChooseSingle(bool chooseSingle) {
    SendMsg(2113, chooseSingle, 0);
}

// Retrieve whether a single item auto-completion list automatically choose the item.
bool wxScintilla::AutoCompGetChooseSingle() {
    return SendMsg(2114, 0, 0) != 0;
}

// Set whether case is significant when performing auto-completion searches.
void wxScintilla::AutoCompSetIgnoreCase(bool ignoreCase) {
    SendMsg(2115, ignoreCase, 0);
}

// Retrieve state of ignore case flag.
bool wxScintilla::AutoCompGetIgnoreCase() {
    return SendMsg(2116, 0, 0) != 0;
}

// Display a list of strings and send notification when user chooses one.
void wxScintilla::UserListShow(int listType, const wxString& itemList) {
    SendMsg(2117, listType, (sptr_t)(const char*)wx2sci(itemList));
}

// Set whether or not autocompletion is hidden automatically when nothing matches.
void wxScintilla::AutoCompSetAutoHide(bool autoHide) {
    SendMsg(2118, autoHide, 0);
}

// Retrieve whether or not autocompletion is hidden automatically when nothing matches.
bool wxScintilla::AutoCompGetAutoHide() {
    return SendMsg(2119, 0, 0) != 0;
}

// Set whether or not autocompletion deletes any word characters
// after the inserted text upon completion.
void wxScintilla::AutoCompSetDropRestOfWord(bool dropRestOfWord) {
    SendMsg(2270, dropRestOfWord, 0);
}

// Retrieve whether or not autocompletion deletes any word characters
// after the inserted text upon completion.
bool wxScintilla::AutoCompGetDropRestOfWord() {
    return SendMsg(2271, 0, 0) != 0;
}

// Register an image for use in autocompletion lists.
void wxScintilla::RegisterImage(int type, const wxBitmap& bmp) {
        // convert bmp to a xpm in a string
        wxMemoryOutputStream strm;
        wxImage img = bmp.ConvertToImage();
        if (img.HasAlpha())
            img.ConvertAlphaToMask();
        img.SaveFile(strm, wxBITMAP_TYPE_XPM);
        size_t len = strm.GetSize();
        char* buff = new char[len+1];
        strm.CopyTo(buff, len);
        buff[len] = 0;
        SendMsg(2405, type, (sptr_t)buff);
        delete [] buff;
     
}

// Clear all the registered images.
void wxScintilla::ClearRegisteredImages() {
    SendMsg(2408, 0, 0);
}

// Retrieve the auto-completion list type-separator character.
int wxScintilla::AutoCompGetTypeSeparator() {
    return SendMsg(2285, 0, 0);
}

// Change the type-separator character in the string setting up an auto-completion list.
// Default is '?' but can be changed if items contain '?'.
void wxScintilla::AutoCompSetTypeSeparator(int separatorCharacter) {
    SendMsg(2286, separatorCharacter, 0);
}

// Set the maximum width, in characters, of auto-completion and user lists.
// Set to 0 to autosize to fit longest item, which is the default.
void wxScintilla::AutoCompSetMaxWidth(int characterCount) {
    SendMsg(2208, characterCount, 0);
}

// Get the maximum width, in characters, of auto-completion and user lists.
int wxScintilla::AutoCompGetMaxWidth() {
    return SendMsg(2209, 0, 0);
}

// Set the maximum height, in rows, of auto-completion and user lists.
// The default is 5 rows.
void wxScintilla::AutoCompSetMaxHeight(int rowCount) {
    SendMsg(2210, rowCount, 0);
}

// Set the maximum height, in rows, of auto-completion and user lists.
int wxScintilla::AutoCompGetMaxHeight() {
    return SendMsg(2211, 0, 0);
}

// Set the number of spaces used for one level of indentation.
void wxScintilla::SetIndent(int indentSize) {
    SendMsg(2122, indentSize, 0);
}

// Retrieve indentation size.
int wxScintilla::GetIndent() {
    return SendMsg(2123, 0, 0);
}

// Indentation will only use space characters if useTabs is false, otherwise
// it will use a combination of tabs and spaces.
void wxScintilla::SetUseTabs(bool useTabs) {
    SendMsg(2124, useTabs, 0);
}

// Retrieve whether tabs will be used in indentation.
bool wxScintilla::GetUseTabs() {
    return SendMsg(2125, 0, 0) != 0;
}

// Change the indentation of a line to a number of columns.
void wxScintilla::SetLineIndentation(int line, int indentSize) {
    SendMsg(2126, line, indentSize);
}

// Retrieve the number of columns that a line is indented.
int wxScintilla::GetLineIndentation(int line) {
    return SendMsg(2127, line, 0);
}

// Retrieve the position before the first non indentation character on a line.
int wxScintilla::GetLineIndentPosition(int line) {
    return SendMsg(2128, line, 0);
}

// Retrieve the column number of a position, taking tab width into account.
int wxScintilla::GetColumn(int pos) {
    return SendMsg(2129, pos, 0);
}

// Show or hide the horizontal scroll bar.
void wxScintilla::SetUseHorizontalScrollBar(bool show) {
    SendMsg(2130, show, 0);
}

// Is the horizontal scroll bar visible?
bool wxScintilla::GetUseHorizontalScrollBar() {
    return SendMsg(2131, 0, 0) != 0;
}

// Show or hide indentation guides.
void wxScintilla::SetIndentationGuides(int indentView) {
    SendMsg(2132, indentView, 0);
}

// Are the indentation guides visible?
int wxScintilla::GetIndentationGuides() {
    return SendMsg(2133, 0, 0);
}

// Set the highlighted indentation guide column.
// 0 = no highlighted guide.
void wxScintilla::SetHighlightGuide(int column) {
    SendMsg(2134, column, 0);
}

// Get the highlighted indentation guide column.
int wxScintilla::GetHighlightGuide() {
    return SendMsg(2135, 0, 0);
}

// Get the position after the last visible characters on a line.
int wxScintilla::GetLineEndPosition(int line) {
    return SendMsg(2136, line, 0);
}

// Get the code page used to interpret the bytes of the document as characters.
int wxScintilla::GetCodePage() {
    return SendMsg(2137, 0, 0);
}

// Get the foreground colour of the caret.
wxColour wxScintilla::GetCaretForeground() {
    long c = SendMsg(2138, 0, 0);
    return wxColourFromLong(c);
}

// In read-only mode?
bool wxScintilla::GetReadOnly() {
    return SendMsg(2140, 0, 0) != 0;
}

// Sets the position of the caret.
void wxScintilla::SetCurrentPos(int pos) {
    SendMsg(2141, pos, 0);
}

// Sets the position that starts the selection - this becomes the anchor.
void wxScintilla::SetSelectionStart(int pos) {
    SendMsg(2142, pos, 0);
}

// Returns the position at the start of the selection.
int wxScintilla::GetSelectionStart() {
    return SendMsg(2143, 0, 0);
}

// Sets the position that ends the selection - this becomes the currentPosition.
void wxScintilla::SetSelectionEnd(int pos) {
    SendMsg(2144, pos, 0);
}

// Returns the position at the end of the selection.
int wxScintilla::GetSelectionEnd() {
    return SendMsg(2145, 0, 0);
}

// Sets the print magnification added to the point size of each style for printing.
void wxScintilla::SetPrintMagnification(int magnification) {
    SendMsg(2146, magnification, 0);
}

// Returns the print magnification.
int wxScintilla::GetPrintMagnification() {
    return SendMsg(2147, 0, 0);
}

// Modify colours when printing for clearer printed text.
void wxScintilla::SetPrintColourMode(int mode) {
    SendMsg(2148, mode, 0);
}

// Returns the print colour mode.
int wxScintilla::GetPrintColourMode() {
    return SendMsg(2149, 0, 0);
}

// Find some text in the document.
int wxScintilla::FindText(int minPos, int maxPos,
               const wxString& text,
               int flags) {
            TextToFind  ft;
            ft.chrg.cpMin = minPos;
            ft.chrg.cpMax = maxPos;
            wxWX2MBbuf buf = (wxWX2MBbuf)wx2sci(text);
            ft.lpstrText = (char*)(const char*)buf;

            return SendMsg(2150, flags, (sptr_t)&ft);
}

// On Windows, will draw the document into a display context such as a printer.
 int wxScintilla::FormatRange(bool   doDraw,
                int    startPos,
                int    endPos,
                wxDC*  draw,
                wxDC*  target, 
                wxRect renderRect,
                wxRect pageRect) {
             RangeToFormat fr;

             if (endPos < startPos) {
                 int temp = startPos;
                 startPos = endPos;
                 endPos = temp;
             }
             fr.hdc = draw;
             fr.hdcTarget = target;
             fr.rc.top = renderRect.GetTop();
             fr.rc.left = renderRect.GetLeft();
             fr.rc.right = renderRect.GetRight();
             fr.rc.bottom = renderRect.GetBottom();
             fr.rcPage.top = pageRect.GetTop();
             fr.rcPage.left = pageRect.GetLeft();
             fr.rcPage.right = pageRect.GetRight();
             fr.rcPage.bottom = pageRect.GetBottom();
             fr.chrg.cpMin = startPos;
             fr.chrg.cpMax = endPos;

             return SendMsg(2151, doDraw, (sptr_t)&fr);
}

// Retrieve the display line at the top of the display.
int wxScintilla::GetFirstVisibleLine() {
    return SendMsg(2152, 0, 0);
}

// Retrieve the contents of a line.
wxString wxScintilla::GetLine(int line) {
         int len = LineLength(line);
         if (!len) return wxEmptyString;

         wxMemoryBuffer mbuf(len+1);
         char* buf = (char*)mbuf.GetWriteBuf(len+1);
         SendMsg(2153, line, (sptr_t)buf);
         mbuf.UngetWriteBuf(len);
         mbuf.AppendByte(0);
         return sci2wx(buf);
}

// Returns the number of lines in the document. There is always at least one.
int wxScintilla::GetLineCount() {
    return SendMsg(2154, 0, 0);
}

// Sets the size in pixels of the left margin.
void wxScintilla::SetMarginLeft(int pixelWidth) {
    SendMsg(2155, 0, pixelWidth);
}

// Returns the size in pixels of the left margin.
int wxScintilla::GetMarginLeft() {
    return SendMsg(2156, 0, 0);
}

// Sets the size in pixels of the right margin.
void wxScintilla::SetMarginRight(int pixelWidth) {
    SendMsg(2157, 0, pixelWidth);
}

// Returns the size in pixels of the right margin.
int wxScintilla::GetMarginRight() {
    return SendMsg(2158, 0, 0);
}

// Is the document different from when it was last saved?
bool wxScintilla::GetModify() {
    return SendMsg(2159, 0, 0) != 0;
}

// Select a range of text.
void wxScintilla::SetSelection(int start, int end) {
    SendMsg(2160, start, end);
}

// Retrieve the selected text.
wxString wxScintilla::GetSelectedText() {
         int   start;
         int   end;

         GetSelection(&start, &end);
         int   len  = end - start;
         if (!len) return wxEmptyString;

         wxMemoryBuffer mbuf(len+2);
         char* buf = (char*)mbuf.GetWriteBuf(len+1);
         SendMsg(2161, 0, (sptr_t)buf);
         mbuf.UngetWriteBuf(len);
         mbuf.AppendByte(0);
         return sci2wx(buf);
}

// Retrieve a range of text.
wxString wxScintilla::GetTextRange(int startPos, int endPos) {
         if (endPos < startPos) {
             int temp = startPos;
             startPos = endPos;
             endPos = temp;
         }
         int   len  = endPos - startPos;
         if (!len) return wxEmptyString;
         wxMemoryBuffer mbuf(len+1);
         char* buf = (char*)mbuf.GetWriteBuf(len);
         TextRange tr;
         tr.lpstrText = buf;
         tr.chrg.cpMin = startPos;
         tr.chrg.cpMax = endPos;
         SendMsg(2162, 0, (sptr_t)&tr);
         mbuf.UngetWriteBuf(len);
         mbuf.AppendByte(0);
         return sci2wx(buf);
}

// Draw the selection in normal style or with selection highlighted.
void wxScintilla::HideSelection(bool normal) {
    SendMsg(2163, normal, 0);
}

// Retrieve the line containing a position.
int wxScintilla::LineFromPosition(int pos) {
    return SendMsg(2166, pos, 0);
}

// Retrieve the position at the start of a line.
int wxScintilla::PositionFromLine(int line) {
    return SendMsg(2167, line, 0);
}

// Scroll horizontally and vertically.
void wxScintilla::LineScroll(int columns, int lines) {
    SendMsg(2168, columns, lines);
}

// Ensure the caret is visible.
void wxScintilla::EnsureCaretVisible() {
    SendMsg(2169, 0, 0);
}

// Replace the selected text with the argument text.
void wxScintilla::ReplaceSelection(const wxString& text) {
    SendMsg(2170, 0, (sptr_t)(const char*)wx2sci(text));
}

// Set to read only or read write.
void wxScintilla::SetReadOnly(bool readOnly) {
    SendMsg(2171, readOnly, 0);
}

// Will a paste succeed?
bool wxScintilla::CanPaste() {
    return SendMsg(2173, 0, 0) != 0;
}

// Are there any undoable actions in the undo history?
bool wxScintilla::CanUndo() {
    return SendMsg(2174, 0, 0) != 0;
}

// Delete the undo history.
void wxScintilla::EmptyUndoBuffer() {
    SendMsg(2175, 0, 0);
}

// Undo one action in the undo history.
void wxScintilla::Undo() {
    SendMsg(2176, 0, 0);
}

// Cut the selection to the clipboard.
void wxScintilla::Cut() {
    SendMsg(2177, 0, 0);
}

// Copy the selection to the clipboard.
void wxScintilla::Copy() {
    SendMsg(2178, 0, 0);
}

// Paste the contents of the clipboard into the document replacing the selection.
void wxScintilla::Paste() {
    SendMsg(2179, 0, 0);
}

// Clear the selection.
void wxScintilla::Clear() {
    SendMsg(2180, 0, 0);
}

// Replace the contents of the document with the argument text.
void wxScintilla::SetText(const wxString& text) {
    SendMsg(2181, 0, (sptr_t)(const char*)wx2sci(text));
}

// Retrieve all the text in the document.
wxString wxScintilla::GetText() {
         int len  = GetTextLength();
         wxMemoryBuffer mbuf(len+1);   // leave room for the null...
         char* buf = (char*)mbuf.GetWriteBuf(len+1);
         SendMsg(2182, len+1, (sptr_t)buf);
         mbuf.UngetWriteBuf(len);
         mbuf.AppendByte(0);
         return sci2wx(buf);
}

// Retrieve the number of characters in the document.
int wxScintilla::GetTextLength() {
    return SendMsg(2183, 0, 0);
}

// Set to overtype (true) or insert mode.
void wxScintilla::SetOvertype(bool overtype) {
    SendMsg(2186, overtype, 0);
}

// Returns true if overtype mode is active otherwise false is returned.
bool wxScintilla::GetOvertype() {
    return SendMsg(2187, 0, 0) != 0;
}

// Set the width of the insert mode caret.
void wxScintilla::SetCaretWidth(int pixelWidth) {
    SendMsg(2188, pixelWidth, 0);
}

// Returns the width of the insert mode caret.
int wxScintilla::GetCaretWidth() {
    return SendMsg(2189, 0, 0);
}

// Sets the position that starts the target which is used for updating the
// document without affecting the scroll position.
void wxScintilla::SetTargetStart(int pos) {
    SendMsg(2190, pos, 0);
}

// Get the position that starts the target.
int wxScintilla::GetTargetStart() {
    return SendMsg(2191, 0, 0);
}

// Sets the position that ends the target which is used for updating the
// document without affecting the scroll position.
void wxScintilla::SetTargetEnd(int pos) {
    SendMsg(2192, pos, 0);
}

// Get the position that ends the target.
int wxScintilla::GetTargetEnd() {
    return SendMsg(2193, 0, 0);
}

// Replace the target text with the argument text.
// Text is counted so it can contain NULs.
// Returns the length of the replacement text.

     int wxScintilla::ReplaceTarget(const wxString& text) {
         wxWX2MBbuf buf = (wxWX2MBbuf)wx2sci(text);
         return SendMsg(2194, (uptr_t)strlen(buf), (sptr_t)(const char*)buf);
}

// Replace the target text with the argument text after \d processing.
// Text is counted so it can contain NULs.
// Looks for \d where d is between 1 and 9 and replaces these with the strings
// matched in the last search operation which were surrounded by \( and \).
// Returns the length of the replacement text including any change
// caused by processing the \d patterns.

     int wxScintilla::ReplaceTargetRE(const wxString& text) {
         wxWX2MBbuf buf = (wxWX2MBbuf)wx2sci(text);
         return SendMsg(2195, (uptr_t)strlen(buf), (sptr_t)(const char*)buf);
}

// Search for a counted string in the target and set the target to the found
// range. Text is counted so it can contain NULs.
// Returns length of range or -1 for failure in which case target is not moved.

     int wxScintilla::SearchInTarget(const wxString& text) {
         wxWX2MBbuf buf = (wxWX2MBbuf)wx2sci(text);
         return SendMsg(2197, (uptr_t)strlen(buf), (sptr_t)(const char*)buf);
}

// Set the search flags used by SearchInTarget.
void wxScintilla::SetSearchFlags(int flags) {
    SendMsg(2198, flags, 0);
}

// Get the search flags used by SearchInTarget.
int wxScintilla::GetSearchFlags() {
    return SendMsg(2199, 0, 0);
}

// Show a call tip containing a definition near position pos.
void wxScintilla::CallTipShow(int pos, const wxString& definition) {
    SendMsg(2200, pos, (sptr_t)(const char*)wx2sci(definition));
}

// Remove the call tip from the screen.
void wxScintilla::CallTipCancel() {
    SendMsg(2201, 0, 0);
}

// Is there an active call tip?
bool wxScintilla::CallTipActive() {
    return SendMsg(2202, 0, 0) != 0;
}

// Retrieve the position where the caret was before displaying the call tip.
int wxScintilla::CallTipPosAtStart() {
    return SendMsg(2203, 0, 0);
}

// Highlight a segment of the definition.
void wxScintilla::CallTipSetHighlight(int start, int end) {
    SendMsg(2204, start, end);
}

// Set the background colour for the call tip.
void wxScintilla::CallTipSetBackground(const wxColour& back) {
    SendMsg(2205, wxColourAsLong(back), 0);
}

// Set the foreground colour for the call tip.
void wxScintilla::CallTipSetForeground(const wxColour& fore) {
    SendMsg(2206, wxColourAsLong(fore), 0);
}

// Set the foreground colour for the highlighted part of the call tip.
void wxScintilla::CallTipSetForegroundHighlight(const wxColour& fore) {
    SendMsg(2207, wxColourAsLong(fore), 0);
}

// Enable use of STYLE_CALLTIP and set call tip tab size in pixels.
void wxScintilla::CallTipUseStyle(int tabSize) {
    SendMsg(2212, tabSize, 0);
}

// Find the display line of a document line taking hidden lines into account.
int wxScintilla::VisibleFromDocLine(int line) {
    return SendMsg(2220, line, 0);
}

// Find the document line of a display line taking hidden lines into account.
int wxScintilla::DocLineFromVisible(int lineDisplay) {
    return SendMsg(2221, lineDisplay, 0);
}

// The number of display lines needed to wrap a document line
int wxScintilla::WrapCount(int line) {
    return SendMsg(2235, line, 0);
}

// Set the fold level of a line.
// This encodes an integer level along with flags indicating whether the
// line is a header and whether it is effectively white space.
void wxScintilla::SetFoldLevel(int line, int level) {
    SendMsg(2222, line, level);
}

// Retrieve the fold level of a line.
int wxScintilla::GetFoldLevel(int line) {
    return SendMsg(2223, line, 0);
}

// Find the last child line of a header line.
int wxScintilla::GetLastChild(int line, int level) {
    return SendMsg(2224, line, level);
}

// Find the parent line of a child line.
int wxScintilla::GetFoldParent(int line) {
    return SendMsg(2225, line, 0);
}

// Make a range of lines visible.
void wxScintilla::ShowLines(int lineStart, int lineEnd) {
    SendMsg(2226, lineStart, lineEnd);
}

// Make a range of lines invisible.
void wxScintilla::HideLines(int lineStart, int lineEnd) {
    SendMsg(2227, lineStart, lineEnd);
}

// Is a line visible?
bool wxScintilla::GetLineVisible(int line) {
    return SendMsg(2228, line, 0) != 0;
}

// Show the children of a header line.
void wxScintilla::SetFoldExpanded(int line, bool expanded) {
    SendMsg(2229, line, expanded);
}

// Is a header line expanded?
bool wxScintilla::GetFoldExpanded(int line) {
    return SendMsg(2230, line, 0) != 0;
}

// Switch a header line between expanded and contracted.
void wxScintilla::ToggleFold(int line) {
    SendMsg(2231, line, 0);
}

// Ensure a particular line is visible by expanding any header line hiding it.
void wxScintilla::EnsureVisible(int line) {
    SendMsg(2232, line, 0);
}

// Set some style options for folding.
void wxScintilla::SetFoldFlags(int flags) {
    SendMsg(2233, flags, 0);
}

// Ensure a particular line is visible by expanding any header line hiding it.
// Use the currently set visibility policy to determine which range to display.
void wxScintilla::EnsureVisibleEnforcePolicy(int line) {
    SendMsg(2234, line, 0);
}

// Sets whether a tab pressed when caret is within indentation indents.
void wxScintilla::SetTabIndents(bool tabIndents) {
    SendMsg(2260, tabIndents, 0);
}

// Does a tab pressed when caret is within indentation indent?
bool wxScintilla::GetTabIndents() {
    return SendMsg(2261, 0, 0) != 0;
}

// Sets whether a backspace pressed when caret is within indentation unindents.
void wxScintilla::SetBackSpaceUnIndents(bool bsUnIndents) {
    SendMsg(2262, bsUnIndents, 0);
}

// Does a backspace pressed when caret is within indentation unindent?
bool wxScintilla::GetBackSpaceUnIndents() {
    return SendMsg(2263, 0, 0) != 0;
}

// Sets the time the mouse must sit still to generate a mouse dwell event.
void wxScintilla::SetMouseDwellTime(int periodMilliseconds) {
    SendMsg(2264, periodMilliseconds, 0);
}

// Retrieve the time the mouse must sit still to generate a mouse dwell event.
int wxScintilla::GetMouseDwellTime() {
    return SendMsg(2265, 0, 0);
}

// Get position of start of word.
int wxScintilla::WordStartPosition(int pos, bool onlyWordCharacters) {
    return SendMsg(2266, pos, onlyWordCharacters);
}

// Get position of end of word.
int wxScintilla::WordEndPosition(int pos, bool onlyWordCharacters) {
    return SendMsg(2267, pos, onlyWordCharacters);
}

// Sets whether text is word wrapped.
void wxScintilla::SetWrapMode(int mode) {
    SendMsg(2268, mode, 0);
}

// Retrieve whether text is word wrapped.
int wxScintilla::GetWrapMode() {
    return SendMsg(2269, 0, 0);
}

// Set the display mode of visual flags for wrapped lines.
void wxScintilla::SetWrapVisualFlags(int wrapVisualFlags) {
    SendMsg(2460, wrapVisualFlags, 0);
}

// Retrive the display mode of visual flags for wrapped lines.
int wxScintilla::GetWrapVisualFlags() {
    return SendMsg(2461, 0, 0);
}

// Set the location of visual flags for wrapped lines.
void wxScintilla::SetWrapVisualFlagsLocation(int wrapVisualFlagsLocation) {
    SendMsg(2462, wrapVisualFlagsLocation, 0);
}

// Retrive the location of visual flags for wrapped lines.
int wxScintilla::GetWrapVisualFlagsLocation() {
    return SendMsg(2463, 0, 0);
}

// Set the start indent for wrapped lines.
void wxScintilla::SetWrapStartIndent(int indent) {
    SendMsg(2464, indent, 0);
}

// Retrive the start indent for wrapped lines.
int wxScintilla::GetWrapStartIndent() {
    return SendMsg(2465, 0, 0);
}

// Sets the degree of caching of layout information.
void wxScintilla::SetLayoutCache(int mode) {
    SendMsg(2272, mode, 0);
}

// Retrieve the degree of caching of layout information.
int wxScintilla::GetLayoutCache() {
    return SendMsg(2273, 0, 0);
}

// Sets the document width assumed for scrolling.
void wxScintilla::SetScrollWidth(int pixelWidth) {
    SendMsg(2274, pixelWidth, 0);
}

// Retrieve the document width assumed for scrolling.
int wxScintilla::GetScrollWidth() {
    return SendMsg(2275, 0, 0);
}

// Sets whether the maximum width line displayed is used to set scroll width.
void wxScintilla::SetScrollWidthTracking(bool tracking) {
    SendMsg(2516, tracking, 0);
}

// Retrieve whether the scroll width tracks wide lines.
bool wxScintilla::GetScrollWidthTracking() {
    return SendMsg(2517, 0, 0) != 0;
}

// Measure the pixel width of some text in a particular style.
// NUL terminated text argument.
// Does not handle tab or control characters.
int wxScintilla::TextWidth(int style, const wxString& text) {
    return SendMsg(2276, style, (sptr_t)(const char*)wx2sci(text));
}

// Sets the scroll range so that maximum scroll position has
// the last line at the bottom of the view (default).
// Setting this to false allows scrolling one page below the last line.
void wxScintilla::SetEndAtLastLine(bool endAtLastLine) {
    SendMsg(2277, endAtLastLine, 0);
}

// Retrieve whether the maximum scroll position has the last
// line at the bottom of the view.
bool wxScintilla::GetEndAtLastLine() {
    return SendMsg(2278, 0, 0) != 0;
}

// Retrieve the height of a particular line of text in pixels.
int wxScintilla::TextHeight(int line) {
    return SendMsg(2279, line, 0);
}

// Show or hide the vertical scroll bar.
void wxScintilla::SetUseVerticalScrollBar(bool show) {
    SendMsg(2280, show, 0);
}

// Is the vertical scroll bar visible?
bool wxScintilla::GetUseVerticalScrollBar() {
    return SendMsg(2281, 0, 0) != 0;
}

// Append a string to the end of the document without changing the selection.
void wxScintilla::AppendText(const wxString& text) {
                    wxWX2MBbuf buf = (wxWX2MBbuf)wx2sci(text);
                    SendMsg(2282, (uptr_t)strlen(buf), (sptr_t)(const char*)buf);
}

// Is drawing done in two phases with backgrounds drawn before faoregrounds?
bool wxScintilla::GetTwoPhaseDraw() {
    return SendMsg(2283, 0, 0) != 0;
}

// In twoPhaseDraw mode, drawing is performed in two phases, first the background
// and then the foreground. This avoids chopping off characters that overlap the next run.
void wxScintilla::SetTwoPhaseDraw(bool twoPhase) {
    SendMsg(2284, twoPhase, 0);
}

// Make the target range start and end be the same as the selection range start and end.
void wxScintilla::TargetFromSelection() {
    SendMsg(2287, 0, 0);
}

// Join the lines in the target.
void wxScintilla::LinesJoin() {
    SendMsg(2288, 0, 0);
}

// Split the lines in the target into lines that are less wide than pixelWidth
// where possible.
void wxScintilla::LinesSplit(int pixelWidth) {
    SendMsg(2289, pixelWidth, 0);
}

// Set the colours used as a chequerboard pattern in the fold margin
void wxScintilla::SetFoldMarginColour(bool useSetting, const wxColour& back) {
    SendMsg(2290, useSetting, wxColourAsLong(back));
}
void wxScintilla::SetFoldMarginHiColour(bool useSetting, const wxColour& fore) {
    SendMsg(2291, useSetting, wxColourAsLong(fore));
}

// Move caret down one line.
void wxScintilla::LineDown() {
    SendMsg(2300, 0, 0);
}

// Move caret down one line extending selection to new caret position.
void wxScintilla::LineDownExtend() {
    SendMsg(2301, 0, 0);
}

// Move caret up one line.
void wxScintilla::LineUp() {
    SendMsg(2302, 0, 0);
}

// Move caret up one line extending selection to new caret position.
void wxScintilla::LineUpExtend() {
    SendMsg(2303, 0, 0);
}

// Move caret left one character.
void wxScintilla::CharLeft() {
    SendMsg(2304, 0, 0);
}

// Move caret left one character extending selection to new caret position.
void wxScintilla::CharLeftExtend() {
    SendMsg(2305, 0, 0);
}

// Move caret right one character.
void wxScintilla::CharRight() {
    SendMsg(2306, 0, 0);
}

// Move caret right one character extending selection to new caret position.
void wxScintilla::CharRightExtend() {
    SendMsg(2307, 0, 0);
}

// Move caret left one word.
void wxScintilla::WordLeft() {
    SendMsg(2308, 0, 0);
}

// Move caret left one word extending selection to new caret position.
void wxScintilla::WordLeftExtend() {
    SendMsg(2309, 0, 0);
}

// Move caret right one word.
void wxScintilla::WordRight() {
    SendMsg(2310, 0, 0);
}

// Move caret right one word extending selection to new caret position.
void wxScintilla::WordRightExtend() {
    SendMsg(2311, 0, 0);
}

// Move caret to first position on line.
void wxScintilla::Home() {
    SendMsg(2312, 0, 0);
}

// Move caret to first position on line extending selection to new caret position.
void wxScintilla::HomeExtend() {
    SendMsg(2313, 0, 0);
}

// Move caret to last position on line.
void wxScintilla::LineEnd() {
    SendMsg(2314, 0, 0);
}

// Move caret to last position on line extending selection to new caret position.
void wxScintilla::LineEndExtend() {
    SendMsg(2315, 0, 0);
}

// Move caret to first position in document.
void wxScintilla::DocumentStart() {
    SendMsg(2316, 0, 0);
}

// Move caret to first position in document extending selection to new caret position.
void wxScintilla::DocumentStartExtend() {
    SendMsg(2317, 0, 0);
}

// Move caret to last position in document.
void wxScintilla::DocumentEnd() {
    SendMsg(2318, 0, 0);
}

// Move caret to last position in document extending selection to new caret position.
void wxScintilla::DocumentEndExtend() {
    SendMsg(2319, 0, 0);
}

// Move caret one page up.
void wxScintilla::PageUp() {
    SendMsg(2320, 0, 0);
}

// Move caret one page up extending selection to new caret position.
void wxScintilla::PageUpExtend() {
    SendMsg(2321, 0, 0);
}

// Move caret one page down.
void wxScintilla::PageDown() {
    SendMsg(2322, 0, 0);
}

// Move caret one page down extending selection to new caret position.
void wxScintilla::PageDownExtend() {
    SendMsg(2323, 0, 0);
}

// Switch from insert to overtype mode or the reverse.
void wxScintilla::EditToggleOvertype() {
    SendMsg(2324, 0, 0);
}

// Cancel any modes such as call tip or auto-completion list display.
void wxScintilla::Cancel() {
    SendMsg(2325, 0, 0);
}

// Delete the selection or if no selection, the character before the caret.
void wxScintilla::DeleteBack() {
    SendMsg(2326, 0, 0);
}

// If selection is empty or all on one line replace the selection with a tab character.
// If more than one line selected, indent the lines.
void wxScintilla::Tab() {
    SendMsg(2327, 0, 0);
}

// Dedent the selected lines.
void wxScintilla::BackTab() {
    SendMsg(2328, 0, 0);
}

// Insert a new line, may use a CRLF, CR or LF depending on EOL mode.
void wxScintilla::NewLine() {
    SendMsg(2329, 0, 0);
}

// Insert a Form Feed character.
void wxScintilla::FormFeed() {
    SendMsg(2330, 0, 0);
}

// Move caret to before first visible character on line.
// If already there move to first character on line.
void wxScintilla::VCHome() {
    SendMsg(2331, 0, 0);
}

// Like VCHome but extending selection to new caret position.
void wxScintilla::VCHomeExtend() {
    SendMsg(2332, 0, 0);
}

// Magnify the displayed text by increasing the sizes by 1 point.
void wxScintilla::ZoomIn() {
    SendMsg(2333, 0, 0);
}

// Make the displayed text smaller by decreasing the sizes by 1 point.
void wxScintilla::ZoomOut() {
    SendMsg(2334, 0, 0);
}

// Delete the word to the left of the caret.
void wxScintilla::DelWordLeft() {
    SendMsg(2335, 0, 0);
}

// Delete the word to the right of the caret.
void wxScintilla::DelWordRight() {
    SendMsg(2336, 0, 0);
}

// Delete the word to the right of the caret, but not the trailing non-word characters.
void wxScintilla::DelWordRightEnd() {
    SendMsg(2518, 0, 0);
}

// Cut the line containing the caret.
void wxScintilla::LineCut() {
    SendMsg(2337, 0, 0);
}

// Delete the line containing the caret.
void wxScintilla::LineDelete() {
    SendMsg(2338, 0, 0);
}

// Switch the current line with the previous.
void wxScintilla::LineTranspose() {
    SendMsg(2339, 0, 0);
}

// Duplicate the current line.
void wxScintilla::LineDuplicate() {
    SendMsg(2404, 0, 0);
}

// Transform the selection to lower case.
void wxScintilla::LowerCase() {
    SendMsg(2340, 0, 0);
}

// Transform the selection to upper case.
void wxScintilla::UpperCase() {
    SendMsg(2341, 0, 0);
}

// Scroll the document down, keeping the caret visible.
void wxScintilla::LineScrollDown() {
    SendMsg(2342, 0, 0);
}

// Scroll the document up, keeping the caret visible.
void wxScintilla::LineScrollUp() {
    SendMsg(2343, 0, 0);
}

// Delete the selection or if no selection, the character before the caret.
// Will not delete the character before at the start of a line.
void wxScintilla::DeleteBackNotLine() {
    SendMsg(2344, 0, 0);
}

// Move caret to first position on display line.
void wxScintilla::HomeDisplay() {
    SendMsg(2345, 0, 0);
}

// Move caret to first position on display line extending selection to
// new caret position.
void wxScintilla::HomeDisplayExtend() {
    SendMsg(2346, 0, 0);
}

// Move caret to last position on display line.
void wxScintilla::LineEndDisplay() {
    SendMsg(2347, 0, 0);
}

// Move caret to last position on display line extending selection to new
// caret position.
void wxScintilla::LineEndDisplayExtend() {
    SendMsg(2348, 0, 0);
}

// These are like their namesakes Home(Extend)?, LineEnd(Extend)?, VCHome(Extend)?
// except they behave differently when word-wrap is enabled:
// They go first to the start / end of the display line, like (Home|LineEnd)Display
// The difference is that, the cursor is already at the point, it goes on to the start
// or end of the document line, as appropriate for (Home|LineEnd|VCHome)(Extend)?.
void wxScintilla::HomeWrap() {
    SendMsg(2349, 0, 0);
}
void wxScintilla::HomeWrapExtend() {
    SendMsg(2450, 0, 0);
}
void wxScintilla::LineEndWrap() {
    SendMsg(2451, 0, 0);
}
void wxScintilla::LineEndWrapExtend() {
    SendMsg(2452, 0, 0);
}
void wxScintilla::VCHomeWrap() {
    SendMsg(2453, 0, 0);
}
void wxScintilla::VCHomeWrapExtend() {
    SendMsg(2454, 0, 0);
}

// Copy the line containing the caret.
void wxScintilla::LineCopy() {
    SendMsg(2455, 0, 0);
}

// Move the caret inside current view if it's not there already.
void wxScintilla::MoveCaretInsideView() {
    SendMsg(2401, 0, 0);
}

// How many characters are on a line, including end of line characters?
int wxScintilla::LineLength(int line) {
    return SendMsg(2350, line, 0);
}

// Highlight the characters at two positions.
void wxScintilla::BraceHighlight(int pos1, int pos2) {
    SendMsg(2351, pos1, pos2);
}

// Highlight the character at a position indicating there is no matching brace.
void wxScintilla::BraceBadLight(int pos) {
    SendMsg(2352, pos, 0);
}

// Find the position of a matching brace or INVALID_POSITION if no match.
int wxScintilla::BraceMatch(int pos) {
    return SendMsg(2353, pos, 0);
}

// Are the end of line characters visible?
bool wxScintilla::GetViewEOL() {
    return SendMsg(2355, 0, 0) != 0;
}

// Make the end of line characters visible or invisible.
void wxScintilla::SetViewEOL(bool visible) {
    SendMsg(2356, visible, 0);
}

// Retrieve a pointer to the document object.
void* wxScintilla::GetDocPointer() {
         return (void*)SendMsg(2357);
}

// Change the document object used.
void wxScintilla::SetDocPointer(void* docPointer) {
         SendMsg(2358, 0, (sptr_t)docPointer);
}

// Set which document modification events are sent to the container.
void wxScintilla::SetModEventMask(int mask) {
    SendMsg(2359, mask, 0);
}

// Retrieve the column number which text should be kept within.
int wxScintilla::GetEdgeColumn() {
    return SendMsg(2360, 0, 0);
}

// Set the column number of the edge.
// If text goes past the edge then it is highlighted.
void wxScintilla::SetEdgeColumn(int column) {
    SendMsg(2361, column, 0);
}

// Retrieve the edge highlight mode.
int wxScintilla::GetEdgeMode() {
    return SendMsg(2362, 0, 0);
}

// The edge may be displayed by a line (EDGE_LINE) or by highlighting text that
// goes beyond it (EDGE_BACKGROUND) or not displayed at all (EDGE_NONE).
void wxScintilla::SetEdgeMode(int mode) {
    SendMsg(2363, mode, 0);
}

// Retrieve the colour used in edge indication.
wxColour wxScintilla::GetEdgeColour() {
    long c = SendMsg(2364, 0, 0);
    return wxColourFromLong(c);
}

// Change the colour used in edge indication.
void wxScintilla::SetEdgeColour(const wxColour& edgeColour) {
    SendMsg(2365, wxColourAsLong(edgeColour), 0);
}

// Sets the current caret position to be the search anchor.
void wxScintilla::SearchAnchor() {
    SendMsg(2366, 0, 0);
}

// Find some text starting at the search anchor.
// Does not ensure the selection is visible.
int wxScintilla::SearchNext(int flags, const wxString& text) {
    return SendMsg(2367, flags, (sptr_t)(const char*)wx2sci(text));
}

// Find some text starting at the search anchor and moving backwards.
// Does not ensure the selection is visible.
int wxScintilla::SearchPrev(int flags, const wxString& text) {
    return SendMsg(2368, flags, (sptr_t)(const char*)wx2sci(text));
}

// Retrieves the number of lines completely visible.
int wxScintilla::LinesOnScreen() {
    return SendMsg(2370, 0, 0);
}

// Set whether a pop up menu is displayed automatically when the user presses
// the wrong mouse button.
void wxScintilla::UsePopUp(bool allowPopUp) {
    SendMsg(2371, allowPopUp, 0);
}

// Is the selection rectangular? The alternative is the more common stream selection.
bool wxScintilla::SelectionIsRectangle() {
    return SendMsg(2372, 0, 0) != 0;
}

// Set the zoom level. This number of points is added to the size of all fonts.
// It may be positive to magnify or negative to reduce.
void wxScintilla::SetZoom(int zoom) {
    SendMsg(2373, zoom, 0);
}

// Retrieve the zoom level.
int wxScintilla::GetZoom() {
    return SendMsg(2374, 0, 0);
}

// Create a new document object.
// Starts with reference count of 1 and not selected into editor.
void* wxScintilla::CreateDocument() {
         return (void*)SendMsg(2375);
}

// Extend life of document.
void wxScintilla::AddRefDocument(void* docPointer) {
         SendMsg(2376, 0, (sptr_t)docPointer);
}

// Release a reference to the document, deleting document if it fades to black.
void wxScintilla::ReleaseDocument(void* docPointer) {
         SendMsg(2377, 0, (sptr_t)docPointer);
}

// Get which document modification events are sent to the container.
int wxScintilla::GetModEventMask() {
    return SendMsg(2378, 0, 0);
}

// Change internal focus flag.
void wxScintilla::SetSCIFocus(bool focus) {
    SendMsg(2380, focus, 0);
}

// Get internal focus flag.
bool wxScintilla::GetSCIFocus() {
    return SendMsg(2381, 0, 0) != 0;
}

// Change error status - 0 = OK.
void wxScintilla::SetStatus(int statusCode) {
    SendMsg(2382, statusCode, 0);
}

// Get error status.
int wxScintilla::GetStatus() {
    return SendMsg(2383, 0, 0);
}

// Set whether the mouse is captured when its button is pressed.
void wxScintilla::SetMouseDownCaptures(bool captures) {
    SendMsg(2384, captures, 0);
}

// Get whether mouse gets captured.
bool wxScintilla::GetMouseDownCaptures() {
    return SendMsg(2385, 0, 0) != 0;
}

// Sets the cursor to one of the SC_CURSOR* values.
void wxScintilla::SetSCICursor(int cursorType) {
    SendMsg(2386, cursorType, 0);
}

// Get cursor type.
int wxScintilla::GetSCICursor() {
    return SendMsg(2387, 0, 0);
}

// Change the way control characters are displayed:
// If symbol is < 32, keep the drawn way, else, use the given character.
void wxScintilla::SetControlCharSymbol(int symbol) {
    SendMsg(2388, symbol, 0);
}

// Get the way control characters are displayed.
int wxScintilla::GetControlCharSymbol() {
    return SendMsg(2389, 0, 0);
}

// Move to the previous change in capitalisation.
void wxScintilla::WordPartLeft() {
    SendMsg(2390, 0, 0);
}

// Move to the previous change in capitalisation extending selection
// to new caret position.
void wxScintilla::WordPartLeftExtend() {
    SendMsg(2391, 0, 0);
}

// Move to the change next in capitalisation.
void wxScintilla::WordPartRight() {
    SendMsg(2392, 0, 0);
}

// Move to the next change in capitalisation extending selection
// to new caret position.
void wxScintilla::WordPartRightExtend() {
    SendMsg(2393, 0, 0);
}

// Set the way the display area is determined when a particular line
// is to be moved to by Find, FindNext, GotoLine, etc.
void wxScintilla::SetVisiblePolicy(int visiblePolicy, int visibleSlop) {
    SendMsg(2394, visiblePolicy, visibleSlop);
}

// Delete back from the current position to the start of the line.
void wxScintilla::DelLineLeft() {
    SendMsg(2395, 0, 0);
}

// Delete forwards from the current position to the end of the line.
void wxScintilla::DelLineRight() {
    SendMsg(2396, 0, 0);
}

// Get and Set the xOffset (ie, horizonal scroll position).
void wxScintilla::SetXOffset(int newOffset) {
    SendMsg(2397, newOffset, 0);
}
int wxScintilla::GetXOffset() {
    return SendMsg(2398, 0, 0);
}

// Set the last x chosen value to be the caret x position.
void wxScintilla::ChooseCaretX() {
    SendMsg(2399, 0, 0);
}

// Set the way the caret is kept visible when going sideway.
// The exclusion zone is given in pixels.
void wxScintilla::SetXCaretPolicy(int caretPolicy, int caretSlop) {
    SendMsg(2402, caretPolicy, caretSlop);
}

// Set the way the line the caret is on is kept visible.
// The exclusion zone is given in lines.
void wxScintilla::SetYCaretPolicy(int caretPolicy, int caretSlop) {
    SendMsg(2403, caretPolicy, caretSlop);
}

// Set printing to line wrapped (SC_WRAP_WORD) or not line wrapped (SC_WRAP_NONE).
void wxScintilla::SetPrintWrapMode(int mode) {
    SendMsg(2406, mode, 0);
}

// Is printing line wrapped?
int wxScintilla::GetPrintWrapMode() {
    return SendMsg(2407, 0, 0);
}

// Set a fore colour for active hotspots.
void wxScintilla::SetHotspotActiveForeground(bool useSetting, const wxColour& fore) {
    SendMsg(2410, useSetting, wxColourAsLong(fore));
}

// Get the fore colour for active hotspots.
wxColour wxScintilla::GetHotspotActiveForeground() {
    long c = SendMsg(2494, 0, 0);
    return wxColourFromLong(c);
}

// Set a back colour for active hotspots.
void wxScintilla::SetHotspotActiveBackground(bool useSetting, const wxColour& back) {
    SendMsg(2411, useSetting, wxColourAsLong(back));
}

// Get the back colour for active hotspots.
wxColour wxScintilla::GetHotspotActiveBackground() {
    long c = SendMsg(2495, 0, 0);
    return wxColourFromLong(c);
}

// Enable / Disable underlining active hotspots.
void wxScintilla::SetHotspotActiveUnderline(bool underline) {
    SendMsg(2412, underline, 0);
}

// Get whether underlining for active hotspots.
bool wxScintilla::GetHotspotActiveUnderline() {
    return SendMsg(2496, 0, 0) != 0;
}

// Limit hotspots to single line so hotspots on two lines don't merge.
void wxScintilla::SetHotspotSingleLine(bool singleLine) {
    SendMsg(2421, singleLine, 0);
}

// Get the HotspotSingleLine property
bool wxScintilla::GetHotspotSingleLine() {
    return SendMsg(2497, 0, 0) != 0;
}

// Move caret between paragraphs (delimited by empty lines).
void wxScintilla::ParaDown() {
    SendMsg(2413, 0, 0);
}
void wxScintilla::ParaDownExtend() {
    SendMsg(2414, 0, 0);
}
void wxScintilla::ParaUp() {
    SendMsg(2415, 0, 0);
}
void wxScintilla::ParaUpExtend() {
    SendMsg(2416, 0, 0);
}

// Given a valid document position, return the previous position taking code
// page into account. Returns 0 if passed 0.
int wxScintilla::PositionBefore(int pos) {
    return SendMsg(2417, pos, 0);
}

// Given a valid document position, return the next position taking code
// page into account. Maximum value returned is the last position in the document.
int wxScintilla::PositionAfter(int pos) {
    return SendMsg(2418, pos, 0);
}

// Copy a range of text to the clipboard. Positions are clipped into the document.
void wxScintilla::CopyRange(int start, int end) {
    SendMsg(2419, start, end);
}

// Copy argument text to the clipboard.
void wxScintilla::CopyText(int length, const wxString& text) {
    SendMsg(2420, length, (sptr_t)(const char*)wx2sci(text));
}

// Set the selection mode to stream (SC_SEL_STREAM) or rectangular (SC_SEL_RECTANGLE) or
// by lines (SC_SEL_LINES).
void wxScintilla::SetSelectionMode(int mode) {
    SendMsg(2422, mode, 0);
}

// Get the mode of the current selection.
int wxScintilla::GetSelectionMode() {
    return SendMsg(2423, 0, 0);
}

// Retrieve the position of the start of the selection at the given line (INVALID_POSITION if no selection on this line).
int wxScintilla::GetLineSelStartPosition(int line) {
    return SendMsg(2424, line, 0);
}

// Retrieve the position of the end of the selection at the given line (INVALID_POSITION if no selection on this line).
int wxScintilla::GetLineSelEndPosition(int line) {
    return SendMsg(2425, line, 0);
}

// Move caret down one line, extending rectangular selection to new caret position.
void wxScintilla::LineDownRectExtend() {
    SendMsg(2426, 0, 0);
}

// Move caret up one line, extending rectangular selection to new caret position.
void wxScintilla::LineUpRectExtend() {
    SendMsg(2427, 0, 0);
}

// Move caret left one character, extending rectangular selection to new caret position.
void wxScintilla::CharLeftRectExtend() {
    SendMsg(2428, 0, 0);
}

// Move caret right one character, extending rectangular selection to new caret position.
void wxScintilla::CharRightRectExtend() {
    SendMsg(2429, 0, 0);
}

// Move caret to first position on line, extending rectangular selection to new caret position.
void wxScintilla::HomeRectExtend() {
    SendMsg(2430, 0, 0);
}

// Move caret to before first visible character on line.
// If already there move to first character on line.
// In either case, extend rectangular selection to new caret position.
void wxScintilla::VCHomeRectExtend() {
    SendMsg(2431, 0, 0);
}

// Move caret to last position on line, extending rectangular selection to new caret position.
void wxScintilla::LineEndRectExtend() {
    SendMsg(2432, 0, 0);
}

// Move caret one page up, extending rectangular selection to new caret position.
void wxScintilla::PageUpRectExtend() {
    SendMsg(2433, 0, 0);
}

// Move caret one page down, extending rectangular selection to new caret position.
void wxScintilla::PageDownRectExtend() {
    SendMsg(2434, 0, 0);
}

// Move caret to top of page, or one page up if already at top of page.
void wxScintilla::StutteredPageUp() {
    SendMsg(2435, 0, 0);
}

// Move caret to top of page, or one page up if already at top of page, extending selection to new caret position.
void wxScintilla::StutteredPageUpExtend() {
    SendMsg(2436, 0, 0);
}

// Move caret to bottom of page, or one page down if already at bottom of page.
void wxScintilla::StutteredPageDown() {
    SendMsg(2437, 0, 0);
}

// Move caret to bottom of page, or one page down if already at bottom of page, extending selection to new caret position.
void wxScintilla::StutteredPageDownExtend() {
    SendMsg(2438, 0, 0);
}

// Move caret left one word, position cursor at end of word.
void wxScintilla::WordLeftEnd() {
    SendMsg(2439, 0, 0);
}

// Move caret left one word, position cursor at end of word, extending selection to new caret position.
void wxScintilla::WordLeftEndExtend() {
    SendMsg(2440, 0, 0);
}

// Move caret right one word, position cursor at end of word.
void wxScintilla::WordRightEnd() {
    SendMsg(2441, 0, 0);
}

// Move caret right one word, position cursor at end of word, extending selection to new caret position.
void wxScintilla::WordRightEndExtend() {
    SendMsg(2442, 0, 0);
}

// Set the set of characters making up whitespace for when moving or selecting by word.
// Should be called after SetWordChars.
void wxScintilla::SetWhitespaceChars(const wxString& characters) {
    SendMsg(2443, 0, (sptr_t)(const char*)wx2sci(characters));
}

// Reset the set of characters for whitespace and word characters to the defaults.
void wxScintilla::SetCharsDefault() {
    SendMsg(2444, 0, 0);
}

// Get currently selected item position in the auto-completion list
int wxScintilla::AutoCompGetCurrent() {
    return SendMsg(2445, 0, 0);
}

// Enlarge the document to a particular size of text bytes.
void wxScintilla::Allocate(int bytes) {
    SendMsg(2446, bytes, 0);
}

// Find the position of a column on a line taking into account tabs and
// multi-byte characters. If beyond end of line, return line end position.
int wxScintilla::FindColumn(int line, int column) {
    return SendMsg(2456, line, column);
}

// Can the caret preferred x position only be changed by explicit movement commands?
bool wxScintilla::GetCaretSticky() {
    return SendMsg(2457, 0, 0) != 0;
}

// Stop the caret preferred x position changing when the user types.
void wxScintilla::SetCaretSticky(bool useCaretStickyBehaviour) {
    SendMsg(2458, useCaretStickyBehaviour, 0);
}

// Switch between sticky and non-sticky: meant to be bound to a key.
void wxScintilla::ToggleCaretSticky() {
    SendMsg(2459, 0, 0);
}

// Enable/Disable convert-on-paste for line endings
void wxScintilla::SetPasteConvertEndings(bool convert) {
    SendMsg(2467, convert, 0);
}

// Get convert-on-paste setting
bool wxScintilla::GetPasteConvertEndings() {
    return SendMsg(2468, 0, 0) != 0;
}

// Duplicate the selection. If selection empty duplicate the line containing the caret.
void wxScintilla::SelectionDuplicate() {
    SendMsg(2469, 0, 0);
}

// Set background alpha of the caret line.
void wxScintilla::SetCaretLineBackAlpha(int alpha) {
    SendMsg(2470, alpha, 0);
}

// Get the background alpha of the caret line.
int wxScintilla::GetCaretLineBackAlpha() {
    return SendMsg(2471, 0, 0);
}

// Set the style of the caret to be drawn.
void wxScintilla::SetCaretStyle(int caretStyle) {
    SendMsg(2512, caretStyle, 0);
}

// Returns the current style of the caret.
int wxScintilla::GetCaretStyle() {
    return SendMsg(2513, 0, 0);
}

// Set the indicator used for IndicatorFillRange and IndicatorClearRange
void wxScintilla::SetIndicatorCurrent(int indicator) {
    SendMsg(2500, indicator, 0);
}

// Get the current indicator
int wxScintilla::GetIndicatorCurrent() {
    return SendMsg(2501, 0, 0);
}

// Set the value used for IndicatorFillRange
void wxScintilla::SetIndicatorValue(int value) {
    SendMsg(2502, value, 0);
}

// Get the current indicator vaue
int wxScintilla::GetIndicatorValue() {
    return SendMsg(2503, 0, 0);
}

// Turn a indicator on over a range.
void wxScintilla::IndicatorFillRange(int position, int fillLength) {
    SendMsg(2504, position, fillLength);
}

// Turn a indicator off over a range.
void wxScintilla::IndicatorClearRange(int position, int clearLength) {
    SendMsg(2505, position, clearLength);
}

// Are any indicators present at position?
int wxScintilla::IndicatorAllOnFor(int position) {
    return SendMsg(2506, position, 0);
}

// What value does a particular indicator have at at a position?
int wxScintilla::IndicatorValueAt(int indicator, int position) {
    return SendMsg(2507, indicator, position);
}

// Where does a particular indicator start?
int wxScintilla::IndicatorStart(int indicator, int position) {
    return SendMsg(2508, indicator, position);
}

// Where does a particular indicator end?
int wxScintilla::IndicatorEnd(int indicator, int position) {
    return SendMsg(2509, indicator, position);
}

// Set number of entries in position cache
void wxScintilla::SetPositionCacheSize(int size) {
    SendMsg(2514, size, 0);
}

// How many entries are allocated to the position cache?
int wxScintilla::GetPositionCacheSize() {
    return SendMsg(2515, 0, 0);
}

// Start notifying the container of all key presses and commands.
void wxScintilla::StartRecord() {
    SendMsg(3001, 0, 0);
}

// Stop notifying the container of all key presses and commands.
void wxScintilla::StopRecord() {
    SendMsg(3002, 0, 0);
}

// Set the lexing language of the document.
void wxScintilla::SetLexer(int lexer) {
    SendMsg(4001, lexer, 0);
}

// Retrieve the lexing language of the document.
int wxScintilla::GetLexer() {
    return SendMsg(4002, 0, 0);
}

// Colourise a segment of the document using the current lexing language.
void wxScintilla::Colourise(int start, int end) {
    SendMsg(4003, start, end);
}

// Set up a value that may be used by a lexer for some optional feature.
void wxScintilla::SetProperty(const wxString& key, const wxString& value) {
    SendMsg(4004, (uptr_t)(const char*)wx2sci(key), (sptr_t)(const char*)wx2sci(value));
}

// Set up the key words used by the lexer.
void wxScintilla::SetKeyWords(int keywordSet, const wxString& keyWords) {
    SendMsg(4005, keywordSet, (sptr_t)(const char*)wx2sci(keyWords));
}

// Set the lexing language of the document based on string name.
void wxScintilla::SetLexerLanguage(const wxString& language) {
    SendMsg(4006, 0, (sptr_t)(const char*)wx2sci(language));
}

// Retrieve a 'property' value previously set with SetProperty.
wxString wxScintilla::GetProperty(const wxString& key) {
         int len = SendMsg(SCI_GETPROPERTY, (uptr_t)(const char*)wx2sci(key), 0);
         if (!len) return wxEmptyString;

         wxMemoryBuffer mbuf(len+1);
         char* buf = (char*)mbuf.GetWriteBuf(len+1);
         SendMsg(4008, (uptr_t)(const char*)wx2sci(key), (sptr_t)buf);
         mbuf.UngetWriteBuf(len);
         mbuf.AppendByte(0);
         return sci2wx(buf);
}

// Retrieve a 'property' value previously set with SetProperty,
// with '$()' variable replacement on returned buffer.
wxString wxScintilla::GetPropertyExpanded(const wxString& key) {
         int len = SendMsg(SCI_GETPROPERTYEXPANDED, (sptr_t)(const char*)wx2sci(key), 0);
         if (!len) return wxEmptyString;

         wxMemoryBuffer mbuf(len+1);
         char* buf = (char*)mbuf.GetWriteBuf(len+1);
         SendMsg(4009, (uptr_t)(const char*)wx2sci(key), (sptr_t)buf);
         mbuf.UngetWriteBuf(len);
         mbuf.AppendByte(0);
         return sci2wx(buf);
}

// Retrieve a 'property' value previously set with SetProperty,
// interpreted as an int AFTER any '$()' variable replacement.
int wxScintilla::GetPropertyInt(const wxString& key) {
    return SendMsg(4010, (uptr_t)(const char*)wx2sci(key), 0);
}

// Retrieve the number of bits the current lexer needs for styling.
int wxScintilla::GetStyleBitsNeeded() {
    return SendMsg(4011, 0, 0);
}

// END of generated section
//----------------------------------------------------------------------


// Returns the line number of the line with the caret.
int wxScintilla::GetCurrentLine () {
    int line = LineFromPosition (GetCurrentPos());
    return line;
}

// Retrieve the point in the window where a position is displayed.
wxPoint wxScintilla::PointFromPosition (int pos) {
    int x = SendMsg(SCI_POINTXFROMPOSITION, 0, pos);
    int y = SendMsg(SCI_POINTYFROMPOSITION, 0, pos);
    return wxPoint (x, y);
}

// Extract style settings from a spec-string which is composed of one or
// more of the following comma separated elements:
//
//      bold                    turns on bold
//      italic                  turns on italics
//      fore:[name or #RRGGBB]  sets the foreground colour
//      back:[name or #RRGGBB]  sets the background colour
//      face:[facename]         sets the font face name to use
//      size:[num]              sets the font size in points
//      eol                     turns on eol filling
//      underline               turns on underlining
//
void wxScintilla::StyleSetSpec (int styleNum, const wxString& spec) {

    wxStringTokenizer tkz (spec, _T(","));
    while (tkz.HasMoreTokens()) {
        wxString token = tkz.GetNextToken();

        wxString option = token.BeforeFirst (':');
        wxString val = token.AfterFirst (':');

        if (option == _T("bold"))
            StyleSetBold (styleNum, true);

        else if (option == _T("italic"))
            StyleSetItalic (styleNum, true);

        else if (option == _T("underline"))
            StyleSetUnderline (styleNum, true);

        else if (option == _T("eol"))
            StyleSetEOLFilled (styleNum, true);

        else if (option == _T("size")) {
            long points;
            if (val.ToLong (&points))
                StyleSetSize (styleNum, points);
        }

        else if (option == _T("face"))
            StyleSetFaceName (styleNum, val);

        else if (option == _T("fore"))
            StyleSetForeground (styleNum, wxColourFromSpec (val));

        else if (option == _T("back"))
            StyleSetBackground (styleNum, wxColourFromSpec (val));
    }
}


// Set style size, face, bold, italic, and underline attributes from
// a wxFont's attributes.
void wxScintilla::StyleSetFont (int styleNum, wxFont& font) {
#ifdef __WXGTK__
    // Ensure that the native font is initialized
    int x, y;
    GetTextExtent (_T("X"), &x, &y, NULL, NULL, &font);
#endif
    int            size     = font.GetPointSize();
    wxString       faceName = font.GetFaceName();
    bool           bold     = font.GetWeight() == wxBOLD;
    bool           italic   = font.GetStyle() == wxITALIC;
    bool           under    = font.GetUnderlined();
    wxFontEncoding encoding = font.GetEncoding();

    StyleSetFontAttr (styleNum, size, faceName, bold, italic, under, encoding);
}

// Set all font style attributes at once.
void wxScintilla::StyleSetFontAttr (int styleNum, int size, const wxString& faceName,
                                    bool bold, bool italic, bool underline,
                                    wxFontEncoding encoding) {
    StyleSetSize (styleNum, size);
    StyleSetFaceName (styleNum, faceName);
    StyleSetBold (styleNum, bold);
    StyleSetItalic (styleNum, italic);
    StyleSetUnderline (styleNum, underline);
    StyleSetFontEncoding (styleNum, encoding);
}

// Set the character set of the font in a style.
void wxScintilla::StyleSetCharacterSet (int style, int characterSet) {
    wxFontEncoding encoding;

    // Translate the Scintilla characterSet to a wxFontEncoding
    switch (characterSet) {
        default:
        case wxSCI_CHARSET_ANSI:
        case wxSCI_CHARSET_DEFAULT:
            encoding = wxFONTENCODING_DEFAULT;
            break;
        case wxSCI_CHARSET_BALTIC:
            encoding = wxFONTENCODING_ISO8859_13;
            break;
        case wxSCI_CHARSET_CHINESEBIG5:
            encoding = wxFONTENCODING_CP950;
            break;
        case wxSCI_CHARSET_EASTEUROPE:
            encoding = wxFONTENCODING_ISO8859_2;
            break;
        case wxSCI_CHARSET_GB2312:
            encoding = wxFONTENCODING_CP936;
            break;
        case wxSCI_CHARSET_GREEK:
            encoding = wxFONTENCODING_ISO8859_7;
            break;
        case wxSCI_CHARSET_HANGUL:
            encoding = wxFONTENCODING_CP949;
            break;
        case wxSCI_CHARSET_MAC:
            encoding = wxFONTENCODING_DEFAULT;
            break;
        case wxSCI_CHARSET_OEM:
            encoding = wxFONTENCODING_DEFAULT;
            break;
        case wxSCI_CHARSET_RUSSIAN:
            encoding = wxFONTENCODING_KOI8;
            break;
        case wxSCI_CHARSET_SHIFTJIS:
            encoding = wxFONTENCODING_CP932;
            break;
        case wxSCI_CHARSET_SYMBOL:
            encoding = wxFONTENCODING_DEFAULT;
            break;
        case wxSCI_CHARSET_TURKISH:
            encoding = wxFONTENCODING_ISO8859_9;
            break;
        case wxSCI_CHARSET_JOHAB:
            encoding = wxFONTENCODING_DEFAULT;
            break;
        case wxSCI_CHARSET_HEBREW:
            encoding = wxFONTENCODING_ISO8859_8;
            break;
        case wxSCI_CHARSET_ARABIC:
            encoding = wxFONTENCODING_ISO8859_6;
            break;
        case wxSCI_CHARSET_VIETNAMESE:
            encoding = wxFONTENCODING_DEFAULT;
            break;
        case wxSCI_CHARSET_THAI:
            encoding = wxFONTENCODING_ISO8859_11;
            break;
        case wxSCI_CHARSET_CYRILLIC:
            encoding = wxFONTENCODING_ISO8859_5;
            break;
        case wxSCI_CHARSET_8859_15:
            encoding = wxFONTENCODING_ISO8859_15;
            break;
    }

    // We just have Scintilla track the wxFontEncoding for us.  It gets used
    // in Font::Create in PlatWX.cpp.  We add one to the value so that the
    // effective wxFONENCODING_DEFAULT == SC_SHARSET_DEFAULT and so when
    // Scintilla internally uses SC_CHARSET_DEFAULT we will translate it back
    // to wxFONENCODING_DEFAULT in Font::Create.
    SendMsg (SCI_STYLESETCHARACTERSET, style, encoding+1);
}

// Set the font encoding to be used by a style.
void wxScintilla::StyleSetFontEncoding(int style, wxFontEncoding encoding) {
    SendMsg (SCI_STYLESETCHARACTERSET, style, encoding+1);
}

// Perform one of the operations defined by the wxSCI_CMD_* constants.
void wxScintilla::CmdKeyExecute (int cmd) {
    SendMsg (cmd);
}


// Set the left and right margin in the edit area, measured in pixels.
void wxScintilla::SetMargins (int left, int right) {
    SetMarginLeft (left);
    SetMarginRight (right);
}


// Retrieve the start and end positions of the current selection.
void wxScintilla::GetSelection (int* startPos, int* endPos) {
    if (startPos != NULL) *startPos = SendMsg (SCI_GETSELECTIONSTART);
    if (endPos != NULL) *endPos = SendMsg (SCI_GETSELECTIONEND);
}


// Scroll enough to make the given line visible
void wxScintilla::ScrollToLine (int line) {
    m_swx->DoScrollToLine (line);
}


// Scroll enough to make the given column visible
void wxScintilla::ScrollToColumn (int column) {
    m_swx->DoScrollToColumn (column);
}


bool wxScintilla::SaveFile (const wxString& filename) {
    wxFile file (filename, wxFile::write);
    if (!file.IsOpened()) return false;

    bool success = file.Write (GetText(), *wxConvCurrent);
    if (success) {
        SetSavePoint();
    }

    return success;
}

bool wxScintilla::LoadFile (const wxString& filename) {
    wxFile file (filename, wxFile::read);
    if (!file.IsOpened()) return false;

    // get the file size (assume it is not huge file...)
    size_t len = file.Length();

    bool success = false;
    if (len > 0) {
#if wxUSE_UNICODE
        wxMemoryBuffer buffer (len+1);
        success = (file.Read (buffer.GetData(), len) == (int)len);
        if (success) {
            ((char*)buffer.GetData())[len] = 0;
            SetText (wxString (buffer, *wxConvCurrent, len));
        }
#else
        wxString buffer;
        success = (file.Read (wxStringBuffer (buffer, len), len) == (int)len);
        if (success) {
            SetText (buffer);
        }
#endif
    }else if (len == 0) {
        success = true; // empty file is ok
        SetText (wxEmptyString);
    }else{
        success = false; // len == wxInvalidOffset
    }

    if (success) {
        EmptyUndoBuffer();
        SetSavePoint();
    }

    return success;
}


#if SCI_USE_DND
wxDragResult wxScintilla::DoDragOver (wxCoord x, wxCoord y, wxDragResult def) {
    return m_swx->DoDragOver (x, y, def);
}

bool wxScintilla::DoDropText (long x, long y, const wxString& data) {
    return m_swx->DoDropText (x, y, data);
}

wxDragResult wxScintilla::DoDragEnter (wxCoord x, wxCoord y, wxDragResult def) {
    return m_swx->DoDragOver (x, y, def);
}

void wxScintilla::DoDragLeave () {
    m_swx->DoDragLeave ();
}
#endif


void wxScintilla::SetUseAntiAliasing (bool useAA) {
    m_swx->SetUseAntiAliasing (useAA);
}

bool wxScintilla::GetUseAntiAliasing() {
    return m_swx->GetUseAntiAliasing();
}

#if wxCHECK_VERSION(2, 5, 0)
// Raw text handling for UTF-8
void wxScintilla::AddTextRaw (const char* text) {
    SendMsg (SCI_ADDTEXT, (uptr_t)strlen(text), (sptr_t)text);
}

void wxScintilla::InsertTextRaw (int pos, const char* text) {
    SendMsg (SCI_INSERTTEXT, pos, (sptr_t)text);
}

wxCharBuffer wxScintilla::GetCurLineRaw (int* linePos) {
    int len = LineLength (GetCurrentLine());
    if (!len) {
        if (linePos)  *linePos = 0;
        wxCharBuffer empty;
        return empty;
    }
    wxCharBuffer buf(len);
    int pos = SendMsg (SCI_GETCURLINE, len, (sptr_t)buf.data());
    if (linePos) *linePos = pos;
    return buf;
}

wxCharBuffer wxScintilla::GetLineRaw (int line) {
    int len = LineLength (line);
    if (!len) {
        wxCharBuffer empty;
        return empty;
    }
    wxCharBuffer buf(len);
    SendMsg (SCI_GETLINE, line, (sptr_t)buf.data());
    return buf;
}

wxCharBuffer wxScintilla::GetSelectedTextRaw() {
    int start;
    int end;
    GetSelection (&start, &end);
    int len = end - start;
    if (!len) {
        wxCharBuffer empty;
        return empty;
    }
    wxCharBuffer buf(len);
    SendMsg (SCI_GETSELTEXT, 0, (sptr_t)buf.data());
    return buf;
}

wxCharBuffer wxScintilla::GetTextRangeRaw (int startPos, int endPos) {
    if (endPos < startPos) {
        int temp = startPos;
        startPos = endPos;
        endPos = temp;
    }
    int len  = endPos - startPos;
    if (!len) {
        wxCharBuffer empty;
        return empty;
    }
    wxCharBuffer buf(len);
    TextRange tr;
    tr.lpstrText = buf.data();
    tr.chrg.cpMin = startPos;
    tr.chrg.cpMax = endPos;
    SendMsg (SCI_GETTEXTRANGE, 0, (sptr_t)&tr);
    return buf;
}

void wxScintilla::SetTextRaw (const char* text) {
    SendMsg (SCI_SETTEXT, 0, (sptr_t)text);
}

wxCharBuffer wxScintilla::GetTextRaw() {
    int len = GetTextLength();
    wxCharBuffer buf(len);
    SendMsg (SCI_GETTEXT, len, (sptr_t)buf.data());
    return buf;
}

void wxScintilla::AppendTextRaw (const char* text) {
    SendMsg (SCI_APPENDTEXT, (uptr_t)strlen(text), (sptr_t)text);
}
#endif


//----------------------------------------------------------------------
// Event handlers

void wxScintilla::OnPaint (wxPaintEvent& WXUNUSED(evt)) {
    wxPaintDC dc(this);
    m_swx->DoPaint (&dc, GetUpdateRegion().GetBox());
}

void wxScintilla::OnScrollWin (wxScrollWinEvent& evt) {
    if (evt.GetOrientation() == wxHORIZONTAL)
        m_swx->DoHScroll (evt.GetEventType(), evt.GetPosition());
    else
        m_swx->DoVScroll (evt.GetEventType(), evt.GetPosition());
}

void wxScintilla::OnScroll (wxScrollEvent& evt) {
    wxScrollBar* sb = wxDynamicCast (evt.GetEventObject(), wxScrollBar);
    if (sb) {
        if (sb->IsVertical())
            m_swx->DoVScroll (evt.GetEventType(), evt.GetPosition());
        else
            m_swx->DoHScroll (evt.GetEventType(), evt.GetPosition());
    }
}

void wxScintilla::OnSize (wxSizeEvent& WXUNUSED(evt)) {
    if (m_swx) {
        wxSize sz = GetClientSize();
        m_swx->DoSize (sz.x, sz.y);
    }
}

void wxScintilla::OnMouseLeftDown (wxMouseEvent& evt) {
    SetFocus();
    wxPoint pt = evt.GetPosition();
    m_swx->DoLeftButtonDown (Point(pt.x, pt.y), m_stopWatch.Time(),
                             evt.ShiftDown(), evt.ControlDown(), evt.AltDown());
}

void wxScintilla::OnMouseMove (wxMouseEvent& evt) {
    wxPoint pt = evt.GetPosition();
    m_swx->DoLeftButtonMove (Point(pt.x, pt.y));
}

void wxScintilla::OnMouseLeftUp (wxMouseEvent& evt) {
    wxPoint pt = evt.GetPosition();
    m_swx->DoLeftButtonUp (Point(pt.x, pt.y), m_stopWatch.Time(),
                           evt.ControlDown());
}


void wxScintilla::OnMouseRightUp (wxMouseEvent& evt) {
    wxPoint pt = evt.GetPosition();
    m_swx->DoContextMenu (Point(pt.x, pt.y));
}


void wxScintilla::OnMouseMiddleUp (wxMouseEvent& evt) {
    wxPoint pt = evt.GetPosition();
    m_swx->DoMiddleButtonUp (Point(pt.x, pt.y));
}

void wxScintilla::OnContextMenu (wxContextMenuEvent& evt) {
    wxPoint pt = evt.GetPosition();
    ScreenToClient (&pt.x, &pt.y);
    /*
      Show context menu at event point if it's within the window,
      or at caret location if not
    */
    wxHitTest ht = this->HitTest(pt);
    if (ht != wxHT_WINDOW_INSIDE) {
        pt = this->PointFromPosition (this->GetCurrentPos());
    }
    m_swx->DoContextMenu (Point(pt.x, pt.y));
}

void wxScintilla::OnMouseWheel (wxMouseEvent& evt) {
    m_swx->DoMouseWheel (evt.GetWheelRotation(),
                         evt.GetWheelDelta(),
                         evt.GetLinesPerAction(),
                         evt.ControlDown(),
                         evt.IsPageScroll());
}


void wxScintilla::OnChar (wxKeyEvent& evt) {
    // On (some?) non-US keyboards the AltGr key is required to enter some
    // common characters.  It comes to us as both Alt and Ctrl down so we need
    // to let the char through in that case, otherwise if only ctrl or only
    // alt let's skip it.
    bool ctrl = evt.ControlDown();
#ifdef __WXMAC__
    // On the Mac the Alt key is just a modifier key (like Shift) so we need
    // to allow the char events to be processed when Alt is pressed.
    // TODO:  Should we check MetaDown instead in this case?
    bool alt = false;
#else
    bool alt  = evt.AltDown();
#endif
    bool skip = ((ctrl || alt) && ! (ctrl && alt));

    if (!m_lastKeyDownConsumed && !skip) {
#if wxUSE_UNICODE
#if !wxCHECK_VERSION(2, 5, 0)
        int key = evt.m_rawCode;
#else
        int key = evt.GetUnicodeKey();
#endif
        bool keyOk = true;

        // if the unicode key code is not really a unicode character (it may
        // be a function key or etc., the platforms appear to always give us a
        // small value in this case) then fallback to the ascii key code but
        // don't do anything for function keys or etc.
        if (key <= 127) {
            key = evt.GetKeyCode();
            keyOk = (key <= 127);
        }
        if (keyOk) {
            m_swx->DoAddChar (key);
            return;
        }
#else
        int key = evt.GetKeyCode();
#if !wxCHECK_VERSION(2, 5, 0)
        if ( (key <= WXK_START || key > WXK_NUMPAD_DIVIDE)) {
#else
        if ( (key <= WXK_START || key > WXK_COMMAND)) {
#endif
            m_swx->DoAddChar (key);
            return;
        }
#endif
    }
    evt.Skip();
}

void wxScintilla::OnKeyDown (wxKeyEvent& evt) {
    int processed = m_swx->DoKeyDown (evt, &m_lastKeyDownConsumed);
    if (!processed && !m_lastKeyDownConsumed) {
        evt.Skip();
    }
}

void wxScintilla::OnLoseFocus (wxFocusEvent& evt) {
    m_swx->DoLoseFocus();
    evt.Skip();
}


void wxScintilla::OnGainFocus (wxFocusEvent& evt) {
    m_swx->DoGainFocus();
    evt.Skip();
}


void wxScintilla::OnSysColourChanged (wxSysColourChangedEvent& WXUNUSED(evt)) {
    m_swx->DoSysColourChange();
}


void wxScintilla::OnEraseBackground (wxEraseEvent& WXUNUSED(evt)) {
    // do nothing to help avoid flashing
}



void wxScintilla::OnMenu (wxCommandEvent& evt) {
    m_swx->DoCommand (evt.GetId());
}


void wxScintilla::OnListBox (wxCommandEvent& WXUNUSED(evt)) {
    m_swx->DoOnListBox ();
}


void wxScintilla::OnIdle (wxIdleEvent& evt) {
    m_swx->DoOnIdle (evt);
}


wxSize wxScintilla::DoGetBestSize() const
{
    // What would be the best size for a wxSintilla?
    // Just give a reasonable minimum until something else can be figured out.
    return wxSize(600,440);
}


//----------------------------------------------------------------------
// Turn notifications from Scintilla into events


void wxScintilla::NotifyChange() {
    wxScintillaEvent evt (wxEVT_SCI_CHANGE, GetId());
    evt.SetEventObject (this);
    GetEventHandler()->ProcessEvent(evt);
}

static void SetEventText (wxScintillaEvent& evt, const char* text, size_t length) {
    if(!text) return;
    evt.SetText(sci2wx(text, length));
}

void wxScintilla::NotifyParent (SCNotification* _scn) {
    SCNotification& scn = *_scn;
    wxScintillaEvent evt (0, GetId());
    evt.SetEventObject (this);
    evt.SetPosition (scn.position);
    evt.SetKey (scn.ch);
    evt.SetModifiers (scn.modifiers);

    switch (scn.nmhdr.code) {
    case SCN_STYLENEEDED:
        evt.SetEventType (wxEVT_SCI_STYLENEEDED);
        break;

    case SCN_CHARADDED:
        evt.SetEventType (wxEVT_SCI_CHARADDED);
        break;

    case SCN_SAVEPOINTREACHED:
        evt.SetEventType (wxEVT_SCI_SAVEPOINTREACHED);
        break;

    case SCN_SAVEPOINTLEFT:
        evt.SetEventType (wxEVT_SCI_SAVEPOINTLEFT);
        break;

    case SCN_MODIFYATTEMPTRO:
        evt.SetEventType (wxEVT_SCI_ROMODIFYATTEMPT);
        break;

    case SCN_KEY:
        evt.SetEventType (wxEVT_SCI_KEY);
        break;

    case SCN_DOUBLECLICK:
        evt.SetEventType (wxEVT_SCI_DOUBLECLICK);
        break;

    case SCN_UPDATEUI:
        evt.SetEventType (wxEVT_SCI_UPDATEUI);
        break;

    case SCN_MODIFIED:
        evt.SetEventType (wxEVT_SCI_MODIFIED);
        evt.SetModificationType (scn.modificationType);
        SetEventText (evt, scn.text, scn.length);
        evt.SetLength (scn.length);
        evt.SetLinesAdded (scn.linesAdded);
        evt.SetLine (scn.line);
        evt.SetFoldLevelNow (scn.foldLevelNow);
        evt.SetFoldLevelPrev (scn.foldLevelPrev);
        break;

    case SCN_MACRORECORD:
        evt.SetEventType (wxEVT_SCI_MACRORECORD);
        evt.SetMessage (scn.message);
        evt.SetWParam (scn.wParam);
        evt.SetLParam (scn.lParam);
        break;

    case SCN_MARGINCLICK:
        evt.SetEventType (wxEVT_SCI_MARGINCLICK);
        evt.SetMargin (scn.margin);
        break;

    case SCN_NEEDSHOWN:
        evt.SetEventType (wxEVT_SCI_NEEDSHOWN);
        evt.SetLength (scn.length);
        break;

    case SCN_PAINTED:
        evt.SetEventType (wxEVT_SCI_PAINTED);
        break;

    case SCN_USERLISTSELECTION:
        evt.SetEventType (wxEVT_SCI_USERLISTSELECTION);
        evt.SetListType (scn.listType);
        SetEventText (evt, scn.text, (sptr_t)strlen(scn.text));
        break;

    case SCN_URIDROPPED:
        evt.SetEventType (wxEVT_SCI_URIDROPPED);
        SetEventText (evt, scn.text, (sptr_t)strlen(scn.text));
        break;

    case SCN_DWELLSTART:
        evt.SetEventType (wxEVT_SCI_DWELLSTART);
        evt.SetX (scn.x);
        evt.SetY (scn.y);
        break;

    case SCN_DWELLEND:
        evt.SetEventType (wxEVT_SCI_DWELLEND);
        evt.SetX (scn.x);
        evt.SetY (scn.y);
        break;

    case SCN_ZOOM:
        evt.SetEventType (wxEVT_SCI_ZOOM);
        break;

    case SCN_HOTSPOTCLICK:
        evt.SetEventType (wxEVT_SCI_HOTSPOT_CLICK);
		evt.SetText (sci2wx(scn.text));
        break;

    case SCN_HOTSPOTDOUBLECLICK:
        evt.SetEventType (wxEVT_SCI_HOTSPOT_DCLICK);
        break;

    case SCN_CALLTIPCLICK:
        evt.SetEventType (wxEVT_SCI_CALLTIP_CLICK);
        break;

    case SCN_AUTOCSELECTION:
        evt.SetEventType (wxEVT_SCI_AUTOCOMP_SELECTION);
        break;

    default:
        return;
    }

    GetEventHandler()->ProcessEvent (evt);
}


//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

wxScintillaEvent::wxScintillaEvent (wxEventType commandType, int id)
                : wxCommandEvent (commandType, id) {
    m_position = 0;
    m_key = 0;
    m_modifiers = 0;
    m_modificationType = 0;
    m_length = 0;
    m_linesAdded = 0;
    m_line = 0;
    m_foldLevelNow = 0;
    m_foldLevelPrev = 0;
    m_margin = 0;
    m_message = 0;
    m_wParam = 0;
    m_lParam = 0;
    m_listType = 0;
    m_x = 0;
    m_y = 0;
    m_dragAllowMove = FALSE;
#if wxUSE_DRAG_AND_DROP
    m_dragResult = wxDragNone;
#endif
}

bool wxScintillaEvent::GetShift() const { return (m_modifiers & SCI_SHIFT) != 0; }
bool wxScintillaEvent::GetControl() const { return (m_modifiers & SCI_CTRL) != 0; }
bool wxScintillaEvent::GetAlt() const { return (m_modifiers & SCI_ALT) != 0; }


wxScintillaEvent::wxScintillaEvent (const wxScintillaEvent& event)
                : wxCommandEvent(event) {
    m_position =         event.m_position;
    m_key =              event.m_key;
    m_modifiers =        event.m_modifiers;
    m_modificationType = event.m_modificationType;
    m_text =             event.m_text;
    m_length =           event.m_length;
    m_linesAdded =       event.m_linesAdded;
    m_line =             event.m_line;
    m_foldLevelNow =     event.m_foldLevelNow;
    m_foldLevelPrev =    event.m_foldLevelPrev;
    m_margin =           event.m_margin;
    m_message =          event.m_message;
    m_wParam =           event.m_wParam;
    m_lParam =           event.m_lParam;
    m_listType =         event.m_listType;
    m_x =                event.m_x;
    m_y =                event.m_y;
    m_dragText =         event.m_dragText;
    m_dragAllowMove =    event.m_dragAllowMove;
#if wxUSE_DRAG_AND_DROP
    m_dragResult =       event.m_dragResult;
#endif
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
