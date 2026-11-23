import BattleRoom
import socket
import ipaddress
import threading
PORT = 9001
BUFFSIZE = 100
NAMESIZE = 20
playing = True
again = True

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
    while playing:
        data = sock.recv(100).decode('ascii').rstrip('\0')
        # Avoid printing end messages sent by C server
        if data not in ('\n', '\r\n', ''):
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
        playing = False
        t.join()
        send_new = input("play again? [y/n]").casefold().startswith('y')
        if send_new:
            sequence = 0
            print("Sending rejoin request to server...")
            sock.send(f"{player_name}`a`{sequence}".encode('ascii'))
            t = threading.Thread(target=recv_data,args=(sock,))
            playing = True
            t.start()
        else:
            sock.close()
            break
    else:
        sequence += 1
        bytestr = f"{player_name}`{action}`{sequence}".encode('ascii')
        sock.send(bytestr)
# Flag to stop our recv_thread from looping eternally.
# Join the thread
print("Closing socket...")
sock.close()
