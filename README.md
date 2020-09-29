# B+ Tree

This is a B+ tree implementation using C++ for NTU's CZ4031 course of Database Systems Principles. The data given is a list of movies, with each record containing the movie ID, its average rating and the number of votes.

<center>
|  tconst   | averageRating | numVotes |
| :-------: | :-----------: | :------: |
| tt0000001 |      5.6      |   1645   |
| tt0000002 |      6.1      |   198    |
| tt0000003 |      6.5      |   1342   |
</center>

There are more than 9 million records in the data. Our goal is to build a B+ tree index on the average rating field, and query the data based on the index for efficient retrival.

## Implementation details:

- Block size is 100B each.
- Each record (movie) has a fixed size of 40B (TBC).
- Multiple records can be stored per block.
- B+ tree's memory is dynamically allocated on creation.
- Each B+ tree node is the size of one block.
- Leaf nodes are linked in a doubly linked list.
- Leaf nodes maintain pointers to the actual data (data is stored in main memory).
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
