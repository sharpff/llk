 #!/usr/bin/env python
 # UDP Echo Server -  udpserver.py
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