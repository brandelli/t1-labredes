import struct
import socket
import binascii
import os


class Sender:
    #0x0800 ipv4
    #0x86dd ipv6
    #17 UDP
    #6  TCP
    #def __init__(self, src, dest, data):
    def __init__(self, fileName, etherType, ipProtocolType, destMac, srcMac):
        self._file = self.prepareFile(fileName, 1480)
        self._etherType = self.defineEtherType(etherType)
        self._ipProtocolType = self.defineIpProtocol(ipProtocolType)
        self._destMac = destMac
        self._srcMac = srcMac
        self._etherFrame = self.ethframe()
        self.create()
        #self._src = src
        #self._dest = dest
        #self._data = data
        #self._packet = self.create()

    @staticmethod
    def src(packet):
        offset = 6
        mac = binascii.hexlify(struct.unpack('6s', packet[offset:offset+6])[0])
        offset = 14+12
        ip = socket.inet_ntoa(packet[offset:offset+4])
        offset = 14+20
        port = struct.unpack('!H', packet[offset:offset+2])[0]
        addr = net.NetAddr(mac=mac, ip=ip, port=port)
        return addr

    @staticmethod
    def dest(packet):
        offset = 0
        mac = binascii.hexlify(struct.unpack('6s', packet[offset:offset+6])[0])
        offset = 14+16
        ip = socket.inet_ntoa(packet[offset:offset+4])
        offset = 14+22
        port = struct.unpack('!H', packet[offset:offset+2])[0]
        addr = net.NetAddr(mac=mac, ip=ip, port=port)
        return addr

    def prepareFile(self, strFileName, maxFileSize):
        file = open(strFileName, 'r+b')
        fileSize = os.stat(strFileName).st_size
        offset = 0
        arrFile = []
        while (offset < fileSize):
            file.seek(0,1)
            fileChunk =  file.read(maxFileSize)
            offset+= maxFileSize
            arrFile.append(fileChunk)
        file.close()
        return arrFile


    def defineEtherType(self, etherType):
        if(etherType == 'IPV4'):
            return 0x0800
        return 0x86DD


    def defineIpProtocol(self, ipProtocolType):
        if(ipProtocolType == 'UDP'):
            return 17
        return 6

    def create(self):
        #for file in self._file:
        #    print file

        print self._etherType
        print self._ipProtocolType
        print self._etherFrame


    def udpframe(self):
        return struct.pack('HHHH',
            socket.htons(self._src.port),    # src port
            socket.htons(self._dest.port),   # dest port
            socket.htons(8+len(self._data)), # length
            0)                               # checksum

    def ipv4frame(self, plen):
        return struct.pack('BBHHHBBH4s4s',
            69,                              # version, ihl
            0,                               # dscp, ecn
            socket.htons(20+plen),           # length
            0,                               # ident
            0,                               # flags, fragment offset
            255,                             # ttl
            socket.IPPROTO_UDP,              # protocol
            0,                               # checksum
            socket.inet_aton(self._src.ip),  # src ip
            socket.inet_aton(self._dest.ip)) # dest ip

    def ethframe(self):
        print self._destMac.encode()
        print self._srcMac.encode()
        return struct.pack('6s6s2B',
            self._destMac,   # dest
            self._srcMac,    # src
            self._etherType, # etherType
            0)               # data
