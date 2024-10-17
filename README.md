## 哈工大计算机网络实验（2024 年）

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

## 实验 2 & 3 GBN/SR 协议的设计与实现

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

