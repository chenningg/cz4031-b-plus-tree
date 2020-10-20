// B+ tree in C++

#include <iostream>
#include <sstream>
#include <cmath>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include "types.h"

using namespace std;
int MAX_KEYS = 3;

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
		void removeInternal(int, Node *, Node *);
		Node *findParent(Node *, Node *);

	public:
		BPTree();
		void insert(int);
		void remove(int);
		Node *search(int, int);
		//   void searchRange(int, int);
		void display(Node *, int level);
		void displayLL(Node *);
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
Node* BPTree::search(int lower, int upper) {
	if (root != NULL) {
		Node *cursor = root;
		bool found = false;

		// still within search range
		while (lower <= upper && !found) {
			// try and find the leaf node that should contain lower 
			while (!cursor->IS_LEAF) {
				for (int i = 0; i < cursor->num_keys; i++) {
					if (lower < cursor->keys[i]) {
						cursor = cursor->pointers[i];
						break;
					}
					if (i == cursor->num_keys - 1) {
						cursor = cursor->pointers[i + 1];
						break;
					}
				}
			}
			
			// check if lower is actually contained
			for (int i = 0; i < cursor->num_keys; i++) {
				// if leaf node contains lower
				if (cursor->keys[i] == lower) {
					cout << "\nFound: ";
					found = true;
					break;
				} 
			}

			// lower not contained, increment until we hit lower > upper
			if (!found) {
				lower++;
			}
		}

		if (lower > upper) {
			cout << "Not found\n";
			return NULL;
		} 

		else {
			// find the index of lower inside this cursor that is guaranteed to contain lower 
			int index;
			for (int i = 0; i < cursor->num_keys; i++) {
				if (cursor->keys[i] == lower) {
					index = i;
					break;
				}
			}

			bool exceedRange = false;

			// as long as we are still within range of the upper bound
			while (!exceedRange) {
				
				// check if we have gone beyond upper. if we have, end this function 
				if (cursor->keys[index] > upper) {
					exceedRange = true;
					break;
				}
				
				displayLL(cursor->pointers[index]);
				// we are at the last key already
				if (index+1 == cursor->num_keys) {
					// if there is no pointer from this bptree node to another leaf, end
					if (cursor->pointers[cursor->num_keys] == NULL) {
						exceedRange = true;
						break;
					} 
					// else we move to the next leaf 
					else {
						cursor = cursor->pointers[cursor->num_keys];
						index = 0;
					}
				}
				// safe to increment index since we are still within the leaf node
				else {
					index++;
				}
			}
		}
	}
	else {
		cout << "Tree is empty\n";
		return NULL;
	}
}


// Insert Operation
void BPTree::insert(int x) {
	if (root != NULL) {
		Node *cursor = root;
		Node *parent;

		// search downwards until we reach the leaf node 
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

		// leaf node can store the new key
		if (cursor->num_keys < MAX_KEYS) {
			int i = 0;
			while (x > cursor->keys[i] && i < cursor->num_keys)
				i++;

			// check if the identified position has a duplicate key value
			if (cursor->keys[i] == x) {	// duplicate key
				// add onto the head of the linked list
				if (cursor->pointers[i]->num_keys < MAX_KEYS) { // linked list head has space
					for (int j = MAX_KEYS; j > 0; j--) {
						cursor->pointers[i]->pointers[j] = cursor->pointers[i]->pointers[j - 1];
						cursor->pointers[i]->keys[j] = cursor->pointers[i]->keys[j - 1];
					}
					cursor->pointers[i]->keys[0] = x;
					cursor->pointers[i]->pointers[0] = NULL; // the disk address of the key just inserted
					cursor->pointers[i]->num_keys++;
				}
				else { // linked list head has no space, make new linked list node
					Node *LLNode = new Node;
					LLNode->IS_LEAF = false;
					LLNode->keys[0] = x;
					LLNode->num_keys = 1;
					LLNode->pointers[0] = NULL;				   // the disk address of the key just inserted
					LLNode->pointers[1] = cursor->pointers[i]; // the address of the previous leaf node put as the second pointer

					cursor->pointers[i] = LLNode; // set the cursor to point to the new leaf node
					// cout << "\naddress of new head for LL for integer " << x << ": " << LLNode;
				}
		} 
		
		else { // new key 
			for (int j = cursor->num_keys; j > i; j--) {
				cursor->keys[j] = cursor->keys[j - 1];
			}
			cursor->keys[i] = x;
			cursor->num_keys++;
			cursor->pointers[cursor->num_keys] = cursor->pointers[cursor->num_keys - 1];
			cursor->pointers[cursor->num_keys - 1] = NULL;

			// create a linked-list node for the leaf to link to
			Node *LLNode = new Node;
			LLNode->IS_LEAF = false;
			LLNode->keys[0] = x;
			LLNode->num_keys = 1;
			LLNode->pointers[0] = NULL; // the disk address of the key just inserted

			cursor->pointers[i] = LLNode;
			// cout << "\naddress of LL for integer " << x << ": " << LLNode;
		}

		}


		// leaf node cannot store the new key since maxed out
		else {
		int i = 0;
		while (x > cursor->keys[i] && i < cursor->num_keys)
			i++;
		
		// check if the identified position has a duplicate key value 
		if (cursor->keys[i] == x) { // duplicate key
			// add onto the head of the linked list 
			if (cursor->pointers[i]->num_keys < MAX_KEYS) { // linked list head has space
				for (int j = cursor->pointers[i]->num_keys; j > 0; j--) {
					cursor->pointers[i]->pointers[j] = cursor->pointers[i]->pointers[j-1];
					cursor->pointers[i]->keys[j] = cursor->pointers[i]->keys[j - 1];
				}
				cursor->pointers[i]->keys[0] = x;
				cursor->pointers[i]->pointers[0] = NULL; // the disk address of the key just inserted
				cursor->pointers[i]->num_keys++;

			} else { // linked list head has no space, make new linked list node
				Node *LLNode = new Node;
				LLNode->IS_LEAF = false;
				LLNode->keys[0] = x;
				LLNode->num_keys = 1;
				LLNode->pointers[0] = NULL; // the disk address of the key just inserted			
				LLNode->pointers[1] = cursor->pointers[i]; // the address of the previous leaf node put as the second pointer
				
				cursor->pointers[i] = LLNode; // set the cursor to point to the new leaf node
				// cout << "\naddress of new head for LL for integer " << x << ": " << LLNode;
			}
		} else { // new key 
			Node *newLeaf = new Node;

			int tempKeyList[MAX_KEYS + 1];
			Node *tempPointerList[MAX_KEYS + 2];
			for (int i = 0; i < MAX_KEYS; i++) {
				tempKeyList[i] = cursor->keys[i];
			}
			for (int i = 0; i < MAX_KEYS + 1; i++) {
				tempPointerList[i] = cursor->pointers[i];
			}		
			int i = 0, j;
			while (x > tempKeyList[i] && i < MAX_KEYS)
				i++;

			for (int j = MAX_KEYS + 1; j > i; j--) {
				tempKeyList[j] = tempKeyList[j - 1];
			}
			tempKeyList[i] = x;
			for (int j = MAX_KEYS + 2; j > i + 1; j--) {
				tempPointerList[j] = tempPointerList[j - 1];
			}

			// create a new linked list node for the new key
			Node *LLNode = new Node;
			LLNode->IS_LEAF = false;
			LLNode->keys[0] = x;
			LLNode->num_keys = 1;
			LLNode->pointers[0] = NULL; // the disk address of the key just inserted			
			tempPointerList[i] = LLNode; // set the new LL to be in the sorted temp order of LL
			// cout << "\naddress of new head for LL for integer " << x << ": " << LLNode;

			newLeaf->IS_LEAF = true;
			cursor->num_keys = (MAX_KEYS + 1) / 2;
			newLeaf->num_keys = MAX_KEYS + 1 - (MAX_KEYS + 1) / 2;

			// assigning the correct pointers and keys
			for (i = 0, j = cursor->num_keys; i < newLeaf->num_keys; i++, j++) {
				newLeaf->keys[i] = tempKeyList[j];
			}
			for (i = 0, j = cursor->num_keys ; i < newLeaf->num_keys; i++, j++) {
				newLeaf->pointers[i] = tempPointerList[j];
			}		
			cursor->pointers[cursor->num_keys] = newLeaf;

			for (i = cursor->num_keys+1; i < MAX_KEYS+1; i++) {
				cursor->pointers[i] = NULL;
			}
			for (i = newLeaf->num_keys; i < MAX_KEYS+1; i++) {
				newLeaf->pointers[i] = NULL;
			}		

			// can just create a new root node with two children
			if (cursor == root) {
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
			
			// must insert a new internal node since there is a parent
			else {
				insertInternal(newLeaf->keys[0], parent, newLeaf);
			}
		}
		}
	}
  	else {
		root = new Node;
		root->IS_LEAF = true;
		root->num_keys = 1;
		root->keys[0] = x;

		// create a linked-list node for the leaf to link to
		Node *LLNode = new Node;
		LLNode->IS_LEAF = false;
		LLNode->keys[0] = x;
		LLNode->num_keys = 1;
		LLNode->pointers[0] = NULL; // the disk address of the key just inserted

		root->pointers[0] = LLNode;
		// cout << "\naddress of LL for integer " << x << ": " << LLNode;
	}
}

// Insert Operation
void BPTree::insertInternal(int x, Node *cursor, Node *child) {
	// can save to a parent node 
	if (cursor->num_keys < MAX_KEYS) {
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
	// cannot save to a parent node
	else {
		// make a new one 
		Node *newInternal = new Node;

		// temporary holders
		int tempKeyList[MAX_KEYS + 1];
		Node *tempPointerList[MAX_KEYS + 2];
		for (int i = 0; i < MAX_KEYS; i++) {
			tempKeyList[i] = cursor->keys[i];
		}
		for (int i = 0; i < MAX_KEYS + 1; i++) {
			tempPointerList[i] = cursor->pointers[i];
		}
		int i = 0, j;
		while (x > tempKeyList[i] && i < MAX_KEYS)
			i++;

		for (int j = MAX_KEYS + 1; j > i; j--) {
			tempKeyList[j] = tempKeyList[j - 1];
		}
		tempKeyList[i] = x;
		for (int j = MAX_KEYS + 2; j > i + 1; j--) {
			tempPointerList[j] = tempPointerList[j - 1];
		}
		tempPointerList[i + 1] = child;

		// split across the two nodes
		newInternal->IS_LEAF = false;
		cursor->num_keys = (MAX_KEYS + 1) / 2;
		newInternal->num_keys = MAX_KEYS - (MAX_KEYS + 1) / 2;

		for (i = 0, j = cursor->num_keys + 1; i < newInternal->num_keys; i++, j++) {
			newInternal->keys[i] = tempKeyList[j];
		}
		for (i = 0, j = cursor->num_keys + 1; i < newInternal->num_keys + 1; i++, j++) {
			newInternal->pointers[i] = tempPointerList[j];
		}
		for (int i = cursor->num_keys; i < MAX_KEYS + 1; i++) {
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
		}
		else {
			insertInternal(cursor->keys[cursor->num_keys], findParent(root, cursor), newInternal);
		}
  }
}

// Find the parent
Node *BPTree::findParent(Node *cursor, Node *child)
{
  Node *parent;
  if (cursor->IS_LEAF || (cursor->pointers[0])->IS_LEAF) {
    return NULL;
  }
  for (int i = 0; i < cursor->num_keys + 1; i++) {
    if (cursor->pointers[i] == child) {
      parent = cursor;
      return parent;
    }
    else {
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
void BPTree::remove(int x) {
	//delete logic
	if (root == NULL) {
		cout<<"Tree empty\n";
	}
	else {
		Node* cursor = root;
		Node* parent;
		int leftSibling, rightSibling;
		
		// in the following while loop, cursor will will travel to the leaf node possibly consisting the key
		while (!cursor->IS_LEAF) {
			for (int i = 0; i < cursor->num_keys; i++) {
				parent = cursor;
				leftSibling = i-1; // leftSibling is the index of left sibling in the parent node
				rightSibling =  i+1; // rightSibling is the index of right sibling in the parent node
				if (x < cursor->keys[i]) {	
					cursor = cursor->pointers[i];
					break;
				}
				if (i == cursor->num_keys - 1) {
					leftSibling = i;
					rightSibling = i+2;
					cursor = cursor->pointers[i+1];
					break;
				}
			}
		}
		
		// in the following for loop, we search for the key if it exists
		bool found = false;
		int pos;
		for (pos = 0; pos < cursor->num_keys; pos++) {
			if (cursor->keys[pos] == x) {
				found = true;
				break;
			}
		}

		if (!found) { // if keys does not exist in that leaf node 
			cout<<"Not found\n";
			return;
		}
					


		// bomb the link list at position found
		Node *tempPointer = cursor->pointers[pos];

		while (tempPointer != NULL) {
			for (int i = 0; i < tempPointer->num_keys; i++)  {
				tempPointer->keys[i] = NULL; // remove the key from the LLNode
				tempPointer->pointers[i] = NULL; // remove the record from the disk space
			}

			if (tempPointer->pointers[tempPointer->num_keys] != NULL) {
				tempPointer = tempPointer->pointers[tempPointer->num_keys];
			} else {
				tempPointer = NULL;
			}
		}

		cout << "\nFinished bombing key " << x << "\n";

		// deleting the keys and shifting the pointers
		for (int i = pos; i < cursor->num_keys; i++) {
			cursor->keys[i] = cursor->keys[i+1];
		}
		for (int i = pos; i < cursor->num_keys + 1; i++) {
			cursor->pointers[i] = cursor->pointers[i + 1];
		}
		cursor->num_keys--;			
		
		if (cursor == root) { //if it is root node 
			cout<<"Deleted "<<x<<" from leaf node successfully\n";
			
			// for (int i = 0; i < MAX_KEYS+1; i++) {
			// 	cursor->pointers[i] = NULL;
			// }

			if (cursor->num_keys == 0) { //if all keys are deleted 
				cout<<"Tree died\n";
				delete[] cursor->keys;
				delete[] cursor->pointers;
				delete cursor;
				root = NULL;
			}
			return;
		}

		
		// deleting from a leaf that is not root 
		if (cursor->num_keys >= (MAX_KEYS + 1) / 2) { //no underflow
			return;
		}

		cout << "Underflow in leaf node!\n";
		//underflow condition
		//first we try to transfer a keys from sibling node
		//check if left sibling exists


		if (leftSibling >= 0) {
			Node *leftNode = parent->pointers[leftSibling];
			
			//check if it is possible to transfer
			if (leftNode->num_keys >= (MAX_KEYS+1)/2+1) {
				//make space for transfer
				for (int i = cursor->num_keys; i > 0; i--) {
					cursor->keys[i] = cursor->keys[i-1];
				}

				// shift pointers of cursor up one to make space on the left 
				for (int i = cursor->num_keys; i > 0; i--) {
					cursor->pointers[i + 1] = cursor->pointers[i];
				}				
				cursor->num_keys++;
							
							// //shift pointer to next leaf
							// cursor->num_keys++;
							// cursor->pointers[cursor->num_keys] = cursor->pointers[cursor->num_keys-1];
							// cursor->pointers[cursor->num_keys-1] = NULL;
				
				//transfer key and pointer from left node 
				cursor->keys[0] = leftNode->keys[leftNode->num_keys-1];
				cursor->pointers[0] = leftNode->pointers[leftNode->num_keys - 1];

				//shift pointer of leftsibling
				leftNode->num_keys--;
				leftNode->pointers[leftNode->num_keys] = cursor;
				leftNode->pointers[leftNode->num_keys+1] = NULL;
			
				//update parent
				parent->keys[leftSibling] = cursor->keys[0];
				cout<<"Transferred "<<cursor->keys[0]<<" from left sibling of leaf node\n";
				return;
			}
		}

		if (rightSibling <= parent->num_keys) { //check if right sibling exist 
			Node *rightNode = parent->pointers[rightSibling];

			cout << "\nMerge 7aaaaaaaaaaa rightNode ";
			for (int i = 0; i < rightNode->num_keys; i++) {
				cout << " " << rightNode->keys[i];
			}	
			cout << "\n cursors ";
			for (int i = 0; i < rightNode->num_keys+1; i++) {
				cout << " " << rightNode->pointers[i];
			}				

			//check if it is possible to transfer
			if (rightNode->num_keys >= (MAX_KEYS+1)/2+1) {
				//shift pointer to next index location in the cursor node 
				cursor->num_keys++;
				cursor->pointers[cursor->num_keys] = cursor->pointers[cursor->num_keys-1];
				cursor->pointers[cursor->num_keys-1] = NULL;

				//transfer the key and pointer over from the right node 
				cursor->keys[cursor->num_keys-1] = rightNode->keys[0];
				cursor->pointers[cursor->num_keys - 1] = rightNode->pointers[0];

				//shift pointers of rightsibling
				rightNode->num_keys--;

				for (int i = 0; i < rightNode->num_keys+1; i++) {
					rightNode->pointers[i] = rightNode->pointers[i + 1];
				}
				
				// rightNode->pointers[rightNode->num_keys] = rightNode->pointers[rightNode->num_keys + 1];
				// rightNode->pointers[rightNode->num_keys+1] = NULL;
				
				//shift keys of right sibling
				for (int i = 0; i < rightNode->num_keys; i++) {
					rightNode->keys[i] = rightNode->keys[i+1];
				}

				rightNode->pointers[rightNode->num_keys + 1] = NULL; // delete the record from disk
				rightNode->keys[rightNode->num_keys ] = NULL; // delete the key from node 

				//update parent
				parent->keys[rightSibling-1] = rightNode->keys[0];
				cout<<"Transferred "<<cursor->keys[cursor->num_keys-1]<<" from right sibling of leaf node\n";
				return;
			}
		}

		// // print current cursor state
		// cout << "\nMerge 5 ";
		// for (int i = 0; i < cursor->num_keys; i++) {
		// 	cout << " " << cursor->keys[i];
		// }	
		// cout << "\n cursors ";
		// for (int i = 0; i < cursor->num_keys+1; i++) {
		// 	cout << " " << cursor->pointers[i];
		// }	

		//must merge and delete a node
		if (leftSibling >= 0) { //if left sibling exist
			Node* leftNode = parent->pointers[leftSibling];	

			// transfer all keys to leftsibling 
			for (int i = leftNode->num_keys, j = 0; j < cursor->num_keys; i++, j++) {
				leftNode->keys[i] = cursor->keys[j];
			}
			// transfer all pointers to leftsibling 
			for (int i = leftNode->num_keys, j = 0; j < cursor->num_keys+1; i++, j++) {
				leftNode->pointers[i] = cursor->pointers[j];
			}

			leftNode->num_keys += cursor->num_keys;

			cout << "Merging two leaf nodes\n";
			
			
			// KIVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
			removeInternal(parent->keys[leftSibling],parent,cursor);// delete parent node keys
			delete[] cursor->keys;
			delete[] cursor->pointers;
			delete cursor;
		}
		else if (rightSibling <= parent->num_keys) { //if right sibling exist
			Node* rightNode = parent->pointers[rightSibling];
			
			// transfer all keys to cursor and then transfer pointer to next leaf node
			for (int i = cursor->num_keys, j = 0; j < rightNode->num_keys; i++, j++) {
				cursor->keys[i] = rightNode->keys[j];
			}
			// transfer all pointers from rightsibling to cursor
			for (int i = cursor->num_keys, j = 0; j < rightNode->num_keys+1; i++, j++) {
				cursor->pointers[i] = rightNode->pointers[j];
			}

			cursor->num_keys += rightNode->num_keys;

			cout<<"Merging two leaf nodes\n";
			


			// KIVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
			removeInternal(parent->keys[rightSibling-1],parent,rightNode);// delete parent node keys
			delete[] rightNode->keys;
			delete[] rightNode->pointers;
			delete rightNode;
		}
	}
}
void BPTree::removeInternal(int x, Node* cursor, Node* child) {
	//deleting the key x first
	//checking if key from root is to be deleted
	
	if (cursor == root)	{
		if (cursor->num_keys == 1) { //if only one key is left, change root
			if (cursor->pointers[1] == child) {			

				// KIVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVvvvv
				delete[] child->keys;
				delete[] child->pointers;
				delete child;
				root = cursor->pointers[0];
				delete[] cursor->keys;
				delete[] cursor->pointers;
				delete cursor;
				cout<<"Changed root node\n";
				return;
			}
			else if (cursor->pointers[0] == child) {
				delete[] child->keys;
				delete[] child->pointers;
				delete child;
				root = cursor->pointers[1];
				delete[] cursor->keys;
				delete[] cursor->pointers;
				delete cursor;
				cout<<"Changed root node\n";
				return;
			}
		}
	}
	
	int pos;
	for (pos = 0; pos < cursor->num_keys; pos++) {
		if (cursor->keys[pos] == x)	{
			break;
		}
	}

	// move keys and pointers forward to fill the deleted spot
	for (int i = pos; i < cursor->num_keys; i++) {
		cursor->keys[i] = cursor->keys[i+1];
	}

	//now deleting the pointer child
	for (pos = 0; pos < cursor->num_keys + 1; pos++) {
		if (cursor->pointers[pos] == child) {
			break;
		}
	}
	for (int i = pos; i < cursor->num_keys+1; i++) {
		cursor->pointers[i] = cursor->pointers[i+1];
	}
	// cursor->pointers[cursor->num_keys] = NULL;		// maybe remove 
	cursor->num_keys--;
	
	if (cursor->num_keys >= (MAX_KEYS+1)/2-1) {//no underflow
		cout<<"Deleted "<<x<<" from internal node successfully\n";
		return;
	}
	cout<<"Underflow in internal node!\n";
	
	//underflow, try to transfer first
	if (cursor==root)
		return;
	
	Node* parent = findParent(root, cursor);
	int leftSibling, rightSibling;
	
	//finding left n right sibling of cursor
	for (pos = 0; pos < parent->num_keys+1; pos++) {
		if (parent->pointers[pos] == cursor) {
			leftSibling = pos - 1;
			rightSibling = pos + 1;
			break;
		}
	}
	
	//try to transfer
	if (leftSibling >= 0) { //if left sibling exists
		Node *leftNode = parent->pointers[leftSibling];
		//check if it is possible to transfer
		if (leftNode->num_keys >= (MAX_KEYS+1)/2) {
			//make space for transfer of keys
			for (int i = cursor->num_keys; i > 0; i--) {
				cursor->keys[i] = cursor->keys[i-1];
			}
			//transfer keys from left sibling through parent
			cursor->keys[0] = parent->keys[leftSibling];
			parent->keys[leftSibling] = leftNode->keys[leftNode->num_keys-1];
			//transfer last pointer from leftnode to cursor
			//make space for transfer of ptr
			for (int i = cursor->num_keys+1; i > 0; i--) {
				cursor->pointers[i] = cursor->pointers[i-1];
			}
			//transfer ptr
			cursor->pointers[0] = leftNode->pointers[leftNode->num_keys];
			cursor->num_keys++;
			leftNode->num_keys--;
			cout<<"Transferred "<<cursor->keys[0]<<" from left sibling of internal node\n";
			return;
		}
	}
	if (rightSibling <= parent->num_keys) { //check if right sibling exist
		Node *rightNode = parent->pointers[rightSibling];
		//check if it is possible to transfer
		if (rightNode->num_keys >= (MAX_KEYS+1)/2) {
			//transfer keys from right sibling through parent
			cursor->keys[cursor->num_keys] = parent->keys[pos];
			parent->keys[pos] = rightNode->keys[0];
			for (int i = 0; i < rightNode->num_keys -1; i++) {
				rightNode->keys[i] = rightNode->keys[i+1];
			}
			//transfer first pointer from rightnode to cursor
			//transfer ptr
			cursor->pointers[cursor->num_keys+1] = rightNode->pointers[0];
			for (int i = 0; i < rightNode->num_keys; ++i) {
				rightNode->pointers[i] = rightNode->pointers[i+1];
			}
			cursor->num_keys++;
			rightNode->num_keys--;
			cout<<"Transferred "<<cursor->keys[0]<<" from right sibling of internal node\n";
			return;
		}
	}
	//transfer wasnt posssible hence do merging
	if (leftSibling >= 0) {
		//leftnode + parent keys + cursor
		Node *leftNode = parent->pointers[leftSibling];
		leftNode->keys[leftNode->num_keys] = parent->keys[leftSibling];
		for (int i = leftNode->num_keys+1, j = 0; j < cursor->num_keys; j++) {
			leftNode->keys[i] = cursor->keys[j];
		}
		for (int i = leftNode->num_keys+1, j = 0; j < cursor->num_keys+1; j++) {
			leftNode->pointers[i] = cursor->pointers[j];
			cursor->pointers[j] = NULL;
		}
		leftNode->num_keys += cursor->num_keys+1;
		cursor->num_keys = 0;
		//delete cursor
		removeInternal(parent->keys[leftSibling], parent, cursor);
		cout<<"Merged with left sibling\n";

	}
	else if (rightSibling <= parent->num_keys) {
		//cursor + parent keys + rightnode
		Node *rightNode = parent->pointers[rightSibling];
		cursor->keys[cursor->num_keys] = parent->keys[rightSibling-1];
		
		for (int i = cursor->num_keys+1, j = 0; j < rightNode->num_keys; j++) {
			cursor->keys[i] = rightNode->keys[j];
		}
		for (int i = cursor->num_keys+1, j = 0; j < rightNode->num_keys+1; j++)	{
			cursor->pointers[i] = rightNode->pointers[j];
			rightNode->pointers[j] = NULL;
		}
		cursor->num_keys += rightNode->num_keys+1;
		rightNode->num_keys = 0;
		//delete cursor
		removeInternal(parent->keys[rightSibling-1], parent, rightNode);
		cout<<"Merged with right sibling\n";
	}
}


// Print the tree
void BPTree::display(Node *cursor, int level) {
	if (cursor != NULL) {
		// output level formatiing
		cout << cursor;
		for (int i = 0; i < level; i++) {
			cout << "   ";
		}
		cout << " level " << level << ": ";

		// output keys
		for (int i = 0; i < cursor->num_keys; i++) {
			cout << cursor->keys[i] << " ";
		}

		// output empty keys
		for (int i = cursor->num_keys; i < MAX_KEYS; i++) {
			cout << "x ";
		}

		// output pointers
		for (int i = 0; i < MAX_KEYS + 1; i++) {
			if (cursor->pointers[i] == NULL) {
				cout << "|       |";
			}
			else {
				cout << cursor->pointers[i] << " ";
			}
		}

		// go down the tree
		cout << "\n";
		if (cursor->IS_LEAF != true) {
			for (int i = 0; i < cursor->num_keys + 1; i++) {
				display(cursor->pointers[i], level + 1);
			}
		}
  }
}

// used to print an entire linked list for a given key
void BPTree::displayLL(Node *head) {
	if (head == NULL) {
		cout << "\nNo linked list\n";
	}
	else {
		for (int i = 0; i < head->num_keys; i++) {
			cout << head->keys[i] << " ";
		}
		for (int i = head->num_keys; i < MAX_KEYS; i++) {
			cout << "x ";
		}
		if (head->pointers[head->num_keys] != NULL) {
			displayLL(head->pointers[head->num_keys]);
		}
	}
}


// Get the root
Node *BPTree::getRoot() {
  return root;
}

int bpt_2() {
	BPTree node;

	// for (int i = 0; i < 20; i++) {
	// 	node.insert(i);
	// 	node.insert(i);
	// }
	// cout << "\n\n";
	// node.display(node.getRoot(), 1);
	// cout << "\n\n";

	// node.remove(0);
	// node.remove(1);
	// node.remove(2);
	// node.remove(3);
	// node.remove(4);
	// node.remove(5);



	// Create a list of all addresses

	// Open test data
	ifstream file("../data/testdata.tsv");

	// Insert data into database and populate list of addresses
	if (file.is_open()) {
		cout << "\nFile is open\n";

		string line;

		while (getline(file, line)) {
			Record temp;
			stringstream linestream(line);
			string data;

			strcpy(temp.tconst, line.substr(0, line.find("\t")).c_str());

			getline(linestream, data, '\t');
			linestream >> temp.averageRating >> temp.numVotes;

			int tempRating = temp.averageRating * 10;

			// cout << "tempRating: " << tempRating << "\n";
			// for (int i = 0; i < 60;i++){
			// 	node.insert(i);
			// }
				node.insert(tempRating);
			node.display(node.getRoot(), 1);
		}
		cout << "\nFile closed\n";
	}

	cout << "hello";
	cout << "\n\n";
	node.display(node.getRoot(), 1);
	cout << "\n\n";

}