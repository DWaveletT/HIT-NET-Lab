## 哈工大计算机网络实验（2024 年秋）

收集了作者在哈尔滨工业大学大三学年秋季学期计算机网络课程编写的实验项目。持续更新。

因为历年以来公开的参考资料稀少，哈工大实验指导书上提供的代码难以阅读且缺乏注释，同时网络上的资料难以查找且质量良莠不齐，作为已经（正在）费力完成这些实验报告的学生，希望分享这些宝贵的前人经验以供后人参考。本仓库同时也参考了学长（主要是[这个仓库](https://github.com/HIT-SCIR-chichi/hit_computer_network)）以及 ChatGPT 的代码实现，在这里衷心的表示感谢。

由于个人隐私等原因，实验报告不会上传到 Github 上。我也希望大家真的学习这些知识自行编写实验报告。

如果有帮助的话，希望给个 Star）

还在施工中，会陆续补齐一些实验报告的说明。

## 实验 1 HTTP 代理服务器的设计与实现

### 介绍

报告书上给出了利用 Windows 相关库的 C 语言实现。

然而由于众所周知的原因，使用 C 语言编写这类底层代码是既丑陋又缺乏可移植性（比如那份代码在我电脑上就跑不起来，下了个 Vistal Studio 才能跑），所以这里使用 Java 进行编写。

另外需要注意的是，这个实验只能测试使用 HTTP 协议（而不是 HTTPS！）的网站，别的网站会出现无法访问的情况。如果有验收需求，推荐使用以下网址（没错，哈工大自己的网页一堆 HTTP 协议的）：

- `http://jwts.hit.edu.cn`；
- `http://jwes.hit.edu.cn`；
- `http://today.hit.edu.cn`；
- `http://www.example.com`。

主要的源码文件有两个，`SocketServer.java` 和 `test.py`。前者是代理服务器源码，后者是测试用的 Python 脚本。

### `SocketServer.java`

`SocketServer.java` 的运行环境为 JDK 22.0.1，低一点应该也行（毕竟 Socket 是很多年前的技术了）。如果是 Minecraft 玩家大概率是可以直接编译运行这些代码的。编译和运行指令分别如下：

```java
javac SocketServer.java
java SocketServer.java
```

代理服务器默认监听 808 端口，可以在 136 行修改。Windows 系统配置代理服务器需要打开设置并且搜索“代理”。由于 Chrome 浏览器等自己也会发送很多请求，所以控制台显示会很糟糕，而且会爆一堆 Exception，这个是正常情况，因为该服务器无法处理 HTTPS 协议的请求。

源码说明：

- 23-24 行可以将 `http://jwts.hit.edu.cn` 自动跳转到 `http://jwes.hit.edu.cn`，实现钓鱼网站功能；
- 26 行可以屏蔽 `today.hit.edu.cn`，实现屏蔽特定网站的功能；
- 27 行可以屏蔽 `127.0.0.1`，实现屏蔽特定用户的功能。

理论上向网页发送带有 `If-Modified-Since` 字段的请求，如果页面未修改会返回 304 并且不附带有载荷，但经过测试哈工大的网页都不支持这个功能（返回的都是 200 且带有完整载荷），所以建议使用 `http://www.example.com` 进行测试。生成的缓存会被存储在下面的 cache 文件夹里，文件名为 URL 的哈希值。如果不存在缓存会自动创建一个。

### `test.py`

`test.py` 的运行环境为 Python 3，需要 requests 库（没有的话就用 pip 装一下）。功能很简单，以 808 端口的 SocketServer 作为代理服务器，每次要你输入地址，然后会返回 response 的状态码以及具体内容。

使用 `test.py` 就不需要开启设置里的全局代理了，SocketServer 的控制台日志输出也会很清净（因为只会接收到 test.py 发的请求）。适合向助教/老师验收。

注意事项：

- 如果访问了被屏蔽的网站/是被屏蔽的用户，由于 SocketServer **不会返回任何东西就结束 Socket**，所以 `test.py` 一定会**报 Exception 并退出**，这个是正常现象。

## 实验 2 GBN/SR 协议的设计与实现

报告书上给出了利用 Windows 相关库的 C 语言实现。同样的我们不用 C 而是主要用 Python 实现。

主要使用 Python 3 编写。

需要包含的库如下：

```python
import toml

import random
import socket
import select
import time
```

如果没有就自己用 pip 装一下。

### `config.toml`

是 TOML 格式的配置文件。

第一部分包含 Host 的端口号以及在通信过程中的名字。可以自己换一个喜欢的。

第二部分是 GBN 和 SR 协议的配置文件，其中 `loss` 是丢失率（包括传输数据包的以及传输 ACK 的，SR 协议里这俩分开了，因为 SR 是我后写的，回头懒得改 GBN 了）。`windows_size` 和 `identi_size` 分别是窗口大小和通信发包的 Seq 段的范围。

- 网上实现的 GBN 协议的 `identi_size` 大小都很大，我有点困惑（我尝试把它设置成窗口大小加一，但这样和 GBN 的通常实现不符），但是老师没问就没管；
- SR 协议要求 `windows_size` 不大于 `identi_size` 的一半，这个是协议本身决定的。

### `proto_gbn.py` 和 `proto_sr.py`

分别放置了 GBN 协议的服务/客户端、SR 协议的服务/客户端的代码。

### `send_data.txt` 和 `recv_data.txt`

前者是发送数据，后者是接收数据。后者记得验收时清空。

可以使用 `diana.txt` 里的文章作为测试数据，也可以直接用更朴素的 `digit.txt`。取决于你是不是夹心糖。

### `test.py`

可以用来测试 GBN 协议和 SR 协议，里面的调试信息应该已经很清楚了。

### `test_host1.py` 和 `test_host2.py`

用于测试两个主机使用 GBN 协议通信。**需要同时运行**。

两个控制台都可以直接输入，消息会被传输到对面控制台上。

有小 bug，因为每两个线程共用一个控制台，所以会出现比较尴尬的 IO 阻塞问题。但如果你运气好在测试的时候没有发生丢包，那就不会出现这个 bug。因为我验收的时候赌赢了 10% 的概率没有出 bug，所以没修，有缘人可以自己修一下。

## 实验 4 IP 数据报的收发与转发

代码主要参考了实验指导书上下发内容。

这个实验需要配置虚拟机环境。虚拟机软件推荐使用 VMWare Workstation Pro，**不推荐 VMWare Player**。因为实验环境的搭建需要同时运行多台虚拟机，Workstation 提供**链接拷贝功能**（拷贝结果占用空间很小）而 Player 只能完整复制（拷贝结果占用空间很大，而且管理不方便）。Workstation 的个人使用不需要许可证，挺方便的。

虚拟机操作系统使用 Debian 64 位，资源占用少，配置方便。

### 虚拟机环境配置说明

以下是配置完全后在 VMWare 查看的界面：

![配置](/images/lab4-config.png)

为了方便虚拟机执行程序，以及方便主机对源码进行修改并及时编译运行，建议配置**共享文件夹**。在 VMWare 里设置共享文件夹需要预先安装 VMWare Tools（有些操作系统镜像安装完成后自带该工具，不需要重新安装）。这部分资料网上很多，可以自行查找。

设置好共享文件夹后需要运行如下指令**将其挂载到虚拟机文件系统下面**：

```bash
vmhgfs-fuse .host:/ /mnt/hgfs
```

然后可以使用 cd 命令进入挂载的共享文件夹，例如我们将文件夹 `Machine 1` 挂在给了某台虚拟机，就可以在终端里执行以下指令：

```bash
cd "/mnt/hgfs/Machine 1"
```

![配置](/images/lab4-config-folder.png)

通过如下方式配置虚拟机网卡的 MAC 地址：

![配置](/images/lab4-config-mac.png)

通过如下方式新建虚拟机网卡：

![配置](/images/lab4-config-ifr.png)

打开虚拟机，运行如下指令查看当前虚拟机安装的网口信息：

```bash
ip a
```

一般来说，网口的名字是 ens33。

![配置](/images/lab4-config-card.png)

打开虚拟机，运行如下指令打开网络适配器配置文件，对上一步查看到的适配器进行配置：

```bash
sudo nano /etc/network/interfaces
```

![配置](/images/lab4-config-nano.png)

完成上述过程，即可实现对虚拟机网卡数量的配置、对 MAC 地址的配置、对 IP 地址以及局域网环境的配置。下面不再赘述如何配置机器。

需要注意实验代码里面硬编码了相关的 IP 地址、端口、MAC 地址，如果看不懂程序请严格按照下面描述的 IP 地址、MAC 地址等进行配置。

### 内容 1

使用三台单网口虚拟机实现消息的发送、转发和接收。用到的虚拟机如下：

- 发送机：配置 IP 地址 192.168.1.1，MAC 地址为 `AA:AA:AA:AA:AA:AA`，运行的程序源码被放在了 `NET4/Machine 1/t1-send.c` 位置；
- 接收机：配置 IP 地址 192.168.1.2，MAC 地址为 `AA:AA:AA:AA:AA:BB`，运行的程序源码被放在了 `NET4/Machine 2/t1-recv.c` 位置；
- 转发机：配置 IP 地址 192.168.1.3，MAC 地址为 `AA:AA:AA:AA:AA:CC`，运行的程序源码被放在了 `NET4/Machine 3/t1-forward.c` 位置。

在发送机输入要发送的信息，信息会经过转发机转发到达接收机。

### 内容 2

使用五台单网口虚拟机实现消息的发送、转发和接收。用到的虚拟机如下：

- 发送机：配置 IP 地址 192.168.1.1，MAC 地址为 `AA:AA:AA:AA:AA:AA`，运行的程序源码被放在了 `NET4/Machine 1/t2-send.c` 位置；
- 接收机：配置 IP 地址 192.168.1.2，MAC 地址为 `AA:AA:AA:AA:AA:BB`，运行的程序源码被放在了 `NET4/Machine 2/t2-recv.c` 位置；
- 路由机 1：配置 IP 地址 192.168.1.3，MAC 地址为 `AA:AA:AA:AA:AA:CC`，运行的程序源码被放在了 `NET4/Machine 3/t2-route.c` 位置。
- 路由机 2：配置 IP 地址 192.168.1.4，MAC 地址为 `AA:AA:AA:AA:AA:DD`，运行的程序源码被放在了 `NET4/Machine 4/t2-route.c` 位置。
- 路由机 3：配置 IP 地址 192.168.1.5，MAC 地址为 `AA:AA:AA:AA:AA:EE`，运行的程序源码被放在了 `NET4/Machine 5/t2-route.c` 位置。

在发送机输入要发送的信息，信息会依次经过路由机 1、路由机 2、路由机 3 转发到接收机。路由机的终端下会显示发送的以太帧的源 MAC 地址、目的 MAC 地址、源 IP、目的 IP、TTL 等信息。

注意：在我的环境下五台机器的网口名字都叫 ens33。如果你通过 ip a 指令查询到的名字不是这个，可能需要针对代码进行修改。

### 内容 3

使用两台单网口虚拟机和一台双网口虚拟机模拟两个子网间通过一台双网口机器进行转发。用到的虚拟机如下：

- 发送机：配置 IP 地址 192.168.1.1，MAC 地址为 `AA:AA:AA:AA:AA:AA`，运行的程序源码被放在了 `NET4/Task 3/t3-send.c` 位置；
- 接收机：配置 IP 地址 192.168.2.1，MAC 地址为 `AA:AA:AA:AA:AA:BB`，运行的程序源码被放在了 `NET4/Task 3/t3-recv.c` 位置；
- 路由机：该路由器运行的程序源码被放在了 `NET4/Task 3/t3-route.c` 位置。该机器的两个网口同时处于两个子网内。分别配置如下：
  - 网口 1：配置 IP 地址 192.168.1.2，MAC 地址为 `AA:AA:AA:AA:AA:CC`；
  - 网口 2：配置 IP 地址 192.168.2.2，MAC 地址为 `AA:AA:AA:AA:AA:DD`。

在正确的配置下，使用发送机直接 ping 接收机，或者用接收机直接 ping 发送机都是 ping 不通的。发送机可以 ping 通路由机的 192.168.1.2 地址，接收机可以 ping 通路由机的 192.168.2.2 地址。

为了演示双向通信，当同时运行三台机器的程序后，发送机处于等待终端输入状态，而接收机处于等待发送机发送来的消息的状态。当发送机经路由机转发向接收机发送一条消息后，**两者的角色互换**。

注意：在我的环境下前两台机器的网口名字都叫 ens33，路由机的网口名字分别是 ens33 和 ens36。如果你通过 ip a 指令查询到的名字不是这个，可能需要针对代码进行修改。