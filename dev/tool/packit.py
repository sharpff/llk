#!/usr/bin/env python
import sys
import struct, os

# SIZE_SCRIPT 0X4000 # 16K


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
	ret = 0x00
	# data = struct.unpack('i', data)
	for byte in data:
		byte = ord(byte)
		ret = (crcByte(ret ^ byte))
	return ret

# crc8 test 1
# a = ['m', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'c', 'e']
# tmpByte = crc8(a)
# print "crc8", hex(tmpByte)

# crc8 test 2
# param = sys.argv[3:]
# size = os.path.getsize(param[0])
# content = ''
# hdlSrc = open(param[0], 'rb')
# tmpSize = size
# while tmpSize > 0:
# 	tmp = hdlSrc.read()
# 	if not tmp:
# 		break
# 	tmpSize -= len(tmp)
# 	if not len(content):
# 		content = tmp
# 	else:
# 		content += tmp
# hdlSrc.close()
# tmpList = []
# for a in content:
# 	tmpList.append(a)
# tmpByte = crc8(tmpList)
# print "size", len(tmpList), "crc8", hex(tmpByte)

def doPackFileByBuf(fileDst, buf):
	i = 0
	testLen = 0
	content = ''
	append = ''
	# expectSize = int(expectSize[2:], 16)
	hdlDst = open(fileDst, 'ab')
	hdlDst.write(buf)
	hdlDst.close()


def doPackFileByFile(fileDst, fileSrc, expectFileSize, appendix):
	i = 0
	testLen = 0
	content = ''
	append = ''
	# expectFileSize = int(expectFileSize[2:], 16)

	# write the file content
	print 'packing file:', fileSrc, hex(expectFileSize)
	realFileSize = 0
	hdlSrc = open(fileSrc, 'rb')
	while expectFileSize > 0:
		tmp = hdlSrc.read()
		if not tmp:
			break
		realFileSize += len(tmp)
		expectFileSize -= len(tmp)
		if not len(content):
			content = tmp
		else:
			content += tmp
	doPackFileByBuf(fileDst, content)	
	hdlSrc.close()

	# appending 0xff
	oneByte = struct.pack('B', appendix)
	# print len(content), expectFileSize, len(content)+expectFileSize
	while expectFileSize > 0:
		if not len(append):
			append = oneByte
		else:
			append += oneByte
		expectFileSize -= 1
	# print len(append)
	doPackFileByBuf(fileDst, append)

	print fileDst, "packed DONE"

def doSth():
	whatType = sys.argv[1]
	fileDst = sys.argv[2]

	# print "Type: ", whatType, "fileDst: ", fileDst
	param = sys.argv[3:]

	if whatType == '--pack':
		counts = len(sys.argv) - 3
		print whatType
		i = 0
		testLen = 0
		if os.path.exists(fileDst):
			os.remove(fileDst)

		while i < counts:
			fileSrc = param[i]
			expectSize = int(param[i + 1][2:], 16)
			doPackFileByFile(fileDst, fileSrc, expectSize, 255)
			i += 2
	elif whatType == '--genS1':
		if os.path.exists(fileDst):
			os.remove(fileDst)

		size = os.path.getsize(param[0])
		structFileSize = 4 + int(param[1][2:], 16) + 1
		totalSize = int(param[2][2:], 16)
		if structFileSize < size or totalSize < structFileSize:
			print "file size error"
			return
		data1st = struct.pack('i', size)
		# print size, structFileSize, data1st

		# structrue - size (int)
		doPackFileByBuf(fileDst, data1st)

		# structrue - fw script 
		content = ''
		doPackFileByFile(fileDst, param[0], structFileSize - 1 - 4, 0)

		# structrue - crc8 
		hdlSrc = open(param[0], 'rb')
		tmpSize = size
		while tmpSize > 0:
			tmp = hdlSrc.read()
			if not tmp:
				break
			tmpSize -= len(tmp)
			if not len(content):
				content = tmp
			else:
				content += tmp
		hdlSrc.close()
		tmpList = []
		for a in data1st:
			tmpList.append(a)
		for a in content:
			tmpList.append(a)
		tmpSize1 = structFileSize - 1 - 4 - len(content)
		while tmpSize1 > 0:
			tmpList.append(chr(0x00))
			tmpSize1 -= 1

		tmpByte = crc8(tmpList)
		# print "crc8", hex(tmpByte)
		doPackFileByBuf(fileDst, struct.pack('B', tmpByte))

		# append 0xff
		tmpSize2 = totalSize - structFileSize
		content = ''
		while tmpSize2 > 0:
			if not len(content):
				content = struct.pack('B', 255)
			else:
				content += struct.pack('B', 255)
			tmpSize2 -= 1
		doPackFileByBuf(fileDst, content)
	elif whatType == '--genS2':
		print "no implement"
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
#     print "exec --pack [to file] [[from file] [file size] ...]"
#     print "exec --genS1 [to file] [from file] [struct-script size] [total size]"
#     print "exec --retrieve [from file] [[start addr] [file size] [to file] ...]"
