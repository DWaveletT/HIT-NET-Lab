import toml

import threading

from proto_gbn import GBN
from proto_sr  import SR

config = toml.load('./config.toml')

host1_port = config['host1_port']
host1_name = config['host1_name']

host2_port = config['host2_port']
host2_name = config['host2_name']

host1_address = ('127.0.0.1', host1_port)
host2_address = ('127.0.0.1', host2_port)

proto = input('选择测试的协议。"GBN" 表示 GBN 协议，"SR" 表示 SR 协议：')

if proto == 'GBN':
    host1 = GBN(host1_name, host1_address, host2_address, 'send_data.txt')
    host2 = GBN(host2_name, host2_address, host1_address, 'recv_data.txt')
    threading.Thread(target = host1.run_server).start()
    threading.Thread(target = host2.run_client).start()
elif proto == 'SR':
    host1 = SR(host1_name, host1_address, host2_address, 'send_data.txt')
    host2 = SR(host2_name, host2_address, host1_address, 'recv_data.txt')
    threading.Thread(target = host1.run_server).start()
    threading.Thread(target = host2.run_client).start()
else:
    print(f'错误：未知的协议 {proto}。')