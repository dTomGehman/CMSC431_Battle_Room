import java.io.*;
import java.net.*;
import java.util.Scanner;


public class Client
{
    public static final int PORT = 9001;
    public static final int BUFFSIZE = 100;
    public static final int NAMESIZE = 20;

    static DatagramSocket sockfd;

    public static void main(String[] args) throws Exception
    {
        Scanner input = new Scanner(System.in);
        int sequence = 0;
        byte[] buff = new byte[BUFFSIZE];

        System.out.print("Enter address to connect (port " + PORT + "):  ");
        String addrStr = input.nextLine();
        InetAddress server_addr = InetAddress.getByName(addrStr);

        sockfd = new DatagramSocket();

        System.out.print("Enter player name (max len is " + NAMESIZE + " chars):  ");
        String name = input.nextLine();
        if (name.length() < 1 || name.length() >= NAMESIZE)
        {
            System.out.println("Bad name--must not be empty or over 19 chars");
            sockfd.close();
            System.exit(1);
        }
        for (int x = 0; x < name.length(); x++)
        {
            if (name.charAt(x) == '`')
            {
                System.out.println("Bad name--please don't use backquotes (`)");
                sockfd.close();
                System.exit(1);
            }
        }

        //send join packet
        String msg = name + "`a`" + sequence;
        buff = msg.getBytes();
        DatagramPacket packet = new DatagramPacket(buff, buff.length, server_addr, PORT);
        sockfd.send(packet);

        //start thread to receive messages from server
        Thread receiveThread = new Thread(new ReceiveHandler());
        receiveThread.start();

        System.out.println("\nEnter actions (0/1/2) or e to exit");
        System.out.println(" 0:Defend 1:Attack 2:Heal");

        while (true)
        {
            String line = input.next();
            char action = line.charAt(0);

            if (action == 'e')
            {
                sequence++;
                msg = name + "`e`" + sequence;
                buff = msg.getBytes();
                packet = new DatagramPacket(buff, buff.length, server_addr, PORT);
                sockfd.send(packet);
                break;
            }

            if (action < '0' || action > '2')
            {
                System.out.println("Error:  enter a single char:  0, 1, 2, or e");
                continue;
            }

            sequence++;
            msg = name + "`" + action + "`" + sequence;
            buff = msg.getBytes();
            packet = new DatagramPacket(buff, buff.length, server_addr, PORT);
            sockfd.send(packet);
        }

        sockfd.close();
        System.exit(0);
    }

    static class ReceiveHandler implements Runnable
    {
        public void run()
        {
            byte[] buff2 = new byte[BUFFSIZE];
            while (true)
            {
                try
                {
                    DatagramPacket recvPacket = new DatagramPacket(buff2, buff2.length);
                    sockfd.receive(recvPacket);
                    String text = new String(recvPacket.getData(), 0, recvPacket.getLength()).trim();
                    System.out.println(text);
                }
                catch (Exception e)
                {
                    break;
                }
            }
        }
    }
}