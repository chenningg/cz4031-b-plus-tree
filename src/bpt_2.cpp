// B+ tree in C++

#include <iostream>
#include <cmath>

using namespace std;
int MAX_KEYS = 5;

// BP node
class Node
{
private:
  bool IS_LEAF;
  int *keys, num_keys;
  Node **pointers;
  friend class BPTree;

public:
  Node();
};

// BP tree
class BPTree
{
private:
  Node *root;
  void insertInternal(int, Node *, Node *);
  void removeInternal(int, Node *, Node *);
  Node *findParent(Node *, Node *);

public:
  BPTree();
  void search(int);
  void insert(int);
  void remove(int);
  void display(Node *, int level);
  Node *getRoot();
};

Node::Node()
{
  keys = new int[MAX_KEYS];
  pointers = new Node *[MAX_KEYS + 1];
  for (int i = 0; i < MAX_KEYS; i++)
  {
    pointers[i] = NULL;
  }
}

BPTree::BPTree()
{
  root = NULL;
}

// Search operation
void BPTree::search(int x)
{
  if (root != NULL)
  {
    Node *cursor = root;
    while (!cursor->IS_LEAF)
    {
      for (int i = 0; i < cursor->num_keys; i++)
      {
        if (x < cursor->keys[i])
        {
          cursor = cursor->pointers[i];
          break;
        }
        if (i == cursor->num_keys - 1)
        {
          cursor = cursor->pointers[i + 1];
          break;
        }
      }
    }
    for (int i = 0; i < cursor->num_keys; i++)
    {
      if (cursor->keys[i] == x)
      {
        cout << "Found\n";
        return;
      }
    }
    cout << "Not found\n";
  }
  else
  {
    cout << "Tree is empty\n";
  }
}

// Insert Operation
void BPTree::insert(int x)
{
  if (root != NULL)
  {
    Node *cursor = root;
    Node *parent;

    while (cursor->IS_LEAF == false)
    {
      parent = cursor;
      for (int i = 0; i < cursor->num_keys; i++)
      {
        if (x < cursor->keys[i])
        {
          cursor = cursor->pointers[i];
          break;
        }
        if (i == cursor->num_keys - 1)
        {
          cursor = cursor->pointers[i + 1];
          break;
        }
      }
    }

    if (cursor->num_keys < MAX_KEYS)
    {
      int i = 0;
      while (x > cursor->keys[i] && i < cursor->num_keys)
        i++;
      for (int j = cursor->num_keys; j > i; j--)
      {
        cursor->keys[j] = cursor->keys[j - 1];
      }
      cursor->keys[i] = x;
      cursor->num_keys++;
      cursor->pointers[cursor->num_keys] = cursor->pointers[cursor->num_keys - 1];
      cursor->pointers[cursor->num_keys - 1] = NULL;
    }

    else
    {
      Node *newLeaf = new Node;
      int tempKeyList[MAX_KEYS + 1];
      for (int i = 0; i < MAX_KEYS; i++)
      {
        tempKeyList[i] = cursor->keys[i];
      }
      int i = 0, j;
      while (x > tempKeyList[i] && i < MAX_KEYS)
        i++;
      for (int j = MAX_KEYS + 1; j > i; j--)
      {
        tempKeyList[j] = tempKeyList[j - 1];
      }
      tempKeyList[i] = x;
      newLeaf->IS_LEAF = true;
      cursor->num_keys = (MAX_KEYS + 1) / 2;
      newLeaf->num_keys = MAX_KEYS + 1 - (MAX_KEYS + 1) / 2;

      for (i = cursor->num_keys; i < MAX_KEYS + 1; i++)
      {
        cursor->pointers[i + 1] = NULL;
      }
      for (i = 0; i < cursor->num_keys; i++)
      {
        cursor->keys[i] = tempKeyList[i];
      }
      for (i = 0, j = cursor->num_keys; i < newLeaf->num_keys; i++, j++)
      {
        newLeaf->keys[i] = tempKeyList[j];
      }
      cursor->pointers[cursor->num_keys] = newLeaf;

      if (cursor == root)
      {
        Node *newRoot = new Node;
        newRoot->keys[0] = newLeaf->keys[0];
        newRoot->pointers[0] = cursor;
        newRoot->pointers[1] = newLeaf;
        newRoot->IS_LEAF = false;
        newRoot->num_keys = 1;
        for (int i = newRoot->num_keys; i < MAX_KEYS + 1; i++)
        {
          newRoot->pointers[i + 1] = NULL;
        }
        root = newRoot;
      }
      else
      {
        insertInternal(newLeaf->keys[0], parent, newLeaf);
      }
    }
  }
  else
  {
    root = new Node;
    root->IS_LEAF = true;
    root->num_keys = 1;
    root->keys[0] = x;
  }
}

// Insert Operation
void BPTree::insertInternal(int x, Node *cursor, Node *child)
{
  if (cursor->num_keys < MAX_KEYS)
  {
    int i = 0;
    while (x > cursor->keys[i] && i < cursor->num_keys)
      i++;
    for (int j = cursor->num_keys; j > i; j--)
    {
      cursor->keys[j] = cursor->keys[j - 1];
    }
    for (int j = cursor->num_keys + 1; j > i + 1; j--)
    {
      cursor->pointers[j] = cursor->pointers[j - 1];
    }
    cursor->keys[i] = x;
    cursor->num_keys++;
    cursor->pointers[i + 1] = child;
  }
  else
  {
    Node *newInternal = new Node;

    int tempKeyList[MAX_KEYS + 1];
    Node *tempPointerList[MAX_KEYS + 2];
    for (int i = 0; i < MAX_KEYS; i++)
    {
      tempKeyList[i] = cursor->keys[i];
    }
    for (int i = 0; i < MAX_KEYS + 1; i++)
    {
      tempPointerList[i] = cursor->pointers[i];
    }
    int i = 0, j;
    while (x > tempKeyList[i] && i < MAX_KEYS)
      i++;

    for (int j = MAX_KEYS + 1; j > i; j--)
    {
      tempKeyList[j] = tempKeyList[j - 1];
    }
    tempKeyList[i] = x;
    for (int j = MAX_KEYS + 2; j > i + 1; j--)
    {
      tempPointerList[j] = tempPointerList[j - 1];
    }
    tempPointerList[i + 1] = child;
    newInternal->IS_LEAF = false;
    cursor->num_keys = (MAX_KEYS + 1) / 2;
    newInternal->num_keys = MAX_KEYS - (MAX_KEYS + 1) / 2;

    for (i = 0, j = cursor->num_keys + 1; i < newInternal->num_keys; i++, j++)
    {
      newInternal->keys[i] = tempKeyList[j];
    }
    for (i = 0, j = cursor->num_keys + 1; i < newInternal->num_keys + 1; i++, j++)
    {
      newInternal->pointers[i] = tempPointerList[j];
    }
    for (int i = cursor->num_keys; i < MAX_KEYS + 1; i++)
    {
      cursor->pointers[i + 1] = NULL;
    }
    if (cursor == root)
    {
      Node *newRoot = new Node;
      newRoot->keys[0] = cursor->keys[cursor->num_keys];
      newRoot->pointers[0] = cursor;
      newRoot->pointers[1] = newInternal;
      newRoot->IS_LEAF = false;
      newRoot->num_keys = 1;
      root = newRoot;
    }
    else
    {
      insertInternal(cursor->keys[cursor->num_keys], findParent(root, cursor), newInternal);
    }
  }
}

// Find the parent
Node *BPTree::findParent(Node *cursor, Node *child)
{
  Node *parent;
  if (cursor->IS_LEAF || (cursor->pointers[0])->IS_LEAF)
  {
    return NULL;
  }
  for (int i = 0; i < cursor->num_keys + 1; i++)
  {
    if (cursor->pointers[i] == child)
    {
      parent = cursor;
      return parent;
    }
    else
    {
      parent = findParent(cursor->pointers[i], child);
      if (parent != NULL)
        return parent;
    }
  }
  return parent;
}

// Delete Operation
// void BPTree::remove(int x) {
//   if (root == NULL) {
//     cout << "Root is NULL";
//   } else {
//     Node *cursor = root;
//     Node *parent;

//     // Traverse to find leaf
//     while (cursor->IS_LEAF == false) {
//       parent = cursor;
//       for (int i = 0; i < cursor->num_keys; i++)
//       {
//         if (x < cursor->keys[i])
//         {
//           cursor = cursor->pointers[i];
//           break;
//         }
//         if (i == cursor->num_keys - 1)
//         {
//           cursor = cursor->pointers[i + 1];
//           break;
//         }
//       }
//     } // leaf found
//     if (cursor == root) {
//         if (root->num_keys == 1) {
//           cursor->keys[0] = NULL;
//           cursor->pointers[0] = NULL;
//           cursor->num_keys = 0;
//         } else {
//           int i = 0;
//           while (x > cursor->keys[i] && i < cursor->num_keys)
//             i++;
//           cout << "i: " << i;
//           for (int j = i; j < cursor->num_keys-1; j++)
//           {
//             cursor->keys[j] = cursor->keys[j + 1];
//             cursor->pointers[j+1] = cursor->pointers[j + 1];
//           }
//           cursor->keys[cursor->num_keys] = NULL;
//           cursor->pointers[cursor->num_keys] = NULL;
//           cursor->num_keys--;
//         }
//     }
//     // if cursor is not the root
//     else {
//       // can delete from leaf safely
//       if (   cursor->num_keys >= (cursor->num_keys + 1)/2 + 1  ) {
//         int i = 0;
//         while (x > cursor->keys[i] && i < cursor->num_keys)
//           i++;
//         cout << "\ni: " << i << "\n";
//         for (int j = i; j < cursor->num_keys-1; j++)
//         {
//           cursor->keys[j] = cursor->keys[j + 1];
//           cursor->pointers[j+1] = cursor->pointers[j + 1];
//         }
//         cursor->keys[cursor->num_keys] = NULL;
//         cursor->pointers[cursor->num_keys-1] = cursor->pointers[cursor->num_keys];
//         cursor->pointers[cursor->num_keys] = NULL;
//         cursor->num_keys--;

//         // need to update parent node because parent node has the deleted key as a key value

//       }
//       // cannot delete from leaf safely - leaf has min keys already
//       else {
//         // delete and try to borrow key from sibling

//       }

//     }
//   }
// }

// Delete Operation
void BPTree::remove(int x)
{
  //delete logic
  if (root == NULL)
  {
    cout << "Tree empty\n";
  }
  else
  {
    Node *cursor = root;
    Node *parent;
    int leftSibling, rightSibling;
    //in the following while loop, cursor will will travel to the leaf node possibly consisting the key
    while (!cursor->IS_LEAF)
    {
      for (int i = 0; i < cursor->num_keys; i++)
      {
        parent = cursor;
        leftSibling = i - 1;  //leftSibling is the index of left sibling in the parent node
        rightSibling = i + 1; //rightSibling is the index of right sibling in the parent node
        if (x < cursor->keys[i])
        {
          cursor = cursor->pointers[i];
          break;
        }
        if (i == cursor->num_keys - 1)
        {
          leftSibling = i;
          rightSibling = i + 2;
          cursor = cursor->pointers[i + 1];
          break;
        }
      }
    }
    //in the following for loop, we search for the key if it exists
    bool found = false;
    int pos;
    for (pos = 0; pos < cursor->num_keys; pos++)
    {
      if (cursor->keys[pos] == x)
      {
        found = true;
        break;
      }
    }
    if (!found) //if keys does not exist in that leaf node
    {
      cout << "Not found\n";
      return;
    }
    //deleting the keys
    for (int i = pos; i < cursor->num_keys; i++)
    {
      cursor->keys[i] = cursor->keys[i + 1];
    }
    cursor->num_keys--;
    if (cursor == root) //if it is root node, then make all pointers NULL
    {
      cout << "Deleted " << x << " from leaf node successfully\n";
      for (int i = 0; i < MAX_KEYS + 1; i++)
      {
        cursor->pointers[i] = NULL;
      }
      if (cursor->num_keys == 0) //if all keys are deleted
      {
        cout << "Tree died\n";
        delete[] cursor->keys;
        delete[] cursor->pointers;
        delete cursor;
        root = NULL;
      }
      return;
    }
    cursor->pointers[cursor->num_keys] = cursor->pointers[cursor->num_keys + 1];
    cursor->pointers[cursor->num_keys + 1] = NULL;
    cout << "Deleted " << x << " from leaf node successfully\n";
    if (cursor->num_keys >= (MAX_KEYS + 1) / 2) //no underflow
    {
      return;
    }
    cout << "Underflow in leaf node!\n";
    //underflow condition
    //first we try to transfer a keys from sibling node
    //check if left sibling exists
    if (leftSibling >= 0)
    {
      Node *leftNode = parent->pointers[leftSibling];
      //check if it is possible to transfer
      if (leftNode->num_keys >= (MAX_KEYS + 1) / 2 + 1)
      {
        //make space for transfer
        for (int i = cursor->num_keys; i > 0; i--)
        {
          cursor->keys[i] = cursor->keys[i - 1];
        }
        //shift pointer to next leaf
        cursor->num_keys++;
        cursor->pointers[cursor->num_keys] = cursor->pointers[cursor->num_keys - 1];
        cursor->pointers[cursor->num_keys - 1] = NULL;
        //transfer
        cursor->keys[0] = leftNode->keys[leftNode->num_keys - 1];
        //shift pointer of leftsibling
        leftNode->num_keys--;
        leftNode->pointers[leftNode->num_keys] = cursor;
        leftNode->pointers[leftNode->num_keys + 1] = NULL;
        //update parent
        parent->keys[leftSibling] = cursor->keys[0];
        cout << "Transferred " << cursor->keys[0] << " from left sibling of leaf node\n";
        return;
      }
    }
    if (rightSibling <= parent->num_keys) //check if right sibling exist
    {
      Node *rightNode = parent->pointers[rightSibling];
      //check if it is possible to transfer
      if (rightNode->num_keys >= (MAX_KEYS + 1) / 2 + 1)
      {
        //shift pointer to next leaf
        cursor->num_keys++;
        cursor->pointers[cursor->num_keys] = cursor->pointers[cursor->num_keys - 1];
        cursor->pointers[cursor->num_keys - 1] = NULL;
        //transfer
        cursor->keys[cursor->num_keys - 1] = rightNode->keys[0];
        //shift pointer of rightsibling
        rightNode->num_keys--;
        rightNode->pointers[rightNode->num_keys] = rightNode->pointers[rightNode->num_keys + 1];
        rightNode->pointers[rightNode->num_keys + 1] = NULL;
        //shift conent of right sibling
        for (int i = 0; i < rightNode->num_keys; i++)
        {
          rightNode->keys[i] = rightNode->keys[i + 1];
        }
        //update parent
        parent->keys[rightSibling - 1] = rightNode->keys[0];
        cout << "Transferred " << cursor->keys[cursor->num_keys - 1] << " from right sibling of leaf node\n";
        return;
      }
    }
    //must merge and delete a node
    if (leftSibling >= 0) //if left sibling exist
    {
      Node *leftNode = parent->pointers[leftSibling];
      // transfer all keys to leftsibling and then transfer pointer to next leaf node
      for (int i = leftNode->num_keys, j = 0; j < cursor->num_keys; i++, j++)
      {
        leftNode->keys[i] = cursor->keys[j];
      }
      leftNode->pointers[leftNode->num_keys] = NULL;
      leftNode->num_keys += cursor->num_keys;
      leftNode->pointers[leftNode->num_keys] = cursor->pointers[cursor->num_keys];
      cout << "Merging two leaf nodes\n";
      removeInternal(parent->keys[leftSibling], parent, cursor); // delete parent node keys
      delete[] cursor->keys;
      delete[] cursor->pointers;
      delete cursor;
    }
    else if (rightSibling <= parent->num_keys) //if right sibling exist
    {
      Node *rightNode = parent->pointers[rightSibling];
      // transfer all keys to cursor and then transfer pointer to next leaf node
      for (int i = cursor->num_keys, j = 0; j < rightNode->num_keys; i++, j++)
      {
        cursor->keys[i] = rightNode->keys[j];
      }
      cursor->pointers[cursor->num_keys] = NULL;
      cursor->num_keys += rightNode->num_keys;
      cursor->pointers[cursor->num_keys] = rightNode->pointers[rightNode->num_keys];
      cout << "Merging two leaf nodes\n";
      removeInternal(parent->keys[rightSibling - 1], parent, rightNode); // delete parent node keys
      delete[] rightNode->keys;
      delete[] rightNode->pointers;
      delete rightNode;
    }
  }
}
void BPTree::removeInternal(int x, Node *cursor, Node *child)
{
  //deleting the key x first
  //checking if key from root is to be deleted
  if (cursor == root)
  {
    if (cursor->num_keys == 1) //if only one key is left, change root
    {
      if (cursor->pointers[1] == child)
      {
        delete[] child->keys;
        delete[] child->pointers;
        delete child;
        root = cursor->pointers[0];
        delete[] cursor->keys;
        delete[] cursor->pointers;
        delete cursor;
        cout << "Changed root node\n";
        return;
      }
      else if (cursor->pointers[0] == child)
      {
        delete[] child->keys;
        delete[] child->pointers;
        delete child;
        root = cursor->pointers[1];
        delete[] cursor->keys;
        delete[] cursor->pointers;
        delete cursor;
        cout << "Changed root node\n";
        return;
      }
    }
  }
  int pos;
  for (pos = 0; pos < cursor->num_keys; pos++)
  {
    if (cursor->keys[pos] == x)
    {
      break;
    }
  }
  for (int i = pos; i < cursor->num_keys; i++)
  {
    cursor->keys[i] = cursor->keys[i + 1];
  }
  //now deleting the pointer child
  for (pos = 0; pos < cursor->num_keys + 1; pos++)
  {
    if (cursor->pointers[pos] == child)
    {
      break;
    }
  }
  for (int i = pos; i < cursor->num_keys + 1; i++)
  {
    cursor->pointers[i] = cursor->pointers[i + 1];
  }
  cursor->num_keys--;
  if (cursor->num_keys >= (MAX_KEYS + 1) / 2 - 1) //no underflow
  {
    cout << "Deleted " << x << " from internal node successfully\n";
    return;
  }
  cout << "Underflow in internal node!\n";
  //underflow, try to transfer first
  if (cursor == root)
    return;
  Node *parent = findParent(root, cursor);
  int leftSibling, rightSibling;
  //finding left n right sibling of cursor
  for (pos = 0; pos < parent->num_keys + 1; pos++)
  {
    if (parent->pointers[pos] == cursor)
    {
      leftSibling = pos - 1;
      rightSibling = pos + 1;
      break;
    }
  }
  //try to transfer
  if (leftSibling >= 0) //if left sibling exists
  {
    Node *leftNode = parent->pointers[leftSibling];
    //check if it is possible to transfer
    if (leftNode->num_keys >= (MAX_KEYS + 1) / 2)
    {
      //make space for transfer of keys
      for (int i = cursor->num_keys; i > 0; i--)
      {
        cursor->keys[i] = cursor->keys[i - 1];
      }
      //transfer keys from left sibling through parent
      cursor->keys[0] = parent->keys[leftSibling];
      parent->keys[leftSibling] = leftNode->keys[leftNode->num_keys - 1];
      //transfer last pointer from leftnode to cursor
      //make space for transfer of ptr
      for (int i = cursor->num_keys + 1; i > 0; i--)
      {
        cursor->pointers[i] = cursor->pointers[i - 1];
      }
      //transfer ptr
      cursor->pointers[0] = leftNode->pointers[leftNode->num_keys];
      cursor->num_keys++;
      leftNode->num_keys--;
      cout << "Transferred " << cursor->keys[0] << " from left sibling of internal node\n";
      return;
    }
  }
  if (rightSibling <= parent->num_keys) //check if right sibling exist
  {
    Node *rightNode = parent->pointers[rightSibling];
    //check if it is possible to transfer
    if (rightNode->num_keys >= (MAX_KEYS + 1) / 2)
    {
      //transfer keys from right sibling through parent
      cursor->keys[cursor->num_keys] = parent->keys[pos];
      parent->keys[pos] = rightNode->keys[0];
      for (int i = 0; i < rightNode->num_keys - 1; i++)
      {
        rightNode->keys[i] = rightNode->keys[i + 1];
      }
      //transfer first pointer from rightnode to cursor
      //transfer ptr
      cursor->pointers[cursor->num_keys + 1] = rightNode->pointers[0];
      for (int i = 0; i < rightNode->num_keys; ++i)
      {
        rightNode->pointers[i] = rightNode->pointers[i + 1];
      }
      cursor->num_keys++;
      rightNode->num_keys--;
      cout << "Transferred " << cursor->keys[0] << " from right sibling of internal node\n";
      return;
    }
  }
  //transfer wasnt posssible hence do merging
  if (leftSibling >= 0)
  {
    //leftnode + parent keys + cursor
    Node *leftNode = parent->pointers[leftSibling];
    leftNode->keys[leftNode->num_keys] = parent->keys[leftSibling];
    for (int i = leftNode->num_keys + 1, j = 0; j < cursor->num_keys; j++)
    {
      leftNode->keys[i] = cursor->keys[j];
    }
    for (int i = leftNode->num_keys + 1, j = 0; j < cursor->num_keys + 1; j++)
    {
      leftNode->pointers[i] = cursor->pointers[j];
      cursor->pointers[j] = NULL;
    }
    leftNode->num_keys += cursor->num_keys + 1;
    cursor->num_keys = 0;
    //delete cursor
    removeInternal(parent->keys[leftSibling], parent, cursor);
    cout << "Merged with left sibling\n";
  }
  else if (rightSibling <= parent->num_keys)
  {
    //cursor + parent keys + rightnode
    Node *rightNode = parent->pointers[rightSibling];
    cursor->keys[cursor->num_keys] = parent->keys[rightSibling - 1];
    for (int i = cursor->num_keys + 1, j = 0; j < rightNode->num_keys; j++)
    {
      cursor->keys[i] = rightNode->keys[j];
    }
    for (int i = cursor->num_keys + 1, j = 0; j < rightNode->num_keys + 1; j++)
    {
      cursor->pointers[i] = rightNode->pointers[j];
      rightNode->pointers[j] = NULL;
    }
    cursor->num_keys += rightNode->num_keys + 1;
    rightNode->num_keys = 0;
    //delete cursor
    removeInternal(parent->keys[rightSibling - 1], parent, rightNode);
    cout << "Merged with right sibling\n";
  }
}

// Print the tree
void BPTree::display(Node *cursor, int level)
{
  if (cursor != NULL)
  {
    cout << cursor;
    for (int i = 0; i < level; i++)
    {
      cout << "   ";
    }
    cout << " level " << level << ": ";

    for (int i = 0; i < cursor->num_keys; i++)
    {
      cout << cursor->keys[i] << " ";
    }

    for (int i = cursor->num_keys; i < MAX_KEYS; i++)
    {
      cout << "x ";
    }

    for (int i = 0; i < MAX_KEYS + 1; i++)
    {
      if (cursor->pointers[i] == NULL)
      {
        cout << "|       |";
      }
      else
      {
        cout << cursor->pointers[i] << " ";
      }
    }

    cout << "\n";
    if (cursor->IS_LEAF != true)
    {
      for (int i = 0; i < cursor->num_keys + 1; i++)
      {
        display(cursor->pointers[i], level + 1);
      }
    }
  }
}

// Get the root
Node *BPTree::getRoot()
{
  return root;
}

int bpt_2()
{
  BPTree node;

  cout << "Hello";

  cout << "bye";
  // for (int i = 1; i < 22; i++)
  // {
  //   node.insert(i);
  // }
  for (int i = 5; i < 10; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      node.insert(i);
    }
  }

  // node.display(node.getRoot(), 1);

  // node.remove(4);
  // node.display(node.getRoot(), 1);
  node.remove(6);
  node.display(node.getRoot(), 1);
  node.remove(6);
  node.display(node.getRoot(), 1);
  node.remove(7);
  node.display(node.getRoot(), 1);
  node.remove(8);
  node.display(node.getRoot(), 1);
}