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
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
    perror("Socket");
    return 1;
  } else {
    printf("[SENDER:%10u]: Create Socket successfully.\n", time(0));
  }

  // 目标地址
  struct sockaddr_in fwd_addr;
  fwd_addr.sin_family      = AF_INET;
  fwd_addr.sin_port        = htons(UDP_FWD_PORT);
  fwd_addr.sin_addr.s_addr = inet_addr(UDP_FWD_IP);

  char message[256];
  memset(message, 0, sizeof(message));
  while(1){
    printf("[SENDER:%10u] Please input message: ", time(0));
    fgets(message, sizeof(message), stdin);

    message[strlen(message) - 1] = 0;

    // 发送数据报
    socklen_t addr_len = sizeof(fwd_addr);
    
    if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&fwd_addr, addr_len) < 0) {
      perror("Sendto");
      return 1;
    }
    
    printf("[SENDER:%10u] Datagram sent: [%s](%d).\n", time(0), message, strlen(message));
  }
  return 0;
}