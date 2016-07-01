#!/usr/bin/env python
# UDP log master -  logMaster.py
# remote ctrl cmd: {"key":6,"val":{"log2M":1,"IP":"192.168.3.100","port":1234}}
# IP & port is for log master
import socket, traceback

host = ''
port = 1234

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind((host, port))

while 1:
	try:
		message, address = s.recvfrom(8192)
		print message,
	except (KeyboardInterrupt, SystemExit):
		raise
	except:
		traceback.print_exc()