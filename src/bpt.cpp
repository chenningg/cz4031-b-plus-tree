#include "bpt.h"

#include <iostream>
#include <tuple>
#include <sstream>
#include <string.h>
#include <cmath>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <array>
#include <limits.h>

using namespace std;

int NULL_VALUE = INT_MAX;

TreeNode *initTreeNode(int n, bool isLeaf)
{
  TreeNode *bptnode = new TreeNode;
  bptnode->parent = NULL;
  bptnode->keys = vector<int>(n, NULL_VALUE);
  bptnode->pointer = vector<TreeNode *>(n + 1);
  bptnode->isLeaf = isLeaf;
  bptnode->isEmpty = false;
  bptnode->buffer = NULL;
  return bptnode;
}

void updateEmptyState(TreeNode *parent, TreeNode *child, int value)
{
  if (parent)
  {
    bool found = false;
    for (int i = 1; i < parent->pointer.size(); i++)
    {
      if (parent->pointer[i] == child)
      {
        parent->keys[i - 1] = value;
        found = true;
      }
    }
    if (parent->isEmpty && found)
    {
      updateEmptyState(parent->parent, parent, value);
    }
  }
}

int insert_leaf(TreeNode *nnode, TreeNode *bptnode)
{
  nnode->pointer[bptnode->keys.size()] = bptnode->pointer[bptnode->keys.size()];
  bptnode->pointer[bptnode->keys.size()] = nnode;
  int total = bptnode->keys.size() + 1;
  return (int)ceil(total / 2); // return min
}

array<int, 2> insert_internal(TreeNode *nnode, TreeNode *bptnode, vector<int> keyTemps, vector<TreeNode *> pointerTemps)
{
  int total = bptnode->keys.size() + 2;
  int min = (int)ceil((total) / 2);

  for (int i = 0; i < pointerTemps.size(); i++)
  {
    if (i < min)
    {
      bptnode->pointer[i] = pointerTemps[i];
    }
    else
    {
      nnode->pointer[i - min] = pointerTemps[i];
      nnode->pointer[i - min]->parent = nnode;
      if (i <= bptnode->keys.size())
        bptnode->pointer[i] = NULL;
    }
  }
  min -= 1;
  int newKey = keyTemps[min];
  keyTemps.erase(keyTemps.begin() + min);
  array<int, 2> res;
  res[0] = min;
  res[1] = newKey;
  return res;
}

void splitKeys(TreeNode *nnode, TreeNode *bptnode, vector<int> keyTemps, int min, int indexTemp, int value)
{

  // Split Keys
  for (int i = 0; i < keyTemps.size(); i++)
  {
    if (i >= min)
    {
      nnode->keys[i - min] = keyTemps[i];
      if (i < bptnode->keys.size())
        bptnode->keys[i] = NULL_VALUE;
    }
    else
      bptnode->keys[i] = keyTemps[i];
  }

  // Update the empty state
  if (bptnode->isEmpty && value != bptnode->keys[0] && indexTemp < min)
  {
    bptnode->isEmpty = false;
    updateEmptyState(bptnode->parent, bptnode, value);
  }
}

TreeNode *insertNode(TreeNode *bptnode, int value)
{
  TreeNode *root = NULL;

  // CASE: Node is not full, we can insert
  if (bptnode->keys[bptnode->keys.size() - 1] == NULL_VALUE)
  {
    bool insert = false;
    int keySwap = NULL_VALUE;
    TreeNode *tp = NULL;
    for (int i = 0; i < bptnode->keys.size(); i++)
    {
      if (!insert)
      {
        if (value < bptnode->keys[i] || bptnode->keys[i] == NULL_VALUE)
        {
          insert = true;
          keySwap = bptnode->keys[i];
          bptnode->keys[i] = value;
          if (!bptnode->isLeaf)
          {
            tp = bptnode->pointer[i + 1];
            bptnode->pointer[i + 1] = bptnode->buffer;
          }
        }
        if (value != bptnode->keys[0] && bptnode->isEmpty)
        {
          updateEmptyState(bptnode->parent, bptnode, value);
          bptnode->isEmpty = false;
        }
      }
      else
      {
        int temp = bptnode->keys[i];
        bptnode->keys[i] = keySwap;
        keySwap = temp;
        if (!bptnode->isLeaf)
        {
          TreeNode *tempp = bptnode->pointer[i + 1];
          bptnode->pointer[i + 1] = tp;
          tp = tempp;
        }
      }
    }
  }
  // Node is full
  else
  {
    vector<int> keyTemps = bptnode->keys;
    vector<TreeNode *> pointerTemps = bptnode->pointer;
    // Find index to insert key
    int indexTemp = upper_bound(keyTemps.begin(), keyTemps.end(), value) - keyTemps.begin();
    keyTemps.insert(keyTemps.begin() + indexTemp, value);
    if (!bptnode->isLeaf)
    {
      pointerTemps.insert(pointerTemps.begin() + indexTemp + 1, bptnode->buffer);
    }

    TreeNode *nnode = initTreeNode(bptnode->keys.size(), bptnode->isLeaf);
    nnode->parent = bptnode->parent;

    // Leaf
    int min, newKey;
    if (bptnode->isLeaf)
    {
      min = insert_leaf(nnode, bptnode);
    }

    // Node is internal node
    else
    {
      auto res = insert_internal(nnode, bptnode, keyTemps, pointerTemps);
      min = res[0], newKey = res[1];
    }

    splitKeys(nnode, bptnode, keyTemps, min, indexTemp, value);

    auto start = nnode->keys.begin();
    auto end = nnode->keys.end();
    indexTemp = upper_bound(start, end, bptnode->keys[min - 1]) - start;
    if (nnode->keys[indexTemp] == NULL_VALUE)
    {
      newKey = nnode->keys[0];
      nnode->isEmpty = true;
    }
    else if (bptnode->isLeaf)
    {
      newKey = nnode->keys[indexTemp];
    }

    if (bptnode->parent)
    {
      // update with new value
      bptnode->parent->buffer = nnode;
      root = insertNode(bptnode->parent, newKey);
    }
    else
    // create new root
    {
      root = initTreeNode(bptnode->keys.size(), false);
      root->keys[0] = newKey;
      root->pointer[0] = bptnode;
      root->pointer[1] = nnode;
      bptnode->parent = root;
      nnode->parent = root;
    }
  }
  return root;
}

TreeNode *getLeaf(TreeNode *bptnode, int value, bool f)
{
  while (!bptnode->isLeaf)
  {
    int low = INT_MIN;
    int j;
    for (int i = 0; i < bptnode->keys.size(); i++)
    {
      if (bptnode->keys[i] == NULL_VALUE)
      {
        j = i;
        break;
      }
      int high = bptnode->keys[i];
      if ((low <= high && value == high && !f && bptnode->pointer[i + 1]->isEmpty) || (low <= value && value < high))
      {
        j = i;
        break;
      }
      else
      {
        j = i + 1;
      }
      low = high;
    }
    bptnode = bptnode->pointer[j];
  }
  return bptnode;
}

TreeNode *buildTree(TreeNode *root, int value)
{
  TreeNode *bptnode = insertNode(getLeaf(root, value, true), value);
  if (bptnode)
  {
    root = bptnode;
  }
  return root;
}

void getRange(TreeNode *leaf, int low, int high)
{
  int count = 0, endReached = false;
  float total = 0;
  while (leaf)
  {
    for (int i = 0; i < leaf->keys.size(); i++)
    {
      if (leaf->keys[i] >= low && leaf->keys[i] <= high)
      {
        total += leaf->keys[i];
        count++;
      }
      else if (leaf->keys[i] > high && leaf->keys[i] != NULL_VALUE)
      {
        endReached = true;
        break;
      }
    }
    if (endReached)
      break;
    leaf = leaf->pointer[leaf->pointer.size() - 1];
  }
  cout << "\ntotal: ";
  cout << total;
  cout << "\ncount: ";
  cout << count;
  cout << "\naverage: ";
  cout << total / count << endl;
  return;
}

void displayTree(TreeNode *bptnode, int level)
{
  if (!bptnode)
    return;

  if (!bptnode->isLeaf)
  {
    for (int i = 0; i < bptnode->pointer.size(); i++)
      displayTree(bptnode->pointer[i], level + 1);
  }
  else
  {
    cout << "|-> ";
  }
  cout << "|";
  for (int i = 0; i < bptnode->keys.size(); i++)
  {
    if (bptnode->keys[i] != NULL_VALUE)
    {
      cout << bptnode->keys[i] << "|";
    }
    else
    {
      cout << "x|";
    }
  }
  cout << endl;
}
// set the maximum to 5
int bpt_test()
{
  int n = 5;
  TreeNode *root = initTreeNode(n, true);
  buildTree(root, 1);
  buildTree(root, 2);
  buildTree(root, 3);
  buildTree(root, 4);
  buildTree(root, 5);
  buildTree(root, 6);
  buildTree(root, 7);
  buildTree(root, 8);
  buildTree(root, 9);
  TreeNode *tempRoot = buildTree(root, 10);
  int rangeStart = 0;
  int rangeEnd = 10;
  getRange(getLeaf(root, rangeStart, false), rangeStart, rangeEnd);
  displayTree(tempRoot, 0);
}
