import struct
import socket
import binascii
import os


class Sender:
    #def __init__(self, src, dest, data):
    def __init__(self, fileName):
        self._file = self.prepareFile(fileName, 1480)
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

    @property
    def packet(self):
        return self._packet

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

    def create(self):
        for file in self._file:
            print file
            print '\n testando --------------------------------'
        #udpf = self._udpframe()
        #ipf = self._ipframe(len(udpf)+len(self._data))
        #ethf = self._ethframe()
        #return ethf + ipf + udpf + self._data

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
        return struct.pack('6s6s2B',
            binascii.unhexlify(self._dest.mac), # dest
            binascii.unhexlify(self._src.mac),  # src
            0x08, 0)        # ethtype
