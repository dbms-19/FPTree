### 第二阶段

#### 任务要求

- FPTreeDB查询和更新操作并通过相关测试，对应fptree.cpp的实现和fptree_test.cpp部分的运行

#### 过程

**查询更新**

#### 键值对查询（简单）
##### 直接对应函数
Value InnerNode::find(Key k)
##### 函数说明
这是InnerNode的查询函数，不进行实际的查询。二分查找目标子节点，递归调用其对应子节点的查询函数，直至叶子节点进行实际的查询，返回查询得到的值。查询失败返回MAX_VALUE。

#### 键值对修改（简单）
##### 直接对应函数
bool InnerNode::update(Key k, Value v)

##### 函数说明
这是InnerNode更新键值对的函数，递归调用至叶子节点进行实际的更新，返回值为是否更新成功。因为框架限制，不能执行论文里的先插入再删除策略实现更新，有想法的同学可以将返回值改成KeyNode*，在叶子节点的update里进行先插入在删除（测试代码不兼容，请另外实现一个函数，注明为额外工作）。

**编译运行**
```
cd test
make
cd bin
./f*
```
![image](https://github.com/dbms-19/FPTree/blob/v3/v3.jpg)
