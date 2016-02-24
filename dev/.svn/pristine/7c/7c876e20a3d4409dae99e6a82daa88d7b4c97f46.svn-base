#!/usr/bin/env python
import sys
import struct


def crcByte(data):
	byte = 0
	for i in range(0, 8):
		if ((byte ^ data) & 0x01):
			byte ^= 0x18
			byte >>= 1
			byte |= 0x80
		else:
			byte >>= 1
		data >>= 1
	return byte

def crc8(data):
	ret = 0
	for byte in data:
		ret = (crcByte(ret ^ byte))
	return ret

def doPack(fileDst, param, counts, withCheckSum):
	i = 0
	content = ''
	hdlDst = open(fileDst, 'wb')
	
	while i < counts:
		srcFile = param[i]
		srcFileSize = param[i + 1]
		srcFileSize = int(srcFileSize[2:], 16)
		print 'packing file:', srcFile, hex(srcFileSize)
		hdlSrc = open(srcFile, 'rb')
		while srcFileSize > 0:
			line = hdlSrc.readline()
			content += line
			if not line:
				break
			# hdlDst.write(struct.pack('i', bin(line)))
			hdlDst.write(line)
			srcFileSize -= len(line)
		# print "content len:", len(content)

		# while srcFileSize > 1:
		while srcFileSize > 0:
			tmp = struct.pack('B', 255)
			content += tmp
			hdlDst.write(tmp)
			srcFileSize -= 1
		# print "appended len:", len(content)
		
		lastByte = 255
		if withCheckSum:
			lastByte = crc8(content)
			print "crc8 is", lastByte
		# hdlDst.write(struct.pack('B', lastByte))

		hdlSrc.close()
		i += 2
	
	hdlDst.close()
	print fileDst, "packed DONE"

def doSth():
	whatType = sys.argv[1]
	fileDst = sys.argv[2]

	print "Type: ", whatType, "fileDst: ", fileDst

	if whatType == '--pack':
		print whatType
		doPack(fileDst, sys.argv[3:], len(sys.argv) - 3, False)
	elif whatType == '--genCust':
		doPack(fileDst, sys.argv[3:], len(sys.argv) - 3, True)
	elif whatType == '--retrieve':
		print whatType
		doGet()
	else:
		print whatType

doSth()

# try:
# 	doSth()
# 	# a = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]
# 	# a = [1, 2, 3, 4, 5, 6]
# 	# b = [7, 8, 9, 10, 11, 12]
# 	# c = crc8(a) + crc8(b)
# 	# print hex(c%256)



# except:
#     print "exec --pack [to file] [[from file] [size] ...]"
#     print "exec --genCust [to file] [from file] [size]"
#     print "exec --retrieve [from file] [[start addr] [size] [to file] ...]"
