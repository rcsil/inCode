# inCode IDE

**inCode** is a lightweight IDE focused on PHP development, built in C++ with Qt6, featuring a modern user interface inspired by Visual Studio Code.

## Key Features

*   **Basic IDE Structure:** Main window with a file tree, tabbed code editor, and an integrated terminal.
*   **File Management:** Create new files, open existing files, open project folders, and save files.
*   **Integrated Terminal:** Basic command-line interface within the IDE.
*   **Code Editor:**
    *   Line numbering.
    *   Basic PHP syntax highlighting.
    *   "Go to Definition" functionality (Ctrl+Click) powered by a simple symbol indexer.
*   **Code Analysis:** Detects code repetitions in `app` and `resources` folders, ignoring `use`, `class`, and `namespace` declarations.
*   **Background Indexing:** Project indexing runs in the background with a progress bar, keeping the UI responsive.

## Tech Stack

*   **IDE Framework:** C++ with Qt6
*   **Build System:** CMake
*   **Code Analysis:** Custom implementation for symbol indexing and repetition detection.

## How to Build

1.  **Prerequisites:**
    *   A C++ compiler (g++, clang, etc.)
    *   CMake (version 3.16+)
    *   Qt6 development libraries

2.  **Clone the repository:**
    ```bash
    git clone <repository-url> # Replace <repository-url> with the actual URL
    cd inCode
    ```

3.  **Configure and build:**
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```

4.  **Run the application:**
    ```bash
    ./inCode
    ```