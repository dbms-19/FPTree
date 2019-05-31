### 第二阶段

#### 任务要求

- FPTreeDB插入和重载操作并通过相关测试，对应fptree.cpp的实现和fptree_test.cpp部分的运行

#### 过程

**插入与重载**
#### KeyNode
这个数据结构由一个代表键值和节点的索引组成，用于节点分裂时，将新生成的节点索引返回给上层节点插入记录。其用在插入操作和分裂操作中。如下图的插入操作:  
![](asset/insert.png)

#### InnerNode
管理方法和书本的B+tree基本一致。这是FPTree中间索引节点，其不存放键值对数据信息。这个数据结构的内容已经给出，结构如下：  
![](asset/InnerNode.png)
#### 节点元素细节
通过对节点的合并和分裂操作，保证每个节点的元素个数限制在以下范围(FPTree的根少于d)：  
key个数 : d <= m <= 2d </br>
node指针个数 : d + 1 <= m < = 2d + 1 </br>

记得保持节点元素有序，这样才可以使用二分查找findIndex。**键值的搜索规则是左闭右开原则**，与书本的一直，即Km <= search <Km+1，Km和Km+1所夹住的索引即为目标节点

```
cd src
make
cd bin
./main
```
![image](https://github.com/dbms-19/FPTree/blob/v2/v2.1.jpg)
![image](https://github.com/dbms-19/FPTree/blob/v2/v2.2.jpg)

```
cd test
make
cd bin
./f*
```
![image](https://github.com/dbms-19/FPTree/blob/v2/v2.jpg)
