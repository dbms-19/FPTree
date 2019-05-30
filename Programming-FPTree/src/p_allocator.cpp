#include"utility/p_allocator.h"
#include<iostream>
#include<string.h>
#include<assert.h>
using namespace std;

// the file that store the information of allocator
const string P_ALLOCATOR_CATALOG_NAME = "p_allocator_catalog";
// a list storing the free leaves
const string P_ALLOCATOR_FREE_LIST    = "free_list";

PAllocator* PAllocator::pAllocator = new PAllocator();

PAllocator* PAllocator::getAllocator() {
    // 判断是否拥有pAllocator
    if (PAllocator::pAllocator == NULL) {
        PAllocator::pAllocator = new PAllocator();
    }
    return PAllocator::pAllocator;
}

/* data storing structure of allocator
   In the catalog file, the data structure is listed below
   | maxFileId(8 bytes) | freeNum = m | treeStartLeaf(the PPointer) |
   In freeList file:
   | freeList{(fId, offset)1,...(fId, offset)m} |
*/
PAllocator::PAllocator() {
    string allocatorCatalogPath = DATA_DIR + P_ALLOCATOR_CATALOG_NAME;
    string freeListPath         = DATA_DIR + P_ALLOCATOR_FREE_LIST;
    ifstream allocatorCatalog(allocatorCatalogPath, ios::in|ios::binary);
    ifstream freeListFile(freeListPath, ios::in|ios::binary);
    // judge if the catalog exists
    if (allocatorCatalog.is_open() && freeListFile.is_open()) {
        // exist
        // TODO
        // 根据格式读取数据到变量中
        allocatorCatalog.read((char*)&(maxFileId), sizeof(uint64_t));
        allocatorCatalog.read((char*)&(freeNum), sizeof(uint64_t));
        allocatorCatalog.read((char*)&(startLeaf), sizeof(PPointer));
        //read FreeListFile
        // 读取数据到叶子空闲列表中
        PPointer flp;
        for(uint64_t i = 0;i < this->freeNum ; i++) {
            freeListFile.read((char*)&(flp),sizeof(PPointer));
            this->freeList.push_back(flp);
        }
        // 关闭文件
        allocatorCatalog.close();
        freeListFile.close();
    } else {
        // not exist, create catalog and free_list file, then open.
        // TODO
        // 若不存在，则创建Catalog与free_list文件，catalog文件需要初始化变量再读入
        fstream file1;
        file1.open(allocatorCatalogPath,ios::out);
        if(!file1) {return;}
        // 初始化变量
        this->maxFileId = 1;
        this->freeNum = 0;
        this->startLeaf.fileId = 0;
        this->startLeaf.offset = LEAF_GROUP_HEAD;
        file1.write((char*)&(maxFileId),sizeof(uint64_t));
        file1.write((char*)&(freeNum),sizeof(uint64_t));
        file1.write((char*)&(startLeaf),sizeof(PPointer));
        file1.close();
        fstream file2;
        file2.open(freeListPath,ios::out);
        if(!file2) {return;}
        file2.close();
    }
    this->initFilePmemAddr();
    
}

PAllocator::~PAllocator() {
    // TODO
    auto it= fId2PmAddr.begin();
    //traversal to sync and unmap leaf groups
    while(it != fId2PmAddr.end())
    {
        int leaf_group_len = 8 + LEAF_GROUP_AMOUNT + LEAF_GROUP_AMOUNT*calLeafSize();
        // 持久化变量，并解除映射关系
        pmem_msync(it->second,leaf_group_len);//sync the whole leaf group
        pmem_unmap(it->second,leaf_group_len);
        ++it;         
    }
    persistCatalog();
    if(PAllocator::pAllocator!=NULL) PAllocator::pAllocator=NULL;
}

// memory map all leaves to pmem address, storing them in the fId2PmAddr
void PAllocator::initFilePmemAddr() {
    // TODO
    int is_pmem;
    size_t mapped_len;
    int leaf_len = 8 +  LEAF_GROUP_AMOUNT + LEAF_GROUP_AMOUNT*calLeafSize();
    for (uint64_t i = 0;i < maxFileId;i++) {
        char * pmemaddr;
        string file_path = DATA_DIR + to_string(i);
        if ((pmemaddr = (char*)pmem_map_file(file_path.c_str(), leaf_len, PMEM_FILE_CREATE,
            0666, &mapped_len, &is_pmem)) == NULL) {
            assert(pmemaddr != NULL);
        }
        fId2PmAddr.insert(pair<uint64_t,char*>(i,pmemaddr));
    }
}

// get the pmem address of the target PPointer from the map fId2PmAddr
char* PAllocator::getLeafPmemAddr(PPointer p) {
    // TODO
    map<uint64_t,char*>::iterator iter = fId2PmAddr.find(p.fileId);
    if(iter != fId2PmAddr.end()) {
        char* addr = iter->second;
        if(p.offset <= LEAF_GROUP_HEAD + (LEAF_GROUP_AMOUNT - 1)*calLeafSize()) {
            return addr + p.offset;
        }
        return NULL;
    }
    return NULL;
}

// get and use a leaf for the fptree leaf allocation
// return 
bool PAllocator::getLeaf(PPointer &p, char* &pmem_addr) {
    // TODO
    // 检查是否有空闲块
    if(this->freeNum == 0) {
        if(!newLeafGroup()) return false;
    }
    // 使用最后一个，并从vector中删除
    this->freeNum--;
    p.fileId = freeList[this->freeNum].fileId;
    p.offset = freeList[this->freeNum].offset;
    this->freeList.pop_back();
    // 得到地址
    pmem_addr = getLeafPmemAddr(p);
    // 更改叶组的使用数量，以及对应位置一
    map<uint64_t,char*>::iterator iter = fId2PmAddr.find(p.fileId);
    char* addre = iter->second;
    uint64_t usdnum;
    memcpy(&usdnum,addre,sizeof(uint64_t));
    usdnum++;
    memcpy(addre,&usdnum,sizeof(uint64_t));
    uint64_t weizhi = (p.offset - LEAF_GROUP_HEAD)/calLeafSize();
    *(addre + 8 + weizhi) = 1;
    // 持久化变量
    int leaf_len = LEAF_GROUP_HEAD + LEAF_GROUP_AMOUNT * calLeafSize();
    pmem_msync(addre,leaf_len);
    return true;

}

bool PAllocator::ifLeafUsed(PPointer p) {
    // TODO
    // 若fileID大于max则不存在
    if(p.fileId >= maxFileId) return false;
    // 若offset大过叶组长度，则不存在
    if(p.offset > LEAF_GROUP_HEAD + (LEAF_GROUP_AMOUNT - 1)*calLeafSize()) return false;
    // 检查是否存在于空闲叶子列表里
    for(uint64_t i = 0; i < freeList.size();i++) {
        if (freeList[i].fileId == p.fileId && freeList[i].offset == p.offset)
        {
            /* code */
            return false;
        }
    }
    return true;
}

bool PAllocator::ifLeafFree(PPointer p) {
    // TODO
    // 先检查叶子是否存在
    if(!ifLeafExist(p)) return false;
    // 在检查叶子是否已使用
    if(ifLeafUsed(p)) return false;
    return true;
}

// judge whether the leaf with specific PPointer exists. 
bool PAllocator::ifLeafExist(PPointer p) {
    // TODO
    // 检查fileID以及offset
    if(p.fileId >= maxFileId) return false;
    if(p.offset > LEAF_GROUP_HEAD + (LEAF_GROUP_AMOUNT - 1)*calLeafSize()) return false;
    return true;
}

// free and reuse a leaf
bool PAllocator::freeLeaf(PPointer p) {
    // TODO
    // 若叶子已使用或者以为空则不需要free（返回值不一样）
    if(!ifLeafExist(p)) return false;
    if(ifLeafFree(p)) return true;
    // 更改对应叶组的使用叶子数以及对应位置0
    map<uint64_t,char*>::iterator iter = fId2PmAddr.find(p.fileId);
    char* addre = iter->second;
    uint64_t usdnum;
    memcpy(&usdnum,addre,sizeof(uint64_t));
    usdnum--;
    memcpy(addre,&usdnum,sizeof(uint64_t));
    int weizhi = (p.offset - LEAF_GROUP_HEAD)/calLeafSize();
    *(addre + 8 + weizhi) = 0;
    // 持久化变量
    int leaf_len = LEAF_GROUP_HEAD + LEAF_GROUP_AMOUNT * calLeafSize();
    pmem_msync(addre, leaf_len);
    //将叶子放回空闲列表
    freeList.push_back(p);
    freeNum++;
    return true;
}

bool PAllocator::persistCatalog() {
    // TODO
    // 两个文件位置
    string allcat_path = DATA_DIR + P_ALLOCATOR_CATALOG_NAME;
    string frefile_path = DATA_DIR + P_ALLOCATOR_FREE_LIST;
    // 打开文件以及检查是否成功
    ofstream allocat(allcat_path,ios::out);
    ofstream freelist_file(frefile_path,ios::out);
    if (!(allocat.is_open() && freelist_file.is_open()))
    {
        /* code */
        return false;
    }
    // 写入文件
    allocat.write((char*)&(this->maxFileId),sizeof(uint64_t));
    allocat.write((char*)&(this->freeNum),sizeof(uint64_t));
    allocat.write((char*)&(this->startLeaf),sizeof(PPointer));
    for (uint64_t i = 0; i < freeNum; ++i)
    {
        /* code */
        freelist_file.write((char*)&(freeList[i]),sizeof(PPointer));
    }
  
    return true;
}

/*
  Leaf group structure: (uncompressed)
  | usedNum(8b) | bitmap(n * byte) | leaf1 |...| leafn |
*/
// create a new leafgroup, one file per leafgroup
bool PAllocator::newLeafGroup() {
    // TODO
    // 创建持久化文件
    char *pmemaddr;
    int is_pmem;
    size_t mapped_len;
    string new_path = DATA_DIR + to_string(maxFileId);
    int leaf_group_len = 8 + LEAF_GROUP_AMOUNT + LEAF_GROUP_AMOUNT*calLeafSize();
    if ((pmemaddr = (char*)pmem_map_file(new_path.c_str(), leaf_group_len, PMEM_FILE_CREATE,
        0666, &mapped_len, &is_pmem)) == NULL){
            assert(pmemaddr!=NULL);
            }
    // 将文件地址对应到id
    fId2PmAddr.insert(pair<uint64_t,char*>(maxFileId,pmemaddr));
    // 把新增加的叶子导入到空闲列表
    for(uint64_t i = 0;i < LEAF_GROUP_AMOUNT;i++) {
        PPointer num0;
        num0.fileId = maxFileId;
        num0.offset = LEAF_GROUP_HEAD + i * calLeafSize();
        freeList.push_back(num0);
        this->freeNum++;
    }
    this->maxFileId++;
    // 判断是否为第一个叶组
    if(maxFileId == 2) {
        startLeaf.fileId = 1;
        startLeaf.offset = LEAF_GROUP_HEAD+(LEAF_GROUP_AMOUNT-1)*calLeafSize();
    }

    // 创建叶组文件，并根据| usedNum(8b) | bitmap(n * byte) | leaf1 |...| leafn |格式导入数据
    ofstream new_file_c(new_path, ios::out);
    uint64_t usdnum = 0;
    new_file_c.write((char*)&(usdnum),sizeof(uint64_t));
    const char* bitmap = "0000000000000000";
    new_file_c.write(bitmap,16);
    char leaf_len[LEAF_GROUP_AMOUNT*calLeafSize()];
    memset(leaf_len,0,sizeof(leaf_len));
    new_file_c.write((char*)&(leaf_len),sizeof(leaf_len));

    return persistCatalog();
}