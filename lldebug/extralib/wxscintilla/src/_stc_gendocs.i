DocStr(wxScintilla::AddText,
"Add text to the document at current position.", "");

DocStr(wxScintilla::AddStyledText,
"Add array of cells to document.", "");

DocStr(wxScintilla::InsertText,
"Insert string at a position.", "");

DocStr(wxScintilla::ClearAll,
"Delete all text in the document.", "");

DocStr(wxScintilla::ClearDocumentStyle,
"Set all style bytes to 0, remove all folding information.", "");

DocStr(wxScintilla::GetLength,
"Returns the number of characters in the document.", "");

DocStr(wxScintilla::GetCharAt,
"Returns the character byte at the position.", "");

DocStr(wxScintilla::GetCurrentPos,
"Returns the position of the caret.", "");

DocStr(wxScintilla::GetAnchor,
"Returns the position of the opposite end of the selection to the caret.", "");

DocStr(wxScintilla::GetStyleAt,
"Returns the style byte at the position.", "");

DocStr(wxScintilla::Redo,
"Redoes the next action on the undo history.", "");

DocStr(wxScintilla::SetUndoCollection,
"Choose between collecting actions into the undo
history and discarding them.", "");

DocStr(wxScintilla::SelectAll,
"Select all the text in the document.", "");

DocStr(wxScintilla::SetSavePoint,
"Remember the current position in the undo history as the position
at which the document was saved.", "");

DocStr(wxScintilla::GetStyledText,
"Retrieve a buffer of cells.", "");

DocStr(wxScintilla::CanRedo,
"Are there any redoable actions in the undo history?", "");

DocStr(wxScintilla::MarkerLineFromHandle,
"Retrieve the line number at which a particular marker is located.", "");

DocStr(wxScintilla::MarkerDeleteHandle,
"Delete a marker.", "");

DocStr(wxScintilla::GetUndoCollection,
"Is undo history being collected?", "");

DocStr(wxScintilla::GetViewWhiteSpace,
"Are white space characters currently visible?
Returns one of SCWS_* constants.", "");

DocStr(wxScintilla::SetViewWhiteSpace,
"Make white space characters invisible, always visible or visible outside indentation.", "");

DocStr(wxScintilla::PositionFromPoint,
"Find the position from a point within the window.", "");

DocStr(wxScintilla::PositionFromPointClose,
"Find the position from a point within the window but return
INVALID_POSITION if not close to text.", "");

DocStr(wxScintilla::GotoLine,
"Set caret to start of a line and ensure it is visible.", "");

DocStr(wxScintilla::GotoPos,
"Set caret to a position and ensure it is visible.", "");

DocStr(wxScintilla::SetAnchor,
"Set the selection anchor to a position. The anchor is the opposite
end of the selection from the caret.", "");

DocStr(wxScintilla::GetCurLine,
"Retrieve the text of the line containing the caret.
Returns the index of the caret on the line.", "");

DocStr(wxScintilla::GetEndStyled,
"Retrieve the position of the last correctly styled character.", "");

DocStr(wxScintilla::ConvertEOLs,
"Convert all line endings in the document to one mode.", "");

DocStr(wxScintilla::GetEOLMode,
"Retrieve the current end of line mode - one of CRLF, CR, or LF.", "");

DocStr(wxScintilla::SetEOLMode,
"Set the current end of line mode.", "");

DocStr(wxScintilla::StartStyling,
"Set the current styling position to pos and the styling mask to mask.
The styling mask can be used to protect some bits in each styling byte from modification.", "");

DocStr(wxScintilla::SetStyling,
"Change style from current styling position for length characters to a style
and move the current styling position to after this newly styled segment.", "");

DocStr(wxScintilla::GetBufferedDraw,
"Is drawing done first into a buffer or direct to the screen?", "");

DocStr(wxScintilla::SetBufferedDraw,
"If drawing is buffered then each line of text is drawn into a bitmap buffer
before drawing it to the screen to avoid flicker.", "");

DocStr(wxScintilla::SetTabWidth,
"Change the visible size of a tab to be a multiple of the width of a space character.", "");

DocStr(wxScintilla::GetTabWidth,
"Retrieve the visible size of a tab.", "");

DocStr(wxScintilla::SetCodePage,
"Set the code page used to interpret the bytes of the document as characters.", "");

DocStr(wxScintilla::MarkerDefine,
"Set the symbol used for a particular marker number,
and optionally the fore and background colours.", "");

DocStr(wxScintilla::MarkerSetForeground,
"Set the foreground colour used for a particular marker number.", "");

DocStr(wxScintilla::MarkerSetBackground,
"Set the background colour used for a particular marker number.", "");

DocStr(wxScintilla::MarkerAdd,
"Add a marker to a line, returning an ID which can be used to find or delete the marker.", "");

DocStr(wxScintilla::MarkerDelete,
"Delete a marker from a line.", "");

DocStr(wxScintilla::MarkerDeleteAll,
"Delete all markers with a particular number from all lines.", "");

DocStr(wxScintilla::MarkerGet,
"Get a bit mask of all the markers set on a line.", "");

DocStr(wxScintilla::MarkerNext,
"Find the next line after lineStart that includes a marker in mask.", "");

DocStr(wxScintilla::MarkerPrevious,
"Find the previous line before lineStart that includes a marker in mask.", "");

DocStr(wxScintilla::MarkerDefineBitmap,
"Define a marker from a bitmap", "");

DocStr(wxScintilla::MarkerAddSet,
"Add a set of markers to a line.", "");

DocStr(wxScintilla::MarkerSetAlpha,
"Set the alpha used for a marker that is drawn in the text area, not the margin.", "");

DocStr(wxScintilla::SetMarginType,
"Set a margin to be either numeric or symbolic.", "");

DocStr(wxScintilla::GetMarginType,
"Retrieve the type of a margin.", "");

DocStr(wxScintilla::SetMarginWidth,
"Set the width of a margin to a width expressed in pixels.", "");

DocStr(wxScintilla::GetMarginWidth,
"Retrieve the width of a margin in pixels.", "");

DocStr(wxScintilla::SetMarginMask,
"Set a mask that determines which markers are displayed in a margin.", "");

DocStr(wxScintilla::GetMarginMask,
"Retrieve the marker mask of a margin.", "");

DocStr(wxScintilla::SetMarginSensitive,
"Make a margin sensitive or insensitive to mouse clicks.", "");

DocStr(wxScintilla::GetMarginSensitive,
"Retrieve the mouse click sensitivity of a margin.", "");

DocStr(wxScintilla::StyleClearAll,
"Clear all the styles and make equivalent to the global default style.", "");

DocStr(wxScintilla::StyleSetForeground,
"Set the foreground colour of a style.", "");

DocStr(wxScintilla::StyleSetBackground,
"Set the background colour of a style.", "");

DocStr(wxScintilla::StyleSetBold,
"Set a style to be bold or not.", "");

DocStr(wxScintilla::StyleSetItalic,
"Set a style to be italic or not.", "");

DocStr(wxScintilla::StyleSetSize,
"Set the size of characters of a style.", "");

DocStr(wxScintilla::StyleSetFaceName,
"Set the font of a style.", "");

DocStr(wxScintilla::StyleSetEOLFilled,
"Set a style to have its end of line filled or not.", "");

DocStr(wxScintilla::StyleResetDefault,
"Reset the default style to its state at startup", "");

DocStr(wxScintilla::StyleSetUnderline,
"Set a style to be underlined or not.", "");

DocStr(wxScintilla::StyleGetForeground,
"Get the foreground colour of a style.", "");

DocStr(wxScintilla::StyleGetBackground,
"Get the background colour of a style.", "");

DocStr(wxScintilla::StyleGetBold,
"Get is a style bold or not.", "");

DocStr(wxScintilla::StyleGetItalic,
"Get is a style italic or not.", "");

DocStr(wxScintilla::StyleGetSize,
"Get the size of characters of a style.", "");

DocStr(wxScintilla::StyleGetFaceName,
"Get the font facename of a style", "");

DocStr(wxScintilla::StyleGetEOLFilled,
"Get is a style to have its end of line filled or not.", "");

DocStr(wxScintilla::StyleGetUnderline,
"Get is a style underlined or not.", "");

DocStr(wxScintilla::StyleGetCase,
"Get is a style mixed case, or to force upper or lower case.", "");

DocStr(wxScintilla::StyleGetCharacterSet,
"Get the character get of the font in a style.", "");

DocStr(wxScintilla::StyleGetVisible,
"Get is a style visible or not.", "");

DocStr(wxScintilla::StyleGetChangeable,
"Get is a style changeable or not (read only).
Experimental feature, currently buggy.", "");

DocStr(wxScintilla::StyleGetHotSpot,
"Get is a style a hotspot or not.", "");

DocStr(wxScintilla::StyleSetCase,
"Set a style to be mixed case, or to force upper or lower case.", "");

DocStr(wxScintilla::StyleSetHotSpot,
"Set a style to be a hotspot or not.", "");

DocStr(wxScintilla::SetSelForeground,
"Set the foreground colour of the selection and whether to use this setting.", "");

DocStr(wxScintilla::SetSelBackground,
"Set the background colour of the selection and whether to use this setting.", "");

DocStr(wxScintilla::GetSelAlpha,
"Get the alpha of the selection.", "");

DocStr(wxScintilla::SetSelAlpha,
"Set the alpha of the selection.", "");

DocStr(wxScintilla::GetSelEOLFilled,
"Is the selection end of line filled?", "");

DocStr(wxScintilla::SetSelEOLFilled,
"Set the selection to have its end of line filled or not.", "");

DocStr(wxScintilla::SetCaretForeground,
"Set the foreground colour of the caret.", "");

DocStr(wxScintilla::CmdKeyAssign,
"When key+modifier combination km is pressed perform msg.", "");

DocStr(wxScintilla::CmdKeyClear,
"When key+modifier combination km is pressed do nothing.", "");

DocStr(wxScintilla::CmdKeyClearAll,
"Drop all key mappings.", "");

DocStr(wxScintilla::SetStyleBytes,
"Set the styles for a segment of the document.", "");

DocStr(wxScintilla::StyleSetVisible,
"Set a style to be visible or not.", "");

DocStr(wxScintilla::GetCaretPeriod,
"Get the time in milliseconds that the caret is on and off.", "");

DocStr(wxScintilla::SetCaretPeriod,
"Get the time in milliseconds that the caret is on and off. 0 = steady on.", "");

DocStr(wxScintilla::SetWordChars,
"Set the set of characters making up words for when moving or selecting by word.
First sets deaults like SetCharsDefault.", "");

DocStr(wxScintilla::BeginUndoAction,
"Start a sequence of actions that is undone and redone as a unit.
May be nested.", "");

DocStr(wxScintilla::EndUndoAction,
"End a sequence of actions that is undone and redone as a unit.", "");

DocStr(wxScintilla::IndicatorSetStyle,
"Set an indicator to plain, squiggle or TT.", "");

DocStr(wxScintilla::IndicatorGetStyle,
"Retrieve the style of an indicator.", "");

DocStr(wxScintilla::IndicatorSetForeground,
"Set the foreground colour of an indicator.", "");

DocStr(wxScintilla::IndicatorGetForeground,
"Retrieve the foreground colour of an indicator.", "");

DocStr(wxScintilla::IndicatorSetUnder,
"Set an indicator to draw under text or over(default).", "");

DocStr(wxScintilla::IndicatorGetUnder,
"Retrieve whether indicator drawn under or over text.", "");

DocStr(wxScintilla::SetWhitespaceForeground,
"Set the foreground colour of all whitespace and whether to use this setting.", "");

DocStr(wxScintilla::SetWhitespaceBackground,
"Set the background colour of all whitespace and whether to use this setting.", "");

DocStr(wxScintilla::SetStyleBits,
"Divide each styling byte into lexical class bits (default: 5) and indicator
bits (default: 3). If a lexer requires more than 32 lexical states, then this
is used to expand the possible states.", "");

DocStr(wxScintilla::GetStyleBits,
"Retrieve number of bits in style bytes used to hold the lexical state.", "");

DocStr(wxScintilla::SetLineState,
"Used to hold extra styling information for each line.", "");

DocStr(wxScintilla::GetLineState,
"Retrieve the extra styling information for a line.", "");

DocStr(wxScintilla::GetMaxLineState,
"Retrieve the last line number that has line state.", "");

DocStr(wxScintilla::GetCaretLineVisible,
"Is the background of the line containing the caret in a different colour?", "");

DocStr(wxScintilla::SetCaretLineVisible,
"Display the background of the line containing the caret in a different colour.", "");

DocStr(wxScintilla::GetCaretLineBackground,
"Get the colour of the background of the line containing the caret.", "");

DocStr(wxScintilla::SetCaretLineBackground,
"Set the colour of the background of the line containing the caret.", "");

DocStr(wxScintilla::StyleSetChangeable,
"Set a style to be changeable or not (read only).
Experimental feature, currently buggy.", "");

DocStr(wxScintilla::AutoCompShow,
"Display a auto-completion list.
The lenEntered parameter indicates how many characters before
the caret should be used to provide context.", "");

DocStr(wxScintilla::AutoCompCancel,
"Remove the auto-completion list from the screen.", "");

DocStr(wxScintilla::AutoCompActive,
"Is there an auto-completion list visible?", "");

DocStr(wxScintilla::AutoCompPosStart,
"Retrieve the position of the caret when the auto-completion list was displayed.", "");

DocStr(wxScintilla::AutoCompComplete,
"User has selected an item so remove the list and insert the selection.", "");

DocStr(wxScintilla::AutoCompStops,
"Define a set of character that when typed cancel the auto-completion list.", "");

DocStr(wxScintilla::AutoCompSetSeparator,
"Change the separator character in the string setting up an auto-completion list.
Default is space but can be changed if items contain space.", "");

DocStr(wxScintilla::AutoCompGetSeparator,
"Retrieve the auto-completion list separator character.", "");

DocStr(wxScintilla::AutoCompSelect,
"Select the item in the auto-completion list that starts with a string.", "");

DocStr(wxScintilla::AutoCompSetCancelAtStart,
"Should the auto-completion list be cancelled if the user backspaces to a
position before where the box was created.", "");

DocStr(wxScintilla::AutoCompGetCancelAtStart,
"Retrieve whether auto-completion cancelled by backspacing before start.", "");

DocStr(wxScintilla::AutoCompSetFillUps,
"Define a set of characters that when typed will cause the autocompletion to
choose the selected item.", "");

DocStr(wxScintilla::AutoCompSetChooseSingle,
"Should a single item auto-completion list automatically choose the item.", "");

DocStr(wxScintilla::AutoCompGetChooseSingle,
"Retrieve whether a single item auto-completion list automatically choose the item.", "");

DocStr(wxScintilla::AutoCompSetIgnoreCase,
"Set whether case is significant when performing auto-completion searches.", "");

DocStr(wxScintilla::AutoCompGetIgnoreCase,
"Retrieve state of ignore case flag.", "");

DocStr(wxScintilla::UserListShow,
"Display a list of strings and send notification when user chooses one.", "");

DocStr(wxScintilla::AutoCompSetAutoHide,
"Set whether or not autocompletion is hidden automatically when nothing matches.", "");

DocStr(wxScintilla::AutoCompGetAutoHide,
"Retrieve whether or not autocompletion is hidden automatically when nothing matches.", "");

DocStr(wxScintilla::AutoCompSetDropRestOfWord,
"Set whether or not autocompletion deletes any word characters
after the inserted text upon completion.", "");

DocStr(wxScintilla::AutoCompGetDropRestOfWord,
"Retrieve whether or not autocompletion deletes any word characters
after the inserted text upon completion.", "");

DocStr(wxScintilla::RegisterImage,
"Register an image for use in autocompletion lists.", "");

DocStr(wxScintilla::ClearRegisteredImages,
"Clear all the registered images.", "");

DocStr(wxScintilla::AutoCompGetTypeSeparator,
"Retrieve the auto-completion list type-separator character.", "");

DocStr(wxScintilla::AutoCompSetTypeSeparator,
"Change the type-separator character in the string setting up an auto-completion list.
Default is '?' but can be changed if items contain '?'.", "");

DocStr(wxScintilla::AutoCompSetMaxWidth,
"Set the maximum width, in characters, of auto-completion and user lists.
Set to 0 to autosize to fit longest item, which is the default.", "");

DocStr(wxScintilla::AutoCompGetMaxWidth,
"Get the maximum width, in characters, of auto-completion and user lists.", "");

DocStr(wxScintilla::AutoCompSetMaxHeight,
"Set the maximum height, in rows, of auto-completion and user lists.
The default is 5 rows.", "");

DocStr(wxScintilla::AutoCompGetMaxHeight,
"Set the maximum height, in rows, of auto-completion and user lists.", "");

DocStr(wxScintilla::SetIndent,
"Set the number of spaces used for one level of indentation.", "");

DocStr(wxScintilla::GetIndent,
"Retrieve indentation size.", "");

DocStr(wxScintilla::SetUseTabs,
"Indentation will only use space characters if useTabs is false, otherwise
it will use a combination of tabs and spaces.", "");

DocStr(wxScintilla::GetUseTabs,
"Retrieve whether tabs will be used in indentation.", "");

DocStr(wxScintilla::SetLineIndentation,
"Change the indentation of a line to a number of columns.", "");

DocStr(wxScintilla::GetLineIndentation,
"Retrieve the number of columns that a line is indented.", "");

DocStr(wxScintilla::GetLineIndentPosition,
"Retrieve the position before the first non indentation character on a line.", "");

DocStr(wxScintilla::GetColumn,
"Retrieve the column number of a position, taking tab width into account.", "");

DocStr(wxScintilla::SetUseHorizontalScrollBar,
"Show or hide the horizontal scroll bar.", "");

DocStr(wxScintilla::GetUseHorizontalScrollBar,
"Is the horizontal scroll bar visible?", "");

DocStr(wxScintilla::SetIndentationGuides,
"Show or hide indentation guides.", "");

DocStr(wxScintilla::GetIndentationGuides,
"Are the indentation guides visible?", "");

DocStr(wxScintilla::SetHighlightGuide,
"Set the highlighted indentation guide column.
0 = no highlighted guide.", "");

DocStr(wxScintilla::GetHighlightGuide,
"Get the highlighted indentation guide column.", "");

DocStr(wxScintilla::GetLineEndPosition,
"Get the position after the last visible characters on a line.", "");

DocStr(wxScintilla::GetCodePage,
"Get the code page used to interpret the bytes of the document as characters.", "");

DocStr(wxScintilla::GetCaretForeground,
"Get the foreground colour of the caret.", "");

DocStr(wxScintilla::GetReadOnly,
"In read-only mode?", "");

DocStr(wxScintilla::SetCurrentPos,
"Sets the position of the caret.", "");

DocStr(wxScintilla::SetSelectionStart,
"Sets the position that starts the selection - this becomes the anchor.", "");

DocStr(wxScintilla::GetSelectionStart,
"Returns the position at the start of the selection.", "");

DocStr(wxScintilla::SetSelectionEnd,
"Sets the position that ends the selection - this becomes the currentPosition.", "");

DocStr(wxScintilla::GetSelectionEnd,
"Returns the position at the end of the selection.", "");

DocStr(wxScintilla::SetPrintMagnification,
"Sets the print magnification added to the point size of each style for printing.", "");

DocStr(wxScintilla::GetPrintMagnification,
"Returns the print magnification.", "");

DocStr(wxScintilla::SetPrintColourMode,
"Modify colours when printing for clearer printed text.", "");

DocStr(wxScintilla::GetPrintColourMode,
"Returns the print colour mode.", "");

DocStr(wxScintilla::FindText,
"Find some text in the document.", "");

DocStr(wxScintilla::FormatRange,
"On Windows, will draw the document into a display context such as a printer.", "");

DocStr(wxScintilla::GetFirstVisibleLine,
"Retrieve the display line at the top of the display.", "");

DocStr(wxScintilla::GetLine,
"Retrieve the contents of a line.", "");

DocStr(wxScintilla::GetLineCount,
"Returns the number of lines in the document. There is always at least one.", "");

DocStr(wxScintilla::SetMarginLeft,
"Sets the size in pixels of the left margin.", "");

DocStr(wxScintilla::GetMarginLeft,
"Returns the size in pixels of the left margin.", "");

DocStr(wxScintilla::SetMarginRight,
"Sets the size in pixels of the right margin.", "");

DocStr(wxScintilla::GetMarginRight,
"Returns the size in pixels of the right margin.", "");

DocStr(wxScintilla::GetModify,
"Is the document different from when it was last saved?", "");

DocStr(wxScintilla::SetSelection,
"Select a range of text.", "");

DocStr(wxScintilla::GetSelectedText,
"Retrieve the selected text.", "");

DocStr(wxScintilla::GetTextRange,
"Retrieve a range of text.", "");

DocStr(wxScintilla::HideSelection,
"Draw the selection in normal style or with selection highlighted.", "");

DocStr(wxScintilla::LineFromPosition,
"Retrieve the line containing a position.", "");

DocStr(wxScintilla::PositionFromLine,
"Retrieve the position at the start of a line.", "");

DocStr(wxScintilla::LineScroll,
"Scroll horizontally and vertically.", "");

DocStr(wxScintilla::EnsureCaretVisible,
"Ensure the caret is visible.", "");

DocStr(wxScintilla::ReplaceSelection,
"Replace the selected text with the argument text.", "");

DocStr(wxScintilla::SetReadOnly,
"Set to read only or read write.", "");

DocStr(wxScintilla::CanPaste,
"Will a paste succeed?", "");

DocStr(wxScintilla::CanUndo,
"Are there any undoable actions in the undo history?", "");

DocStr(wxScintilla::EmptyUndoBuffer,
"Delete the undo history.", "");

DocStr(wxScintilla::Undo,
"Undo one action in the undo history.", "");

DocStr(wxScintilla::Cut,
"Cut the selection to the clipboard.", "");

DocStr(wxScintilla::Copy,
"Copy the selection to the clipboard.", "");

DocStr(wxScintilla::Paste,
"Paste the contents of the clipboard into the document replacing the selection.", "");

DocStr(wxScintilla::Clear,
"Clear the selection.", "");

DocStr(wxScintilla::SetText,
"Replace the contents of the document with the argument text.", "");

DocStr(wxScintilla::GetText,
"Retrieve all the text in the document.", "");

DocStr(wxScintilla::GetTextLength,
"Retrieve the number of characters in the document.", "");

DocStr(wxScintilla::SetOvertype,
"Set to overtype (true) or insert mode.", "");

DocStr(wxScintilla::GetOvertype,
"Returns true if overtype mode is active otherwise false is returned.", "");

DocStr(wxScintilla::SetCaretWidth,
"Set the width of the insert mode caret.", "");

DocStr(wxScintilla::GetCaretWidth,
"Returns the width of the insert mode caret.", "");

DocStr(wxScintilla::SetTargetStart,
"Sets the position that starts the target which is used for updating the
document without affecting the scroll position.", "");

DocStr(wxScintilla::GetTargetStart,
"Get the position that starts the target.", "");

DocStr(wxScintilla::SetTargetEnd,
"Sets the position that ends the target which is used for updating the
document without affecting the scroll position.", "");

DocStr(wxScintilla::GetTargetEnd,
"Get the position that ends the target.", "");

DocStr(wxScintilla::ReplaceTarget,
"Replace the target text with the argument text.
Text is counted so it can contain NULs.
Returns the length of the replacement text.", "");

DocStr(wxScintilla::ReplaceTargetRE,
"Replace the target text with the argument text after \d processing.
Text is counted so it can contain NULs.
Looks for \d where d is between 1 and 9 and replaces these with the strings
matched in the last search operation which were surrounded by \( and \).
Returns the length of the replacement text including any change
caused by processing the \d patterns.", "");

DocStr(wxScintilla::SearchInTarget,
"Search for a counted string in the target and set the target to the found
range. Text is counted so it can contain NULs.
Returns length of range or -1 for failure in which case target is not moved.", "");

DocStr(wxScintilla::SetSearchFlags,
"Set the search flags used by SearchInTarget.", "");

DocStr(wxScintilla::GetSearchFlags,
"Get the search flags used by SearchInTarget.", "");

DocStr(wxScintilla::CallTipShow,
"Show a call tip containing a definition near position pos.", "");

DocStr(wxScintilla::CallTipCancel,
"Remove the call tip from the screen.", "");

DocStr(wxScintilla::CallTipActive,
"Is there an active call tip?", "");

DocStr(wxScintilla::CallTipPosAtStart,
"Retrieve the position where the caret was before displaying the call tip.", "");

DocStr(wxScintilla::CallTipSetHighlight,
"Highlight a segment of the definition.", "");

DocStr(wxScintilla::CallTipSetBackground,
"Set the background colour for the call tip.", "");

DocStr(wxScintilla::CallTipSetForeground,
"Set the foreground colour for the call tip.", "");

DocStr(wxScintilla::CallTipSetForegroundHighlight,
"Set the foreground colour for the highlighted part of the call tip.", "");

DocStr(wxScintilla::CallTipUseStyle,
"Enable use of STYLE_CALLTIP and set call tip tab size in pixels.", "");

DocStr(wxScintilla::VisibleFromDocLine,
"Find the display line of a document line taking hidden lines into account.", "");

DocStr(wxScintilla::DocLineFromVisible,
"Find the document line of a display line taking hidden lines into account.", "");

DocStr(wxScintilla::WrapCount,
"The number of display lines needed to wrap a document line", "");

DocStr(wxScintilla::SetFoldLevel,
"Set the fold level of a line.
This encodes an integer level along with flags indicating whether the
line is a header and whether it is effectively white space.", "");

DocStr(wxScintilla::GetFoldLevel,
"Retrieve the fold level of a line.", "");

DocStr(wxScintilla::GetLastChild,
"Find the last child line of a header line.", "");

DocStr(wxScintilla::GetFoldParent,
"Find the parent line of a child line.", "");

DocStr(wxScintilla::ShowLines,
"Make a range of lines visible.", "");

DocStr(wxScintilla::HideLines,
"Make a range of lines invisible.", "");

DocStr(wxScintilla::GetLineVisible,
"Is a line visible?", "");

DocStr(wxScintilla::SetFoldExpanded,
"Show the children of a header line.", "");

DocStr(wxScintilla::GetFoldExpanded,
"Is a header line expanded?", "");

DocStr(wxScintilla::ToggleFold,
"Switch a header line between expanded and contracted.", "");

DocStr(wxScintilla::EnsureVisible,
"Ensure a particular line is visible by expanding any header line hiding it.", "");

DocStr(wxScintilla::SetFoldFlags,
"Set some style options for folding.", "");

DocStr(wxScintilla::EnsureVisibleEnforcePolicy,
"Ensure a particular line is visible by expanding any header line hiding it.
Use the currently set visibility policy to determine which range to display.", "");

DocStr(wxScintilla::SetTabIndents,
"Sets whether a tab pressed when caret is within indentation indents.", "");

DocStr(wxScintilla::GetTabIndents,
"Does a tab pressed when caret is within indentation indent?", "");

DocStr(wxScintilla::SetBackSpaceUnIndents,
"Sets whether a backspace pressed when caret is within indentation unindents.", "");

DocStr(wxScintilla::GetBackSpaceUnIndents,
"Does a backspace pressed when caret is within indentation unindent?", "");

DocStr(wxScintilla::SetMouseDwellTime,
"Sets the time the mouse must sit still to generate a mouse dwell event.", "");

DocStr(wxScintilla::GetMouseDwellTime,
"Retrieve the time the mouse must sit still to generate a mouse dwell event.", "");

DocStr(wxScintilla::WordStartPosition,
"Get position of start of word.", "");

DocStr(wxScintilla::WordEndPosition,
"Get position of end of word.", "");

DocStr(wxScintilla::SetWrapMode,
"Sets whether text is word wrapped.", "");

DocStr(wxScintilla::GetWrapMode,
"Retrieve whether text is word wrapped.", "");

DocStr(wxScintilla::SetWrapVisualFlags,
"Set the display mode of visual flags for wrapped lines.", "");

DocStr(wxScintilla::GetWrapVisualFlags,
"Retrive the display mode of visual flags for wrapped lines.", "");

DocStr(wxScintilla::SetWrapVisualFlagsLocation,
"Set the location of visual flags for wrapped lines.", "");

DocStr(wxScintilla::GetWrapVisualFlagsLocation,
"Retrive the location of visual flags for wrapped lines.", "");

DocStr(wxScintilla::SetWrapStartIndent,
"Set the start indent for wrapped lines.", "");

DocStr(wxScintilla::GetWrapStartIndent,
"Retrive the start indent for wrapped lines.", "");

DocStr(wxScintilla::SetLayoutCache,
"Sets the degree of caching of layout information.", "");

DocStr(wxScintilla::GetLayoutCache,
"Retrieve the degree of caching of layout information.", "");

DocStr(wxScintilla::SetScrollWidth,
"Sets the document width assumed for scrolling.", "");

DocStr(wxScintilla::GetScrollWidth,
"Retrieve the document width assumed for scrolling.", "");

DocStr(wxScintilla::SetScrollWidthTracking,
"Sets whether the maximum width line displayed is used to set scroll width.", "");

DocStr(wxScintilla::GetScrollWidthTracking,
"Retrieve whether the scroll width tracks wide lines.", "");

DocStr(wxScintilla::TextWidth,
"Measure the pixel width of some text in a particular style.
NUL terminated text argument.
Does not handle tab or control characters.", "");

DocStr(wxScintilla::SetEndAtLastLine,
"Sets the scroll range so that maximum scroll position has
the last line at the bottom of the view (default).
Setting this to false allows scrolling one page below the last line.", "");

DocStr(wxScintilla::GetEndAtLastLine,
"Retrieve whether the maximum scroll position has the last
line at the bottom of the view.", "");

DocStr(wxScintilla::TextHeight,
"Retrieve the height of a particular line of text in pixels.", "");

DocStr(wxScintilla::SetUseVerticalScrollBar,
"Show or hide the vertical scroll bar.", "");

DocStr(wxScintilla::GetUseVerticalScrollBar,
"Is the vertical scroll bar visible?", "");

DocStr(wxScintilla::AppendText,
"Append a string to the end of the document without changing the selection.", "");

DocStr(wxScintilla::GetTwoPhaseDraw,
"Is drawing done in two phases with backgrounds drawn before faoregrounds?", "");

DocStr(wxScintilla::SetTwoPhaseDraw,
"In twoPhaseDraw mode, drawing is performed in two phases, first the background
and then the foreground. This avoids chopping off characters that overlap the next run.", "");

DocStr(wxScintilla::TargetFromSelection,
"Make the target range start and end be the same as the selection range start and end.", "");

DocStr(wxScintilla::LinesJoin,
"Join the lines in the target.", "");

DocStr(wxScintilla::LinesSplit,
"Split the lines in the target into lines that are less wide than pixelWidth
where possible.", "");

DocStr(wxScintilla::SetFoldMarginColour,
"Set the colours used as a chequerboard pattern in the fold margin", "");

DocStr(wxScintilla::SetFoldMarginHiColour,
"", "");

DocStr(wxScintilla::LineDown,
"Move caret down one line.", "");

DocStr(wxScintilla::LineDownExtend,
"Move caret down one line extending selection to new caret position.", "");

DocStr(wxScintilla::LineUp,
"Move caret up one line.", "");

DocStr(wxScintilla::LineUpExtend,
"Move caret up one line extending selection to new caret position.", "");

DocStr(wxScintilla::CharLeft,
"Move caret left one character.", "");

DocStr(wxScintilla::CharLeftExtend,
"Move caret left one character extending selection to new caret position.", "");

DocStr(wxScintilla::CharRight,
"Move caret right one character.", "");

DocStr(wxScintilla::CharRightExtend,
"Move caret right one character extending selection to new caret position.", "");

DocStr(wxScintilla::WordLeft,
"Move caret left one word.", "");

DocStr(wxScintilla::WordLeftExtend,
"Move caret left one word extending selection to new caret position.", "");

DocStr(wxScintilla::WordRight,
"Move caret right one word.", "");

DocStr(wxScintilla::WordRightExtend,
"Move caret right one word extending selection to new caret position.", "");

DocStr(wxScintilla::Home,
"Move caret to first position on line.", "");

DocStr(wxScintilla::HomeExtend,
"Move caret to first position on line extending selection to new caret position.", "");

DocStr(wxScintilla::LineEnd,
"Move caret to last position on line.", "");

DocStr(wxScintilla::LineEndExtend,
"Move caret to last position on line extending selection to new caret position.", "");

DocStr(wxScintilla::DocumentStart,
"Move caret to first position in document.", "");

DocStr(wxScintilla::DocumentStartExtend,
"Move caret to first position in document extending selection to new caret position.", "");

DocStr(wxScintilla::DocumentEnd,
"Move caret to last position in document.", "");

DocStr(wxScintilla::DocumentEndExtend,
"Move caret to last position in document extending selection to new caret position.", "");

DocStr(wxScintilla::PageUp,
"Move caret one page up.", "");

DocStr(wxScintilla::PageUpExtend,
"Move caret one page up extending selection to new caret position.", "");

DocStr(wxScintilla::PageDown,
"Move caret one page down.", "");

DocStr(wxScintilla::PageDownExtend,
"Move caret one page down extending selection to new caret position.", "");

DocStr(wxScintilla::EditToggleOvertype,
"Switch from insert to overtype mode or the reverse.", "");

DocStr(wxScintilla::Cancel,
"Cancel any modes such as call tip or auto-completion list display.", "");

DocStr(wxScintilla::DeleteBack,
"Delete the selection or if no selection, the character before the caret.", "");

DocStr(wxScintilla::Tab,
"If selection is empty or all on one line replace the selection with a tab character.
If more than one line selected, indent the lines.", "");

DocStr(wxScintilla::BackTab,
"Dedent the selected lines.", "");

DocStr(wxScintilla::NewLine,
"Insert a new line, may use a CRLF, CR or LF depending on EOL mode.", "");

DocStr(wxScintilla::FormFeed,
"Insert a Form Feed character.", "");

DocStr(wxScintilla::VCHome,
"Move caret to before first visible character on line.
If already there move to first character on line.", "");

DocStr(wxScintilla::VCHomeExtend,
"Like VCHome but extending selection to new caret position.", "");

DocStr(wxScintilla::ZoomIn,
"Magnify the displayed text by increasing the sizes by 1 point.", "");

DocStr(wxScintilla::ZoomOut,
"Make the displayed text smaller by decreasing the sizes by 1 point.", "");

DocStr(wxScintilla::DelWordLeft,
"Delete the word to the left of the caret.", "");

DocStr(wxScintilla::DelWordRight,
"Delete the word to the right of the caret.", "");

DocStr(wxScintilla::DelWordRightEnd,
"Delete the word to the right of the caret, but not the trailing non-word characters.", "");

DocStr(wxScintilla::LineCut,
"Cut the line containing the caret.", "");

DocStr(wxScintilla::LineDelete,
"Delete the line containing the caret.", "");

DocStr(wxScintilla::LineTranspose,
"Switch the current line with the previous.", "");

DocStr(wxScintilla::LineDuplicate,
"Duplicate the current line.", "");

DocStr(wxScintilla::LowerCase,
"Transform the selection to lower case.", "");

DocStr(wxScintilla::UpperCase,
"Transform the selection to upper case.", "");

DocStr(wxScintilla::LineScrollDown,
"Scroll the document down, keeping the caret visible.", "");

DocStr(wxScintilla::LineScrollUp,
"Scroll the document up, keeping the caret visible.", "");

DocStr(wxScintilla::DeleteBackNotLine,
"Delete the selection or if no selection, the character before the caret.
Will not delete the character before at the start of a line.", "");

DocStr(wxScintilla::HomeDisplay,
"Move caret to first position on display line.", "");

DocStr(wxScintilla::HomeDisplayExtend,
"Move caret to first position on display line extending selection to
new caret position.", "");

DocStr(wxScintilla::LineEndDisplay,
"Move caret to last position on display line.", "");

DocStr(wxScintilla::LineEndDisplayExtend,
"Move caret to last position on display line extending selection to new
caret position.", "");

DocStr(wxScintilla::HomeWrap,
"These are like their namesakes Home(Extend)?, LineEnd(Extend)?, VCHome(Extend)?
except they behave differently when word-wrap is enabled:
They go first to the start / end of the display line, like (Home|LineEnd)Display
The difference is that, the cursor is already at the point, it goes on to the start
or end of the document line, as appropriate for (Home|LineEnd|VCHome)(Extend)?.", "");

DocStr(wxScintilla::HomeWrapExtend,
"", "");

DocStr(wxScintilla::LineEndWrap,
"", "");

DocStr(wxScintilla::LineEndWrapExtend,
"", "");

DocStr(wxScintilla::VCHomeWrap,
"", "");

DocStr(wxScintilla::VCHomeWrapExtend,
"", "");

DocStr(wxScintilla::LineCopy,
"Copy the line containing the caret.", "");

DocStr(wxScintilla::MoveCaretInsideView,
"Move the caret inside current view if it's not there already.", "");

DocStr(wxScintilla::LineLength,
"How many characters are on a line, including end of line characters?", "");

DocStr(wxScintilla::BraceHighlight,
"Highlight the characters at two positions.", "");

DocStr(wxScintilla::BraceBadLight,
"Highlight the character at a position indicating there is no matching brace.", "");

DocStr(wxScintilla::BraceMatch,
"Find the position of a matching brace or INVALID_POSITION if no match.", "");

DocStr(wxScintilla::GetViewEOL,
"Are the end of line characters visible?", "");

DocStr(wxScintilla::SetViewEOL,
"Make the end of line characters visible or invisible.", "");

DocStr(wxScintilla::GetDocPointer,
"Retrieve a pointer to the document object.", "");

DocStr(wxScintilla::SetDocPointer,
"Change the document object used.", "");

DocStr(wxScintilla::SetModEventMask,
"Set which document modification events are sent to the container.", "");

DocStr(wxScintilla::GetEdgeColumn,
"Retrieve the column number which text should be kept within.", "");

DocStr(wxScintilla::SetEdgeColumn,
"Set the column number of the edge.
If text goes past the edge then it is highlighted.", "");

DocStr(wxScintilla::GetEdgeMode,
"Retrieve the edge highlight mode.", "");

DocStr(wxScintilla::SetEdgeMode,
"The edge may be displayed by a line (EDGE_LINE) or by highlighting text that
goes beyond it (EDGE_BACKGROUND) or not displayed at all (EDGE_NONE).", "");

DocStr(wxScintilla::GetEdgeColour,
"Retrieve the colour used in edge indication.", "");

DocStr(wxScintilla::SetEdgeColour,
"Change the colour used in edge indication.", "");

DocStr(wxScintilla::SearchAnchor,
"Sets the current caret position to be the search anchor.", "");

DocStr(wxScintilla::SearchNext,
"Find some text starting at the search anchor.
Does not ensure the selection is visible.", "");

DocStr(wxScintilla::SearchPrev,
"Find some text starting at the search anchor and moving backwards.
Does not ensure the selection is visible.", "");

DocStr(wxScintilla::LinesOnScreen,
"Retrieves the number of lines completely visible.", "");

DocStr(wxScintilla::UsePopUp,
"Set whether a pop up menu is displayed automatically when the user presses
the wrong mouse button.", "");

DocStr(wxScintilla::SelectionIsRectangle,
"Is the selection rectangular? The alternative is the more common stream selection.", "");

DocStr(wxScintilla::SetZoom,
"Set the zoom level. This number of points is added to the size of all fonts.
It may be positive to magnify or negative to reduce.", "");

DocStr(wxScintilla::GetZoom,
"Retrieve the zoom level.", "");

DocStr(wxScintilla::CreateDocument,
"Create a new document object.
Starts with reference count of 1 and not selected into editor.", "");

DocStr(wxScintilla::AddRefDocument,
"Extend life of document.", "");

DocStr(wxScintilla::ReleaseDocument,
"Release a reference to the document, deleting document if it fades to black.", "");

DocStr(wxScintilla::GetModEventMask,
"Get which document modification events are sent to the container.", "");

DocStr(wxScintilla::SetSCIFocus,
"Change internal focus flag.", "");

DocStr(wxScintilla::GetSCIFocus,
"Get internal focus flag.", "");

DocStr(wxScintilla::SetStatus,
"Change error status - 0 = OK.", "");

DocStr(wxScintilla::GetStatus,
"Get error status.", "");

DocStr(wxScintilla::SetMouseDownCaptures,
"Set whether the mouse is captured when its button is pressed.", "");

DocStr(wxScintilla::GetMouseDownCaptures,
"Get whether mouse gets captured.", "");

DocStr(wxScintilla::SetSCICursor,
"Sets the cursor to one of the SC_CURSOR* values.", "");

DocStr(wxScintilla::GetSCICursor,
"Get cursor type.", "");

DocStr(wxScintilla::SetControlCharSymbol,
"Change the way control characters are displayed:
If symbol is < 32, keep the drawn way, else, use the given character.", "");

DocStr(wxScintilla::GetControlCharSymbol,
"Get the way control characters are displayed.", "");

DocStr(wxScintilla::WordPartLeft,
"Move to the previous change in capitalisation.", "");

DocStr(wxScintilla::WordPartLeftExtend,
"Move to the previous change in capitalisation extending selection
to new caret position.", "");

DocStr(wxScintilla::WordPartRight,
"Move to the change next in capitalisation.", "");

DocStr(wxScintilla::WordPartRightExtend,
"Move to the next change in capitalisation extending selection
to new caret position.", "");

DocStr(wxScintilla::SetVisiblePolicy,
"Set the way the display area is determined when a particular line
is to be moved to by Find, FindNext, GotoLine, etc.", "");

DocStr(wxScintilla::DelLineLeft,
"Delete back from the current position to the start of the line.", "");

DocStr(wxScintilla::DelLineRight,
"Delete forwards from the current position to the end of the line.", "");

DocStr(wxScintilla::SetXOffset,
"Get and Set the xOffset (ie, horizonal scroll position).", "");

DocStr(wxScintilla::GetXOffset,
"", "");

DocStr(wxScintilla::ChooseCaretX,
"Set the last x chosen value to be the caret x position.", "");

DocStr(wxScintilla::SetXCaretPolicy,
"Set the way the caret is kept visible when going sideway.
The exclusion zone is given in pixels.", "");

DocStr(wxScintilla::SetYCaretPolicy,
"Set the way the line the caret is on is kept visible.
The exclusion zone is given in lines.", "");

DocStr(wxScintilla::SetPrintWrapMode,
"Set printing to line wrapped (SC_WRAP_WORD) or not line wrapped (SC_WRAP_NONE).", "");

DocStr(wxScintilla::GetPrintWrapMode,
"Is printing line wrapped?", "");

DocStr(wxScintilla::SetHotspotActiveForeground,
"Set a fore colour for active hotspots.", "");

DocStr(wxScintilla::GetHotspotActiveForeground,
"Get the fore colour for active hotspots.", "");

DocStr(wxScintilla::SetHotspotActiveBackground,
"Set a back colour for active hotspots.", "");

DocStr(wxScintilla::GetHotspotActiveBackground,
"Get the back colour for active hotspots.", "");

DocStr(wxScintilla::SetHotspotActiveUnderline,
"Enable / Disable underlining active hotspots.", "");

DocStr(wxScintilla::GetHotspotActiveUnderline,
"Get whether underlining for active hotspots.", "");

DocStr(wxScintilla::SetHotspotSingleLine,
"Limit hotspots to single line so hotspots on two lines don't merge.", "");

DocStr(wxScintilla::GetHotspotSingleLine,
"Get the HotspotSingleLine property", "");

DocStr(wxScintilla::ParaDown,
"Move caret between paragraphs (delimited by empty lines).", "");

DocStr(wxScintilla::ParaDownExtend,
"", "");

DocStr(wxScintilla::ParaUp,
"", "");

DocStr(wxScintilla::ParaUpExtend,
"", "");

DocStr(wxScintilla::PositionBefore,
"Given a valid document position, return the previous position taking code
page into account. Returns 0 if passed 0.", "");

DocStr(wxScintilla::PositionAfter,
"Given a valid document position, return the next position taking code
page into account. Maximum value returned is the last position in the document.", "");

DocStr(wxScintilla::CopyRange,
"Copy a range of text to the clipboard. Positions are clipped into the document.", "");

DocStr(wxScintilla::CopyText,
"Copy argument text to the clipboard.", "");

DocStr(wxScintilla::SetSelectionMode,
"Set the selection mode to stream (SC_SEL_STREAM) or rectangular (SC_SEL_RECTANGLE) or
by lines (SC_SEL_LINES).", "");

DocStr(wxScintilla::GetSelectionMode,
"Get the mode of the current selection.", "");

DocStr(wxScintilla::GetLineSelStartPosition,
"Retrieve the position of the start of the selection at the given line (INVALID_POSITION if no selection on this line).", "");

DocStr(wxScintilla::GetLineSelEndPosition,
"Retrieve the position of the end of the selection at the given line (INVALID_POSITION if no selection on this line).", "");

DocStr(wxScintilla::LineDownRectExtend,
"Move caret down one line, extending rectangular selection to new caret position.", "");

DocStr(wxScintilla::LineUpRectExtend,
"Move caret up one line, extending rectangular selection to new caret position.", "");

DocStr(wxScintilla::CharLeftRectExtend,
"Move caret left one character, extending rectangular selection to new caret position.", "");

DocStr(wxScintilla::CharRightRectExtend,
"Move caret right one character, extending rectangular selection to new caret position.", "");

DocStr(wxScintilla::HomeRectExtend,
"Move caret to first position on line, extending rectangular selection to new caret position.", "");

DocStr(wxScintilla::VCHomeRectExtend,
"Move caret to before first visible character on line.
If already there move to first character on line.
In either case, extend rectangular selection to new caret position.", "");

DocStr(wxScintilla::LineEndRectExtend,
"Move caret to last position on line, extending rectangular selection to new caret position.", "");

DocStr(wxScintilla::PageUpRectExtend,
"Move caret one page up, extending rectangular selection to new caret position.", "");

DocStr(wxScintilla::PageDownRectExtend,
"Move caret one page down, extending rectangular selection to new caret position.", "");

DocStr(wxScintilla::StutteredPageUp,
"Move caret to top of page, or one page up if already at top of page.", "");

DocStr(wxScintilla::StutteredPageUpExtend,
"Move caret to top of page, or one page up if already at top of page, extending selection to new caret position.", "");

DocStr(wxScintilla::StutteredPageDown,
"Move caret to bottom of page, or one page down if already at bottom of page.", "");

DocStr(wxScintilla::StutteredPageDownExtend,
"Move caret to bottom of page, or one page down if already at bottom of page, extending selection to new caret position.", "");

DocStr(wxScintilla::WordLeftEnd,
"Move caret left one word, position cursor at end of word.", "");

DocStr(wxScintilla::WordLeftEndExtend,
"Move caret left one word, position cursor at end of word, extending selection to new caret position.", "");

DocStr(wxScintilla::WordRightEnd,
"Move caret right one word, position cursor at end of word.", "");

DocStr(wxScintilla::WordRightEndExtend,
"Move caret right one word, position cursor at end of word, extending selection to new caret position.", "");

DocStr(wxScintilla::SetWhitespaceChars,
"Set the set of characters making up whitespace for when moving or selecting by word.
Should be called after SetWordChars.", "");

DocStr(wxScintilla::SetCharsDefault,
"Reset the set of characters for whitespace and word characters to the defaults.", "");

DocStr(wxScintilla::AutoCompGetCurrent,
"Get currently selected item position in the auto-completion list", "");

DocStr(wxScintilla::Allocate,
"Enlarge the document to a particular size of text bytes.", "");

DocStr(wxScintilla::FindColumn,
"Find the position of a column on a line taking into account tabs and
multi-byte characters. If beyond end of line, return line end position.", "");

DocStr(wxScintilla::GetCaretSticky,
"Can the caret preferred x position only be changed by explicit movement commands?", "");

DocStr(wxScintilla::SetCaretSticky,
"Stop the caret preferred x position changing when the user types.", "");

DocStr(wxScintilla::ToggleCaretSticky,
"Switch between sticky and non-sticky: meant to be bound to a key.", "");

DocStr(wxScintilla::SetPasteConvertEndings,
"Enable/Disable convert-on-paste for line endings", "");

DocStr(wxScintilla::GetPasteConvertEndings,
"Get convert-on-paste setting", "");

DocStr(wxScintilla::SelectionDuplicate,
"Duplicate the selection. If selection empty duplicate the line containing the caret.", "");

DocStr(wxScintilla::SetCaretLineBackAlpha,
"Set background alpha of the caret line.", "");

DocStr(wxScintilla::GetCaretLineBackAlpha,
"Get the background alpha of the caret line.", "");

DocStr(wxScintilla::SetCaretStyle,
"Set the style of the caret to be drawn.", "");

DocStr(wxScintilla::GetCaretStyle,
"Returns the current style of the caret.", "");

DocStr(wxScintilla::SetIndicatorCurrent,
"Set the indicator used for IndicatorFillRange and IndicatorClearRange", "");

DocStr(wxScintilla::GetIndicatorCurrent,
"Get the current indicator", "");

DocStr(wxScintilla::SetIndicatorValue,
"Set the value used for IndicatorFillRange", "");

DocStr(wxScintilla::GetIndicatorValue,
"Get the current indicator vaue", "");

DocStr(wxScintilla::IndicatorFillRange,
"Turn a indicator on over a range.", "");

DocStr(wxScintilla::IndicatorClearRange,
"Turn a indicator off over a range.", "");

DocStr(wxScintilla::IndicatorAllOnFor,
"Are any indicators present at position?", "");

DocStr(wxScintilla::IndicatorValueAt,
"What value does a particular indicator have at at a position?", "");

DocStr(wxScintilla::IndicatorStart,
"Where does a particular indicator start?", "");

DocStr(wxScintilla::IndicatorEnd,
"Where does a particular indicator end?", "");

DocStr(wxScintilla::SetPositionCacheSize,
"Set number of entries in position cache", "");

DocStr(wxScintilla::GetPositionCacheSize,
"How many entries are allocated to the position cache?", "");

DocStr(wxScintilla::StartRecord,
"Start notifying the container of all key presses and commands.", "");

DocStr(wxScintilla::StopRecord,
"Stop notifying the container of all key presses and commands.", "");

DocStr(wxScintilla::SetLexer,
"Set the lexing language of the document.", "");

DocStr(wxScintilla::GetLexer,
"Retrieve the lexing language of the document.", "");

DocStr(wxScintilla::Colourise,
"Colourise a segment of the document using the current lexing language.", "");

DocStr(wxScintilla::SetProperty,
"Set up a value that may be used by a lexer for some optional feature.", "");

DocStr(wxScintilla::SetKeyWords,
"Set up the key words used by the lexer.", "");

DocStr(wxScintilla::SetLexerLanguage,
"Set the lexing language of the document based on string name.", "");

DocStr(wxScintilla::GetProperty,
"Retrieve a 'property' value previously set with SetProperty.", "");

DocStr(wxScintilla::GetPropertyExpanded,
"Retrieve a 'property' value previously set with SetProperty,
with '$()' variable replacement on returned buffer.", "");

DocStr(wxScintilla::GetPropertyInt,
"Retrieve a 'property' value previously set with SetProperty,
interpreted as an int AFTER any '$()' variable replacement.", "");

DocStr(wxScintilla::GetStyleBitsNeeded,
"Retrieve the number of bits the current lexer needs for styling.", "");
