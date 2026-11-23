"""
Author: Daniel Morine
Purpose: This is the client for the UDP MUD inspired game.  Entirely authored by Daniel Morine using python.org and man pages for research.
"""
import BattleRoom
import socket
import ipaddress
import threading
PORT = 9001
BUFFSIZE = 100
NAMESIZE = 20
playing = True
again = True




######## Functions ########
#// Author: Daniel Morine
#// Purpose: get the ip of the player and ensure its formatted properly
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
#// Author: Daniel Morine
#// Purpose: get the name of the player ensure formatting
def get_player_name():
    print("Enter your player name (20 chars max)")
    while True:
        name = input()
        if not name == '' and not '`' in name:
            return name
        print("Name is empty or contains backticks. Retrying...")
#// Author: Daniel Morine
#// Purpose: get the name of the player action + ensure formatting
def get_action():
    while True:
        action = input().strip() 
        if len(action) != 1:
            print("Error:  enter a single char:  0, 1, 2, or e")
        if action in {'e', '0', '1', '2'}:
            return action
        print("Error: Invalid action chosen.")
#// Author: Daniel Morine
#// Purpose: Threaded, receives data
def recv_data(sock):
    while playing:
        data = sock.recv(100).decode('ascii').rstrip('\0')
        log_packet(log_file, (ipaddr,PORT), data, sock.getsockname(),is_action=False)

        # Avoid printing end messages sent by C server
        if data not in ('\n', '\r\n', ''):
            print(data)

def log_packet(log_file, src, content, dst, is_action=False):
    src_str = f"{src[0]}:{src[1]}" if isinstance(src, tuple) else src
    dst_str = f"{dst[0]}:{dst[1]}" if isinstance(dst, tuple) else dst

    if is_action:
        log_file.write(f"{src_str} sent {content} to {dst_str}\n")
    else:
        escaped = str(content).replace('\r', '\\r').replace('\n', '\\n')
        log_file.write(f'{src_str} sent "{escaped}" to {dst_str}\n')

sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
#sock.bind(('',PORT))
ipaddr = get_server_ip() 
sock.connect((ipaddr,PORT))
player_name = get_player_name()
log_file = open(f"{player_name}_log", "a", buffering=1, encoding="utf-8")
sequence = 0
bytestr=f"{player_name}`a`{sequence}".encode('ascii')
sock.send(bytestr)
log_packet(log_file, sock.getsockname(), bytestr, (ipaddr,PORT), is_action=True)
t = threading.Thread(target=recv_data,args=(sock,))
t.start()
print("Enter actions (0/1/2) or e to exit")
print("0:Defend 1:Attack 2:Heal")

#// Author: Daniel Morine
#// Purpose: Main loop, handles player quit/rejoin and sending actions.
while True:
    action = get_action()
    if action == 'e':
        sequence+=1
        bytestr = f"{player_name}`e`{sequence}".encode('ascii')
        sock.send(bytestr)
        log_packet(log_file, sock.getsockname(), bytestr, (ipaddr,PORT), is_action=True)
        playing = False
        t.join()
        send_new = input("play again? [y/n]").casefold().startswith('y')
        if send_new:
            sequence = 0
            print("Sending rejoin request to server...")
            bytestr = f"{player_name}`a`{sequence}".encode('ascii')
            sock.send(bytestr)
            log_packet(log_file, sock.getsockname(), bytestr, (ipaddr,PORT), is_action=True)
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
        log_packet(log_file, sock.getsockname(), bytestr, (ipaddr,PORT), is_action=True)
print("Closing socket...")
sock.close()
