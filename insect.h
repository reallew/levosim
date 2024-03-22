/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim
 * Copyright (C)  2019 <Lew Palm>
 * 
 */

#ifndef _INSECT_H_
#define _INSECT_H_

#include "agent.h"
#include "bushworld.h"
#include "debug_macros.h"

class Insect;
typedef std::shared_ptr<Insect> insect_ptr;

/**
 * An Insect is a meta-Agent for the Bushworld. It can not live alone (you have
 * to write a derived class). Insect contains all mechanism for children 
 * Fly and Wasp which they need both.
 */
class Insect : public Agent {
public:
	Insect(const genome_ptr mygen) : Agent(mygen),
					 last_branch_arrival_time(0.0), last_branch_leaving_time(-1.0), 
					 branch_hopping(true), avg_branch_time(-1.0), travel_time_sum(0.0), 
					 cluster_jumps(0.0), reward_rate_sum(0.0)
	{agent_type = "Insect";}

	void set_position(const int branch_pos, const int fruit_pos);
	void set_fruit_pos(const int fruit_pos);
	void set_branch_pos(const int new_branch_pos);
	bush_position get_position() const; 
	turn_counter get_last_branch_arrival_time() const;
	//void set_last_branch_arrival_time(const turn_counter new_bat);
	double get_cluster_jumps() const;
	void set_cluster_jumps(double new_jumps);
	virtual action cognite(const perception* agents_personal_perception) = 0;
	virtual bool is_parasitoid();
	bool is_between_branches();
	turn_counter get_avg_branch_time();
	void set_avg_branch_time(turn_counter new_avg_b_t);
	turn_counter get_travel_time_sum();
	turn_counter get_average_travel_time();

protected: 
	void cognition_start_statistics(const perception* perc);
	//void after_death_statistics();
		
	/** Last point in time when the insect arrived at a branch. */
	turn_counter last_branch_arrival_time;
	turn_counter last_branch_leaving_time;
	/** This is true if the insect changes the branch. */
	bool branch_hopping;

	turn_counter avg_branch_time;

	turn_counter travel_time_sum;
	/** Number of left clusters. */
	double cluster_jumps;
	/** Sum of all cluster-reward-rates until now. */
	double reward_rate_sum;
		
private:
	/** Current insects position in the world. */
	bush_position insect_position;
};

#endif // _INSECT_H_
