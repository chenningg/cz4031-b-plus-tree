# B+ Tree

B+ tree implementation for NTU's CZ4031 course of Database Systems Principles.

## Implementation details:

- Block size is 100B each.
- B+ tree's memory dynamically allocated on creation.
- Each B+ tree node is the size of one block.
- Leaf nodes are linked in a doubly linked list.
- Leaf nodes point to the datastore (data is stored in main memory)
- Datastore is statically allocated as a memory pool of 500MB.

## Setup

- Ensure that you have a C++ compiler (we suggest [mingw](https://sourceforge.net/projects/mingw-w64/))
- Setup your environment and ensure all C++ files are included in compilation:

```
{
  "code-runner.executorMap": {
    "cpp": "cd $dir && g++ *.cpp -o $fileNameWithoutExt && $dir$fileNameWithoutExt",
  }
}
```

- `cd` to main.cpp under `src` and run.
