### 第一阶段

#### 任务要求

- PAllocator实现并通过utility测试
- LevelDB的使用以及测试，对应lycsb.cpp，p_allocator.cpp的实现和运行，utility_test.cpp的运行

#### 过程

**PAllocator**

- 初始化仓库
- 初步完成系统说明书
- 实现lycsb.cpp

- 初始化README.md
- 完成测试代码makefile的编写
  根据[gtest仓库的示例makefile](https://github.com/google/googletest/blob/master/googletest/make/Makefile)完成了测试代码的makefile
  
- 完成p_allocator.cpp的实现和运行，utility_test.cpp的运行
- 更改makefile的编写以及测试。
- 测试：
  + 命令：
     - cd test
     - make
     - cd bin
     - mkdir data
     - ./utility_test
  + 测试结果截图：
     - ![编译](https://github.com/dbms-19/FPTree/blob/master/1_complie.png)
     - ![结果](https://github.com/dbms-19/FPTree/blob/master/1_result.png)

- 理解：这是使用树结构来存储数据，每个叶子节点就是一个叶组，每个叶组里有16个相对内存地址以及对应的id，相对内存地址加上叶组所在的地址就是指向数据所在的地址。

---
<br>
参考资料  <br>

[NVM编程文档](https://docs.pmem.io/getting-started-guide/introduction)  <br>

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

![image](https://github.com/dbms-19/FPTree/blob/master/pic/2.jpg)

```
读取到的与(key=apple)对应的(value=A)
```

然后对lycsb.cpp进行编写，编译运行得到结果：

![image](https://github.com/dbms-19/FPTree/blob/master/pic/1.jpg)
