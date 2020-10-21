//Author: Shashikant Kadam
//Roll number 16CSE1026
/*****B+ Tree*****/
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <climits>
using namespace std;
int maxKeys;  //numKeys of each node
class BPTree; //self explanatory classes
class Node
{
  bool isLeaf;
  int *keys, numKeys;
  Node **pointers;
  friend class BPTree;

public:
  Node();
};
class BPTree
{
  Node *root;
  void insertInternal(int, Node *, Node *);
  void removeInternal(int, Node *, Node *);
  Node *findParent(Node *, Node *);

public:
  BPTree();
  void search(int);
  void insert(int);
  void remove(int);
  void display(Node *);
  Node *getRoot();
  void cleanUp(Node *);
  ~BPTree();
};
//give command line argument to load a tree from log
//to create a fresh tree, do not give any command line argument
int main(int argc, char *argv[])
{
  BPTree bpt; //B+ tree object that carries out all the operations
  string command;
  int x;
  bool close = false;
  string logBuffer; //used to save into log
  ifstream fin;
  ofstream fout;
  //create tree from log file from command line input
  if (argc > 1)
  {
    fin.open(argv[1]); //open file
    if (!fin.is_open())
    {
      cout << "File not found\n";
      return 0;
    }
    int i = 1;
    getline(fin, logBuffer, '\0'); //copy log from file to logBuffer for saving purpose
    fin.close();
    fin.open(argv[1]); //reopening file
    getline(fin, command);
    stringstream max(command); //first line of log contains the max degree
    max >> maxKeys;
    while (getline(fin, command)) //iterating over every line ie command
    {
      if (!command.substr(0, 6).compare("insert"))
      {
        stringstream argument(command.substr(7));
        argument >> x;
        bpt.insert(x);
      }
      else if (!command.substr(0, 6).compare("delete"))
      {
        stringstream argument(command.substr(7));
        argument >> x;
        bpt.remove(x);
      }
      else
      {
        cout << "Unknown command: " << command << " at line #" << i << "\n";
        return 0;
      }
      i++;
    }
    cout << "Tree loaded successfully from: \"" << argv[1] << "\"\n";
    fin.close();
  }
  else //create fresh tree
  {
    cout << "Enter the max degree\n";
    cin >> command;
    stringstream max(command);
    max >> maxKeys;
    logBuffer.append(command);
    logBuffer.append("\n");
    cin.clear();
    cin.ignore(1);
  }
  //command line menu
  cout << "Commands:\nsearch <value> to search\n";
  cout << "insert <value> to insert\n";
  cout << "delete <value> to delete\n";
  cout << "display to display\n";
  cout << "save to save log\n";
  cout << "exit to exit\n";
  do
  {
    cout << "Enter command: ";
    getline(cin, command);
    if (!command.substr(0, 6).compare("search"))
    {
      stringstream argument(command.substr(7));
      argument >> x;
      bpt.search(x);
    }
    else if (!command.substr(0, 6).compare("insert"))
    {
      stringstream argument(command.substr(7));
      argument >> x;
      bpt.insert(x);
      logBuffer.append(command);
      logBuffer.append("\n");
    }
    else if (!command.substr(0, 6).compare("delete"))
    {
      stringstream argument(command.substr(7));
      argument >> x;
      bpt.remove(x);
      logBuffer.append(command);
      logBuffer.append("\n");
    }
    else if (!command.compare("display"))
    {
      bpt.display(bpt.getRoot());
    }
    else if (!command.compare("save"))
    {
      cout << "Enter file name: ";
      string filename;
      cin >> filename;
      fout.open(filename);
      fout << logBuffer;
      fout.close();
      cout << "Saved successfully into file: \"" << filename << "\"\n";
      cin.clear();
      cin.ignore(1);
    }
    else if (!command.compare("exit"))
    {
      close = true;
    }
    else
    {
      cout << "Invalid command\n";
    }
  } while (!close);
  return 0;
}
Node::Node()
{
  //dynamic memory allocation
  keys = new int[maxKeys];
  pointers = new Node *[maxKeys + 1];
}
BPTree::BPTree()
{
  root = NULL;
}
void BPTree::search(int x)
{
  //search logic
  if (root == NULL)
  {
    //empty
    cout << "Tree empty\n";
  }
  else
  {
    Node *cursor = root;
    //in the following while loop, cursor will travel to the leaf node possibly consisting the keys
    while (cursor->isLeaf == false)
    {
      for (int i = 0; i < cursor->numKeys; i++)
      {
        if (x < cursor->keys[i])
        {
          cursor = cursor->pointers[i];
          break;
        }
        if (i == cursor->numKeys - 1)
        {
          cursor = cursor->pointers[i + 1];
          break;
        }
      }
    }
    //in the following for loop, we search for the keys if it exists
    for (int i = 0; i < cursor->numKeys; i++)
    {
      if (cursor->keys[i] == x)
      {
        cout << "Found\n";
        return;
      }
    }
    cout << "Not found\n";
  }
}
void BPTree::insert(int x)
{
  //insert logic
  if (root == NULL)
  {
    root = new Node;
    root->keys[0] = x;
    root->isLeaf = true;
    root->numKeys = 1;
    cout << "Created root\nInserted " << x << " successfully\n";
  }
  else
  {
    Node *cursor = root;
    Node *parent;
    //in the following while loop, cursor will travel to the leaf node possibly consisting the keys
    while (cursor->isLeaf == false)
    {
      parent = cursor;
      for (int i = 0; i < cursor->numKeys; i++)
      {
        if (x < cursor->keys[i])
        {
          cursor = cursor->pointers[i];
          break;
        }
        if (i == cursor->numKeys - 1)
        {
          cursor = cursor->pointers[i + 1];
          break;
        }
      }
    }
    //now cursor is the leaf node in which we'll insert the new keys
    if (cursor->numKeys < maxKeys)
    {
      //if cursor is not full
      //find the correct position for new keys
      int i = 0;
      while (x > cursor->keys[i] && i < cursor->numKeys)
        i++;
      //make space for new keys
      for (int j = cursor->numKeys; j > i; j--)
      {
        cursor->keys[j] = cursor->keys[j - 1];
      }
      cursor->keys[i] = x;
      cursor->numKeys++;
      cursor->pointers[cursor->numKeys] = cursor->pointers[cursor->numKeys - 1];
      cursor->pointers[cursor->numKeys - 1] = NULL;
      cout << "Inserted " << x << " successfully\n";
    }
    else
    {
      cout << "Inserted " << x << " successfully\n";
      cout << "Overflow in leaf node!\nSplitting leaf node\n";
      //overflow condition
      //create new leaf node
      Node *newLeaf = new Node;
      //create a virtual node and insert x into it
      int virtualNode[maxKeys + 1];
      for (int i = 0; i < maxKeys; i++)
      {
        virtualNode[i] = cursor->keys[i];
      }
      int i = 0, j;
      while (x > virtualNode[i] && i < maxKeys)
        i++;
      //make space for new keys
      for (int j = maxKeys + 1; j > i; j--)
      {
        virtualNode[j] = virtualNode[j - 1];
      }
      virtualNode[i] = x;
      newLeaf->isLeaf = true;
      //split the cursor into two leaf nodes
      cursor->numKeys = (maxKeys + 1) / 2;
      newLeaf->numKeys = maxKeys + 1 - (maxKeys + 1) / 2;
      //make cursor point to new leaf node
      cursor->pointers[cursor->numKeys] = newLeaf;
      //make new leaf node point to the next leaf node
      newLeaf->pointers[newLeaf->numKeys] = cursor->pointers[maxKeys];
      cursor->pointers[maxKeys] = NULL;
      //now give elements to new leaf nodes
      for (i = 0; i < cursor->numKeys; i++)
      {
        cursor->keys[i] = virtualNode[i];
      }
      for (i = 0, j = cursor->numKeys; i < newLeaf->numKeys; i++, j++)
      {
        newLeaf->keys[i] = virtualNode[j];
      }
      //modify the parent
      if (cursor == root)
      {
        //if cursor is a root node, we create a new root
        Node *newRoot = new Node;
        newRoot->keys[0] = newLeaf->keys[0];
        newRoot->pointers[0] = cursor;
        newRoot->pointers[1] = newLeaf;
        newRoot->isLeaf = false;
        newRoot->numKeys = 1;
        root = newRoot;
        cout << "Created new root\n";
      }
      else
      {
        //insert new keys in parent node
        insertInternal(newLeaf->keys[0], parent, newLeaf);
      }
    }
  }
}
void BPTree::insertInternal(int x, Node *cursor, Node *child)
{
  if (cursor->numKeys < maxKeys)
  {
    //if cursor is not full
    //find the correct position for new keys
    int i = 0;
    while (x > cursor->keys[i] && i < cursor->numKeys)
      i++;
    //make space for new keys
    for (int j = cursor->numKeys; j > i; j--)
    {
      cursor->keys[j] = cursor->keys[j - 1];
    } //make space for new pointer
    for (int j = cursor->numKeys + 1; j > i + 1; j--)
    {
      cursor->pointers[j] = cursor->pointers[j - 1];
    }
    cursor->keys[i] = x;
    cursor->numKeys++;
    cursor->pointers[i + 1] = child;
    cout << "Inserted keys in an Internal node successfully\n";
  }
  else
  {
    cout << "Inserted keys in an Internal node successfully\n";
    cout << "Overflow in internal node!\nSplitting internal node\n";
    //if overflow in internal node
    //create new internal node
    Node *newInternal = new Node;
    //create virtual Internal Node;
    int virtualKey[maxKeys + 1];
    Node *virtualPtr[maxKeys + 2];
    for (int i = 0; i < maxKeys; i++)
    {
      virtualKey[i] = cursor->keys[i];
    }
    for (int i = 0; i < maxKeys + 1; i++)
    {
      virtualPtr[i] = cursor->pointers[i];
    }
    int i = 0, j;
    while (x > virtualKey[i] && i < maxKeys)
      i++;
    //make space for new keys
    for (int j = maxKeys + 1; j > i; j--)
    {
      virtualKey[j] = virtualKey[j - 1];
    }
    virtualKey[i] = x;
    //make space for new pointers
    for (int j = maxKeys + 2; j > i + 1; j--)
    {
      virtualPtr[j] = virtualPtr[j - 1];
    }
    virtualPtr[i + 1] = child;
    newInternal->isLeaf = false;
    //split cursor into two nodes
    cursor->numKeys = (maxKeys + 1) / 2;
    newInternal->numKeys = maxKeys - (maxKeys + 1) / 2;
    //give elements and pointers to the new node
    for (i = 0, j = cursor->numKeys + 1; i < newInternal->numKeys; i++, j++)
    {
      newInternal->keys[i] = virtualKey[j];
    }
    for (i = 0, j = cursor->numKeys + 1; i < newInternal->numKeys + 1; i++, j++)
    {
      newInternal->pointers[i] = virtualPtr[j];
    }
    // m = cursor->keys[cursor->numKeys]
    if (cursor == root)
    {
      //if cursor is a root node, we create a new root
      Node *newRoot = new Node;
      newRoot->keys[0] = cursor->keys[cursor->numKeys];
      newRoot->pointers[0] = cursor;
      newRoot->pointers[1] = newInternal;
      newRoot->isLeaf = false;
      newRoot->numKeys = 1;
      root = newRoot;
      cout << "Created new root\n";
    }
    else
    {
      //recursion
      //find depth first search to find parent of cursor
      insertInternal(cursor->keys[cursor->numKeys], findParent(root, cursor), newInternal);
    }
  }
}
Node *BPTree::findParent(Node *cursor, Node *child)
{
  //finds parent using depth first traversal and ignores leaf nodes as they cannot be parents
  //also ignores second last level because we will never find parent of a leaf node during insertion using this function
  Node *parent;
  if (cursor->isLeaf || (cursor->pointers[0])->isLeaf)
  {
    return NULL;
  }
  for (int i = 0; i < cursor->numKeys + 1; i++)
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
    //in the following while loop, cursor will will travel to the leaf node possibly consisting the keys
    while (cursor->isLeaf == false)
    {
      for (int i = 0; i < cursor->numKeys; i++)
      {
        parent = cursor;
        leftSibling = i - 1;  //leftSibling is the index of left sibling in the parent node
        rightSibling = i + 1; //rightSibling is the index of right sibling in the parent node
        if (x < cursor->keys[i])
        {
          cursor = cursor->pointers[i];
          break;
        }
        if (i == cursor->numKeys - 1)
        {
          leftSibling = i;
          rightSibling = i + 2;
          cursor = cursor->pointers[i + 1];
          break;
        }
      }
    }
    //in the following for loop, we search for the keys if it exists
    bool found = false;
    int pos;
    for (pos = 0; pos < cursor->numKeys; pos++)
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
    for (int i = pos; i < cursor->numKeys; i++)
    {
      cursor->keys[i] = cursor->keys[i + 1];
    }
    cursor->numKeys--;
    if (cursor == root) //if it is root node, then make all pointers NULL
    {
      cout << "Deleted " << x << " from leaf node successfully\n";
      for (int i = 0; i < maxKeys + 1; i++)
      {
        cursor->pointers[i] = NULL;
      }
      if (cursor->numKeys == 0) //if all keys are deleted
      {
        cout << "Tree died\n";
        delete[] cursor->keys;
        delete[] cursor->pointers;
        delete cursor;
        root = NULL;
      }
      return;
    }
    cursor->pointers[cursor->numKeys] = cursor->pointers[cursor->numKeys + 1];
    cursor->pointers[cursor->numKeys + 1] = NULL;
    cout << "Deleted " << x << " from leaf node successfully\n";
    if (cursor->numKeys >= (maxKeys + 1) / 2) //no underflow
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
      if (leftNode->numKeys >= (maxKeys + 1) / 2 + 1)
      {
        //make space for transfer
        for (int i = cursor->numKeys; i > 0; i--)
        {
          cursor->keys[i] = cursor->keys[i - 1];
        }
        //shift pointer to next leaf
        cursor->numKeys++;
        cursor->pointers[cursor->numKeys] = cursor->pointers[cursor->numKeys - 1];
        cursor->pointers[cursor->numKeys - 1] = NULL;
        //transfer
        cursor->keys[0] = leftNode->keys[leftNode->numKeys - 1];
        //shift pointer of leftsibling
        leftNode->numKeys--;
        leftNode->pointers[leftNode->numKeys] = cursor;
        leftNode->pointers[leftNode->numKeys + 1] = NULL;
        //update parent
        parent->keys[leftSibling] = cursor->keys[0];
        cout << "Transferred " << cursor->keys[0] << " from left sibling of leaf node\n";
        return;
      }
    }
    if (rightSibling <= parent->numKeys) //check if right sibling exist
    {
      Node *rightNode = parent->pointers[rightSibling];
      //check if it is possible to transfer
      if (rightNode->numKeys >= (maxKeys + 1) / 2 + 1)
      {
        //shift pointer to next leaf
        cursor->numKeys++;
        cursor->pointers[cursor->numKeys] = cursor->pointers[cursor->numKeys - 1];
        cursor->pointers[cursor->numKeys - 1] = NULL;
        //transfer
        cursor->keys[cursor->numKeys - 1] = rightNode->keys[0];
        //shift pointer of rightsibling
        rightNode->numKeys--;
        rightNode->pointers[rightNode->numKeys] = rightNode->pointers[rightNode->numKeys + 1];
        rightNode->pointers[rightNode->numKeys + 1] = NULL;
        //shift conent of right sibling
        for (int i = 0; i < rightNode->numKeys; i++)
        {
          rightNode->keys[i] = rightNode->keys[i + 1];
        }
        //update parent
        parent->keys[rightSibling - 1] = rightNode->keys[0];
        cout << "Transferred " << cursor->keys[cursor->numKeys - 1] << " from right sibling of leaf node\n";
        return;
      }
    }
    //must merge and delete a node
    if (leftSibling >= 0) //if left sibling exist
    {
      Node *leftNode = parent->pointers[leftSibling];
      // transfer all keys to leftsibling and then transfer pointer to next leaf node
      for (int i = leftNode->numKeys, j = 0; j < cursor->numKeys; i++, j++)
      {
        leftNode->keys[i] = cursor->keys[j];
      }
      leftNode->pointers[leftNode->numKeys] = NULL;
      leftNode->numKeys += cursor->numKeys;
      leftNode->pointers[leftNode->numKeys] = cursor->pointers[cursor->numKeys];
      cout << "Merging two leaf nodes\n";
      removeInternal(parent->keys[leftSibling], parent, cursor); // delete parent node keys
      delete[] cursor->keys;
      delete[] cursor->pointers;
      delete cursor;
    }
    else if (rightSibling <= parent->numKeys) //if right sibling exist
    {
      Node *rightNode = parent->pointers[rightSibling];
      // transfer all keys to cursor and then transfer pointer to next leaf node
      for (int i = cursor->numKeys, j = 0; j < rightNode->numKeys; i++, j++)
      {
        cursor->keys[i] = rightNode->keys[j];
      }
      cursor->pointers[cursor->numKeys] = NULL;
      cursor->numKeys += rightNode->numKeys;
      cursor->pointers[cursor->numKeys] = rightNode->pointers[rightNode->numKeys];
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
  //deleting the keys x first
  //checking if keys from root is to be deleted
  if (cursor == root)
  {
    if (cursor->numKeys == 1) //if only one keys is left, change root
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
  for (pos = 0; pos < cursor->numKeys; pos++)
  {
    if (cursor->keys[pos] == x)
    {
      break;
    }
  }
  for (int i = pos; i < cursor->numKeys; i++)
  {
    cursor->keys[i] = cursor->keys[i + 1];
  }
  //now deleting the pointer child
  for (pos = 0; pos < cursor->numKeys + 1; pos++)
  {
    if (cursor->pointers[pos] == child)
    {
      break;
    }
  }
  for (int i = pos; i < cursor->numKeys + 1; i++)
  {
    cursor->pointers[i] = cursor->pointers[i + 1];
  }
  cursor->numKeys--;
  if (cursor->numKeys >= (maxKeys + 1) / 2 - 1) //no underflow
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
  for (pos = 0; pos < parent->numKeys + 1; pos++)
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
    if (leftNode->numKeys >= (maxKeys + 1) / 2)
    {
      //make space for transfer of keys
      for (int i = cursor->numKeys; i > 0; i--)
      {
        cursor->keys[i] = cursor->keys[i - 1];
      }
      //transfer keys from left sibling through parent
      cursor->keys[0] = parent->keys[leftSibling];
      parent->keys[leftSibling] = leftNode->keys[leftNode->numKeys - 1];
      //transfer last pointer from leftnode to cursor
      //make space for transfer of pointers
      for (int i = cursor->numKeys + 1; i > 0; i--)
      {
        cursor->pointers[i] = cursor->pointers[i - 1];
      }
      //transfer pointers
      cursor->pointers[0] = leftNode->pointers[leftNode->numKeys];
      cursor->numKeys++;
      leftNode->numKeys--;
      cout << "Transferred " << cursor->keys[0] << " from left sibling of internal node\n";
      return;
    }
  }
  if (rightSibling <= parent->numKeys) //check if right sibling exist
  {
    Node *rightNode = parent->pointers[rightSibling];
    //check if it is possible to transfer
    if (rightNode->numKeys >= (maxKeys + 1) / 2)
    {
      //transfer keys from right sibling through parent
      cursor->keys[cursor->numKeys] = parent->keys[pos];
      parent->keys[pos] = rightNode->keys[0];
      for (int i = 0; i < rightNode->numKeys - 1; i++)
      {
        rightNode->keys[i] = rightNode->keys[i + 1];
      }
      //transfer first pointer from rightnode to cursor
      //transfer pointers
      cursor->pointers[cursor->numKeys + 1] = rightNode->pointers[0];
      for (int i = 0; i < rightNode->numKeys; ++i)
      {
        rightNode->pointers[i] = rightNode->pointers[i + 1];
      }
      cursor->numKeys++;
      rightNode->numKeys--;
      cout << "Transferred " << cursor->keys[0] << " from right sibling of internal node\n";
      return;
    }
  }
  //transfer wasnt posssible hence do merging
  if (leftSibling >= 0)
  {
    //leftnode + parent keys + cursor
    Node *leftNode = parent->pointers[leftSibling];
    leftNode->keys[leftNode->numKeys] = parent->keys[leftSibling];
    for (int i = leftNode->numKeys + 1, j = 0; j < cursor->numKeys; j++)
    {
      leftNode->keys[i] = cursor->keys[j];
    }
    for (int i = leftNode->numKeys + 1, j = 0; j < cursor->numKeys + 1; j++)
    {
      leftNode->pointers[i] = cursor->pointers[j];
      cursor->pointers[j] = NULL;
    }
    leftNode->numKeys += cursor->numKeys + 1;
    cursor->numKeys = 0;
    //delete cursor
    removeInternal(parent->keys[leftSibling], parent, cursor);
    cout << "Merged with left sibling\n";
  }
  else if (rightSibling <= parent->numKeys)
  {
    //cursor + parent keys + rightnode
    Node *rightNode = parent->pointers[rightSibling];
    cursor->keys[cursor->numKeys] = parent->keys[rightSibling - 1];
    for (int i = cursor->numKeys + 1, j = 0; j < rightNode->numKeys; j++)
    {
      cursor->keys[i] = rightNode->keys[j];
    }
    for (int i = cursor->numKeys + 1, j = 0; j < rightNode->numKeys + 1; j++)
    {
      cursor->pointers[i] = rightNode->pointers[j];
      rightNode->pointers[j] = NULL;
    }
    cursor->numKeys += rightNode->numKeys + 1;
    rightNode->numKeys = 0;
    //delete cursor
    removeInternal(parent->keys[rightSibling - 1], parent, rightNode);
    cout << "Merged with right sibling\n";
  }
}
void BPTree::display(Node *cursor)
{
  //depth first display
  if (cursor != NULL)
  {
    for (int i = 0; i < cursor->numKeys; i++)
    {
      cout << cursor->keys[i] << " ";
    }
    cout << "\n";
    if (cursor->isLeaf != true)
    {
      for (int i = 0; i < cursor->numKeys + 1; i++)
      {
        display(cursor->pointers[i]);
      }
    }
  }
}
Node *BPTree::getRoot()
{
  return root;
}
void BPTree::cleanUp(Node *cursor)
{
  //clean up logic
  if (cursor != NULL)
  {
    if (cursor->isLeaf != true)
    {
      for (int i = 0; i < cursor->numKeys + 1; i++)
      {
        cleanUp(cursor->pointers[i]);
      }
    }
    for (int i = 0; i < cursor->numKeys; i++)
    {
      cout << "Deleted keys from memory: " << cursor->keys[i] << "\n";
    }
    delete[] cursor->keys;
    delete[] cursor->pointers;
    delete cursor;
  }
}
BPTree::~BPTree()
{
  //calling cleanUp routine
  cleanUp(root);
}
