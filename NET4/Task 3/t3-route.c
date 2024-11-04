#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include <netinet/ether.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <time.h>

#define BUFFER_SIZE 65536

struct Hop {
  const char* addr;     // 子网地址
  const char* mask;     // 子网掩码

  int ifr;              // 网口出口
}table_hop[256];

struct Mac {
  const char *addr;     // IP 地址

  // 对应的 MAC 地址
  unsigned char hop_mac0;
  unsigned char hop_mac1;
  unsigned char hop_mac2;
  unsigned char hop_mac3;
  unsigned char hop_mac4;
  unsigned char hop_mac5;
}table_mac[256];

const char IFRS[][10] = {     // 不同 IFC ID 对应的名称
  "",
  "ens33",
  "ens36"
};

unsigned short checksum(void *b, int len)
{
  unsigned short *buf = b;
  unsigned int sum = 0;
  unsigned short result;
  for (sum = 0; len > 1; len -= 2)
    sum += *buf++;
  if (len == 1)
    sum += *(unsigned char *)buf;
  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  result = ~sum;
  return result;
}

unsigned int parse_ip(const char *s){
  unsigned int ip[4] = { 0, 0, 0, 0 };
  int c = 0, w = 0;
  for(int p = 0;s[p];++ p){
    if(s[p] == '.'){
      ip[c ++] = w, w = 0;
    } else {
      w = w * 10 + s[p] - '0';
    }
  }
  return ((ip[0] * 256 + ip[1]) * 256 + ip[2]) * 256 + ip[3];
}

int main()
{
  // 初始化地址表
  int table_mac_size = 2;
  table_mac[1].addr = "192.168.1.1";
  table_mac[1].hop_mac0 = 0xAA;
  table_mac[1].hop_mac1 = 0xAA;
  table_mac[1].hop_mac2 = 0xAA;
  table_mac[1].hop_mac3 = 0xAA;
  table_mac[1].hop_mac4 = 0xAA;
  table_mac[1].hop_mac5 = 0xAA;

  table_mac[2].addr = "192.168.2.1";
  table_mac[2].hop_mac0 = 0xAA;
  table_mac[2].hop_mac1 = 0xAA;
  table_mac[2].hop_mac2 = 0xAA;
  table_mac[2].hop_mac3 = 0xAA;
  table_mac[2].hop_mac4 = 0xAA;
  table_mac[2].hop_mac5 = 0xBB;
  
  printf("MAC Table:\n");
  printf("========================================\n");
  printf("|       addr       |         mac       |\n");
  printf("+------------------+-------------------+\n");
  // printf("+   192.168.1.1    | aa:aa:aa:aa:aa:aa |\n");
  for(int i = 1;i <= table_mac_size;++ i){
    printf("|  %15s | %02X:%02X:%02X:%02X:%02X:%02X |\n",
      table_mac[i].addr,
      table_mac[i].hop_mac0,
      table_mac[i].hop_mac1,
      table_mac[i].hop_mac2,
      table_mac[i].hop_mac3,
      table_mac[i].hop_mac4,
      table_mac[i].hop_mac5
    );
  }
  printf("========================================\n");

  // 初始化路由表
  int table_hop_size = 2;

  table_hop[1].addr = "192.168.1.0";
  table_hop[1].mask = "255.255.255.0";
  table_hop[1].ifr = 1;

  table_hop[2].addr = "192.168.2.0";
  table_hop[2].mask = "255.255.255.0";
  table_hop[2].ifr = 2;

  printf("Route Table:\n");
  printf("=============================================\n");
  printf("|       addr       |       mask       | ifr |\n");
  printf("+------------------+------------------+-----+\n");
  for(int i = 1;i <= table_hop_size;++ i){
    // printf("|      src_ip      |      dst_ip      | ifr |\n");
    printf("|  %15s |  %15s | %3d |\n",
      table_hop[i].addr, table_hop[i].mask, table_hop[i].ifr
    );
  }
  printf("=============================================\n");

  struct sockaddr saddr;
  char buffer[BUFFER_SIZE];

  // 创建 UDP 套接字
  int sockfd;
  sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
  if (sockfd < 0)
  {
    perror("Socket");
    return 1;
  }
  while (1) {
    int saddr_len = sizeof(saddr);
    int data_size = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, &saddr, (socklen_t *)&saddr_len);
    if (data_size < 0) {
      perror("Recvfrom");
      return 1;
    }

    struct ethhdr *eth_header = (struct ethhdr *)buffer;
    struct iphdr *ip_header = (struct iphdr *)(buffer + sizeof(struct ethhdr));
    struct udphdr *udph = (struct udphdr *)(buffer + sizeof(struct ethhdr) + sizeof(struct iphdr));
    char src_ip[INET_ADDRSTRLEN];
    char dst_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(ip_header->saddr), src_ip, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(ip_header->daddr), dst_ip, INET_ADDRSTRLEN);

    unsigned src_ip_val = parse_ip(src_ip);
    unsigned dst_ip_val = parse_ip(dst_ip);

    int p = 0;
    for(int i = 1;i <= table_hop_size;++ i){
      unsigned addr = parse_ip(table_hop[i].addr);
      unsigned mask = parse_ip(table_hop[i].mask);
      
      if((dst_ip_val & mask) == addr){
        p = table_hop[i].ifr;
        break;
      }
    }

    if(p == 0){     // 未在路由表里找到信息，忽略
      // printf("[ROUTER:%10u] Ignored packet from %s to %s\n", time(0), src_ip, dst_ip);
      continue;
    }

    int q = 0;
    for(int i = 1;i <= table_mac_size;++ i){
      unsigned addr = parse_ip(table_mac[i].addr);
      
      if((!strcmp(dst_ip, table_mac[i].addr))){
        q = i;
        break;
      }
    }

    if(p == 0 || q == 0){       // 未在路由表里找到信息，忽略
      // printf("[ROUTER:%10u] Ignored packet from %s to %s\n", time(0), src_ip, dst_ip);
      continue;
    }

    // 获取当前系统时间
    time_t rawtime;
    struct tm *timeinfo;
    char time_str[100];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    // 格式化时间字符串
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    // 打印信息
    printf("[ROUTER:%10u] At %s captured packet from %s to %s\n", time(0), time_str, src_ip, dst_ip);
    printf("[ROUTER:%10u] SRC_IP = %15s\n", time(0), src_ip);
    printf("[ROUTER:%10u] DST_IP = %15s\n", time(0), dst_ip);
    printf("[ROUTER:%10u] SRC_MAC = %02X:%02X:%02X:%02X:%02X:%02X\n",
      time(0),
      eth_header->h_source[0],
      eth_header->h_source[1],
      eth_header->h_source[2],
      eth_header->h_source[3],
      eth_header->h_source[4],
      eth_header->h_source[5]
    );
    printf("[ROUTER:%10u] DST_MAC = %02X:%02X:%02X:%02X:%02X:%02X\n",
      time(0),
      eth_header->h_dest[0],
      eth_header->h_dest[1],
      eth_header->h_dest[2],
      eth_header->h_dest[3],
      eth_header->h_dest[4],
      eth_header->h_dest[5]
    );
    // printf("[ROUTER:%10u] SRC Port = %d\n", time(0), udph->len);
    // printf("[ROUTER:%10u] SRC Port = %d\n", time(0), udph->source);
    // printf("[ROUTER:%10u] DST Port = %d\n", time(0), udph->dest);
    printf("[ROUTER:%10u] TTL = %d\n", time(0), ip_header->ttl);

    // 修改 TTL
    ip_header->ttl -= 1;
    ip_header->check = 0;
    //ip_header->tot_len = htons(20 + 8 + 30); // 总长度=IP首部长度+IP数据长度

        ip_header->check = checksum((unsigned short *)ip_header, ip_header->ihl * 4);
    
    // 发送数据包到目的主机
    struct ifreq ifr, ifr_mac;
    struct sockaddr_ll hop;
    
    // 获取网卡接口索引
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), IFRS[p]);
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0)
    {
      perror("Ioctl");
      return 1;
    }
    // 获取网卡接口 MAC 地址
    memset(&ifr_mac, 0, sizeof(ifr_mac));
    snprintf(ifr_mac.ifr_name, sizeof(ifr_mac.ifr_name), IFRS[p]);
    if (ioctl(sockfd, SIOCGIFHWADDR, &ifr_mac) < 0)
    {
      perror("Ioctl");
      return 1;
    }

    // 设置目标 MAC 地址
    unsigned char target_mac[ETH_ALEN] = {
      table_mac[q].hop_mac0, table_mac[q].hop_mac1, table_mac[q].hop_mac2, 
      table_mac[q].hop_mac3, table_mac[q].hop_mac4, table_mac[q].hop_mac5
    };

    memset(&hop, 0, sizeof(hop));
    hop.sll_ifindex = ifr.ifr_ifindex;
    hop.sll_halen = ETH_ALEN;
    memcpy(hop.sll_addr, target_mac, ETH_ALEN);
    // 构造新的以太网帧头
    memcpy(eth_header->h_dest, target_mac, ETH_ALEN);                   // 目标 MAC 地址
    memcpy(eth_header->h_source, ifr_mac.ifr_hwaddr.sa_data, ETH_ALEN); // 源 MAC   地址
    eth_header->h_proto = htons(ETH_P_IP);                              // 以太网类型为 IP
    
    printf("[ROUTER:%10u] Interface name: %s, index: %d\n", time(0), ifr.ifr_name, ifr.ifr_ifindex);
    if (sendto(sockfd, buffer, data_size, 0, (struct sockaddr *)&hop, sizeof(hop)) < 0)
    {
      perror("Sendto");
      return 1;
    }
    printf("[ROUTER:%10u] Datagram forwarded.\n", time(0));
  }
  close(sockfd);
  return 0;
}