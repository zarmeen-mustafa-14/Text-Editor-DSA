#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include "Document.h"
#include "FileManager.h"
#include "Menu.h"
#include "TextAlgorithms.h"
#include "UndoStack.h"
#include "MyString.h"

// Coordinates the terminal interface, document, file operations, algorithms,
<<<<<<< HEAD
// and linked-list undo history. Text is stored using the project's MyString.
=======
// and linked-list undo history.
//
// Menu structure:
//   Main menu -> File | Edit | Find & Replace | View | Help | Exit Text is stored using the project's MyString.
>>>>>>> 3ebdde3 (Fixed every function that was causing any kind of issue)
class TextEditor
{
private:
    Document document;
    UndoStack undoStack;
    Menu menu;

    MyString currentFilename;
<<<<<<< HEAD
=======
    bool modified;      // true when there are unsaved changes
>>>>>>> 3ebdde3 (Fixed every function that was causing any kind of issue)
    bool running;

    // ---- menu navigation ----
    void handleMainChoice(int choice);
    void fileMenu();
    void editMenu();
    void findMenu();
    void viewMenu();

    // Called when the input stream ends (Ctrl+D / Ctrl+Z / closed pipe).
    void handleInputEnded();

    // Asks before throwing away unsaved changes. True = safe to continue.
    bool confirmDiscardChanges();

public:
    TextEditor();

    void run();

    void newDocument();
    void openDocument();
    void saveDocument();
    void saveDocumentAs();
    void deleteFile();
    void closeDocument();
    void exitEditor();

    void insertText();
    void deleteText();
    void insertLine();
    void deleteLine();

    void searchText();
    void replaceText();
    void replaceAllText();

    void undo();

    void showDocument();    // document text followed by statistics
};

#endif // TEXTEDITOR_H
