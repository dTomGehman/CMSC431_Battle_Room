import java.io.*;
import java.net.*;

public class Server
{
    public static final int PORT = 9001;
    public static final int NAMELEN = 20;

    public static void main(String[] args) throws Exception
    {
        DatagramSocket sockfd = new DatagramSocket(PORT);
        byte[] buffer = new byte[100];

        while (true)
        {
            DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
            sockfd.receive(packet);

            //message -> NAME`ACTION`SEQ
            String msg = new String(packet.getData(), 0, packet.getLength()).trim();
            String[] parts = msg.split("`");
            String player_name = parts[0];
            char action = parts[1].charAt(0);
            int seq = Integer.parseInt(parts[2].trim());

            if (action == 'a')
            {
                System.out.println(player_name + " Has joined the room");
            }
            else if (action == 'e')
            {
                System.out.println(player_name + " Has left the room");
            }
            else
            {
            }


            System.out.println("Received action " + action + " from " + player_name + ", seq " + seq);
        }
    }
}
