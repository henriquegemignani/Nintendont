# /bin/env python2

import struct
import socket
import datetime

host = '192.168.1.187'
port = 43673

mapIP = "127.0.0.1"
mapPort = 43673

structfmt = ">BIH"  # type and code
structfmt += "3f"  # speed
structfmt += "3f"  # pos
structfmt += "IIIf"  # world id, world status, room, health
structfmt += str(0x29 * 2) + "I"  # inventory
structfmt += "Q"  # timer

struct = struct.Struct(structfmt)

print structfmt
print str(struct.size)

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((host, port))

mapSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # UDP

def timestamp():
    return int((datetime.datetime.utcnow() - datetime.datetime(1970, 1, 1)).total_seconds() * 1000)

start = timestamp()
count = 0
buff = ""
print "Connected"
while True:
    read = sock.recv(1024)
    if not read:
        break
    buff += read
    if len(buff) >= struct.size:
        count += 1
        if timestamp() > start + 1000:
            end = timestamp()
            time = end - start
            start = end
            time_per = time / count
            bandwidth = (count * struct.size) / (time / 1000.0)
            print "Count: " + str(count) + ", Time per: " + str(time_per) + "ms, bandwidth: " + str(bandwidth) + "B/s"
            count = 0
        packet = buff[:struct.size]
        buff = buff[struct.size:]
        mapSock.sendto(packet, (mapIP, mapPort))
        unpacked = struct.unpack(packet)
        # print unpacked

print "Disconnected"