# 论文阅读与前期工作总结
### 姓名：覃伟，汤万鹏，韩俊柠
### 学号：16340200，16340205，16340068
---
## 前期工作

### 使用示意图展示普通文件IO方式(fwrite等)的流程，即进程与系统内核，磁盘之间的数据交换如何进行？为什么写入完成后要调用fsync？
通过博客https://www.cnblogs.com/losing-1216/p/5073051.html

我们得到示意图，如图所示，在文件IO操作中，有fopen，fwrite，fread，fclose。

![image](https://github.com/dbms-19/First-part/blob/master/pic/%E9%A2%981%E7%A4%BA%E6%84%8F%E5%9B%BE.jpg)

**fwrite是系统提供的最上层接口，也是最常用的接口。它在用户进程空间开辟一个CLib buffer，将多次小数据量相邻写操作(application buffer)先缓存起来，合并，最终调用write函数一次性写入（或者将大块数据分解多次write调用）**

**write函数通过调用系统调用接口，将数据从应用层copy到内核层，所以write会触发内核态/用户态切换。当数据到达page cache后，内核并不会立即把数据往下传递。而是返回用户空间。数据什么时候写入硬盘，有内核IO调度决定，所以write是一个异步调用**

**read调用是先检查page cache里面是否有数据，如果有，就取出来返回用户，如果没有，就同步传递下去并等待有数据，再返回用户，所以read是一个同步过程**

**fclose隐含fflush函数,fflush只负责把数据从Clibbuffer拷贝到pagecache中返回，并没有刷新到磁盘上，刷新到磁盘上可以使用fsync函数**

**因为写入完后，fclose并没有将数据刷新到磁盘上，而是将数据刷新到存储介质，fflush函数只是把数据从CLib buffer拷贝到page cache中，并没有刷新到磁盘上，所以调用fsync函数可以将数据刷新到磁盘上**

通过以上解释，得到如下示意图，内核中分页，即page：

![image](https://github.com/dbms-19/First-part/blob/master/pic/%E9%A2%981%E7%A4%BA%E6%84%8F%E5%9B%BE2.jpg)

### 简述文件映射的方式如何操作文件。与普通IO区别？为什么写入完成后要调用msync？文件内容什么时候被载入内存？
通过博客https://www.cnblogs.com/volcao/p/8818199.html 和 https://www.cnblogs.com/alantu2018/p/8506381.html

函数：
- void mmap(void *addr, size_t len, int prot,int flags, int fildes, off_t off) mmap的作用是映射文件描述符和指定文件的(off_t off)区域至调用进程的(addr,addr *len)的内存区域，mmap返回的是用户进程空间的虚拟地址，在stack和heap 之间的空闲逻辑空间(虚拟空间) 就是用来提供映射的，文件将会被映射到这一区域的某块虚拟内存上，具体哪一块若是用户没有指定，则由内核来分配。一般上，用户不该去指定这个映射的起始地址，因为栈和堆都是在向块区域进行扩展的，所以这块区域的大小会一直在变化，若是用户指定，用户根本就无法知道这块地址是否被堆用去了还是被栈用去了。
- int msync(void *addr, size_t len, int flags) 进程在映射空间的对共享内容的改变写回到磁盘文件中，此“冲洗”非彼冲洗，不同于用户缓冲区，此时的冲洗不会洗掉映射存储区的内容，会保留，此冲洗更像是复制写入文件的同步，这也是写入完成后要调用msync的原因，要写入磁盘中。
- int munmap(void *addr, size_t len) 释放存储映射区

**内存映射步骤：**
 - 用open系统调用打开文件, 并返回描述符fd.
 - 用mmap建立内存映射, 并返回映射首地址指针start.
 - 对映射(文件)进行各种操作, 显示(printf), 修改(sprintf).
 - 用munmap(void *start, size_t lenght)关闭内存映射.
 - 用close系统调用关闭文件fd（只要保证在mmap成功了之后就都可以）.
 - msync
 
 **注意事项**
 在修改映射的文件时, 只能在原长度上修改, 不能增加文件长度, 因为内存是已经分配好的.

在如图过程3时，文件内容载入内存。
- 过程1，内存映射。
- 过程2，mmap()会返回一个指针ptr，它指向进程逻辑地址空间中的一个地址，这样以后，进程无需再调用read或write对文件进行读写，而只需要通过ptr就能够操作文件。但是ptr所指向的是一个逻辑地址，要操作其中的数据，必须通过MMU将逻辑地址转换成物理地址，这个过程与内存映射无关。**
- 过程3：建立内存映射并没有实际拷贝数据，这时，MMU在地址映射表中是无法找到与ptr相对应的物理地址的，也就是MMU失败，将产生一个缺页中断，缺页中断的中断响应函数会在swap中寻找相对应的页面，如果找不到（也就是该文件从来没有被读入内存的情况），则会通过mmap()建立的映射关系，从硬盘上将文件读取到物理内存中，这个过程与内存映射无关
- 过程4：如果在拷贝数据时，发现物理内存不够用，则会通过虚拟内存机制（swap）将暂时不用的物理页面交换到硬盘上，这个过程也与内存映射无关。

![image](https://github.com/dbms-19/First-part/blob/master/pic/%E9%A2%98%E7%9B%AE2.jpg)


### 参考[Intel的NVM模拟教程](https://software.intel.com/zh-cn/articles/how-to-emulate-persistent-memory-on-an-intel-architecture-server)模拟NVM环境，用fio等工具测试模拟NVM的性能并与磁盘对比（关键步骤结果截图）。
（推荐Ubuntu 18.04LTS下配置，跳过内核配置，编译和安装步骤）
配置
虚拟机内存:8G
虚拟机镜像：Ubuntu 18.04
运行
````sh
su passwd
123
123

su
123

apt install vim -y
vim /etc/default/grub
````
增加一行
````sh
GRUB_CMDLINE_LINUX_DEFAULT="quiet splash memmap=4G!4G"
````
修改后如图
![grub_modify](./pic/walikrence/grub.png)


配置grub
教程上的用不了，要把grub2全变成grub才行
```sh
grub-mkconfig -o /boot/grub/grub.cfg
````
![pei_zhi_grub](./pic/walikrence/pei_zhi_grub.png)

重启
```sh
reboot
```
查看内核中是否内置了 DAX 和 PMEM
```sh
egrep '(DAX|PMEM)' /boot/config-（按下tab自动补全）
```
![cha_kan](./pic/walikrence/cha_kan.png)

安装包含DAX的文件系统，制作文件系统，映射
````sh
mkdir /mnt/pmemdir
mkfs.ext4 /dev/pmem0
mount -o dax /dev/pmem0 /mnt/pmemdir
lsblk
````
![p4](./pic/walikrence/p4.png)
![p5](./pic/walikrence/p5.png)

安装fio
```sh
apt install fio
```

在NVM上用fio测试性能

![p7](./pic/walikrence/p7.png)
![p6](./pic/walikrence/p6.png)

在磁盘上用fio测试性能

![p8](./pic/walikrence/p8.png)
![p9](./pic/walikrence/p9.png)

可以看到NVM比磁盘快很多




### 使用[PMDK的libpmem库](http://pmem.io/pmdk/libpmem/)编写样例程序操作模拟NVM（关键实验结果截图，附上编译命令和简单样例程序）。
（样例程序使用教程的即可，主要工作是编译安装并链接PMDK库）<br/>
（安装教程：https://docs.pmem.io/getting-started-guide/installing-pmdk/compiling-pmdk-from-source）<br/>
 安装PMDK步骤：
1. 先安装NDCTL
   - 查询存储库以确定是否使用aptitude，apt-cache或apt实用程序传递ndctl
     + $ aptitude search ndctl 
     + $ apt-cache search ndctl 
     + $ apt search ndctl
   - 验证当前是否安装了ndctl软件包并检查版本
     + $ apt list --installed ndctl
   - 安装ndctl软件包或更新已安装的软件包
     + $ sudo apt-get install ndctl
   - 安装依赖包
     + $ sudo apt install -y git gcc g++ autoconf automake asciidoc asciidoctor 
       bash-completion xmlto libtool pkg-config libglib2.0-0 libglib2.0-dev libfabric1 
       libfabric-dev doxygen graphviz pandoc libncurses5 libkmod2 libkmod-dev 
       libudev-dev uuid-dev libjson-c-dev libkeyutils-dev
   - 克隆GitHub存储库
     + $ git config --global http.proxy http://proxyUsername:proxyPassword@proxy.server.com:port
     + $ git config --global https.proxy https://proxyUsername:proxyPassword@proxy.server.com:port
     + $ sudo chmod +w /downloads
     + $ cd /downloads
     + $ sudo git clone https://github.com/pmem/ndctl
     + $ cd ndctl
   - 建立
     + $ ./autogen.sh
     + $ ./configure CFLAGS='-g -O2' --prefix=/usr/local --sysconfdir=/etc --libdir=/usr/local/lib64
     + $ make
     + 构建调试版本:
     + $ ./autogen.sh
     + $ ./configure CFLAGS='-g -O2' --enable-debug --prefix=/usr/local --sysconfdir=/etc --libdir=/usr/local/lib64  
     + $ make
   - 安装
     + $ sudo make install
2. 安装PMDK库
   - 编译器要求
     + $ sudo dnf install gcc gcc-c++
     + $ sudo apt install gcc g++
   - 克隆github
     + $ sudo chmod +w /downloads
     + $ cd /downloads
     + $ git clone https://github.com/pmem/pmdk
     + $ cd pmdk
   - 编译
     + $ make
     + $ make CC=clang CXX=clang++
   - 安装
     + $ sudo make install
     + $ sudo make install prefix=/usr/local
     
<br/>（编译安装PMDK库的过程太过冗长，以至于没有截图，但是有成功运行的截图表明成功安装了PMDK库）<br/>
 ![image](/pic/lbpmem.png)
 <br/>
 ```php
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#endif
#include <string.h>
#include <libpmem.h>

/* just copying 4k to pmem for this example */
#define BUF_LEN 4096

int main(int argc, char *argv[])
{
        int srcfd;
        char buf[BUF_LEN];
        char *pmemaddr;
        size_t mapped_len;
        int is_pmem;
        int cc;

        if (argc != 3) {
                fprintf(stderr, "usage: %s src-file dst-file\n", argv[0]);
                exit(1);
        }

        /* open src-file */
        if ((srcfd = open(argv[1], O_RDONLY)) < 0) {
                perror(argv[1]);
                exit(1);
        }

        /* create a pmem file and memory map it */
        if ((pmemaddr = pmem_map_file(argv[2], BUF_LEN,
                                PMEM_FILE_CREATE|PMEM_FILE_EXCL,
                                0666, &mapped_len, &is_pmem)) == NULL) {
                perror("pmem_map_file");
                exit(1);
        }

        /* read up to BUF_LEN from srcfd */
        if ((cc = read(srcfd, buf, BUF_LEN)) < 0) {
                pmem_unmap(pmemaddr, mapped_len);
                perror("read");
                exit(1);
        }
        /* write it to the pmem */
        if (is_pmem) {
                pmem_memcpy_persist(pmemaddr, buf, cc);
        } else {
                memcpy(pmemaddr, buf, cc);
                pmem_msync(pmemaddr, cc);
        }

        close(srcfd);
        pmem_unmap(pmemaddr, mapped_len);

        exit(0);
}
```

---
## 论文阅读

### 总结一下本文的主要贡献和观点(500字以内)(不能翻译摘要)。
（回答本文工作的动机背景是什么，做了什么，有什么技术原理，解决了什么问题，其意义是什么） 
- 动机背景
SCM的出现，对于研究下一代超高性能存储系统有了新方法，推动对存储系统的重新思考，从而提出一种混合并发方案FPTree。
- 主要工作和所解决的问题
提出了FPTree，并对这其与NV-Tree和wBTree做对比进行全面评估：证明了FPTree的性能优于具有不同SCM延迟的最先进的持久树，最高可达8.2倍；证明了FPTree在具有88个逻辑核心的机器上可以很好运行，与FPTree、NV-Tree和wBTree集成到memcached和数据库原型中，证明FPTree的性能开销几乎可以忽略不计，低于其他tree。
- 技术原理
  - Fingerprinting，指纹是内嵌键的单字节哈希值，连续放置在叶子的第一个缓存行大小的片段中。
  - Selective Persistence，选择性持久性包括将主要数据存储在SCM中，将非主要数据存储在DRAM中。
  - Selective Concurrency，分为为瞬态和持久部分使用不同的并发方案。
  - Sound programming model，确定了四个SCM编程挑战：数据一致性，部分写入，数据恢复和内存泄漏。
- 意义
提出SCM中的持久树——FPTree，为存储系统的发展迈出了很大的一步。
### SCM硬件有什么特性？与普通磁盘有什么区别？普通数据库以页的粒度读写磁盘的方式适合操作SCM吗？
特性：
- SCM有容量大的特性, 容量可以达到1TB甚至更高, 并且价格合理, 能够让大部分人负担得起
- 非易失，固态，无移动区
- SCM具有快速读取的特性, 读取速度仅仅与DRAM相当, 写入速度较DRAM而言则相差10到100倍以上

不同：
- SCM的存储速度远远大于磁盘，能耗低
- 可按字节寻址，可以直连内存总线并使用访存指令进行读写
- 写入的次数有限, 写入几百万次时可能会造成永久失效的问题. 而磁盘寿命非常长。

由于写入次数少，而普通数据库读写磁盘会产生不必要的读写次数，并且SCM具有字粒度，以普通数据库的页粒度可能产生问题。


### 操作SCM为什么要调用CLFLUSH等指令？
(写入后不调用，发生系统崩溃有什么后果)  
- CLFLUSH。CLFLUSH（Cache Line Flush，缓存行刷回）能够把指定缓存行（Cache Line）从所有级缓存中淘汰，若该缓存行中的数据被修改过，则将该数据写入主存；支持现状：目前主流处理器均支持该指令。
- 操作SCM如果不调用CLFLUSH等指令，可能导致读取数据产生错误，跟普通数据库缓冲区类似。

### FPTree的指纹技术有什么重要作用？
- 指纹是在叶内一字节的哈希表，通过记录叶键的一个字节长度的哈希，防止探测没有匹配的键，提高性能。

### 为了保证指纹技术的数学证明成立，哈希函数应如何选取？
（哈希函数生成的哈希值具有什么特征，能简单对键值取模生成吗？）  
- 哈希函数需要满足均匀分布m个元素在n个可能的哈希值中，不可以对键值进行简单取模。

### 持久化指针的作用是什么？与课上学到的什么类似？
- 内存泄露，非法指针，数据恢复等问题。
