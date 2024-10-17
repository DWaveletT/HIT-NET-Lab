import java.io.*;
import java.net.ServerSocket;
import java.net.SocketTimeoutException;
import java.net.Socket;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * 简易的面向socket编程实现的HTTP代理。
 */
public class SocketServer {
  private static Map<String, String> redirectHostMap = new HashMap<>();   // 重定向网站 map
  private static Map<String, String> redirectAddrMap = new HashMap<>();   // 重定向网址 map

  private static Set<String> forbidHost = new HashSet<>();                // 禁止访问的网址
  private static Set<String> forbidUser = new HashSet<>();                // 禁止访问的用户

  static {
    // 更改这些内容达到屏蔽访问或钓鱼的目的

    redirectAddrMap.put("jwts.hit.edu.cn", "http://jwes.hit.edu.cn/");
    redirectHostMap.put("jwts.hit.edu.cn", "jwes.hit.edu.cn");

     forbidHost.add("today.hit.edu.cn");
    //  forbidUser.add("127.0.0.1");
  }

  // 判断网页是否被屏蔽
  private static boolean isForbiddenHost(String host) {
    for (String keyword : forbidHost) {
      if(host.contains(keyword))
        return true;
    }
    return false;
  }

  // 判断用户是否被屏蔽
  private static boolean isForbiddenUser(String user) {
    for (String keyword : forbidUser) {
      if(user.contains(keyword))
        return true;
    }
    return false;
  }

  // 处理网站重定向
  private static String redirectHost(String oriHost) {
    for (String keyword : redirectHostMap.keySet()) {
      if (oriHost.contains(keyword)) {
        String redHost = oriHost.replace(keyword, redirectHostMap.get(keyword));
        System.out.println("原始主机：" + oriHost);
        System.out.println("定向主机：" + redHost);
        return redHost;
      }
    }
    return oriHost;
  }

  // 处理网址重定向
  private static String redirectAddr(String oriAddr) {
    for (String keyword : redirectAddrMap.keySet()) {
      if (oriAddr != null && oriAddr.contains(keyword)) {
        String redAddr = oriAddr.replace(keyword, redirectHostMap.get(keyword));
        System.out.println("原始网址: " + oriAddr);
        System.out.println("定向网址: " + redAddr);
        return redAddr;
      }
    }
    return oriAddr;
  }

  /**
   * 通过header解析各个参数.
   *
   * @param header HTTP报文头部
   * @return map，包含HTTP版本，method，访问内容，主机，端口
   */
  private static Map<String, String> parse(String header) {
    if (header.length() == 0) {
      return new HashMap<>();
    }
    String[] lines = header.split("\\n");
    String method = null;       // 请求方法
    String visitAddr = null;    // 请求地址
    String visitPort = null;    // 请求端口
    String visitHost = null;    // 请求网站
    String httpVersion = null;  // 协议版本
    for (String line : lines) {
      if ((line.contains("GET") || line.contains("POST") || line.contains("CONNECT")) && method == null) {
        // 形如 METHOD VISITADDR HTTPVERSION
        String[] temp1 = line.split("\\s");  // 按空格分割
        method      = temp1[0];
        visitAddr   = temp1[1];
        httpVersion = temp1[2];
        
        // 获取协议号和端口
        if (visitAddr.contains("http://") || visitAddr.contains("https://")) {
          // 包含协议，形如 https://XXX.XXX.XXX:PORT 端口下标为 2
          String[] temp2 = visitAddr.split(":");
          if (temp2.length >= 3) {
            visitPort = temp2[2];
          }
        } else {
          // 不包含协议，形如 XXX.XXX.XXX:PORT 端口下标为 1
          String[] temp2 = visitAddr.split(":");
          if (temp2.length >= 2) {
            visitPort = temp2[1];
          }
        }
      } else if (line.contains("Host: ") && visitHost == null) {
        String[] temp1 = line.split("\\s");
        visitHost = temp1[1];
        int maohaoIndex = visitHost.indexOf(':');
        if (maohaoIndex != -1) {
          visitHost = visitHost.substring(0, maohaoIndex);
        }
      }
    }
    if(visitPort == null){
      visitPort = "80";       // 使用默认端口
    }

    Map<String, String> map = new HashMap<>();
    map.put("method", method);
    map.put("visitAddr", visitAddr);
    map.put("visitHost", visitHost);
    map.put("visitPort", visitPort);
    map.put("httpVersion", httpVersion);
    return map;
  }

  public static void main(String[] args) throws IOException {
    // 监听指定的端口
    int port = 808;
    ServerSocket server = new ServerSocket(port);
    // server将一直等待连接的到来
    System.out.println("[INFO]: server 将一直等待连接的到来");

    while (true) {
      // 等待得到一个新的连接
      Socket socket = server.accept();

      String user = socket.getInetAddress().getHostAddress();
      System.out.println("[INFO]: 获取到一个连接！来自 " + user);

      // 创建新线程处理该连接
      new Thread(new Runnable() {
        @Override
        public void run() {
          try {
            // 解析 Header
            InputStream  is = socket. getInputStream();
            OutputStream os = socket.getOutputStream();     // 输出到浏览器

            BufferedOutputStream bo = new BufferedOutputStream(os);

            BufferedReader bi = new BufferedReader(new InputStreamReader(is));
              // 用于从输入流当中进行整行读入

            StringBuilder header = new StringBuilder();
            // 解析头部
            for(
              String readLine = bi.readLine();
              readLine != null && !readLine.equals("");
              readLine = bi.readLine()
            ){
              System.out.println("[DEBUG]: " + readLine);
              header.append(readLine).append("\n");
            }
            header.append("\n");

            // 打印原始头部看看
            System.out.println(header.toString());

            // 判断用户是否被屏蔽
            if (isForbiddenUser(user)) {
              System.out.println("[INFO]: 来自被屏蔽的用户");
              socket.close();
              return;
            }

            // 打印参数表
            Map<String, String> map = parse(header.toString());

            String visitHost = map.get("visitHost");    // 访问的网站
            String visitPort = map.get("visitPort");    // 访问的端口
            String visitAddr = map.get("visitAddr");    // 访问的地址
            String method    = map.get("method");       // 方法

            // 判断是否屏蔽掉这个网站
            if (visitHost != null && isForbiddenHost(visitHost)) {
              // 被屏蔽，不允许访问
              System.out.println("[INFO]: 访问被屏蔽的网站");
            } else {
              // 获得跳转主机和资源
              visitHost = redirectHost(visitHost);
              visitAddr = redirectAddr(visitAddr);

              System.out.println("尝试访问 " + visitHost + " | " + visitPort + " | " + visitAddr);

              // 创建新的 socket 连接远程服务器
              Socket webSocket = new Socket(visitHost, Integer.parseInt(visitPort));
              webSocket.setSoTimeout(1000);
              
              OutputStream webos = webSocket.getOutputStream();
              OutputStreamWriter webow = new OutputStreamWriter(webos);

              String lastModified = "Thu, 01 Jul 1970 20:00:00 GMT";

              File cache = new File("cache/" + visitAddr.hashCode() + ".mycache");

              if (cache.exists() && cache.length() != 0) {
                  System.out.println("[INFO]: 网页存在缓存");

                  // 获得修改时间
                  Calendar cal = Calendar.getInstance();
                  long time = cache.lastModified();
                  SimpleDateFormat formatter = new SimpleDateFormat("EEE, dd MMM yyyy HH:mm:ss 'GMT'", Locale.ENGLISH);
                  cal.setTimeInMillis(time);
                  cal.set(Calendar.HOUR, -7);
                  cal.setTimeZone(TimeZone.getTimeZone("GMT"));
                  lastModified = formatter.format(cal.getTime());
                  System.out.println("[DEBUG]: 修改时间：" + lastModified);
              }

              String request = String.format("""
%s %s HTTP/1.1
HOST: %s
Accept:text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
Accept-Encoding:gzip, deflate, sdch
Accept-Language:zh-CN,zh;q=0.8
If-Modified-Since: %s
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/13.0.2 Safari/605.1.15
Encoding:UTF-8

""", method, visitAddr, visitHost, lastModified);
              // 打印看看
              System.out.println("[DEBUG]: 将会向网站发送以下报文：");
              System.out.println(request);

              webow.write(request); // 发送报文
              webow.flush();
              
              byte[] buff1 = new byte[32];     // 用于读入头部的小缓存
              byte[] buff2 = new byte[10240];  // 用于读入内容的大缓存
              
              InputStream webis = webSocket.getInputStream(); // 从网站输入
              
              int len = 0, cnt = 0;
              System.out.println("[INFO]: 读入网站返回数据开始");

              len = webis.read(buff1, 0, 20);

              String res = new String(buff1, 0, len);
              if(res.contains("304")){    // 服务器说明信息没有被修改，使用缓存
              // if(true){    // 强制使用缓存
                System.out.println("[DEBUG]: 没有修改，使用缓存");

                FileInputStream fi = new FileInputStream(cache);
                
                System.out.println("[DEBUG]: 从缓存读入内容给浏览器");
                while(true) {
                  len = fi.read(buff2, 0, 10240);
                  cnt ++;
                  System.out.println("[DEBUG]: 读入 " + len + " " + cnt);
                  if(len == -1)
                    break;
                  bo.write(buff2, 0, len);
                }

                System.out.println("[DEBUG]: 从网站读入内容，丢弃");
                // 继续读入剩下内容
                try {
                  while(true) {
                    len = webis.read(buff2, 0, 10240);
                    cnt ++;
                    System.out.println("[DEBUG]: 读入 " + len + " " + cnt);
                    if(len == -1)
                      break;
                  }
                } catch(SocketTimeoutException e){
                  System.out.println("[INFO]: 通信阻塞，判断内容读取完毕");
                }
              } else {
                System.out.println("[DEBUG]: 可能修改，重新读入");
                
                // 文件输出流，用来写入缓存
                FileOutputStream fo = new FileOutputStream(cache);
                
                fo.write(buff1, 0, len);    // 小头部也要返回写给缓存
                bo.write(buff1, 0, len);    // 小头部也要返回写给浏览器
                // 继续读入剩下内容
                try {
                  while(true) {
                    len = webis.read(buff2, 0, 10240);
                    cnt ++;
                    System.out.println("[DEBUG]: 读入 " + len + " " + cnt);
                    if(len == -1)
                      break;
                    bo.write(buff2, 0, len);
                    fo.write(buff2, 0, len);  // 给缓存写一份
                  }
                } catch(SocketTimeoutException e){
                  System.out.println("[INFO]: 通信阻塞，判断内容读取完毕");
                }
                fo.flush();
              }

              System.out.println("读入网站返回数据完成");
              bo.flush();     // 刷新缓存到浏览器

              System.out.println("[INFO]: 关闭远程 socket 连接");
              webSocket.close();    // 关闭连接远程服务器的 socket
            }
            System.out.println("[INFO]: 关闭本地 socket 连接");
            socket.close();             // 关闭浏览器与程序的 socket
          } catch (IOException e) {
            e.printStackTrace();
          }
        }
      }).start();
    }
  }
}
