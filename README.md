# FPTree
FPTree

## 工作进度
---

|第一阶段|系统说明书|PAllocator实现|通过utility测试|LevelDB的使用以及测试|lycsb.cpp|p_allocator.cpp|utility_test.cpp的运行|
| ------ | ------ | ------ | ------ | ------ | ------ | ------ | ------ |
|完成情况|[覃伟][汤万鹏][]|[]|[]|[覃伟]|[覃伟][汤万鹏]|[]|[]|
|第二阶段|FPTreeDB插入|FPTreeDB重载|fptree.cpp|fptree_test.cpp|测试文件makefil|
|完成情况|[]|[]|[]|[]|[汤万鹏]|
|第三阶段|FPTreeDB查询|FPTreeDB更新|fptree.cpp|fptree_test.cpp|
|完成情况|[]|[]|[]|[]|
|第四阶段|FPTreeDB删除操作|所有剩下实现以及测试|fptree.cpp|fptree_test.cpp|
|完成情况|[]|[]|[]|[]|

---
### 第一阶段

#### 任务要求

- PAllocator实现并通过utility测试
- LevelDB的使用以及测试，对应lycsb.cpp，p_allocator.cpp的实现和运行，utility_test.cpp的运行

#### 过程

**PAllocator**

**LevelDB**

Leveldb是一个google实现的非常高效的kv数据库，目前的版本1.2能够支持billion级别的数据量了。 在这个数量级别下还有着非常高的性能，主要归功于它的良好的设计。特别是LSM算法。

通过开源网站（<https://github.com/google/leveldb>），我们将levelDB源码下载。

通过说明书和博客，我们得知下面编译安装步骤：

**编译源码：**

```linux
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build .
```

**生成本地环境**，编译完后，在build文件中生成所编译的文件，我们需要编译生成的静态库，**libleveldb.a**，将其复制到本地编译库中：

```
sudo cp build/libleveldb.a /usr/local/lib/
sudo cp -r include/leveldb/ /usr/local/include/
```

这样便在本地搭建了levelDB编译环境，可以通过简单代码进行测试：

新建文件demo.cpp

```c++
#include <cassert>
#include <iostream>
#include <string>
#include <leveldb/db.h>

int main() {
  leveldb::DB* db;
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
  assert(status.ok());

  std::string key = "apple";
  std::string value = "A";
  std::string get;

  leveldb::Status s = db->Put(leveldb::WriteOptions(), key, value);
  
  if (s.ok()) s = db->Get(leveldb::ReadOptions(), key, &get);
  if (s.ok()) std::cout << "读取到的与(key=" << key << ")对应的(value=" << get << ")" << std::endl;
  else std::cout << "读取失败!" << std::endl;

  delete db;

  return 0;
}
```

终端输入编译命令：

```
g++ -o demo demo.cpp -pthread -lleveldb -std=c++11
$ ./demo
```

结果如下，代表成功：
!(image)[https://github.com/dbms-19/FPTree/tree/master/pic]

```
读取到的与(key=apple)对应的(value=A)
```

然后对lycsb.cpp进行编写，编译运行得到结果：

---
加分项

|加分项|完成情况|实现人|
|---|---|---|
|实现原始FPTree的micro-log机制 |[]| |
|实现另外版本的叶子分裂和叶子删除函数 |[]| |
|自写google test单元测试 |[]| |
|实现多线程版本的FPTree |[]| |
|基于简单的节点加锁机制实现 |[]| |
|对一个节点读写时相应加上共享锁和排它锁 |[]| |
|另外用一个项目实现CFPTree |[]| |



---
## 参考资料
[内存计算技术研究综述](http://www.jos.org.cn/html/2016/8/5103.htm#top) <br>
[内存计算介绍](https://www.kancloud.cn/digest/in-memory-computing/202153) <br>
[模拟对象](https://zh.wikipedia.org/wiki/%E6%A8%A1%E6%8B%9F%E5%AF%B9%E8%B1%A1) <br>
[gtest基础](https://github.com/google/googletest/blob/master/googletest/docs/primer.md) <br>
[gtest官方repo](https://github.com/google/googletest) <br>
[LevelDB基础操作](https://github.com/google/leveldb/blob/master/doc/index.md) <br>
[levelDB官方repo](https://github.com/google/leveldb) <br>


