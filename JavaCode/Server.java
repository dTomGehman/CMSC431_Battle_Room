import java.io.*;
import java.net.*;
import java.util.ArrayList;

public class Server
{
    public static final int PORT = 9001;
    public static final int NAMELEN = 20;

    static Room room;

    static class Node
    {
        String player_name;
        int seq;
        Entity me;

        public Node(String name, int s, Entity e)
        {
            player_name = name;
            seq = s;
            me = e;
        }
    }

    static ArrayList<Node> players = new ArrayList<Node>();

    public static void list_add_player(String player_name, int seq, Entity me)
    {
        players.add(new Node(player_name, seq, me));
    }

    public static void list_print_players()
    {
        for (int i = 0; i < players.size(); i++)
        {
            System.out.print(players.get(i).player_name + " ");
        }
        System.out.println();
    }

    public static Node list_get_node(String player_name)
    {
        for (int i = 0; i < players.size(); i++)
        {
            if (players.get(i).player_name.equals(player_name))
            {
                return players.get(i);
            }
        }
        return null;
    }

    public static Entity list_get_player(String player_name)
    {
        Node n = list_get_node(player_name);
        if (n != null)
        {
            return n.me;
        }
        return null;
    }

    public static void list_remove_player(String player_name)
    {
        for (int i = 0; i < players.size(); i++)
        {
            if (players.get(i).player_name.equals(player_name))
            {
                players.remove(i);
                return;
            }
        }
    }

    public static void main(String[] args) throws Exception
    {
        // create socket
        DatagramSocket sockfd = new DatagramSocket(PORT);

        // create room
        room = new Room(1.0, 20000, sockfd);

        // start thread
        room.start();

        // receive data from clients (joining, actions, exiting)
        byte[] buffer = new byte[100];

        while (true)
        {
            DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
            sockfd.receive(packet);

            // message consists of NAME`ACTION`SEQ
            String msg = new String(packet.getData(), 0, packet.getLength()).trim();
            int nullIdx = msg.indexOf('\0');
            if (nullIdx != -1)
            {
                msg = msg.substring(0, nullIdx);
            }

            String[] parts = msg.split("`");
            String player_name = parts[0];
            char action = parts[1].charAt(0);
            int seq = Integer.parseInt(parts[2].trim());

            if (action == 'a')
            {
                Entity me = new Entity(player_name, 100, 100, 10, 0.6, 1000, 5000, packet.getAddress(), packet.getPort());
                room.addPlayer(me);
                list_add_player(player_name, seq, me);
                System.out.print(player_name + " Has joined the room\nPlayers include:  ");
                list_print_players();
            }
            else
            {
                Node node = list_get_node(player_name);
                if (node == null)
                {
                    System.out.println("Unknown player: " + player_name);
                    continue;
                }

                int expected = node.seq + 1;
                if (seq > expected)
                {
                    System.out.println("Warning: missed packet(s) from " + player_name + " (expected seq " + expected + ", got " + seq + ")");
                    node.seq = seq;
                }
                else if (seq < expected)
                {
                    System.out.println("Warning: out-of-order packet from " + player_name + " (expected seq " + expected + ", got " + seq + ") - discarding");
                    continue;
                }
                else
                {
                    node.seq = seq;
                }

                if (action == 'e')
                {
                    list_remove_player(player_name);
                    room.removePlayer(player_name);
                    System.out.print(player_name + " Has left the room\nPlayers include:  ");
                    list_print_players();
                }
                else
                {
                    // send the action
                    node.me.setAction(action - '0');
                }
            }

            // for debugging
            System.out.println("Received action " + action + " from " + player_name + ", seq " + seq);
        }
    }
}