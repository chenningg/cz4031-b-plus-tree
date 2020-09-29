# B+ Tree

B+ tree implementation for NTU's CZ4031 course of Database Systems Principles.

## Implementation details:

- Block size is 100B each.
- B+ tree's memory dynamically allocated on creation.
- Each B+ tree node is the size of one block.
- Leaf nodes are linked in a doubly linked list.
- Leaf nodes point to the database (data is stored in main memory)
- Database is statically allocated as a memory pool of 500MB.

## Setup

- Ensure that you have a C++ compiler (we suggest [mingw](https://sourceforge.net/projects/mingw-w64/) for Windows).
- Setup your environment and ensure all C++ files are included in compilation.

  For example, if we use the [code runner](https://marketplace.visualstudio.com/items?itemName=formulahendry.code-runner) extension for VSCode, we would add this in `settings.json`:

  ```
  {
    "code-runner.executorMap": {
      "cpp": "cd $dir && g++ *.cpp -o $fileNameWithoutExt && $dir$fileNameWithoutExt",
    }
  }
  ```

  If we were running C++ using VSCode directly, we would define the `tasks.json` file with corresponding `args` to include all C++ files:

  ```
  {
    "tasks": [
      {
        "args": ["-g", "${workspaceFolder}\\*.cpp", "-o", "${fileDirname}\\${fileBasenameNoExtension}.exe"],
      }
    ]
  }
  ```

- `cd` to `main.cpp` under the `src` folder and compile the executable.
