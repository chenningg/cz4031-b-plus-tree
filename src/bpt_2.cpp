// B+ tree in C++

#include <iostream>

using namespace std;
int MAX_KEYS = 5;

// BP node
class Node {
  private: 
    bool IS_LEAF;
    int *keys, num_keys;
    Node **pointers;
    friend class BPTree;

  public:
    Node();
};

// BP tree
class BPTree {
  private:
    Node *root;
    void insertInternal(int, Node *, Node *);
    Node *findParent(Node *, Node *);

  public:
    BPTree();
    void search(int);
    void insert(int);
    void display(Node *, int level);
    Node *getRoot();
};

Node::Node() {
  keys = new int[MAX_KEYS];
  pointers = new Node *[MAX_KEYS + 1];
  for (int i = 0; i < MAX_KEYS; i++) {
      pointers[i] = NULL;
  }
}

BPTree::BPTree() {
  root = NULL;
}

// Search operation
void BPTree::search(int x) {
  if (root == NULL) {
    cout << "Tree is empty\n";
  } else {
    Node *cursor = root;
    while (cursor->IS_LEAF == false) {
      for (int i = 0; i < cursor->num_keys; i++) {
        if (x < cursor->keys[i]) { 
          cursor = cursor->pointers[i];
          break;
        }
        if (i == cursor->num_keys - 1) {
          cursor = cursor->pointers[i + 1];
          break;
        }
      }
    }
    for (int i = 0; i < cursor->num_keys; i++) {
      if (cursor->keys[i] == x) {
        cout << "Found\n";
        return;
      }
    }
    cout << "Not found\n";
  }
}

// Insert Operation
void BPTree::insert(int x) {
  if (root == NULL) {
    root = new Node;
    root->keys[0] = x;
    root->IS_LEAF = true;
    root->num_keys = 1;
  } else {
    Node *cursor = root;
    Node *parent;
    while (cursor->IS_LEAF == false) {
      parent = cursor;
      for (int i = 0; i < cursor->num_keys; i++) {
        if (x < cursor->keys[i]) { 
          cursor = cursor->pointers[i];
          break;
        }
        if (i == cursor->num_keys - 1) {
          cursor = cursor->pointers[i + 1];
          break;
        }
      }
    }
    if (cursor->num_keys < MAX_KEYS) {
      int i = 0;
      while (x > cursor->keys[i] && i < cursor->num_keys)
        i++;
      for (int j = cursor->num_keys; j > i; j--) {
        cursor->keys[j] = cursor->keys[j - 1];
      }
      cursor->keys[i] = x;
      cursor->num_keys++;
      cursor->pointers[cursor->num_keys] = cursor->pointers[cursor->num_keys - 1];
      cursor->pointers[cursor->num_keys - 1] = NULL;
    } else {
      Node *newLeaf = new Node;
      int tempKeyList[MAX_KEYS + 1];
      for (int i = 0; i < MAX_KEYS; i++) {
        tempKeyList[i] = cursor->keys[i];
      }
      int i = 0, j;
      while (x > tempKeyList[i] && i < MAX_KEYS)
        i++;
      for (int j = MAX_KEYS + 1; j > i; j--) {
        tempKeyList[j] = tempKeyList[j - 1];
      }
      tempKeyList[i] = x;
      newLeaf->IS_LEAF = true;
      cursor->num_keys = (MAX_KEYS + 1) / 2;
      newLeaf->num_keys = MAX_KEYS + 1 - (MAX_KEYS + 1) / 2;
      cursor->pointers[cursor->num_keys] = newLeaf;
      newLeaf->pointers[newLeaf->num_keys] = cursor->pointers[MAX_KEYS];
      for (i = cursor->num_keys; i < MAX_KEYS + 1; i++) {
        cursor->pointers[i] = NULL;
      }
      for (i = 0; i < cursor->num_keys; i++) {
        cursor->keys[i] = tempKeyList[i];
      }
      for (i = 0, j = cursor->num_keys; i < newLeaf->num_keys; i++, j++) {
        newLeaf->keys[i] = tempKeyList[j];
      }

      cursor->pointers[cursor->num_keys+1] = newLeaf;
      if (cursor == root) {                
        Node *newRoot = new Node;
        newRoot->keys[0] = newLeaf->keys[0];
        newRoot->pointers[0] = cursor;
        newRoot->pointers[1] = newLeaf;
        newRoot->IS_LEAF = false;
        newRoot->num_keys = 1;
        root = newRoot;        
      } else {
        insertInternal(newLeaf->keys[0], parent, newLeaf);
      }
    }
  }
}

// Insert Operation
void BPTree::insertInternal(int x, Node *cursor, Node *child) {
  if (cursor->num_keys < MAX_KEYS) {  
    int i = 0;
    while (x > cursor->keys[i] && i < cursor->num_keys)
      i++;
    for (int j = cursor->num_keys; j > i; j--) {
      cursor->keys[j] = cursor->keys[j - 1];
    }
    for (int j = cursor->num_keys + 1; j > i + 1; j--) {
      cursor->pointers[j] = cursor->pointers[j - 1];
    }
    cursor->keys[i] = x;
    cursor->num_keys++;
    cursor->pointers[i + 1] = child;      
  } else {
    Node *newInternal = new Node;
    int virtualKey[MAX_KEYS + 1];
    Node *virtualPtr[MAX_KEYS + 2];
    for (int i = 0; i < MAX_KEYS; i++) {
      virtualKey[i] = cursor->keys[i];
    }
    for (int i = 0; i < MAX_KEYS + 1; i++) {
      virtualPtr[i] = cursor->pointers[i];
    }
    int i = 0, j;
    while (x > virtualKey[i] && i < MAX_KEYS)
      i++;
    for (int j = MAX_KEYS + 1; j > i; j--) {
      virtualKey[j] = virtualKey[j - 1];
    }
    virtualKey[i] = x;
    for (int j = MAX_KEYS + 2; j > i + 1; j--) {
      virtualPtr[j] = virtualPtr[j - 1];
    }
    virtualPtr[i + 1] = child;
    newInternal->IS_LEAF = false;
    cursor->num_keys = (MAX_KEYS + 1) / 2;
    newInternal->num_keys = MAX_KEYS - (MAX_KEYS + 1) / 2;
    for (i = 0, j = cursor->num_keys + 1; i < newInternal->num_keys; i++, j++) {
      newInternal->keys[i] = virtualKey[j];
    }
    for (i = 0, j = cursor->num_keys + 1; i < newInternal->num_keys + 1; i++, j++) {
      newInternal->pointers[i] = virtualPtr[j];
    }
    for (int i = cursor->num_keys; i < MAX_KEYS+1; i++) {
      cursor->pointers[i + 1] = NULL;
    }   
    if (cursor == root) {
      Node *newRoot = new Node;
      newRoot->keys[0] = cursor->keys[cursor->num_keys];
      newRoot->pointers[0] = cursor;
      newRoot->pointers[1] = newInternal;
      newRoot->IS_LEAF = false;
      newRoot->num_keys = 1;
      root = newRoot;
    } else {
      insertInternal(cursor->keys[cursor->num_keys], findParent(root, cursor), newInternal);
    }
  }
}

// Find the parent
Node *BPTree::findParent(Node *cursor, Node *child) {
  Node *parent;
  if (cursor->IS_LEAF || (cursor->pointers[0])->IS_LEAF) {
    return NULL;
  }
  for (int i = 0; i < cursor->num_keys + 1; i++) {
    if (cursor->pointers[i] == child) {
      parent = cursor;
      return parent;
    } else {
      parent = findParent(cursor->pointers[i], child);
      if (parent != NULL)
        return parent;
    }
  }
  return parent;
}

// Print the tree
void BPTree::display(Node *cursor, int level) {
  if (cursor != NULL) {
    cout << cursor;
    for (int i = 0; i < level; i++) {
      cout << "   ";
    }
    cout << " level " << level << ": ";

    for (int i = 0; i < cursor->num_keys; i++) {
        cout << cursor->keys[i] << " ";
    }

    for (int i = cursor->num_keys; i < MAX_KEYS; i++) {
      cout << "x ";
    }

    for (int i = 0; i < MAX_KEYS+1; i++) {
      if (cursor->pointers[i] == NULL) {
        cout << "|       |";
      } else {
        cout << cursor->pointers[i] << " ";
      }
    }    
    
    cout << "\n";
    if (cursor->IS_LEAF != true) {
      for (int i = 0; i < cursor->num_keys + 1; i++) {
        display(cursor->pointers[i], level+1);
      }
    } 
  }
}

// Get the root
Node *BPTree::getRoot() {
  return root;
}

int bpt_2() {
  BPTree node;

  for (int i = 1; i < 22; i++) {
    node.insert(i);
  }

  node.display(node.getRoot(), 1);

  // node.search(10);
}



// // Searching on a B+ tree in C++

// #include <climits>
// #include <fstream>
// #include <iostream>
// #include <sstream>
// using namespace std;
// int MAX = 5;

// // BP node
// class Node {
//   bool IS_LEAF;
//   int *key, size;
//   Node **ptr;
//   friend class BPTree;

//    public:
//   Node();
// };

// // BP tree
// class BPTree {
//   Node *root;
//   void insertInternal(int, Node *, Node *);
//   Node *findParent(Node *, Node *);

//    public:
//   BPTree();
//   void search(int);
//   void insert(int);
//   void display(Node *, int level);
//   Node *getRoot();
// };

// Node::Node() {
//   key = new int[MAX];
//   ptr = new Node *[MAX + 1];
// }

// BPTree::BPTree() {
//   root = NULL;
// }

// // Search operation
// void BPTree::search(int x) {
//   if (root == NULL) {
//     cout << "Tree is empty\n";
//   } else {
//     Node *cursor = root;
//     while (cursor->IS_LEAF == false) {
//       for (int i = 0; i < cursor->size; i++) {
//         if (x < cursor->key[i]) {
//           cursor = cursor->ptr[i];
//           break;
//         }
//         if (i == cursor->size - 1) {
//           cursor = cursor->ptr[i + 1];
//           break;
//         }
//       }
//     }
//     for (int i = 0; i < cursor->size; i++) {
//       if (cursor->key[i] == x) {
//         cout << "Found\n";
//         return;
//       }
//     }
//     cout << "Not found\n";
//   }
// }

// // Insert Operation
// void BPTree::insert(int x) {
//   if (root == NULL) {
//     root = new Node;
//     root->key[0] = x;
//     root->IS_LEAF = true;
//     root->size = 1;
//   } else {
//     Node *cursor = root;
//     Node *parent;
//     while (cursor->IS_LEAF == false) {
//       parent = cursor;
//       for (int i = 0; i < cursor->size; i++) {
//         if (x < cursor->key[i]) {
//           cursor = cursor->ptr[i];
//           break;
//         }
//         if (i == cursor->size - 1) {
//           cursor = cursor->ptr[i + 1];
//           break;
//         }
//       }
//     }
//     if (cursor->size < MAX) {
//       int i = 0;
//       while (x > cursor->key[i] && i < cursor->size)
//         i++;
//       for (int j = cursor->size; j > i; j--) {
//         cursor->key[j] = cursor->key[j - 1];
//       }
//       cursor->key[i] = x;
//       cursor->size++;
//       cursor->ptr[cursor->size] = cursor->ptr[cursor->size - 1];
//       cursor->ptr[cursor->size - 1] = NULL;
//     } else {
//       Node *newLeaf = new Node;
//       int virtualNode[MAX + 1];
//       for (int i = 0; i < MAX; i++) {
//         virtualNode[i] = cursor->key[i];
//       }
//       int i = 0, j;
//       while (x > virtualNode[i] && i < MAX)
//         i++;
//       for (int j = MAX + 1; j > i; j--) {
//         virtualNode[j] = virtualNode[j - 1];
//       }
//       virtualNode[i] = x;
//       newLeaf->IS_LEAF = true;
//       cursor->size = (MAX + 1) / 2;
//       newLeaf->size = MAX + 1 - (MAX + 1) / 2;
//       cursor->ptr[cursor->size] = newLeaf;
//       newLeaf->ptr[newLeaf->size] = cursor->ptr[MAX];
//       cursor->ptr[MAX] = NULL;
//       for (i = 0; i < cursor->size; i++) {
//         cursor->key[i] = virtualNode[i];
//       }
//       for (i = 0, j = cursor->size; i < newLeaf->size; i++, j++) {
//         newLeaf->key[i] = virtualNode[j];
//       }
//       if (cursor == root) {
//         Node *newRoot = new Node;
//         newRoot->key[0] = newLeaf->key[0];
//         newRoot->ptr[0] = cursor;
//         newRoot->ptr[1] = newLeaf;
//         newRoot->IS_LEAF = false;
//         newRoot->size = 1;
//         root = newRoot;
//       } else {
//         insertInternal(newLeaf->key[0], parent, newLeaf);
//       }
//     }
//   }
// }

// // Insert Operation
// void BPTree::insertInternal(int x, Node *cursor, Node *child) {
//   if (cursor->size < MAX) {
//     int i = 0;
//     while (x > cursor->key[i] && i < cursor->size)
//       i++;
//     for (int j = cursor->size; j > i; j--) {
//       cursor->key[j] = cursor->key[j - 1];
//     }
//     for (int j = cursor->size + 1; j > i + 1; j--) {
//       cursor->ptr[j] = cursor->ptr[j - 1];
//     }
//     cursor->key[i] = x;
//     cursor->size++;
//     cursor->ptr[i + 1] = child;
//   } else {
//     Node *newInternal = new Node;
//     int virtualKey[MAX + 1];
//     Node *virtualPtr[MAX + 2];
//     for (int i = 0; i < MAX; i++) {
//       virtualKey[i] = cursor->key[i];
//     }
//     for (int i = 0; i < MAX + 1; i++) {
//       virtualPtr[i] = cursor->ptr[i];
//     }
//     int i = 0, j;
//     while (x > virtualKey[i] && i < MAX)
//       i++;
//     for (int j = MAX + 1; j > i; j--) {
//       virtualKey[j] = virtualKey[j - 1];
//     }
//     virtualKey[i] = x;
//     for (int j = MAX + 2; j > i + 1; j--) {
//       virtualPtr[j] = virtualPtr[j - 1];
//     }
//     virtualPtr[i + 1] = child;
//     newInternal->IS_LEAF = false;
//     cursor->size = (MAX + 1) / 2;
//     newInternal->size = MAX - (MAX + 1) / 2;
//     for (i = 0, j = cursor->size + 1; i < newInternal->size; i++, j++) {
//       newInternal->key[i] = virtualKey[j];
//     }
//     for (i = 0, j = cursor->size + 1; i < newInternal->size + 1; i++, j++) {
//       newInternal->ptr[i] = virtualPtr[j];
//     }
//     if (cursor == root) {
//       Node *newRoot = new Node;
//       newRoot->key[0] = cursor->key[cursor->size];
//       newRoot->ptr[0] = cursor;
//       newRoot->ptr[1] = newInternal;
//       newRoot->IS_LEAF = false;
//       newRoot->size = 1;
//       root = newRoot;
//     } else {
//       insertInternal(cursor->key[cursor->size], findParent(root, cursor), newInternal);
//     }
//   }
// }

// // Find the parent
// Node *BPTree::findParent(Node *cursor, Node *child) {
//   Node *parent;
//   if (cursor->IS_LEAF || (cursor->ptr[0])->IS_LEAF) {
//     return NULL;
//   }
//   for (int i = 0; i < cursor->size + 1; i++) {
//     if (cursor->ptr[i] == child) {
//       parent = cursor;
//       return parent;
//     } else {
//       parent = findParent(cursor->ptr[i], child);
//       if (parent != NULL)
//         return parent;
//     }
//   }
//   return parent;
// }

// // Print the tree
// void BPTree::display(Node *cursor, int level) {
//   if (cursor != NULL) {
//     cout << cursor;
//     for (int i = 0; i < level; i++) {
//       cout << "   ";
//     }
//     cout << " level " << level << ": ";

//     for (int i = 0; i < cursor->size; i++) {
//         cout << cursor->key[i] << " ";
//     }

//     for (int i = cursor->size; i < MAX; i++) {
//       cout << "x ";
//     }

//     for (int i = 0; i < MAX+1; i++) {
//       if (cursor->ptr[i] == NULL) {
//         cout << "|       |";
//       } else {
//         cout << cursor->ptr[i] << " ";
//       }
//     }    
    
//     cout << "\n";
//     if (cursor->IS_LEAF != true) {
//       for (int i = 0; i < cursor->size + 1; i++) {
//         display(cursor->ptr[i], level+1);
//       }
//     } 
//   }
// }






// // Get the root
// Node *BPTree::getRoot() {
//   return root;
// }

// int bpt_2() {
//   BPTree node;

//   for (int i = 1; i < 22; i++) {
//     node.insert(i);
//   }
 
//   node.display(node.getRoot(), 1);

  
// }