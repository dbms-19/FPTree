### 第四阶段

#### 任务要求

- FPTreeDB查询和更新操作并通过相关测试，对应fptree.cpp的实现和fptree_test.cpp部分的运行

#### 过程

**剩余操作**
#### 键值对删除（困难）
##### 直接对应函数
bool InnerNode::remove(Key k, int index, InnerNode* parent, bool &ifDelete)
##### 函数参数说明
1. k：目标键值对的键
2. index：当前节点属于其父亲节点的第几个元素
3. parent：当前节点的父亲节点
4. ifDelete：当前节点为空则需要删除，返回给父亲节点是否删除的信息

index和parent都是方便节点删除元素后进行重分布或者合并操作，方便获得其兄弟节点。返回值为是否删除成功。

##### 函数说明
这是InnerNode删除键值对元素的函数，其不进行实际的键值对删除，通过递归调用其子节点删除，一直到叶子节点进行实际的删除。对子节点返回的信息进行处理，识别其下层调用的节点是否要被删除。删除子节点的时候，识别当前节点元素是否符合限制，不符合则进行重分布，重分布不行再合并。

##### 细节
当其子节点需要删除时，检查是否需要重分布和合并。合并的时候需要注意其父亲节点是否是根节点且其只有两个孩子，那么就合并这三者。注意的是如果同时有左右兄弟，左兄弟可以进行重分布，右兄弟可以合并，优先重分布。删除过程中一共有7种情况：
1. 子节点没有被删除，ifDelete返回给父亲节点
2. 子节点被删除，出现下面6六种情况：
   1. 当前节点元素足够，不需要进行重分布和合并
   2. 当前节点元素不够，与左兄弟重分布
   3. 当前节点元素不够，与右兄弟重分布
   4. 当前节点元素不够，与左兄弟合并
   5. 当前节点元素不够，与右兄弟合并
   6. 当前节点元素不否，父亲只有两个孩子(左或右)且父亲节点为根节点，合并这三者

##### 调用的函数
1. int InnerNode::findIndex：二分查找目标子节点。
2. void InnerNode::getBrother()：获得中间节点的左右兄弟节点。如果左右兄弟都存在，统一处理右兄弟。
3. void InnerNode::redistributeRight()：与右兄弟重分布
4. void InnerNode::redistributeLeft()：与左兄弟重分布
5. void InnerNode::mergeLeft()：与左兄弟合并
6. void InnerNode::mergeRight()：与右兄弟合并
7. void InnerNode::mergeParentLeft()：与父亲节点以及左兄弟合并
8. void InnerNode::mergeParentRight()：与父亲节点以及右兄弟合并
9. int InnerNode::findIndex()：二分查找

```
cd test
make
cd bin
./f*
```
![image](https://github.com/dbms-19/FPTree/blob/final/final.jpg)

#### 总结
- 本次项目，由于一些原因，一开始并没有着手准备，中途才开始第一阶段，小组分别学习并编译安装leveldb，书写pallocator，大概在15日左右完成第一阶段。
- 第二阶段完成于20日，了解代码和fptree结构后，书写插入和重载部分。
- 第三阶段和第四阶段完成了31日，第三阶段，只更新update，所以并没有花很长时间，第四阶段，难度较大，因为本组都是重修，所以有提问于其他小组完成，特此说明，如果有扣分，即本组责任。
- 最终，赶在31号之前，基本完成了本次项目，ycsb函数没有完成，具体可能没有理解其函数内容，希望TA酌情扣分，项目分值比重大，重修小组还是希望平时分高一点。感谢！

#### ycsb
### 书写ycsb
通过阅读相关内容得知，ycsb和lycsb内容大致相同，过程相同，所以代码仿照lycsb书写即可。
函数主要有3个部分
- read

read部分，通过文件读取，fscanf(fp, "%s %lu", op, &key) == EOF，将键值和是否插入保存。

- operate_leveldb

leveldb与lycsb内容相同，代码直接复制即可。

- operate_Ftptree

为Ftptree操作，如果为插入即执行树的插入，如果不是，即查询。

为了执行ycsb的Ftptree，我们需要基于NVM进行操作，所以我们模拟NVM环境，这里即根据第一部分模拟NVM操作，将Ftptree存储数据的文件挂上NVM。

命令如下：

```
	sudo umount /dev/pmem0
	sudo mkfs.ext4 /dev/pmem0
	sudo mount -o dax /dev/pmem0 ./bin/data
	sudo chmod -R 0777 ./bin/data
```

将命令写入makefile中，进入src文件夹，make，在bin中执行ycsb可执行文件即可。

![image](https://github.com/dbms-19/FPTree/blob/master/ycsb.jpg)





