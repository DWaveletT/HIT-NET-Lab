#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>

// 路由器 1 网口 1 的 MAC 地址
#define DEST_MAC0    0xAA
#define DEST_MAC1    0xAA
#define DEST_MAC2    0xAA
#define DEST_MAC3    0xAA
#define DEST_MAC4    0xAA
#define DEST_MAC5    0xCC

#define ETHER_TYPE   0x0800
#define BUFFER_SIZE  1518

#define UDP_SRC_PORT1   12345   // 发送用的端口
#define UDP_SRC_PORT2   54321   // 接收用的端口
#define UDP_DST_PORT1   12345   // 发送用的端口
#define UDP_DST_PORT2   54321   // 接收用的端口
#define UDP_SRC_IP   "192.168.1.1"
#define UDP_FWD_IP1  "192.168.1.2"
#define UDP_FWD_IP2  "192.168.2.2"
#define UDP_DST_IP   "192.168.2.1"


unsigned short checksum(void *b, int len){
  unsigned short *buf = b;
  unsigned int sum = 0;
  unsigned short result;
  for (sum = 0; len > 1; len -= 2){
    sum += *buf++;
  }
  if (len == 1)
    sum += *(unsigned char *)buf;
  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  result = ~sum;
  return result;
}

/*
以太网帧：以太网头 + IP 头 + UDP 头 + 载荷（信息）。
构建 socket 地址结告诉操作系统，此次 Socket 通信将会通过 ens33 网口发送往指定 MAC 地址，传输的层次为数据链路层使用以太网协议。

vmhgfs-fuse .host:/ /mnt/hgfs; cd "/mnt/hgfs/Machine 1"
gcc t2-route.c;./a.out
*/

int main(){
  char buffer[BUFFER_SIZE];
  
  // 创建原始套接字
  int sockfd;
  if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
    perror("Socket");
    return 1;
  }

  // 获取接口索引
  struct ifreq if_idx;
  memset(&if_idx, 0, sizeof(struct ifreq));
  strncpy(if_idx.ifr_name, "ens33", IFNAMSIZ - 1);
  if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
  {
    perror("SIOCGIFINDEX");
    return 1;
  }
  // 获取接口 MAC 地址
  struct ifreq if_mac;
  memset(&if_mac, 0, sizeof(struct ifreq));
  strncpy(if_mac.ifr_name, "ens33", IFNAMSIZ - 1);
  if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
  {
    perror("SIOCGIFHWADDR");
    return 1;
  }
  
  // 创建原始套接字
  int sockfd_recv;
  if ((sockfd_recv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    perror("Socket");
    return 1;
  }
  
  // 目标地址
  struct sockaddr_in recv_addr;
  recv_addr.sin_family = AF_INET;
  recv_addr.sin_port = htons(UDP_SRC_PORT2);
  recv_addr.sin_addr.s_addr = inet_addr(UDP_SRC_IP);

  // 绑定套接字到本地地址
  if (bind(sockfd_recv, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) < 0) {
    perror("Bind");
    return 1;
  } else {
    printf("[RECVER:%10u] Bind socket successfully.\n", time(0));
  }
  
  char message[256];
  memset(message, 0, sizeof(message));
  while(1){
    printf("[SENDER:%10u] Please input message: ", time(0));
    fgets(message, sizeof(message), stdin);

    message[strlen(message) - 1] = 0;
    
    // 构造以太网头
    struct ether_header *eh = (struct ether_header *)buffer;
    eh->ether_shost[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
    eh->ether_shost[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
    eh->ether_shost[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
    eh->ether_shost[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
    eh->ether_shost[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
    eh->ether_shost[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];

    printf("[SENDER:%10u] SRC MAC Address = ", time(0));
    for (int i = 0; i < 6; i++){
      printf("%02X%c", eh->ether_shost[i], ":\n"[i + 1 == 6]);
    }

    eh->ether_dhost[0] = DEST_MAC0;
    eh->ether_dhost[1] = DEST_MAC1;
    eh->ether_dhost[2] = DEST_MAC2;
    eh->ether_dhost[3] = DEST_MAC3;
    eh->ether_dhost[4] = DEST_MAC4;
    eh->ether_dhost[5] = DEST_MAC5;
    eh->ether_type = htons(ETHER_TYPE);
    
    printf("[SENDER:%10u] DST MAC Address = ", time(0));
    for (int i = 0; i < 6; i++){
      printf("%02X%c", eh->ether_dhost[i], ":\n"[i + 1 == 6]);
    }

    // 构造 IP 头
    struct iphdr *iph = (struct iphdr *)(buffer + sizeof(struct ether_header));
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(message));
    iph->id = htonl(UDP_DST_PORT2);
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_UDP;
    iph->check = 0;
    iph->saddr = inet_addr(UDP_SRC_IP);
    iph->daddr = inet_addr(UDP_DST_IP);
    iph->check = checksum((unsigned short *)iph, sizeof(struct iphdr));
    
    printf("[SENDER:%10u] SRC IP Address = %s\n", time(0), UDP_SRC_IP);
    printf("[SENDER:%10u] DST IP Address = %s\n", time(0), UDP_DST_IP);
    printf("[SENDER:%10u] TTL = %d\n", time(0), iph -> ttl);

    // 构造 UDP 头
    struct udphdr *udph = (struct udphdr *)(buffer + sizeof(struct ether_header) + sizeof(struct iphdr));
    udph->source = htons(UDP_SRC_PORT1);
    udph->dest = htons(UDP_DST_PORT2);
    udph->len = htons(sizeof(struct udphdr) + strlen(message));
    udph->check = 0;

    // 填充数据
    char *data = (char *)(buffer + sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct udphdr));
    strcpy(data, message);

    // 设置 socket 地址结
    struct sockaddr_ll socket_address;
    socket_address.sll_ifindex = if_idx.ifr_ifindex;
    socket_address.sll_halen = ETH_ALEN;
    socket_address.sll_addr[0] = DEST_MAC0;
    socket_address.sll_addr[1] = DEST_MAC1;
    socket_address.sll_addr[2] = DEST_MAC2;
    socket_address.sll_addr[3] = DEST_MAC3;
    socket_address.sll_addr[4] = DEST_MAC4;
    socket_address.sll_addr[5] = DEST_MAC5;

    // 发送数据包
    int len = sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(message);
    printf("[SENDER:%10u] Sending message (load = [%s](%d)), total length = %dByte\n", time(0), message, strlen(message), len);

    if (sendto(sockfd, buffer, len, 0, (struct sockaddr *)&socket_address, sizeof(struct sockaddr_ll)) < 0){
      perror("Sendto");
      return 1;
    }

    // 接收数据报
    printf("[RECVER:%10u] Try to receive message.\n", time(0));
    // 接收数据报
    struct sockaddr_in send_addr;
    socklen_t addr_len = sizeof(send_addr);

    int recv_len = recvfrom(sockfd_recv, message, sizeof(message), 0, (struct sockaddr *)&send_addr, &addr_len);
    if (recv_len < 0) {
      perror("Recvfrom");
      return 1;
    }
    message[recv_len] = '\0';
    printf("[RECVER:%10u] Datagram received: [%s](%d).\n", time(0), message, strlen(message));
  }
  close(sockfd);
  close(sockfd_recv);
  return 0;
}
