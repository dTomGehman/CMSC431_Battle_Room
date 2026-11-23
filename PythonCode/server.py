import BattleRoom
import socket
import threading
# Constants
PORT = 9001
# ANSI ESCAPES
RED = "\x1b[1;31m"
GREEN = "\x1b[1;32m"
RESET = "\x1b[0m"
# Global vars
players = []
def run_game(room):
    BattleRoom.runGame(room) 
def set_get_seq(player_name, seq):
    player = get_player(player_name)
    if player == None:
        return None
    retval = player["seq"]
    player["seq"] = seq
    return retval
def get_player(name):
    for player in players:
        if name in player:
            return player[name]
    return None
# Unconditionally add player to server list.
# For the room, it depends on if a ghost exists already.
def add_player(room,name,seq,addr):
    room_player = room.getPlayer(name)[1]
    if room_player == None:
        new_entity = BattleRoom.Entity(name,100,100,10,.6,1000,5000,addr)
        players.append({name: {"seq":seq, "entity":new_entity}})
        room.addPlayer(new_entity)
    else:
        cur_health = room_player.getHealth()
        # No need to update room, we already have a living player inside.
        if cur_health > 0:
            new_entity = BattleRoom.Entity(name,cur_health,100,10,.6,1000,5000,addr)
            players.append({name: {"seq":seq, "entity":new_entity}})
        else:
            new_entity = BattleRoom.Entity(name,cur_health,100,10,.6,1000,5000,addr)
            players.append({name: {"seq":seq, "entity":new_entity}})
            room.addPlayer(new_entity)
# Removes player from server, and leave a ghost in room
def remove_player(name):
    for index,player in enumerate(players):
        if name in player:
            players.pop(index)
print(GREEN + "Starting game, waiting for players to connect" + RESET);
# Create and bind a socket
sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM,0)
sock.bind(('',PORT))
# Create room and spawn a thread to run it
room = BattleRoom.createRoom(1.0,20000,sock)
t = threading.Thread(target=run_game, args=(room,))
t.start()
# Main thread, handles message sending / actions packets
while True:
    # receive/decode/parse the data
    data,addr = sock.recvfrom(100)
    try:
        data = data.decode('ascii').rstrip('\x00')
        player_name, action, seq = data.split('`')
        seq = int(seq)
    # If you actually get these, drop the exception for more details
    except (UnicodeDecodeError, ValueError):
        print(RED + "Warning: Malformed packet. Dropping..." + RESET)
        continue
    if action == 'a':
        add_player(room,player_name,seq,addr)
        continue
    # Drop packets from dead players
    player = get_player(player_name)
    if action == 'e':
        remove_player(player_name)
        # Want to add functionality to message other players.
        # Might need to add socket information to this thread too.
        #print(f"{player_name} has left the room.")
        # Send a message so clients recv_thread can close
        sock.sendto("\n".encode('ascii'),addr)
        continue

    if player is None:
        print(RED + f"Warning: Player {player_name} is dead. Dropping packet." + RESET)
        continue
    if player["entity"].getHealth() <= 0:
        print(RED + f"Warning: Player {player_name} is dead. Dropping packet." + RESET)
        remove_player(player_name)
        continue

    # check if data out of order
    testseq = set_get_seq(player_name,seq)
    if testseq == None:
        print(RED + "Warning: Received action from dead/non-existent player. Dropping.." + RESET)
    else:
        if testseq+1 < seq:
            print(RED + f"Warning, missing packet from {player_name}" + RESET)
        elif testseq+1 > seq:
            print(RED + f"Warning, out of order packet from {player_name}" + RESET)

    try:
        action = int(action)
    except ValueError:
        print(RED + "Warning: Invalid action. Dropping..." + RESET)
        continue
    if player is not None:
        player["entity"].setAction(action)
