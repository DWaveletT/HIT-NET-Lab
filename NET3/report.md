# 提示：尚未完成

## Wireshark 的使用

![捕获 WLAN](./images/1-capture.png)

![捕获界面](./images/1-tools.png)

打开 Wireshark，捕获无线网（WLAN）里的请求。

![捕获结果](./images/1-homepage.png)

尝试使用浏览器访问 http://www.hit.edu.cn，在 Wireshark 里筛选 HTTP 协议的捕获，可在捕获列表里查询到访问 www.hit.edu.cn 的 GET 请求。结果如上图所示。

## 利用 Wireshark 分析 HTTP 协议

### HTTP GET/response 交互

![捕获结果](./images/2-today.png)

在浏览器里键入地址 http://today.hit.edu.cn，同时使用 Wireshark 捕获 HTTP 协议。结果如上图所示。

![浏览器请求](./images/2-request.png)

![服务器返回](./images/2-response.png)

观察上述浏览器发送的 HTTP 请求以及服务器做出的响应，可在捕获结果中发现：

- 浏览器发送地址是 172.20.93.244，发送使用的 HTTP 协议版本号为 1.1；
- 服务器返回地址是 202.118.254.117，发送使用的 HTTP 协议版本号为 1.1，返回的状态码为 200 OK。

![请求的语言](./images/2-language.png)

此外还可以在浏览器请求信息里看到接受的语言信息为中文（大陆）、中文（台湾）、中文（香港）、英文、日文。

总结：

- 浏览器运行的 HTTP 协议版本号为 1.1，服务器返回的 HTTP 协议版本号为 1.1；
- 浏览器向服务器指出可以接受中文（大陆）、中文（台湾）、中文（香港）、英文、日文的对象；
- 计算机的 IP 地址为 172.20.93.244，服务器的 IP 地址为 202.118.254.117；
- 从服务器返回的状态码为 200。

### HTTP 条件 GET/response 交互

![第一次请求](./images/2-request-1.png)

![第一次响应](./images/2-response-1.png)

![第二次请求](./images/2-request-2.png)

![第二次响应](./images/2-response-2.png)

- 在第一次访问中，浏览器发送的信息里不带有 If-Modified-Since 头部，返回的状态码为 200 且包含完整的网页信息；
- 在第二次访问中，浏览器发送的请求带上了头部 If-Modified-Since: Wed, 30 Oct 2024 06:03:24 GMT，表示询问服务器是否在国际标准时间 2024 年 10 月 30 号 6 点 3 分 24 秒星期三之后发生了修改。服务器返回响应的状态码为 304 Not Modified，告知浏览器网页内容未发生改变，且未返回网页信息，由浏览器直接调用缓存。

## 利用 Wireshark 分析 TCP 协议

![捕获 TCP 协议](./images/3-tcp.png)

向该网站传输 alice.txt，捕获到的 TCP 协议如图所示。

从中可以得到向 gaia.cs.umass.edu 传输数据的客户端 IP 为 172.20.93.244，可在 Info 当中看到客户端发送端口为 61619，服务器接收端口为 80。而 gaia.cs.umass.edu 的 IP 地址为 128.119.245.12，它用于接收该数据所用的端口为 80。

![客户端 SYN](./images/3-client-syn.png)

从客户端发送出的 SYN 报文段序号为 1419927396。在该报文段里通过标记位 SYN=1 来表示这是一个 SYN 报文段。

![服务端 SYN](./images/3-server-syn.png)

从服务端发送出的 SYNACK 报文段序号为 1947904331，该报文段中 ACK 值为 1419927397，刚好为客户端发送的 SYN 报文段的序号值加一。在该报文段里，通过标记位 SYN 和 ACK 均设置为 1 来表示这是一个 SYNACK 报文段。

![三次握手](./images//3-tcp-3.png)

在图中可以看出 TCP 协议三次握手的过程。这三次报文传输的长度都很短，且不包含具体传输内容，只设置了状态位以及 SYN ACK 之类参数。

![POST 命令](./images/3-post.png)

在图中可以看出携带 POST 命令的 TCP 报文，序号为 1419927397，刚好为客户端 SYN 报文序号加一。

![第六个报文](./images/3-sixth-tcp.png)

在图中可以看出第六个 TCP 报文的序列值为 1419933491，相对值为 6095。该报文是在进行三次握手之后发送的第六个 TCP 报文，即整个过程当中第九个 TCP 报文。而该报文的对应的 ACK 序号则是在第三次握手的时候接收的。

![报文段长度](./images/3-length.png)

图中可以看出前六个报文段的长度分别为 654，1360，1360，1360，1360，1360。

![接收方缓存长度 1](./images/3-buffer-1.png)

![接收方缓存长度 2](./images/3-buffer-2.png)

整个追踪过程中接收方公示的最小缓冲长度为 29200，而在绝大多数应答中缓冲长度在不断增加，只有少量波动。说明接收方的缓冲长度始终够用。

![报文接收序号](./images/3-receive.png)

在整个追踪过程中，发现接收方获取到的数据序号保持单调增长，说明没有发生数据重传。

在图片里可以看出 TCP 连接开始建立的时间戳为 1.5761818 秒，而在服务端返回所有 ACK 报文并结束 TCP 通信时的时间戳为 6.548545 秒。整个过程中传输的数据总量为 152975 字节，因此可以得到平均传输速度为 30765 字节每秒。

## 利用 Wireshark 分析 I P 协议



## 利用 Wireshark 分析 Ethernet 数据帧


## 利用 Wireshark 分析 DNS 协议

在浏览器内键入 https://www.baidu.com，同时使用 Wireshark 捕获 DNS 结果如下：

![DNS 查询](./images/6-query.png)

![DNS 结果](./images/6-answer.png)

分析如下：

- 浏览器向 DNS 服务器 8.8.8.8 发送了 DNS 查询，是一个 A 类查询，询问 URL www.baidu.com 的 IP 地址。
- DNS 服务器 8.8.8.8 向浏览器返回的 DNS 查询结果包含三条，分别如下：
  - 第一个结果是一条 CNAME 类回答，说明 www.baidu.com 是一个别名，对应的真实网址为 www.a.shifen.com；
  - 第二个结果是一条 A 类回答，说明 www.a.shifen.com 域名对应一条 IP 地址为 39.156.66.14；
  - 第三个结果是一条 A 类回答，说明 www.a.shifen.com 域名对应一条 IP 地址为 39.156.66.18。

此次查询结果说明 www.baidu.com 映射到网址 www.a.shifen.com，而该网址下又有两个 IP 地址 39.156.66.14 和 39.156.66.18。浏览器可以访问这两个 IP 地址之一来从百度获取资源。

## 利用 Wireshark 分析 UDP 协议


## 利用 Wireshark 分析 ARP 协议

