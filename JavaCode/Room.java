import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.util.ArrayList;

/**
 * There should only be one of these running as a server and players should connect to it as clients.
 * @author cdgira
 *
 */
public class Room extends Thread
{
	private ArrayList<Entity> creatures = new ArrayList<Entity>();
	private ArrayList<Entity> players = new ArrayList<Entity>();
	private ArrayList<String> messages = new ArrayList<String>();
	private double spawn_chance = 0.05;
	/**
	 * Kept in milliseconds.
	 */
	private int check_spawn = 1000;

	private boolean run = true;

	private long presentTime;
	private int creatureID = 1;
	private DatagramSocket sockfd;

	public Room(double sc, int cs)
	{
		spawn_chance = sc;
		check_spawn = cs;
	}

	public Room(double sc, int cs, DatagramSocket s)
	{
		spawn_chance = sc;
		check_spawn = cs;
		sockfd = s;
	}

	public void addPlayer(Entity player)
	{
		players.add(player);
		messages.add(player.getName()+" has entered the room.");
	}

	public void removePlayer(String name)
	{
		for (int i = 0; i < players.size(); i++)
		{
			if (players.get(i).getName().equals(name))
			{
				players.remove(i);
				messages.add(name+" has left the room.");
				break;
			}
		}
	}

	private void attackRandomEntity(Entity attacker,ArrayList<Entity> options)
	{
		int index = (int)(Math.random()*options.size());
		Entity target = options.get(index);
		int targetHealth = target.getHealth();
		target.takeDamage(attacker.getDamage());
		messages.add(attacker.getName()+" attacked "+target.getName()+" doing "+(targetHealth - target.getHealth())+" damage.");
		attacker.setLastAttack(presentTime);
		if (target.getHealth() == 0)
		{
			options.remove(index);
			messages.add(target.getName()+" killed.");
		}

	}

	private void printMessages()
	{
		while (messages.size() > 0)
		{
			String buff = messages.get(0);
			System.out.println(buff);
			if (sockfd != null)
			{
				byte[] data = buff.getBytes();
				for (int j = 0; j < players.size(); j++)
				{
					Entity p = players.get(j);
					if (p != null && p.getAddr() != null)
					{
						try
						{
							DatagramPacket packet = new DatagramPacket(data, data.length, p.getAddr(), p.getPort());
							sockfd.send(packet);
							Server.logMessage("SENT", buff, Server.localIP, p.getAddr().getHostAddress());
						}
						catch (Exception e)
						{
							e.printStackTrace();
						}
					}
				}
			}
			messages.remove(0);
		}
	}

	private void processActions(ArrayList<Entity> entities,ArrayList<Entity> targets)
	{
		for (int i = 0; i < entities.size(); i++)
		{
			Entity entity = entities.get(i);
			int action = entity.getAction();
			if ((action == 1) && (targets.size() > 0))
			{
				long lastAttack = entity.getLastAttack();
				if ((presentTime - lastAttack) > entity.getAttackRate())
				{
					attackRandomEntity(entity,targets);
				}
			}
			if (action == 2)
			{
				long lastHeal = entity.getLastHeal();
				if ((presentTime - lastHeal) > entity.getHealRate())
				{
					int entityHealth = entity.getHealth();
					entity.heal(presentTime);
					messages.add(entity.getName()+" healed for "+(entity.getHealth()-entityHealth));
				}

			}
		}
	}

	public void run()
	{
		long startTime = System.currentTimeMillis();
		long lastSpawnCheck = startTime;
		while (run)
		{
			presentTime = System.currentTimeMillis();
			if ((presentTime - lastSpawnCheck) > check_spawn)
			{
				double check = Math.random();
				if (spawn_chance > check)
				{
					Entity creature = new Entity("Creature"+creatureID,20,20,10,0.6,1000,5000);
					creatures.add(creature);
					messages.add(creature.getName()+" has entered the room.");
					creatureID++;
				}
				lastSpawnCheck = presentTime;
			}

			processActions(players,creatures);
			processActions(creatures,players);

			updateCreatureAction();
			printMessages();
		}
	}

	public void setRun(boolean value)
	{
		run = value;
	}

	private void updateCreatureAction()
	{
		for (int i = 0; i < creatures.size(); i++)
		{
			Entity creature = creatures.get(i);
			long lastAttack = creature.getLastAttack();
			if ((presentTime - lastAttack) > creature.getAttackRate())
				creature.setAction(1); //attack
			else
			{
				long lastHeal = creature.getLastHeal();
				if ((presentTime - lastHeal) > creature.getHealRate())
					creature.setAction(2); //If can't attacK but can heal do that.
				else
					creature.setAction(0); //If can't attack or heal then defend.
			}
		}
	}

}
