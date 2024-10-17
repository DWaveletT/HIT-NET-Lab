import toml

import threading

from proto_gbn import GBN
from proto_sr  import SR

config = toml.load('./config.toml')

host1_port = config['host1_port']
host1_name = config['host1_name']

host2_port = config['host2_port']
host2_name = config['host2_name']

host3_port = config['host3_port']
host3_name = config['host3_name']

host4_port = config['host4_port']
host4_name = config['host4_name']

host1_address = ('127.0.0.1', host1_port)
host2_address = ('127.0.0.1', host2_port)
host3_address = ('127.0.0.1', host3_port)
host4_address = ('127.0.0.1', host4_port)

# host1 用于 A 发送信息，host2 用于 B 接收信息
# host3 用于 A 接收信息，host4 用于 B 发送信息

# host1 = GBN(host1_name, host1_address, host2_address, '')
host2 = GBN(host2_name, host2_address, host1_address, '')
# host3 = GBN(host3_name, host3_address, host4_address, '')
host4 = GBN(host4_name, host4_address, host3_address, '')
threading.Thread(target = host4.run_server).start()
threading.Thread(target = host2.run_client).start()
