#!/usr/bin/env python
# coding:utf-8

import base64
import collections
import os, sys, struct
from optparse import OptionParser

class Param(object):
    def __init__(self):
        self.version = "v0.1"
        self.addr = "10.204.28.134"
        self.port = 5546;
        self.uuid = ""
        self.base64_pk = ""
        self.base64_sig = ""
        self.fws = ""
        self.genpriv = False
        self.gensdev = False
        self.wmode = ""
        self.keepud = False
        self.ssid = ""
        self.passwd = ""
        self.of = "out.bin"
        parser = OptionParser(usage="usage: %prog [options] [[packfile packsize]...]", version="%prog " + self.version)
        parser.add_option("-a", "--addr", type="string",
                      action="store", dest="addr", default=self.addr,
                      help="server address. Defaut to %s" % self.addr)
        parser.add_option("-p", "--port", type="int",
                      action="store", dest="port", default=self.port,
                      help="server port. Defaut to %d" % self.port)
        parser.add_option("-u", "--uuid", type="string",
                      action="store", dest="uuid", default=self.uuid,
                      help="device uuid.")
        parser.add_option("-k", "--pubkey", type="string",
                      action="store", dest="base64_pk", default=self.base64_pk,
                      help="public key, base64")
        parser.add_option("-s", "--sig", type="string",
                      action="store", dest="base64_sig", default=self.base64_sig,
                      help="signature, base64")
        parser.add_option("-t", "--script", type="string",
                      action="store", dest="fws", default=self.fws,
                      help="firware script file path")
        parser.add_option("--genpriv",
                      action="store_true", dest="genpriv", default=self.genpriv,
                      help="Generate private info")
        parser.add_option("--gensdev",
                      action="store_true", dest="gensdev", default=self.gensdev,
                      help="Generate private info")
        parser.add_option("--wmode", type="string",
                      action="store", dest="wmode", default=self.wmode,
                      help="wifi password")
        parser.add_option("--keepud",
                      action="store_true", dest="keepud", default=self.keepud,
                      help="keep user data in reset manually")
        parser.add_option("--ssid", type="string",
                      action="store", dest="ssid", default=self.ssid,
                      help="wifi ssid")
        parser.add_option("--passwd", type="string",
                      action="store", dest="passwd", default=self.passwd,
                      help="wifi password")
        parser.add_option("-o", "--outfile", type="string",
                      action="store", dest="of", default=self.of,
                      help="output file name. Defaut to %s" % self.of)
        options, args = parser.parse_args()
        self.packs = collections.OrderedDict()
        self.addr = options.addr
        self.port = options.port
        self.uuid = options.uuid
        self.base64_pk = options.base64_pk
        self.base64_sig = options.base64_sig
        self.fws = options.fws;
        self.genpriv = options.genpriv;
        self.gensdev = options.gensdev;
        self.wmode = options.wmode;
        self.keepud = options.keepud;
        self.ssid = options.ssid;
        self.passwd = options.passwd;
        self.of = options.of;
        if (len(args) % 2) :
            print "Pack parameter error, not pair"
            parser.print_help()
            sys.exit()
        if (len(args) > 0):
            for i in range(0, len(args) - 1, 2):
                self.packs[args[i]] = int(args[i+1], 16)
    def print_param(self):
        print "serv: %s:%d" % (self.addr, self.port)
        print "uuid: %s" % self.uuid
        print "wifi: %s@%s" % (self.ssid, self.passwd)
        # print "pubkey: %s" % self.base64_pk
        # print "signature: %s" % self.base64_sig

def strCrc(str):
    crc = 0
    bytes = bytearray(str)
    for b in bytes:
        crc ^= b;
        for i in range(8):
            if crc & 0x01:
                crc = (crc >> 1) ^ 0x8c
            else:
                crc >>=1
    return crc & 0xFF

def packPad(byte, len):
    str = ''
    for i in range(len):
        str += byte
    return str

def canReadFile(path):
    if not os.path.isfile(path):
        print("Can't find file \"%s\"." % path)
        sys.exit()
    if not os.access(path, os.R_OK):
        print("Can't read file \"%s\"." % path)
        sys.exit()

def packByFile(path, size):
    PACK_PAD = '\xFF'
    canReadFile(path)
    if os.path.getsize(path) > size:
        print("File(\"%s\") too big ( > %d )." % (path, size))
        sys.exit()
    f = open(path, "rb")
    content = f.read()
    f.close()
    content += packPad(PACK_PAD, (size - len(content)))
    return content

class AuthCfg(object):
    def __init__(self):
        self.PACK_PAD = '\xFF'
        self.MAX_UUID = 32
        self.MAX_ADDR_LEN = 64
        self.PACK_SIZE = (1024 * 4)
        self.MAX_SIG_LEN = (1024 / 8)
        self.MAX_PK_LEN = (self.MAX_SIG_LEN + self.MAX_SIG_LEN / 2)
        self.uuid = ""
        self.pubkeyLen = 0
        self.signatureLen = 0
        self.signature = ""
        self.pubkey = ""
        self.remote = ""
        self.port = 0;
        self.reserved = 0;
        self.content = ""
    def set(self, uuid, pk, sig, addr, port):
        self.uuid = uuid
        self.pubkey = base64.b64decode(pk)
        self.pubkeyLen = len(self.pubkey)
        self.signature = base64.b64decode(sig)
        self.signatureLen = len(self.signature)
        self.remote = addr
        self.port = port
    def pack(self):
        self.content = ""
        self.content += struct.pack("<%ds" % self.MAX_UUID, self.uuid)
        self.content += struct.pack("<I", self.pubkeyLen)
        self.content += struct.pack("<I", self.signatureLen)
        self.content += struct.pack("<%ds" % self.MAX_SIG_LEN, self.signature)
        self.content += struct.pack("<%ds" % self.MAX_PK_LEN, self.pubkey)
        self.content += struct.pack("<%ds" % self.MAX_ADDR_LEN, self.remote)
        self.content += struct.pack("<H", self.port)
        self.content += struct.pack("<H", self.reserved)
        self.content += struct.pack("<B", strCrc(self.content))
        self.content += packPad(self.PACK_PAD, (self.PACK_SIZE - len(self.content)));

class ScriptCfg(object):
    def __init__(self):
        self.PACK_PAD = '\xFF'
        self.PACK_SIZE = (1024 * 20)
        self.MAX_SIZE = (1024 * 16)
        self.script = ""
        self.content = ""
    def set(self, script):
        self.script = script
    def pack(self):
        self.content = ""
        self.content += struct.pack("<I", len(self.script))
        self.content += struct.pack("<%ds" % len(self.script), self.script)
        self.content += packPad(self.PACK_PAD, self.MAX_SIZE - len(self.script))
        self.content += struct.pack("<B", strCrc(self.content))
        self.content += packPad(self.PACK_PAD, (self.PACK_SIZE - len(self.content)));

class PrivateInfo(object):
    def __init__(self):
       self.PACK_PAD = '\xFF'
       self.PACK_SIZE = (1024 * 4)
       self.ssid = '' 
       self.passwd = ''
       self.config = 0
       self.content = ''
       self.unbind = 1
       self.wmode = 'mon'
       self.DEV_CFG_SIZE = 39
       self.NET_CFG_SIZE = 100
       self.IA_CFG_SIZE = 276
       self.MAX_STR_LEN = 36
    def set(self, ssid, passwd):
        self.ssid = ssid
        self.passwd = passwd
        if len(self.ssid) > 0:
            self.config = 1
    def keepUD(self):
        self.unbind = 0
    def setWiFiMode(self, wmode):
        self.wmode = wmode

    def pack(self):
        self.content = ''

        self.content += packPad(self.PACK_PAD, self.DEV_CFG_SIZE-2)
        # wmode
        print("DevCfg self.wmode: %s" % self.wmode)
        if self.wmode == 'mon':
          self.content += packPad('\x00', 1)
        elif self.wmode == 'sap':
          self.content += packPad('\x01', 1)
        else:
          self.content += packPad('\xFF', 1)
        # unbind
        print("DevCfg self.unbind: %d" % self.unbind)
        if self.unbind != 0:
          self.content += packPad('\x01', 1)
        else:
          self.content += packPad('\x00', 1)

        self.content += struct.pack("<I", self.config)
        self.content += struct.pack("<%ds" % (len(self.ssid) + 1), self.ssid)
        self.content += packPad(self.PACK_PAD, self.MAX_STR_LEN - len(self.ssid) - 1)
        self.content += struct.pack("<%ds" % (len(self.passwd) + 1), self.passwd)
        self.content += packPad(self.PACK_PAD, self.MAX_STR_LEN - len(self.passwd) - 1)

        # pad for     
        # uint32_t ipaddr;
        # uint32_t mask;
        # uint32_t gateway;
        # uint32_t dns1;
        # uint32_t dns2;
        self.content += packPad(self.PACK_PAD, 4*5);
        self.content += struct.pack("<B", len(self.ssid))
        self.content += struct.pack("<B", len(self.passwd))

        self.content += packPad(self.PACK_PAD, (self.DEV_CFG_SIZE + self.NET_CFG_SIZE + self.IA_CFG_SIZE - len(self.content)))
        self.content += struct.pack("<B", strCrc(self.content))
        self.content += packPad(self.PACK_PAD, (self.PACK_SIZE - len(self.content)));

class SDevInfo(object):
    def __init__(self):
       self.TOTAL_PAD = '\xFF'
       self.TOTAL_SIZE = (1024*12) # 9344 bytes to 12K

    def pack(self):
       self.content = ''
       self.content += packPad(self.TOTAL_PAD, self.TOTAL_SIZE)



if __name__ == '__main__':
    content = ''
    param = Param()
    print("Begin generate %s" % param.of)
    # packfile
    for k, v in param.packs.items():
        content += packByFile(k, v)
        print("File \'%s\': %d, " % (k, v)),
        print("Total: %d" % len(content))
    # auth
    if len(param.uuid) > 0:
        authcfg = AuthCfg()
        authcfg.set(param.uuid, param.base64_pk, param.base64_sig, param.addr, param.port)
        authcfg.pack()
        content += authcfg.content
        print "AuthCfg: %d, " % len(authcfg.content),
        print("Total: %d" % len(content))
    # firware script
    if len(param.fws) > 0:
        canReadFile(param.fws)
        f = open(param.fws, "rb")
        s = ScriptCfg()
        s.set(f.read())
        s.pack()
        f.close()
        content += s.content
        print("firware script: %d, " % len(s.content)),
        print("Total: %d" % len(content))
    # private info
    if param.genpriv:
        pricfg = PrivateInfo()
        pricfg.set(param.ssid, param.passwd)
        if param.keepud:
            pricfg.keepUD()
        if param.wmode:
            pricfg.setWiFiMode(param.wmode)
        pricfg.pack()
        content += pricfg.content
        print "PrivateInfo: %d, " % len(pricfg.content),
        print("Total: %d" % len(content))
    # sdev info
    if param.gensdev:
        sdev = SDevInfo()
        sdev.pack()
        content += sdev.content
        print "SDevInfo: %d, " % len(sdev.content),
        print("Total: %d" % len(content))
    # save to outfile
    outfile = open(param.of, "wb")
    outfile.write(content)
    outfile.close()
    print("Out file: %s" % param.of)

