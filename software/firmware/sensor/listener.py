import socket
import sys

HOST = "192.168.0.40"
PORT = 50000

# socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print("Socket created")

# allow reuse
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

# bind to specified local host and port
try:
    s.bind((HOST, PORT))
except socket.error as e:
    print("Bind failed: {0}".format(e))

    sys.exit(1)

print("Socket bound")

# listen on socket
s.listen(10)
print("Listening on {0}:{1}".format(HOST, PORT))

try:
    # loop until terminated
    while True:
        # block until connection appears
        conn, addr = s.accept()

        print("Connection from {0}:{1}".format(addr[0], addr[1]))

        # extract data
        data = conn.recv(1024)

        print("\tdata: \"{0}\"".format(data))
finally:
    # close socket
    s.close()
