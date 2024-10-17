import toml

import random   # 用于模拟信息传输的丢失
import socket   # 用于建立两个程序的通信
import select
import time

class SR:

    def __init__(self, name: str, local_host: tuple[int, str], remote_host: tuple[int, str], filename: str):
        config = toml.load("./config.toml")['SR']
        self.name = name                            # 方便测试用的名字
        self.filename = filename                    # 与该客户端关联的文件名

        self.window_size = config['window_size']    # 配置的窗口大小
        self.identi_size = config['identi_size']    # 配置的序列大小
        self.timeout = config['timeout']            # 配置的超时限制
        self.host1_loss = config['host1_loss']      # 发送的数据的丢失率
        self.host2_loss = config['host2_loss']      # 接收的应答的丢失率
        
        self.base = 0       # 窗口起始序列号
        self.next = 0       # 下一个要发送 / 接收的序列号

        self.finish = False

        self.data = [""] * self.identi_size
        self.time = [0]  * self.identi_size

        self.recv_buf_size = 1024
        
        self. local_host =  local_host  # 设置本地 socket 地址
        self.remote_host = remote_host  # 设置远程 socket 地址
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.bind(self.local_host)
            # 绑定套接字的本地 IP 地址和端口号
    
    def make_packet(self, id: int, data: str):
        return (str(id) + ' ' + str(data)).encode('utf-8')
    
    def make_ack(self, id: int):
        return str(id).encode('utf-8')
    
    def send_packet(self):     # 服务器发送数据包
        if self.finish:
            print(f"[{self.name}]: 数据包已发送完毕！等待应答。")
        else:
            if self.next < self.base + self.window_size:
                ind = self.next % self.identi_size
                if self.data[ind] == "":
                    self.data[ind] = self.f.readline()
                    if self.data[ind] == "":
                        self.finish = True
                        self.data[ind] = "FINISH"
                        print(f"[{self.name}]: 文件读取完毕！")
                
                print(f"[{self.name}]: 发送数据包 {ind}。")

                if random.random() < self.host1_loss:
                    print(f"[{self.name}]: 丢失数据包 {ind}！")
                else:
                    self.socket.sendto(
                        self.make_packet(ind, self.data[ind]),
                        self.remote_host
                    )
                        # 向对面发送数据包，按照既定格式

                self.next = self.next + 1
            else:
                print(f"[{self.name}]: 窗口已满，暂停发送。")
    
    def send_again(self):
        for i in range(self.base, self.next):
            ind = i % self.identi_size

            if self.data[ind] == "":
                continue

            self.time[ind] += 1
            if self.time[ind] > self.timeout:
                print(f"[{self.name}]: 超时！重新发送数据包 {ind}。")
                self.time[ind] = 0

                if random.random() < self.host1_loss:
                    print(f"[{self.name}]: 丢失数据包 {ind}！")
                else:
                    self.socket.sendto(
                        self.make_packet(ind, self.data[ind]),
                        self.remote_host
                    )
                        # 向对面发送数据包，按照既定格式
    
    def recv_ack(self):         # 服务器接收 ACK
        readable = select.select([self.socket], [], [], 1)[0]
        if len(readable) > 0:
            recv_ack = self.socket.recvfrom(self.recv_buf_size)[0].decode().split()[0]

            print(f'[{self.name}]: 收到客户端 ACK：[{recv_ack}]。')

            ind = int(recv_ack)

            self.time[ind] = 0  # 重置计时器
            self.data[ind] = "" # 数据已发送，删去缓存

            while self.base < self.next and self.data[self.base % self.identi_size] == "":
                self.base += 1
                # 滑动窗口
            
            if self.base == self.next and self.finish:
                self.base = -1
                self.next = -1
    
    def recv_package(self):     # 客户端接收数据
        readable = select.select([self.socket], [], [], 1)[0]   # 非阻塞接收
        if len(readable) > 0:  # 接收到数据
            recv_data = self.socket.recvfrom(self.recv_buf_size)[0].decode()
            pos = recv_data.find(' ')
            ind = int(recv_data[0 : pos])       # 标号
            get = recv_data[pos + 1 :]          # 数据
            print(f'[{self.name}]: 接收到标记 {ind}。')

            self.data[ind] = get                # 缓存数据

            while self.base < self.next and self.data[self.base % self.identi_size] != "":
                ind = self.base % self.identi_size
                print(f'[{self.name}]: 向文件写入第 {self.base} 个字段：[{self.data[ind].removesuffix('\n')}]。')
                if self.data[ind] == "FINISH":
                    self.base = -1
                    self.next = -1              # 接收完毕
                    break
                else:
                    self.f.write(self.data[ind])
                    self.f.flush()
                self.base += 1
                self.next += 1
                self.data[self.next % self.identi_size] = ""             # 初始化
                # 滑动窗口

            if random.random() < self.host2_loss:
                print(f"[{self.name}]: 丢失 ACK {ind}！")
            else:
                print(f"[{self.name}]: 发送 ACK {ind}。")
                self.socket.sendto(self.make_ack(ind), self.remote_host)

    def run_server(self):       # 运行服务器
        print(f'[{self.name}]: 已启动服务器。')
        time.sleep(0.5)
        
        self.f = open(self.filename, 'r', encoding='utf-8')

        while self.base != -1:
            try:
                self.send_packet()  # 尝试发送新的包
                self.recv_ack()     # 尝试接收新应答
                self.send_again()   # 检查有没有需要重发的包
                time.sleep(0.5)
            except:
                print(f'[{self.name}]: 客户端已关闭，判定传输完成。')
                self.base = -1
        
        print(f'[{self.name}]: 服务器已完成所有数据的传输！')
        self.f.close()
    
    def run_client(self):
        print(f'[{self.name}]: 已启动客户端。')
        time.sleep(0.5)

        self.f = open(self.filename, 'w', encoding='utf-8')
        self.next = self.window_size - 1

        while self.next != -1:
            self.recv_package()
            time.sleep(0.5)
        
        print(f'[{self.name}]: 客户端已完成所有数据的接收！')
        self.f.close()