#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define UDP_SRC_PORT 12345
#define UDP_FWD_PORT 12345
#define UDP_DST_PORT 12345
#define UDP_SRC_IP   "192.168.1.1"
#define UDP_FWD_IP   "192.168.1.3"
#define UDP_DST_IP   "192.168.1.2"

int main()
{
  // 创建 UDP 套接字
  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    perror("Socket");
    return 1;
  }

  // 目标地址
  struct sockaddr_in dst_addr;
  dst_addr.sin_family = AF_INET;
  dst_addr.sin_port = htons(UDP_DST_PORT);
  dst_addr.sin_addr.s_addr = inet_addr(UDP_DST_IP);

  // 绑定套接字到本地地址
  if (bind(sockfd, (struct sockaddr *)&dst_addr, sizeof(dst_addr)) < 0) {
    perror("Bind");
    return 1;
  } else {
    printf("[RECVER:%10u] Bind socket successfully.\n", time(0));
  }

  char message[1024];
  while(1){
    // 接收数据报
    struct sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);

    int recv_len = recvfrom(sockfd, message, sizeof(message), 0, (struct sockaddr *)&src_addr, &addr_len);
    if (recv_len < 0) {
      perror("Recvfrom");
      return 1;
    }
    message[recv_len] = '\0';
    printf("[RECVER:%10u] Datagram received: [%s](%d).\n", time(0), message, strlen(message));
  }
  return 0;
}