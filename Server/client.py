# Copyright: Krzysztof Kowalczyk
# Owner: Krzysztof Kowalczyk
#
# Emulates client (Palm) application by issuing requests to the server.
# For testing the server.

import sys, re, socket

# server must be of form "name:port"
g_serverList = ["localhost:9000", "dict-pc.arslexis.com:9000"]

g_defaultServerNo = 0 # index within g_serverList

def getServerNamePort():
    srv = g_serverList[g_defaultServerNo]
    (name,port) = srv.split(":")
    port = int(port)
    return (name,port)

def socket_readAll(socket):
    result = ""
    while True:
        data = sock.recv(8192)
        if None == data:
            break
        result += data
    return result

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
(serverName, serverPort) = getServerNamePort()
sock.connect((serverName,serverPort))
print "Connected to server"

dt = """Protocol-Version: 1
Client-Version: 0.5
Transaction-ID: 164D7F4E
Cookie: 21a4d1cbACFD2A971e9ef04823d7f1bd
Get-Definition: wall"""

dt = """Protocol-Version: 1
Client-Version: 0.5
Transaction-ID: 164D7F4F
Cookie: 21a4d1cbACFD2A971e9ef04823d7f1bd
Get-Definition: wall"""

sock.sendall(dt)
print "Sent:", dt
response = socket_readAll(sock)
print "Received:", response
sock.close()

