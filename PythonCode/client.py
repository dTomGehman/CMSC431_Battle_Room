import BattleRoom
import socket
import ipaddress
import threading
PORT = 9001
BUFFSIZE = 100
NAMESIZE = 20


def get_server_ip():
    while True:
        try:
            print(f"Please input the ip for addr:{PORT} connection")
            ipaddr = input().strip()
            # This will throw an exception if IP is invalid.
            ipaddress.ip_address(ipaddr)
            return ipaddr
        except ValueError:
            print("Incorrect formatting, retrying...")
def get_player_name():
    print("Enter your player name (20 chars max)")
    while True:
        name = input()
        if not name == '' and not '`' in name:
            return name
        print("Name is empty or contains backticks. Retrying...")
def get_action():
    while True:
        action = input().strip() 
        if len(action) != 1:
            print("Error:  enter a single char:  0, 1, 2, or e")
        if action in {'e', '0', '1', '2'}:
            return action
        print("Error: Invalid action chosen.")
def recv_data(sock):
    while True:
        data = sock.recv(100).decode('ascii').rstrip('\0')
        print(data)
sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
#sock.bind(('',PORT))
ipaddr = get_server_ip() 
sock.connect((ipaddr,PORT))
player_name = get_player_name()
sequence = 0
sock.send(f"{player_name}`a`{sequence}".encode('ascii'))

t = threading.Thread(target=recv_data,args=(sock,))
t.start()
print("Enter actions (0/1/2) or e to exit")
print("0:Defend 1:Attack 2:Heal")
# Get player actions
while True:
    action = get_action()
    if action == 'e':
        sequence+=1
        bytestr = f"{player_name}`e`{sequence}".encode('ascii')
        sock.send(bytestr)
        break
    sequence += 1
    bytestr = f"{player_name}`{action}`{sequence}".encode('ascii')
    sock.send(bytestr)

# Join the thread
t.join()
sock.close()
