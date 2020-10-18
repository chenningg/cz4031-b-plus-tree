#include <vector>
using namespace std;

typedef struct TreeNode
{
    TreeNode *parent;
    vector<int> keys;
    vector<TreeNode *> pointer;
    TreeNode *buffer;
    bool isLeaf;
    bool isEmpty;
} TreeNode;

// class BPlusTree
// {
// public:
// };