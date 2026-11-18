#include "TextEditor.h"

#include <exception>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

TextEditor::TextEditor()
    : document(),
      undoStack(),
      menu(),
      currentFilename(),
      running(true)
{
}

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

void TextEditor::handleChoice(int choice)
{
    switch (choice)
    {
    case 1: newDocument(); break;
    case 2: openDocument(); break;
    case 3: saveDocument(); break;
    case 4: saveDocumentAs(); break;
    case 5: insertText(); break;
    case 6: deleteText(); break;
    case 7: insertLine(); break;
    case 8: deleteLine(); break;
    case 9: searchText(); break;
    case 10: replaceText(); break;
    case 11: replaceAllText(); break;
    case 12: undo(); break;
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
            document.getLineCount(), document.getCharacterCount()
        );
        break;
    }
    case 14: exitEditor(); break;
    default: menu.displayError("Invalid menu choice."); break;
    }
}

void TextEditor::newDocument()
{
    document.clear();
    undoStack.clear();
    currentFilename.clear();
    menu.displayMessage("New document created.");
}

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

    try
    {
        if (!FileManager::openFile(filename, document))
        {
            menu.displayError("Could not open file. Current document preserved.");
            return;
        }

        // Only commit editor metadata after the file has loaded successfully.
        currentFilename.swap(filename);
        undoStack.clear();
        menu.displayMessage("File opened successfully.");
    }
    catch (const std::exception& error)
    {
        menu.displayError(std::string("Could not open file: ") + error.what());
    }
}

void TextEditor::saveDocument()
{
    if (currentFilename.empty())
    {
        saveDocumentAs();
        return;
    }

    try
    {
        if (!FileManager::saveFile(currentFilename, document))
        {
            menu.displayError("Could not save document.");
            return;
        }
        menu.displayMessage("Document saved successfully.");
    }
    catch (const std::exception& error)
    {
        menu.displayError(std::string("Could not save document: ") + error.what());
    }
}

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

    try
    {
        if (!FileManager::saveFile(filename, document))
        {
            menu.displayError("Could not save document.");
            return;
        }
        currentFilename.swap(filename);
        menu.displayMessage("Document saved successfully.");
    }
    catch (const std::exception& error)
    {
        menu.displayError(std::string("Could not save document: ") + error.what());
    }
}

void TextEditor::exitEditor()
{
    running = false;
    menu.displayMessage("Exiting Text Editor.");
}

void TextEditor::insertText()
{
    int lineNumber = 0;
    int position = 0;
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

    bool pushed = false;
    try
    {
        // Record first: if allocation fails, the document remains unchanged.
        undoStack.push(record);
        pushed = true;

        if (!document.insertText(lineNumber, position, text))
        {
            undoStack.discardTop();
            menu.displayError("Invalid line or character position.");
            return;
        }
    }
    catch (const std::exception& error)
    {
        if (pushed) undoStack.discardTop();
        menu.displayError(std::string("Insert failed: ") + error.what());
        return;
    }

    menu.displayMessage("Text inserted successfully.");
}

void TextEditor::deleteText()
{
    int lineNumber = 0;
    int position = 0;
    int length = 0;

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
    if (!document.getLine(lineNumber, line) || position < 0 ||
        position >= static_cast<int>(line.size()))
    {
        menu.displayError("Invalid line or character position.");
        return;
    }

    const int actualLength = (length > static_cast<int>(line.size()) - position)
        ? static_cast<int>(line.size()) - position : length;

    UndoRecord record;
    record.type = OperationType::DELETE_TEXT;
    record.lineNumber = lineNumber;
    record.position = position;
    record.oldText = line.substr(static_cast<std::size_t>(position),
                                 static_cast<std::size_t>(actualLength));

    bool pushed = false;
    try
    {
        undoStack.push(record);
        pushed = true;

        std::string deletedText;
        if (!document.deleteText(lineNumber, position, actualLength, deletedText))
        {
            undoStack.discardTop();
            menu.displayError("Text deletion failed.");
            return;
        }
    }
    catch (const std::exception& error)
    {
        if (pushed) undoStack.discardTop();
        menu.displayError(std::string("Delete failed: ") + error.what());
        return;
    }

    menu.displayMessage("Text deleted successfully.");
}

void TextEditor::insertLine()
{
    int lineNumber = 0;
    std::string text;
    if (!menu.getInteger("Enter line number for insertion: ", lineNumber) ||
        !menu.getString("Enter line text: ", text))
    {
        menu.displayError("Input cancelled.");
        return;
    }

    UndoRecord record;
    record.type = OperationType::INSERT_LINE;
    record.lineNumber = lineNumber;

    bool pushed = false;
    try
    {
        undoStack.push(record);
        pushed = true;
        if (!document.insertLine(lineNumber, text))
        {
            undoStack.discardTop();
            menu.displayError("Invalid line number. Valid range is 1 through line count + 1.");
            return;
        }
    }
    catch (const std::exception& error)
    {
        if (pushed) undoStack.discardTop();
        menu.displayError(std::string("Line insertion failed: ") + error.what());
        return;
    }

    menu.displayMessage("Line inserted successfully.");
}

void TextEditor::deleteLine()
{
    int lineNumber = 0;
    if (!menu.getInteger("Enter line number to delete: ", lineNumber))
    {
        menu.displayError("Input cancelled.");
        return;
    }

    std::string oldLine;
    if (!document.getLine(lineNumber, oldLine))
    {
        menu.displayError("Invalid line number.");
        return;
    }

    UndoRecord record;
    record.type = OperationType::DELETE_LINE;
    record.lineNumber = lineNumber;
    record.oldText = oldLine;

    bool pushed = false;
    try
    {
        undoStack.push(record);
        pushed = true;
        if (!document.deleteLine(lineNumber))
        {
            undoStack.discardTop();
            menu.displayError("Line deletion failed.");
            return;
        }
    }
    catch (const std::exception& error)
    {
        if (pushed) undoStack.discardTop();
        menu.displayError(std::string("Line deletion failed: ") + error.what());
        return;
    }

    menu.displayMessage("Line deleted successfully.");
}

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
        menu.displayMessage("Search failed or no searchable document exists.");
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
        MatchPosition match{};
        if (matches.get(i, match))
        {
            output << "Line " << match.lineNumber
                   << ", Position " << match.characterPosition << '\n';
        }
    }
    menu.displayMessage(output.str());
}

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

    // Locate and prepare the complete undo record before changing the document.
    MatchArray matches;
    if (!TextAlgorithms::findAll(document, pattern, matches) || matches.getSize() == 0)
    {
        menu.displayMessage("No replacement made.");
        return;
    }

    MatchPosition first{};
    if (!matches.get(0, first))
    {
        menu.displayError("Could not prepare replacement.");
        return;
    }

    UndoRecord record;
    record.type = OperationType::REPLACE_TEXT;
    record.lineNumber = first.lineNumber;
    record.position = first.characterPosition;
    record.oldText = pattern;
    record.newText = replacement;

    bool pushed = false;
    try
    {
        undoStack.push(record);
        pushed = true;

        MatchPosition changedPosition{};
        // Allocate/copy the original text before the document is mutated.
        std::string originalText = pattern;
        if (!TextAlgorithms::replaceFirst(
                document, pattern, replacement, changedPosition, originalText))
        {
            undoStack.discardTop();
            menu.displayMessage("No replacement made.");
            return;
        }
    }
    catch (const std::exception& error)
    {
        if (pushed) undoStack.discardTop();
        menu.displayError(std::string("Replacement failed: ") + error.what());
        return;
    }

    menu.displayMessage("First occurrence replaced.");
}

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
            document, pattern, replacement, changes, replacementCount))
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
    catch (const std::exception& error)
    {
        // Best-effort rollback in reverse order. Positions stored in changes
        // refer to the original text; prior replacements shift later positions.
        bool rollbackSucceeded = true;
        try
        {
            for (int i = changes.getSize() - 1; i >= 0; --i)
            {
                TextChange change{};
                if (!changes.get(i, change))
                {
                    rollbackSucceeded = false;
                    break;
                }

                long long position = change.position;
                for (int j = 0; j < i; ++j)
                {
                    TextChange earlier{};
                    if (!changes.get(j, earlier))
                    {
                        rollbackSucceeded = false;
                        break;
                    }
                    if (earlier.lineNumber == change.lineNumber)
                    {
                        position += static_cast<long long>(earlier.newText.size()) -
                                    static_cast<long long>(earlier.oldText.size());
                    }
                }
                if (!rollbackSucceeded || position < 0 ||
                    position > static_cast<long long>(std::numeric_limits<int>::max()))
                {
                    rollbackSucceeded = false;
                    break;
                }

                std::string line;
                if (!document.getLine(change.lineNumber, line) ||
                    position + static_cast<long long>(change.newText.size()) >
                        static_cast<long long>(line.size()))
                {
                    rollbackSucceeded = false;
                    break;
                }

                std::string restored;
                restored.append(line, 0, static_cast<std::size_t>(position));
                restored.append(change.oldText);
                restored.append(line,
                    static_cast<std::size_t>(position + change.newText.size()),
                    std::string::npos);
                if (!document.updateLine(change.lineNumber, restored))
                {
                    rollbackSucceeded = false;
                    break;
                }
            }
        }
        catch (...)
        {
            rollbackSucceeded = false;
        }

        menu.displayError(
            rollbackSucceeded
                ? std::string("Replacement was rolled back because undo recording failed: ") + error.what()
                : std::string("Critical error: undo recording failed and rollback was incomplete.")
        );
        return;
    }

    menu.displayMessage("Total replacements: " + std::to_string(replacementCount));
}

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
    try
    {
        switch (record.type)
        {
        case OperationType::INSERT_TEXT:
        {
            std::string removed;
            success = document.deleteText(
                record.lineNumber, record.position,
                static_cast<int>(record.newText.size()), removed);
            break;
        }
        case OperationType::DELETE_TEXT:
            success = document.insertText(
                record.lineNumber, record.position, record.oldText);
            break;
        case OperationType::REPLACE_TEXT:
        {
            std::string line;
            if (document.getLine(record.lineNumber, line) && record.position >= 0 &&
                static_cast<std::size_t>(record.position) <= line.size() &&
                record.newText.size() <= line.size() - static_cast<std::size_t>(record.position))
            {
                std::string restored;
                restored.append(line, 0, static_cast<std::size_t>(record.position));
                restored.append(record.oldText);
                restored.append(line,
                    static_cast<std::size_t>(record.position) + record.newText.size(),
                    std::string::npos);
                success = document.updateLine(record.lineNumber, restored);
            }
            break;
        }
        case OperationType::REPLACE_ALL:
        {
            success = true;
            // Reverse order is essential. The stored positions are in the
            // original text, so account for all earlier replacements' deltas.
            for (int i = record.changes.getSize() - 1; i >= 0; --i)
            {
                TextChange change{};
                if (!record.changes.get(i, change))
                {
                    success = false;
                    break;
                }

                long long position = change.position;
                for (int j = 0; j < i; ++j)
                {
                    TextChange earlier{};
                    if (!record.changes.get(j, earlier))
                    {
                        success = false;
                        break;
                    }
                    if (earlier.lineNumber == change.lineNumber)
                    {
                        position += static_cast<long long>(earlier.newText.size()) -
                                    static_cast<long long>(earlier.oldText.size());
                    }
                }
                if (!success || position < 0 ||
                    position > static_cast<long long>(std::numeric_limits<int>::max()))
                {
                    success = false;
                    break;
                }

                std::string line;
                if (!document.getLine(change.lineNumber, line) ||
                    position + static_cast<long long>(change.newText.size()) >
                        static_cast<long long>(line.size()))
                {
                    success = false;
                    break;
                }

                std::string restored;
                restored.append(line, 0, static_cast<std::size_t>(position));
                restored.append(change.oldText);
                restored.append(line,
                    static_cast<std::size_t>(position + change.newText.size()),
                    std::string::npos);
                if (!document.updateLine(change.lineNumber, restored))
                {
                    success = false;
                    break;
                }
            }
            break;
        }
        case OperationType::INSERT_LINE:
            success = document.deleteLine(record.lineNumber);
            break;
        case OperationType::DELETE_LINE:
            success = document.insertLine(record.lineNumber, record.oldText);
            break;
        }
    }
    catch (const std::exception& error)
    {
        menu.displayError(std::string("Undo failed: ") + error.what());
        return;
    }

    if (!success)
    {
        menu.displayError("Undo failed. The undo record was retained.");
        return;
    }

    undoStack.discardTop();
    menu.displayMessage("Undo completed successfully.");
}
