#include "TextEditor.h"

#include <iostream>
#include <sstream>
#include <string>
#include <exception>

// ============================================================
// Constructor
// ============================================================

TextEditor::TextEditor()
    : document(),
      undoStack(),
      menu(),
      currentFilename(),
      running(true)
{
}

// ============================================================
// Main application loop
// ============================================================

void TextEditor::run()
{
    running = true;

    while (running)
    {
        menu.displayMainMenu();

        int choice = 0;

        if (!menu.getChoice(choice))
        {
            menu.displayMessage("Input ended. Exiting editor.");
            running = false;
            break;
        }

        handleChoice(choice);
    }
}

// ============================================================
// Menu choice handling
// ============================================================

void TextEditor::handleChoice(int choice)
{
    switch (choice)
    {
    case 1:
        newDocument();
        break;

    case 2:
        openDocument();
        break;

    case 3:
        saveDocument();
        break;

    case 4:
        saveDocumentAs();
        break;

    case 5:
        insertText();
        break;

    case 6:
        deleteText();
        break;

    case 7:
        insertLine();
        break;

    case 8:
        deleteLine();
        break;

    case 9:
        searchText();
        break;

    case 10:
        replaceText();
        break;

    case 11:
        replaceAllText();
        break;

    case 12:
        undo();
        break;

    case 13:
    {
        std::ostringstream output;

        if (document.isEmpty())
        {
            output << "Document is empty.\n";
        }
        else
        {
            for (int i = 1; i <= document.getLineCount(); ++i)
            {
                std::string line;

                if (document.getLine(i, line))
                {
                    output << i << ". " << line << '\n';
                }
            }
        }

        menu.displayDocument(output.str());

        menu.displayDocumentStatistics(
            document.getLineCount(),
            document.getCharacterCount()
        );

        break;
    }

    case 14:
        exitEditor();
        break;

    default:
        menu.displayError("Invalid menu choice.");
        break;
    }
}

// ============================================================
// New document
// ============================================================

void TextEditor::newDocument()
{
    document.clear();
    undoStack.clear();
    currentFilename.clear();

    menu.displayMessage("New document created.");
}

// ============================================================
// Open document
// ============================================================

void TextEditor::openDocument()
{
    std::string filename;

    if (!menu.getString("Enter filename to open: ", filename))
    {
        menu.displayError("Input cancelled.");
        return;
    }

    if (filename.empty())
    {
        menu.displayError("Filename cannot be empty.");
        return;
    }

    if (!FileManager::openFile(filename, document))
    {
        menu.displayError(
            "Could not open file. Current document preserved."
        );
        return;
    }

    currentFilename = filename;
    undoStack.clear();

    menu.displayMessage("File opened successfully.");
}

// ============================================================
// Save document
// ============================================================

void TextEditor::saveDocument()
{
    if (currentFilename.empty())
    {
        saveDocumentAs();
        return;
    }

    if (!FileManager::saveFile(currentFilename, document))
    {
        menu.displayError("Could not save document.");
        return;
    }

    menu.displayMessage("Document saved successfully.");
}

// ============================================================
// Save As
// ============================================================

void TextEditor::saveDocumentAs()
{
    std::string filename;

    if (!menu.getString("Enter filename to save as: ", filename))
    {
        menu.displayError("Input cancelled.");
        return;
    }

    if (filename.empty())
    {
        menu.displayError("Filename cannot be empty.");
        return;
    }

    if (!FileManager::saveFile(filename, document))
    {
        menu.displayError("Could not save document.");
        return;
    }

    currentFilename = filename;

    menu.displayMessage("Document saved successfully.");
}

// ============================================================
// Exit editor
// ============================================================

void TextEditor::exitEditor()
{
    running = false;

    menu.displayMessage("Exiting Text Editor.");
}

// ============================================================
// Insert text
// ============================================================


void TextEditor::insertText()
{
    int lineNumber;
    int position;
    std::string text;

    if (!menu.getInteger("Enter line number: ", lineNumber) ||
        !menu.getInteger("Enter character position: ", position) ||
        !menu.getString("Enter text to insert: ", text))
    {
        menu.displayError("Input cancelled.");
        return;
    }

    if (text.empty())
    {
        menu.displayError("Nothing to insert.");
        return;
    }

    UndoRecord record;
    record.type = OperationType::INSERT_TEXT;
    record.lineNumber = lineNumber;
    record.position = position;
    record.newText = text;

    bool recordPushed = false;

    try
    {
        undoStack.push(record);
        recordPushed = true;

        if (!document.insertText(lineNumber, position, text))
        {
            undoStack.discardTop();
            recordPushed = false;

            menu.displayError("Invalid line or character position.");
            return;
        }
    }
    catch (const std::exception& e)
    {
        if (recordPushed)
        {
            undoStack.discardTop();
        }

        menu.displayError(
            std::string("Insert failed: ") + e.what()
        );
        return;
    }

    menu.displayMessage("Text inserted successfully.");
}


void TextEditor::deleteText()
{
    int lineNumber;
    int position;
    int length;

    if (!menu.getInteger("Enter line number: ", lineNumber) ||
        !menu.getInteger("Enter starting position: ", position) ||
        !menu.getInteger("Enter length to delete: ", length))
    {
        menu.displayError("Input cancelled.");
        return;
    }

    if (length <= 0)
    {
        menu.displayError("Length must be positive.");
        return;
    }

    std::string line;

    if (!document.getLine(lineNumber, line) ||
        position < 0 ||
        position >= static_cast<int>(line.length()))
    {
        menu.displayError("Invalid line or character position.");
        return;
    }

    int actualLength = length;

    if (actualLength > static_cast<int>(line.length()) - position)
    {
        actualLength = static_cast<int>(line.length()) - position;
    }

    std::string deletedText = line.substr(position, actualLength);

    UndoRecord record;
    record.type = OperationType::DELETE_TEXT;
    record.lineNumber = lineNumber;
    record.position = position;
    record.oldText = deletedText;

    bool recordPushed = false;

    try
    {
        undoStack.push(record);
        recordPushed = true;

        if (!document.deleteText(
                lineNumber,
                position,
                length,
                deletedText))
        {
            undoStack.discardTop();
            recordPushed = false;

            menu.displayError("Text deletion failed.");
            return;
        }
    }
    catch (const std::exception& e)
    {
        if (recordPushed)
        {
            undoStack.discardTop();
        }

        menu.displayError(
            std::string("Delete failed: ") + e.what()
        );
        return;
    }

    menu.displayMessage("Text deleted successfully.");
}

// ============================================================
// Search text
// ============================================================

void TextEditor::searchText()
{
    std::string pattern;

    if (!menu.getString("Enter text to search: ", pattern))
    {
        menu.displayError("Input cancelled.");
        return;
    }

    if (pattern.empty())
    {
        menu.displayError("Search pattern cannot be empty.");
        return;
    }

    MatchArray matches;

    if (!TextAlgorithms::findAll(document, pattern, matches))
    {
        menu.displayError("Search failed.");
        return;
    }

    if (matches.getSize() == 0)
    {
        menu.displayMessage("No matches found.");
        return;
    }

    std::ostringstream output;

    output << "Matches found: " << matches.getSize() << '\n';

    for (int i = 0; i < matches.getSize(); ++i)
    {
        MatchPosition match;

        if (matches.get(i, match))
        {
            output << "Line " << match.lineNumber
                  << ", Position " << match.characterPosition
                  << '\n';
        }
    }

    menu.displayMessage(output.str());
}

// ============================================================
// Replace first occurrence
// ============================================================

void TextEditor::replaceText()
{
    std::string pattern;
    std::string replacement;

    if (!menu.getString("Enter text to find: ", pattern) ||
        !menu.getString("Enter replacement text: ", replacement))
    {
        menu.displayError("Input cancelled.");
        return;
    }

    if (pattern.empty())
    {
        menu.displayError("Search pattern cannot be empty.");
        return;
    }

    MatchPosition changedPosition;
    std::string originalText;

    // Prepare undo record before performing the replacement.
    UndoRecord record;
    record.type = OperationType::REPLACE_TEXT;
    record.oldText = pattern;
    record.newText = replacement;

    try
    {
        if (!TextAlgorithms::replaceFirst(
                document,
                pattern,
                replacement,
                changedPosition,
                originalText))
        {
            menu.displayMessage("No replacement made.");
            return;
        }

        record.lineNumber = changedPosition.lineNumber;
        record.position = changedPosition.characterPosition;

        undoStack.push(record);
    }
    catch (const std::exception& e)
    {
        menu.displayError(
            std::string("Replacement failed: ") + e.what()
        );
        return;
    }

    menu.displayMessage("First occurrence replaced.");
}

// ============================================================
// Replace all occurrences
// ============================================================

void TextEditor::replaceAllText()
{
    std::string pattern;
    std::string replacement;

    if (!menu.getString("Enter text to find: ", pattern) ||
        !menu.getString("Enter replacement text: ", replacement))
    {
        menu.displayError("Input cancelled.");
        return;
    }

    if (pattern.empty())
    {
        menu.displayError("Search pattern cannot be empty.");
        return;
    }

    TextChangeArray changes;
    int replacementCount = 0;

    if (!TextAlgorithms::replaceAll(
            document,
            pattern,
            replacement,
            changes,
            replacementCount))
    {
        menu.displayMessage("No replacements made.");
        return;
    }

    UndoRecord record;
    record.type = OperationType::REPLACE_ALL;
    record.changes = changes;

    try
    {
        undoStack.push(record);
    }
    catch (const std::exception& e)
    {
        // Replacement already happened. We cannot silently claim undo exists.
        menu.displayError(
            std::string("Replacement succeeded, but undo recording failed: ")
            + e.what()
        );
        return;
    }

    menu.displayMessage(
        "Total replacements: " + std::to_string(replacementCount)
    );
}

// ============================================================
// Undo
// ============================================================

void TextEditor::undo()
{
    if (undoStack.isEmpty())
    {
        menu.displayMessage("Nothing to undo.");
        return;
    }

    UndoRecord record;

    if (!undoStack.peek(record))
    {
        menu.displayError("Could not read undo record.");
        return;
    }

    bool success = false;

    switch (record.type)
    {
    // --------------------------------------------------------
    // Undo inserted text
    // --------------------------------------------------------
    case OperationType::INSERT_TEXT:
    {
        std::string deleted;

        success = document.deleteText(
            record.lineNumber,
            record.position,
            static_cast<int>(record.newText.length()),
            deleted
        );

        break;
    }

    // --------------------------------------------------------
    // Undo deleted text
    // --------------------------------------------------------
    case OperationType::DELETE_TEXT:
    {
        success = document.insertText(
            record.lineNumber,
            record.position,
            record.oldText
        );

        break;
    }

    // --------------------------------------------------------
    // Undo first replacement
    // --------------------------------------------------------
    case OperationType::REPLACE_TEXT:
    {
        std::string line;

        if (document.getLine(record.lineNumber, line))
        {
            int position = record.position;
            int replacementLength =
                static_cast<int>(record.newText.length());

            if (position >= 0 &&
                position + replacementLength <=
                    static_cast<int>(line.length()))
            {
                std::string restored;

                restored.append(line, 0, position);
                restored.append(record.oldText);
                restored.append(
                    line,
                    position + replacementLength,
                    std::string::npos
                );

                success = document.updateLine(
                    record.lineNumber,
                    restored
                );
            }
        }

        break;
    }
    
    // --------------------------------------------------------
    // Undo Replace All
    // --------------------------------------------------------
    case OperationType::REPLACE_ALL:
    {
        success = true;

        for (int i = record.changes.getSize() - 1; i >= 0; --i)
        {
            TextChange change;

            if (!record.changes.get(i, change))
            {
                success = false;
                break;
            }

            long long adjustedPosition = change.position;

            // Calculate position shifts caused by earlier replacements
            for (int j = 0; j < i; ++j)
            {
                TextChange earlierChange;

                if (!record.changes.get(j, earlierChange))
                {
                    success = false;
                    break;
                }

                if (earlierChange.lineNumber == change.lineNumber)
                {
                    adjustedPosition +=
                        static_cast<long long>(earlierChange.newText.length()) -
                        static_cast<long long>(earlierChange.oldText.length());
                }
            }

            if (!success)
            {
                break;
            }

            std::string line;

            if (!document.getLine(change.lineNumber, line))
            {
                success = false;
                break;
            }

            long long replacementLength =
                static_cast<long long>(change.newText.length());

            if (adjustedPosition < 0 ||
                adjustedPosition + replacementLength >
                    static_cast<long long>(line.length()))
            {
                success = false;
                break;
            }

            std::string restored;

            restored.append(
                line, 0, static_cast<std::size_t>(adjustedPosition)
            );

            restored.append(change.oldText);

            restored.append(
                line,
                static_cast<std::size_t>(adjustedPosition + replacementLength),
                std::string::npos
            );

            if (!document.updateLine(change.lineNumber, restored))
            {
                success = false;
                break;
            }
        }

        break;
    }

    // --------------------------------------------------------
    // Undo inserted line
    // --------------------------------------------------------
    case OperationType::INSERT_LINE:
    {
        success = document.deleteLine(record.lineNumber);
        break;
    }

    // --------------------------------------------------------
    // Undo deleted line
    // --------------------------------------------------------
    case OperationType::DELETE_LINE:
    {
        success = document.insertLine(
            record.lineNumber,
            record.oldText
        );

        break;
    }
    }

    if (!success)
    {
        menu.displayError(
            "Undo failed. The undo record was retained."
        );
        return;
    }

    undoStack.discardTop();

    menu.displayMessage("Undo completed successfully.");
}