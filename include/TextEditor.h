#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include "Document.h"
#include "FileManager.h"
#include "Menu.h"
#include "TextAlgorithms.h"
#include "UndoStack.h"
#include "MyString.h"

// Coordinates the terminal interface, document, file operations, algorithms,
// and linked-list undo history. Text is stored using the project's MyString.
class TextEditor
{
private:
    Document document;
    UndoStack undoStack;
    Menu menu;

    MyString currentFilename;
    bool running;

    void handleChoice(int choice);

public:
    TextEditor();

    void run();

    void newDocument();
    void openDocument();
    void saveDocument();
    void saveDocumentAs();
    void exitEditor();

    void insertText();
    void deleteText();
    void insertLine();
    void deleteLine();

    void searchText();
    void replaceText();
    void replaceAllText();

    void undo();
};

#endif // TEXTEDITOR_H
