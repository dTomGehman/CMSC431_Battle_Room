//*******************************************************
//entity.h
//
//written by Dr. Girard
//with modifications by Tom Gehman
//*******************************************************

#ifndef ENTITY_H
#define ENTITY_H

#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>

struct Entity
{
	char name[20];  // No names over 19 characters
	int health;
	int max_health;
	int damage;
	double hit_chance;
	/**
	 * Kept in terms of milliseconds.
	 */
	int attack_rate; 
	int heal_rate;
	
	clock_t start_time;
	clock_t last_attack;
	clock_t last_heal;
	/**
	 * -1 : Do Nothing
	 *  0 : Defend (take half damage)
	 *  1 : Attack (if player attack random creature, if creature attack random player)
	 *             (only once per attack_rate)
	 *  2 : Heal (5 health, only once per heal_rate)
	 */
	int action;  // Ideally would have an enum for this
    
    //***added by tom gehman
    struct sockaddr_in *addr; //socket addr, which allows the room to communicate with each player
    size_t addr_s; //length of above
};	

void initialize_entity(struct Entity *entity, char *n, int h, int mh, int d, double hc, int ar, int hr, struct sockaddr_in*s, size_t sst);
void heal(struct Entity *entity, long time);
void set_action(struct Entity *entity, int value);
void take_damage(struct Entity *entity, int dmg);

#endif
