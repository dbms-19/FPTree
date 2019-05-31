#include"fptree/fptree.h"
#include<assert.h>
using namespace std;

// Initial the new InnerNode
InnerNode::InnerNode(const int& d, FPTree* const& t, bool _isRoot) {
    // TODO
    tree = t;
    degree = d;
    isLeaf = false;
    nKeys = 0;      // amount of keys
    nChild = 0;     // amount of children
    keys = new Key[2 * d + 1];// max (2 * d + 1) keys
    childrens = new Node*[2 * d + 2];//max (2 * d + 2)  node pointers
    isRoot = _isRoot;
}

// delete the InnerNode
InnerNode::~InnerNode() {
    // TODO
    delete []keys;
    delete []childrens;
}

// binary search the first key in the innernode larger than input key
int InnerNode::findIndex(const Key& k) {
    // TODO
    int left = 0;
    int right = nKeys - 1;
    while(left <= right){
        int mid = left + (right - left) / 2;
        if (k >= keys[mid])
            left = mid + 1;
        else
            right = mid - 1;
    }
    return left;
}

// insert the node that is assumed not full
// insert format:
// ======================
// | key | node pointer |
// ======================
// WARNING: can not insert when it has no entry
void InnerNode::insertNonFull(const Key& k, Node* const& node) {
    // TODO
    //the keys nodekey in node satisfies nodekey>=key;
    //if it has no entry,return
    int pos = findIndex(k);
    //move keys and pointers
    for(int i = nKeys; i > pos; --i) {
        keys[i] = keys[i - 1];
    }
    for(int i = nChild; i > pos; --i) {
        childrens[i + 1] = childrens[i];
    }
    keys[pos] = k;
    childrens[pos + 1] = node; 
    ++nKeys;
    ++nChild;
}

// insert func
// return value is not NULL if split, returning the new child and a key to insert
KeyNode* InnerNode::insert(const Key& k, const Value& v) {
    KeyNode* newChild = NULL;

    // 1.insertion to the first leaf(only one leaf)
    if (this->isRoot && this->nKeys == 0 && this->nChild==0) {
        // TODO
        //it must allocate new leafnode
        LeafNode* newLeaf = new LeafNode(tree);
        //insert newLeaf into Right newLeaf
        newLeaf->insert(k,v);
        //it's the first leaf so it must be empty
        childrens[0]=newLeaf;
        ++nChild;
        return newChild;
    }
    // 2.recursive insertion
    // TODO
    //find insert positon+
    int index = findIndex(k);
    newChild = childrens[index]->insert(k,v);
    if(newChild==NULL) return newChild;
    else{
        //if Innernode has enough space
        if(this->nKeys<degree*2){
            insertNonFull(newChild->key,newChild->node);
            delete newChild;
            newChild=NULL;
            return newChild;
        }
        //no free space, split
        else{
            insertNonFull(newChild->key,newChild->node);//insert overflow key-value
            newChild = this->split();
            if(this->isRoot){
                //setting new root
                InnerNode* newRoot= new InnerNode(degree,tree,true);
                this->isRoot=false;
                newRoot->childrens[0]=this;
                ++(newRoot->nChild);
                newRoot->insertNonFull(newChild->key,newChild->node);
                tree->changeRoot(newRoot);
                delete newChild;
            }
        }
        //
    }
    return newChild;
}

// ensure that the leaves inserted are ordered
// used by the bulkLoading func
// inserted data: | minKey of leaf | LeafNode* |
KeyNode* InnerNode::insertLeaf(const KeyNode& leaf) {
    KeyNode* newChild = NULL;
    // first and second leaf insertion into the tree
    if (this->isRoot && this->nKeys == 0 && this->nChild==0) {
        // TODO
        childrens[0]=leaf.node;
        ++nChild;
        return newChild;
    }
    
    // recursive insert
    // Tip: please judge whether this InnerNode is full
    // next level is not leaf, just insertLeaf
    // TODO

    // next level is leaf, insert to childrens array
    // TODO
    int index = findIndex(leaf.key);
    if(childrens[0]->ifLeaf()){ //if next level is leaf
        if(this->nKeys<degree*2){
            insertNonFull(leaf.key,leaf.node);
        }
        else{
            insertNonFull(leaf.key,leaf.node);//insert overflow key-value
            newChild = this->split();
            if(this->isRoot){
            InnerNode* newRoot= new InnerNode(degree,tree,true);
            this->isRoot=false;
            newRoot->childrens[0]=this;
            ++(newRoot->nChild);
            newRoot->insertNonFull(newChild->key,newChild->node);
            tree->changeRoot(newRoot);
            delete newChild;
            newChild=NULL;}
            return newChild;
        }
    }
    else{ //if next level is not leaf
    newChild = ((InnerNode*)(childrens[index]))->insertLeaf(leaf);
    if(newChild==NULL) return newChild;
    else{
        //if Innernode has enough space
        if(this->nKeys<degree*2){
            insertNonFull(newChild->key,newChild->node);
            delete newChild;
            newChild=NULL;
            return newChild;
        }
        //no free space, split
        else{
            insertNonFull(newChild->key,newChild->node);//insert overflow key-value
            newChild = this->split();
            if(this->isRoot){
                //setting new root
                InnerNode* newRoot= new InnerNode(degree,tree,true);
                this->isRoot=false;
                newRoot->childrens[0]=this;
                ++(newRoot->nChild);
                newRoot->insertNonFull(newChild->key,newChild->node);
                tree->changeRoot(newRoot);
                delete newChild;
                newChild=0;
            }
        }
        //
        }
    }
    return newChild;
}

KeyNode* InnerNode::split() {
    KeyNode* newChild = new KeyNode();
    // right half entries of old node to the new node, others to the old node. 
    //left: d keys d pointers   right: d keys d pointers, push up the d+1th key 
    int d=nKeys / 2;
    Key newMidKey = keys[d];
    InnerNode* newNode = new InnerNode(d,tree,false);
    newNode->nKeys = d;
    newNode->nChild = d + 1;
    for(int i = d + 1;i < nKeys; ++i){
        newNode->keys[i-d-1] = keys[i];
        newNode->childrens[i-d-1] = childrens[i];
    }
    newNode->childrens[nKeys-d-1]=childrens[nKeys];
    nKeys = d;
    nChild = d + 1;
    newChild->key = newMidKey;
    newChild->node = newNode;
    return newChild;
}

// remove the target entry
// return TRUE if the children node is deleted after removement.
// the InnerNode need to be redistributed or merged after deleting one of its children node.
bool InnerNode::remove(const Key& k, const int& index, InnerNode* const& parent, bool &ifDelete) {
    bool ifRemove = false;
    // only have one leaf
    // TODO
    
    // recursive remove
    // TODO
    return ifRemove;
}

// If the leftBro and rightBro exist, the rightBro is prior to be used
void InnerNode::getBrother(const int& index, InnerNode* const& parent, InnerNode* &leftBro, InnerNode* &rightBro) {
    // TODO
}

// merge this node, its parent and left brother(parent is root)
void InnerNode::mergeParentLeft(InnerNode* const& parent, InnerNode* const& leftBro) {
    // TODO
}

// merge this node, its parent and right brother(parent is root)
void InnerNode::mergeParentRight(InnerNode* const& parent, InnerNode* const& rightBro) {
    // TODO
}

// this node and its left brother redistribute
// the left has more entries
void InnerNode::redistributeLeft(const int& index, InnerNode* const& leftBro, InnerNode* const& parent) {
    // TODO
}

// this node and its right brother redistribute
// the right has more entries
void InnerNode::redistributeRight(const int& index, InnerNode* const& rightBro, InnerNode* const& parent) {
    // TODO
}

// merge all entries to its left bro, delete this node after merging.
void InnerNode::mergeLeft(InnerNode* const& leftBro, const Key& k) {
    // TODO
}

// merge all entries to its right bro, delete this node after merging.
void InnerNode::mergeRight(InnerNode* const& rightBro, const Key& k) {
    // TODO
}

// remove a children from the current node, used by remove func
void InnerNode::removeChild(const int& keyIdx, const int& childIdx) {
    // TODO
}

// update the target entry, return true if the update succeed.
bool InnerNode::update(const Key& k, const Value& v) {
    // TODO
    return false;
}

// find the target value with the search key, return MAX_VALUE if it fails.
Value InnerNode::find(const Key& k) {
    // TODO
    int pos = findIndex(k);
    if(pos < 0) return MAX_VALUE;
    return childrens[pos]->find(k);
}

// get the children node of this InnerNode
Node* InnerNode::getChild(const int& idx) {
    // TODO
    if (idx < this->nChild){
        return this->childrens[idx];
    }
    return NULL;
}

// get the key of this InnerNode
Key InnerNode::getKey(const int& idx) {
    if (idx < this->nKeys) {
        return this->keys[idx];
    } else {
        return MAX_KEY;
    }
}

// print the InnerNode
void InnerNode::printNode() {
    cout << "||#|";
    for (int i = 0; i < this->nKeys; i++) {
        cout << " " << this->keys[i] << " |#|";
    }
    cout << "|" << "    ";
}

// print the LeafNode
void LeafNode::printNode() {
    cout << "||";
    for (int i = 0; i < 2 * this->degree; i++) {
        if (this->getBit(i)) {
            cout << " " << this->kv[i].k << " : " << this->kv[i].v << " |";
        }
    }
    cout << "|" << " ====>> ";
}

// new a empty leaf and set the valuable of the LeafNode
LeafNode::LeafNode(FPTree* t) {
    // TODO
    // Leaf : | bitmap | pNext | fingerprints array | KV array |
    //initial Node
    tree = t;
    degree = LEAF_DEGREE;
    isLeaf = true;
    //new a leaf
    PAllocator* PA = PAllocator::getAllocator();
    PA->getLeaf(pPointer, pmem_addr);
    //initial bitmap
    bitmap = (Byte*)pmem_addr;
    bitmapSize = (LEAF_DEGREE*2+7) / 8;
    //initial pNext
    Byte* cursor = (Byte*) (pmem_addr);
    cursor = cursor+bitmapSize;
    pNext = (PPointer*) cursor;
    //initial fingerprints, use cursor to make a pointer move
    cursor = cursor+sizeof(PPointer);
    fingerprints=cursor;
    //initial kv
    cursor = cursor+ LEAF_DEGREE*2 * sizeof(Byte);
    kv = (KeyValue*)cursor;
    n = 0;
    prev = NULL;
    next = NULL;
    filePath = DATA_DIR+ to_string(pPointer.fileId);
}

// reload the leaf with the specific Persistent Pointer
// need to call the PAllocator
LeafNode::LeafNode(PPointer p, FPTree* t) {
    // TODO
    //initial Node
    next = NULL;
    prev = NULL;
    tree = t;
    degree = LEAF_DEGREE;
    isLeaf = true;
    //reload a leaf
    PAllocator* PA = PAllocator::getAllocator();
    pmem_addr = PA->getLeafPmemAddr(p);      // the pmem address of the leaf node
    assert(pmem_addr != NULL);
    pPointer =p;
    //initial  bitmap
    bitmap = (Byte*)pmem_addr;
    bitmapSize = (LEAF_DEGREE*2+7) / 8;
    //initial pNext
    Byte* cursor = (Byte*) (pmem_addr);
    cursor = cursor+bitmapSize;
    pNext = (PPointer*) cursor;
    //initial fingerprints, use cursor to make a pointer move
    cursor = cursor + sizeof(PPointer);
    fingerprints=cursor;
    //initial kv
    cursor = cursor+ LEAF_DEGREE * 2 * sizeof(Byte);
    kv = (KeyValue*)cursor;
    //count n
    cursor = bitmap;
    n = 0;
    for(size_t i = 0; i < bitmapSize; ++i){
        n+=countOneBits(*cursor);
        cursor+=1;
    }
    //initial pointer list
     if (pNext->fileId != 0){
        next = new LeafNode(*pNext, tree);
        next->prev = this;
    }
    filePath=DATA_DIR+ to_string(p.fileId);
}

LeafNode::~LeafNode() {
    // TODO
    this->persist();
}

// insert an entry into the leaf, need to split it if it is full
KeyNode* LeafNode::insert(const Key& k, const Value& v) {
    KeyNode* newChild = NULL;
    // TODO
    if (n >= 2 * degree - 1) {
        newChild = split();
        if (k < newChild->key) {
            insertNonFull(k, v);
        } else {
            dynamic_cast<LeafNode*>(newChild->node)->insertNonFull(k, v);
        }
    } else {
        insertNonFull(k, v);
    }
    return newChild;
}

// insert into the leaf node that is assumed not full
void LeafNode::insertNonFull(const Key& k, const Value& v) {
    // TODO
    //find the first free slot
    int pos = findFirstZero();
    
    int offset = pos % 8;
    int i = pos / 8;
    Byte* cursor = bitmap;
    cursor += i;
    Byte bits = *cursor;
    bits = bits | (1 << offset);
    *(cursor) = bits;

    kv[pos] = {k , v};
    fingerprints[pos] = keyHash(k);
    ++n;
    persist();
}

// split the leaf node
KeyNode* LeafNode::split() {
    KeyNode* newChild = new KeyNode();
    // TODO
    //LeafNode split when n = 2*d-1;
    LeafNode* newLeaf = new LeafNode(tree);
    memset(bitmap, 0, bitmapSize);
    Key splitkey = findSplitKey();
    //original leaf
    for(int i = 0; i < n / 2; ++i){
        fingerprints[i] = keyHash(getKey(i));
        assert(i < 2 * degree);
        int offset = i % 8;
        int pos = i / 8;
        Byte* cursor = bitmap;
        cursor += pos;
        Byte bits = *cursor;
        bits = bits | (1 << offset);
        *(cursor) = bits;
    }
    //new Leaf
    for(int i = n / 2; i < n; ++i){
        newLeaf->insertNonFull(getKey(i), getValue(i));
    }
    n = n / 2;
    *pNext = newLeaf->getPPointer();
    newChild->node = newLeaf;
    newChild->key = splitkey;
    newLeaf->persist();
    this->persist();
    return newChild;
}

// use to find a mediant key and delete entries less then middle
// called by the split func to generate new leaf-node
// qsort first then find
inline int cmp(const void* a,const void* b)
{
    return ((KeyValue*)a)->k > ((KeyValue*)b)->k;
}
Key LeafNode::findSplitKey() {
    Key midKey = 0;
    // TODO
    qsort(kv, n, sizeof(KeyValue), cmp);
    midKey = kv[n/2].k;
    return midKey;
}
// get the target bit in bitmap
// TIPS: bit operation
int LeafNode::getBit(const int& idx) {
    // TODO
    assert(idx<2*degree);
    int offset = idx%8;
    int pos=idx/8;
    Byte* cursor = bitmap;
    cursor += pos;
    Byte bits = *cursor;
    bits = (bits>>offset) & 1;
    return (int) bits;
}
//set the target bit to 1 in bitmap

Key LeafNode::getKey(const int& idx) {
    return this->kv[idx].k;
}

Value LeafNode::getValue(const int& idx) {
    return this->kv[idx].v;
}

PPointer LeafNode::getPPointer() {
    return this->pPointer;
}

// remove an entry from the leaf
// if it has no entry after removement return TRUE to indicate outer func to delete this leaf.
// need to call PAllocator to set this leaf free and reuse it
bool LeafNode::remove(const Key& k, const int& index, InnerNode* const& parent, bool &ifDelete) {
    bool ifRemove = false;
    // TODO
    return ifRemove;
}

// update the target entry
// return TRUE if the update succeed
bool LeafNode::update(const Key& k, const Value& v) {
    bool ifUpdate = false;
    // TODO
    return ifUpdate;
}

// if the entry can not be found, return the max Value
Value LeafNode::find(const Key& k) {
    // TODO
    int hashnum = keyHash(k);
    Byte* cursor = fingerprints;
    for(int i = 0; i < 2 * degree; ++i){
        if(getBit(i) == 1 && fingerprints[i] == hashnum){
            if(getKey(i) == k) {
                return getValue(i);
            }  
        }
        ++cursor;
    }
    return MAX_VALUE;
}

// find the first empty slot
int LeafNode::findFirstZero() {
    // TODO
    int pos;
    for(int i = 0; i < 2 * degree; ++i) {
        if(getBit(i) == 0) {
            pos = i;
            return pos;
        }
    }
    return -1;
}

// persist the entire leaf
// use PMDK
void LeafNode::persist() {
    // TODO
    pmem_msync(pmem_addr,calLeafSize());
}

// call by the ~FPTree(), delete the whole tree
void FPTree::recursiveDelete(Node* n) {
    if (n->isLeaf) {
        delete n;
    } else {
        for (int i = 0; i < ((InnerNode*)n)->nChild; i++) {
            recursiveDelete(((InnerNode*)n)->childrens[i]);
        }
        delete n;
    }
}

FPTree::FPTree(uint64_t t_degree) {
    FPTree* temp = this;
    this->root = new InnerNode(t_degree, temp, true);
    this->degree = t_degree;
    bulkLoading();
}

FPTree::~FPTree() {
    recursiveDelete(this->root);
}

// get the root node of the tree
InnerNode* FPTree::getRoot() {
    return this->root;
}

// change the root of the tree
void FPTree::changeRoot(InnerNode* newRoot) {
    this->root = newRoot;
}

void FPTree::insert(Key k, Value v) {
    if (root != NULL) {
        root->insert(k, v);
    }
}

bool FPTree::remove(Key k) {
    if (root != NULL) {
        bool ifDelete = false;
        InnerNode* temp = NULL;
        return root->remove(k, -1, temp, ifDelete);
    }
    return false;
}

bool FPTree::update(Key k, Value v) {
    if (root != NULL) {
        return root->update(k, v);
    }
    return false;
}

Value FPTree::find(Key k) {
    if (root != NULL) {
        return root->find(k);
    }
    return MAX_VALUE;
}

// call the InnerNode and LeafNode print func to print the whole tree
// TIPS: use Queue
void FPTree::printTree() {
    // TODO
    queue<Node*> q;
    q.push(root);
    while (!q.empty()) {
        Node* cur = q.front();
        q.pop();
        if (!cur->isLeaf) {
            InnerNode* node = dynamic_cast<InnerNode*>(cur);
            for (int i = 0; i < node->nChild; ++i) {
                q.push(node->getChild(i));
            }
        }
        cur->printNode();
    }
}

// bulkLoading the leaf files and reload the tree
// need to traverse leaves chain
// if no tree is reloaded, return FALSE
// need to call the PALlocator
bool FPTree::bulkLoading() {
    // TODO
    PPointer startPointer = PAllocator::getAllocator()->getStartPointer();
    if(startPointer.fileId == 0) {
        return false;
    }
    LeafNode* startLeaf = new LeafNode(startPointer, this);
    //travesal list
    KeyNode bulkLeaf;
    for(auto leafnode = startLeaf; leafnode != NULL; leafnode = leafnode->next) {
        bulkLeaf.node = leafnode;
        Key min = leafnode->getKey(0);
        for(int i = 1; i < leafnode->n; ++i){
            if(leafnode->getKey(i) < min && leafnode->getBit(i)) {
                min = leafnode->getKey(i);
            }
        }
        bulkLeaf.key = min;
        root->insertLeaf(bulkLeaf);
    }
    return true;
}
