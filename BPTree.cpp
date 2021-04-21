#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Node {
 public:
  bool isLeaf;
  Node* parentNode;
  vector<Node*> child;
  vector<int> key;
  int numKey;
  Node* nextLeaf;
  Node(int order) {
    this->key = vector<int>(order);
    this->child = vector<Node*>(order, NULL);
    this->isLeaf = false;
    this->parentNode = NULL;
    this->numKey = 0;
    this->nextLeaf = NULL;
  }
};

class BPlusTree {
  Node* root;
  int order;
  int leafOrder;
  bool dataFound;
  void redistributeNode(Node* leftNode, Node* rightNode, bool isLeaf,
                        int posOfLeftChild, int currNodeL0R1);
  void deleteKeyRec(Node* currNode, int key, int currPos);
  void mergeBlock(Node* leftNode, Node*& rightNode, bool isLeaf,
                  int posOfRight);

 public:
  BPlusTree(int order, int leafOrder)
      : root(NULL), order(order), leafOrder(leafOrder) {}
  void search(int key);
  void search_range(int key1, int key2);
  void range(int key1, int key2);
  void insert(int key);
  void deleteKey(int key);
  void print(void);
};
void printVec(vector<int> A);
void commandParser(BPlusTree& tree);

int main() {
  try {
    cout << "B+ Tree Program\n";
    cout << "Enter order and leafOrder: ";
    int order, leafOrder;
    cin >> order >> leafOrder;
    BPlusTree tree(order, leafOrder);
    commandParser(tree);
  } catch (const char* err) {
    cout << "[ERROR] " << err << "\n";
  }
  return 0;
}

void commandParser(BPlusTree& tree) {
  cout << "Enter operation in the following format: \n";
  cout << "ins <num>\n";
  cout << "del <num>\n";
  cout << "search <num>\n";
  cout << "search_range <num1> <num2> (num1 <= num2)\n";
  cout << "exit (to terminate)\n";
  string op;
  int num1, num2;
  while (1) {
    try {
      cin >> op;
      if (op == "ins" || op == "INS") {
        cin >> num1;
        tree.insert(num1);
        cout << "Inserted " << num1 << "\n";
      } else if (op == "search" || op == "SEARCH") {
        cin >> num1;
        tree.search(num1);
      } else if (op == "search_range" || op == "SEARCH_RANGE") {
        cin >> num1 >> num2;
        tree.search_range(num1, num2);
      } else if (op == "print" || op == "PRINT") {
        tree.print();
      } else if (op == "exit" || op == "EXIT") {
        return;
      } else if (op == "del" || op == "DEL") {
        cin >> num1;
        tree.deleteKey(num1);
        cout << "Deleted " << num1 << "\n";
      }
    } catch (const char* err) {
      cout << "[ERROR] " << err << "\n";
    }
  }
}

void BPlusTree::deleteKey(int key) {
  this->dataFound = false;
  this->deleteKeyRec(this->root, key, 0);
}

void BPlusTree::deleteKeyRec(Node* currNode, int key, int currPos) {
  // backing up the current left most key
  int prevLeftMostkey = currNode->key[0];

  for (int i = 0; dataFound == false && i < currNode->numKey + 1; i++) {
    // recurse for the node containing the key
    if (key < currNode->key[i] && currNode->child[i] != NULL) {
      deleteKeyRec(currNode->child[i], key, i);
      this->print();
      break;
    }
    // if we could find the target key at any leafNode then delete it
    else if (key == currNode->key[i] && currNode->child[i] == NULL) {
      for (int j = i; j < currNode->numKey - 1; ++j) {
        currNode->key[j] = currNode->key[j + 1];
      }
      // decrease number of nodes by one
      currNode->numKey--;
      dataFound = true;
      break;
    } else if (i == currNode->numKey - 1 && currNode->child[i + 1] != NULL) {
      deleteKeyRec(currNode->child[i + 1], key, i + 1);
      this->print();
    }
  }

  // if the root is the only leaf -> We're done!!
  if (currNode->parentNode == NULL && currNode->isLeaf) {
    return;
  }
  // if the currNode is root and it has one pointer only
  if (currNode->parentNode == NULL && currNode->child[0] != NULL &&
      currNode->numKey == 0) {
    this->root = currNode->child[0];
    this->root->parentNode = NULL;
    return;
  }
  // if it's root node -> it can have any num of keys (> 0)
  if (currNode->parentNode == NULL) {
    return;
  }

  // if the currNode has enough nodes -> update the parent and we're done
  if (currNode->isLeaf && currNode->numKey >= (this->leafOrder + 1) / 2) {
    if (currPos > 0) {
      currNode->parentNode->key[currPos - 1] = currNode->key[0];
    }
    return;
  }
  if (!currNode->isLeaf && currNode->numKey + 1 >= (this->order + 1) / 2) {
    if (currPos > 0) {
      currNode->parentNode->key[currPos] = currNode->key[0];
    }
    return;
  }


  if (currNode->isLeaf && currNode->parentNode != NULL) {
    if (currPos == 0) {
      // left-most node
      Node* rightNode = currNode->parentNode->child[currPos + 1];
      // if the rightnode has more than required nodes -> redistribute
      if (rightNode != NULL && rightNode->numKey > (this->leafOrder + 1) / 2) {
        redistributeNode(currNode, rightNode, currNode->isLeaf, 0, 0);
      }
      // else -> merge with the right node
      else if (rightNode != NULL &&
               currNode->numKey + rightNode->numKey < this->leafOrder) {
        mergeBlock(currNode, rightNode, currNode->isLeaf, 1);
      } else {
      }
    } else {
      Node* leftNode = currNode->parentNode->child[currPos - 1];
      Node* rightNode = currNode->parentNode->child[currPos + 1];
      // ========================================
      if (currPos >= currNode->numKey) {
        rightNode = NULL;
      }
      // ========================================

      // check if redistribution is possible
      if (leftNode != NULL && leftNode->numKey > (this->leafOrder + 1) / 2) {
        redistributeNode(leftNode, currNode, currNode->isLeaf, currPos - 1, 1);
      } else if (rightNode != NULL &&
                 rightNode->numKey > (this->leafOrder + 1) / 2) {
        redistributeNode(currNode, rightNode, currNode->isLeaf, currPos, 0);
      } else if (leftNode != NULL &&
                 currNode->numKey + leftNode->numKey < this->leafOrder) {
        mergeBlock(leftNode, currNode, currNode->isLeaf, currPos);
      } else if (rightNode != NULL &&
                 currNode->numKey + rightNode->numKey < this->leafOrder) {
        mergeBlock(currNode, rightNode, currNode->isLeaf, currPos + 1);
      }
    }
  } else if (!currNode->isLeaf && currNode->parentNode != NULL) {
    // non leaf and non parent nodes
    if (currPos == 0) {
      // if the first child -> check rightnode to check if we can borrow
      Node* rightNode = currNode->parentNode->child[1];
      if (rightNode != NULL &&
          rightNode->numKey - 1 >= ceil((this->order - 1) / 2)) {
        redistributeNode(currNode, rightNode, currNode->isLeaf, 0, 0);
      }
      // else -> merge
      else if (rightNode != NULL &&
               currNode->numKey + rightNode->numKey < this->order - 1) {
        mergeBlock(currNode, rightNode, currNode->isLeaf, 1);
      }
    } else {
      Node* leftNode = currNode->parentNode->child[currPos - 1];
      Node* rightNode = currNode->parentNode->child[currPos + 1];
      // ========================================
      if (currPos >= currNode->numKey) {
        rightNode = NULL;
      }
      // ========================================

      // check left and right neighbor if we can borrow else merge
      if (leftNode != NULL &&
          leftNode->numKey - 1 >= ceil((this->order - 1) / 2)) {
        redistributeNode(leftNode, currNode, currNode->isLeaf, currPos - 1, 1);
      } else if (rightNode != NULL &&
                 rightNode->numKey - 1 >= ceil((this->order - 1) / 2)) {
        redistributeNode(currNode, rightNode, currNode->isLeaf, currPos, 0);
      } else if (leftNode != NULL &&
                 currNode->numKey + leftNode->numKey < this->order - 1) {
        mergeBlock(leftNode, currNode, currNode->isLeaf, currPos);
      } else if (rightNode != NULL &&
                 currNode->numKey + rightNode->numKey < this->order - 1) {
        mergeBlock(currNode, rightNode, currNode->isLeaf, currPos + 1);
      }
    }
  }
  if (currNode == NULL) return;
  // delete duplicate from the ancestor (if present)
  Node* tmpNode = currNode->parentNode;
  while (tmpNode != NULL) {
    for (int i = 0; i < tmpNode->numKey; i++) {
      if (tmpNode->key[i] == prevLeftMostkey) {
        tmpNode->key[i] = currNode->key[0];
        break;
      }
    }
    tmpNode = tmpNode->parentNode;
  }
}

void BPlusTree::search(int key) {
  if (this->root == NULL) {
    cout << "The tree is empty\n";
    return;
  }
  Node* curr = root;
  while (!curr->isLeaf) {
    for (int i = 0; i < curr->numKey; ++i) {
      if (key < curr->key[i]) {
        curr = curr->child[i];
        break;
      } else if (i == curr->numKey - 1) {
        curr = curr->child[i + 1];
        break;
      }
    }
  }
  for (int i = 0; i < curr->numKey; ++i) {
    if (curr->key[i] == key) {
      cout << "Found\n";
      return;
    }
  }
  cout << "Not Found\n";
}

void BPlusTree::search_range(int key1, int key2) {
  if (this->root == NULL) {
    cout << "The tree is empty\n";
    return;
  }
  Node* curr = root;
  while (!curr->isLeaf) {
    for (int i = 0; i < curr->numKey; ++i) {
      if (key1 < curr->key[i]) {
        curr = curr->child[i];
        break;
      } else if (i == curr->numKey - 1) {
        curr = curr->child[i + 1];
        break;
      }
    }
  }
  bool isFound = false;
  cout << "Range Search Start\n";
  while (curr) {
    for (int i = 0; i < curr->numKey; ++i) {
      if (curr->key[i] >= key1 && curr->key[i] <= key2) {
        cout << "Found " << curr->key[i] << "\n";
        isFound = true;
      }
    }
    curr = curr->nextLeaf;
  }
  if (!isFound) {
    cout << "Not Found\n";
  }
  cout << "Range Search End\n";
}

void BPlusTree::insert(int key) {
  // if the root is null
  if (this->root == NULL) {
    // create a new node and set it as a root
    this->root = new Node(this->leafOrder);
    this->root->key[0] = key;
    this->root->numKey++;
    this->root->isLeaf = true;
    return;
  }
  // if root is not NULL
  Node* curr = root;
  // traverse till the target leaf node
  while (!curr->isLeaf) {
    // find appropriate child node
    for (int i = 0; i < curr->numKey; ++i) {
      if (key < curr->key[i]) {
        curr = curr->child[i];
        if (curr == NULL) {
          throw "curr->child[i] is NULL";
        }
        break;
      } else if (i == curr->numKey - 1) {
        curr = curr->child[i + 1];
        break;
      }
    }
  }

  // check if the key already exists
  for (int i = 0; i < curr->numKey; ++i) {
    if (curr->key[i] == key) throw "Key already exists";
  }

  // if the target leaf node is not full
  if (curr->numKey < this->leafOrder) {
    // insert the new key at the appropriate location
    // using INSERTION SORT
    int index = curr->numKey - 1;
    while (index >= 0 && curr->key[index] > key) {
      curr->key[index + 1] = curr->key[index];
      index--;
    }
    curr->numKey++;
    curr->key[index + 1] = key;
    return;
  }

  // if the target leaf is full -> Split the leaf

  // create a tmp array
  vector<int> tmpKeys(this->leafOrder + 1);
  for (int i = 0; i < this->leafOrder; ++i) {
    tmpKeys[i] = curr->key[i];
  }

  int index = this->leafOrder - 1;
  while (index >= 0 && tmpKeys[index] > key) {
    tmpKeys[index + 1] = tmpKeys[index];
    index--;
  }
  tmpKeys[index + 1] = key;

  // create a rightNode
  // curr -> used as a left node
  Node* rightNode = new Node(this->order);
  rightNode->isLeaf = true;
  rightNode->parentNode = curr->parentNode;
  curr->numKey = (this->leafOrder + 1) / 2;
  rightNode->numKey = (this->leafOrder + 1) - (this->leafOrder + 1) / 2;

  // copy half the keys in the leftNode and other half in the rightNode
  for (int i = 0; i < curr->numKey; ++i) {
    curr->key[i] = tmpKeys[i];
  }
  for (int i = 0; i < rightNode->numKey; ++i) {
    rightNode->key[i] = tmpKeys[i + curr->numKey];
  }
  // link the left and the right leaves
  rightNode->nextLeaf = curr->nextLeaf;
  curr->nextLeaf = rightNode;

  // if the parent is NULL -> create a new one and set it as the root
  if (curr->parentNode == NULL) {
    this->root = new Node(this->order);
    root->key[0] = rightNode->key[0];
    root->child[0] = curr;
    root->child[1] = rightNode;
    root->numKey++;
    // set parents for left and right child
    curr->parentNode = rightNode->parentNode = root;
    return;
  }

  // else
  // we need to insert {rightNode->key[0], rightNode} in the parent node
  key = rightNode->key[0];
  while (curr->parentNode) {
    // key = rightNode->key[0];
    curr = curr->parentNode;
    // if curr node is not filled completely -> insert directly
    if (curr->numKey < this->order - 1) {
      index = curr->numKey - 1;
      while (index >= 0 && curr->key[index] > key) {
        curr->key[index + 1] = curr->key[index];
        curr->child[index + 1] = curr->child[index];
        index--;
      }
      curr->numKey++;
      curr->key[index + 1] = key;
      curr->child[index + 2] = rightNode;
      return;
    }
    // else -> split the current node
    // and reiterate the same process for its parent
    tmpKeys = vector<int>(this->order);
    vector<Node*> tmpNodes(this->order + 2, NULL);
    for (int i = 0; i < curr->numKey; ++i) {
      tmpKeys[i] = curr->key[i];
      tmpNodes[i] = curr->child[i];
    }
    tmpNodes[curr->numKey] = curr->child[curr->numKey];

    // insert {key, rightNode} in tmp vectors first
    int index = this->order - 2;
    tmpNodes[index + 2] = tmpNodes[index + 1];
    while (index >= 0 && tmpKeys[index] > key) {
      tmpKeys[index + 1] = tmpKeys[index];
      tmpNodes[index + 1] = tmpNodes[index];
      index--;
    }
    tmpKeys[index + 1] = key;
    tmpNodes[index + 2] = rightNode;

    // create a new rightNode (internal right node)
    // curr -> used as a left node
    rightNode = new Node(this->order);
    rightNode->parentNode = curr->parentNode;
    int prevTotKeys = curr->numKey;
    if (prevTotKeys != this->order - 1) {
      throw "prevTotKeys != this->order-1";
    }
    curr->numKey = (prevTotKeys + 1) / 2;
    rightNode->numKey = (prevTotKeys) - (prevTotKeys + 1) / 2;
    // copy half the keys and children in the leftNode and other half in the
    // rightNode
    for (int i = 0; i < curr->numKey; ++i) {
      curr->key[i] = tmpKeys[i];
      curr->child[i] = tmpNodes[i];
      curr->child[i]->parentNode = curr;
    }
    // we need to insert median = curr->key[curr->numkey]
    key = tmpKeys[curr->numKey];
    curr->child[curr->numKey] = tmpNodes[curr->numKey];
    curr->child[curr->numKey]->parentNode = curr;
    for (int i = 0; i < rightNode->numKey; ++i) {
      rightNode->key[i] = tmpKeys[i + curr->numKey + 1];
      rightNode->child[i] = tmpNodes[i + curr->numKey + 1];
      rightNode->child[i]->parentNode = rightNode;
    }
    rightNode->child[rightNode->numKey] =
        tmpNodes[rightNode->numKey + curr->numKey + 1];
    if (rightNode->child[rightNode->numKey] != NULL) {
      rightNode->child[rightNode->numKey]->parentNode = rightNode;
    } else {
      cout << "[WARN]: Last child of right node = null!!\n";
    }

    // if the parent is NULL -> create a new one and set it as the root
    if (curr->parentNode == NULL) {
      this->root = new Node(this->order);
      root->key[0] = key;
      root->child[0] = curr;
      root->child[1] = rightNode;
      root->numKey++;
      // set parents for left and right child
      curr->parentNode = rightNode->parentNode = root;
      return;
    }
  }
}

void BPlusTree::print(void) {
  cout << "B+ Tree:\n";
  vector<Node*> curr = {this->root};
  vector<Node*> next;
  while (!curr.empty()) {
    for (Node* node : curr) {
      if (node == NULL) {
        cout << "PRINT: NULL NODE!!!\n";
        continue;
      }
      cout << "{ ";
      for (int i = 0; i < node->numKey; ++i) {
        cout << " " << node->key[i] << " ";
        if (!node->isLeaf && node->child[i] != NULL) {
          next.push_back(node->child[i]);
        } else {
        };
      }
      if (!node->isLeaf && node->child[node->numKey] != NULL) {
        next.push_back(node->child[node->numKey]);
      };
      cout << "}, ";
    }
    cout << "\n";
    curr = next;
    next.clear();
  }
}

void printVec(vector<int> A) {
  cout << "VEC: ";
  for (int num : A) {
    cout << num << " ";
  }
  cout << "\n";
}

void BPlusTree::redistributeNode(Node* leftNode, Node* rightNode, bool isLeaf,
                                 int posOfleftChild, int currNodeL0R1) {
  if (currNodeL0R1 == 0) {
    // leftNode is currNode -> get one key from the right node and put it in the
    // left node
    if (!isLeaf) {
      // Non leaf node

      // take parent node's key down to the left node
      // make right node's first child as left node's last child
      leftNode->key[leftNode->numKey] =
          leftNode->parentNode->key[posOfleftChild];
      leftNode->child[leftNode->numKey + 1] = rightNode->child[0];
      leftNode->numKey++;

      // in parent node replace the prev key by the first of the right node
      leftNode->parentNode->key[posOfleftChild] = rightNode->key[0];

      // (deletion of one key)
      // shift all keys and children by one in the right node
      for (int j = 0; j < rightNode->numKey - 1; ++j) {
        rightNode->key[j] = rightNode->key[j + 1];
        rightNode->child[j] = rightNode->child[j + 1];
      }
      rightNode->child[rightNode->numKey] =
          rightNode->child[rightNode->numKey + 1];
      rightNode->numKey--;
    } else {
      // leaf node

      // take parent node's key down to the left node
      leftNode->key[leftNode->numKey] = rightNode->key[0];
      leftNode->numKey++;
      // delete the first key of the right node
      // shift by one node to left of the rightNode
      for (int j = 0; j < rightNode->numKey - 1; ++j) {
        rightNode->key[j] = rightNode->key[j + 1];
      }
      rightNode->numKey--;

      leftNode->parentNode->key[posOfleftChild] = rightNode->key[0];
      return;
    }

  } else {
    // rightNode is currNode
    if (!isLeaf) {
      // shift everything in the rightnode by one to make a space for the
      // incoming key and child
      rightNode->numKey++;
      for (int j = rightNode->numKey - 1; j >= 0; --j) {
        rightNode->key[j + 1] = rightNode->key[j];
        rightNode->child[j + 1] = rightNode->child[j];
      }

      // bring down a key from the parent node
      rightNode->key[0] = leftNode->parentNode->key[posOfleftChild];
      // first child of rightnode = last child of leftNode
      rightNode->child[0] = leftNode->child[leftNode->numKey];

      // last key of the leftnode -> move it to the parent
      leftNode->parentNode->key[posOfleftChild] =
          leftNode->key[leftNode->numKey - 1];
      // erase the last child pointer of leftNode
      leftNode->child[leftNode->numKey] = NULL;
      leftNode->numKey--;
    } else {
      // leaf node
      // move all keys to right (to make space for the incoming key)
      rightNode->numKey++;
      for (int j = rightNode->numKey - 1; j >= 0; --j) {
        rightNode->key[j + 1] = rightNode->key[j];
      }
      // left's last key -> right's first key
      rightNode->key[0] = leftNode->key[leftNode->numKey - 1];
      leftNode->numKey--;

      // update corresponding key in the parent
      leftNode->parentNode->key[posOfleftChild] = rightNode->key[0];
    }
  }
  for (int i = 0; i <= leftNode->numKey; ++i) {
    leftNode->child[i]->parentNode = leftNode;
  }
  for (int i = 0; i <= rightNode->numKey; ++i) {
    rightNode->child[i]->parentNode = rightNode;
  }
}

void BPlusTree::mergeBlock(Node* leftNode, Node*& rightNode, bool isLeaf,
                           int posOfRight) {
  if (!isLeaf) {
    // not a leaf node -> absorb the key from the parent
    leftNode->key[leftNode->numKey] = leftNode->parentNode->key[posOfRight - 1];
    leftNode->numKey++;
  }
  leftNode->parentNode->child[posOfRight] = leftNode;
  // copy rightnode's key and children to the left node
  for (int i = 0; i < rightNode->numKey; ++i) {
    leftNode->key[leftNode->numKey] = rightNode->key[i];
    leftNode->child[leftNode->numKey] = rightNode->child[i];
    leftNode->numKey++;
  }
  leftNode->child[leftNode->numKey] = rightNode->child[rightNode->numKey];

  //  remove the leftNode->parentNode->key[posOfRight-1]
  // shift other keys to the left
  for (int i = posOfRight - 1; i < leftNode->parentNode->numKey; ++i) {
    leftNode->parentNode->key[i] = leftNode->parentNode->key[i + 1];
    leftNode->parentNode->child[i] = leftNode->parentNode->child[i + 1];
  }
  leftNode->parentNode->numKey--;

  //  reassign the parentNode after the merger
  for (int i = 0; i <= leftNode->numKey && leftNode->child[i] != NULL; ++i) {
    leftNode->child[i]->parentNode = leftNode;
  }
  if (isLeaf) {
    leftNode->nextLeaf = rightNode->nextLeaf;
  }
  free(rightNode);
  rightNode = NULL;
}