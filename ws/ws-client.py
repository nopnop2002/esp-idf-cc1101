#!/usr/bin/env python
# python3 -m pip install websockets
# https://github.com/python-websockets/websockets

import time
import datetime
import socket
from websockets.sync.client import connect

server = "esp32-server.local"
port = 8080
timezone = datetime.timedelta(hours=9)

try:
	ip = socket.gethostbyname(server)
except:
	print('gethostbyname fail')
	exit()

print("ip={}".format(ip))
uri = "ws://{}:{}".format(ip, port)
print("uri={}".format(uri))
try:
	websocket = connect(uri)
except:
	print('connect fail')
	exit()

while True:
	dt_now = datetime.datetime.now(datetime.timezone(timezone))
	payload = dt_now.strftime('%Y/%m/%d %H:%M:%S')
	websocket.send(payload)
	responce = websocket.recv()
	print("{}-->{}".format(payload, responce))
	time.sleep(1.0)

