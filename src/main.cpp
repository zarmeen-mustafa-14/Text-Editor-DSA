#include <iostream>
#include <exception>

#include "TextEditor.h"

int main()
{
    try
    {
        TextEditor editor;
        editor.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "\nFatal error: " << e.what() << '\n';
        return 1;
    }
    catch (...)
    {
        std::cerr << "\nFatal error: An unknown error occurred.\n";
        return 1;
    }

    return 0;
}