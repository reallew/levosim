/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 */

#ifndef _BUSHWORLD_H_
#define _BUSHWORLD_H_

#include <vector>
#include <map>

#include "world.h"
#include "debug_macros.h"

class Genome;

#define SLEEP_AFTER_GENERATION 500000

/**
 * Position of an insect / agent in this world.
 */
struct bush_position {
	/** The branch (cluster) where the insect resides momentarily. */
	unsigned int branch;
	/** The fruit on the branch where the insect resides momentarily. */
	unsigned int fruit;
};

class Insect;
typedef std::shared_ptr<Insect> insect_ptr;

/**
 * An fly egg has a genome and may be infected with an wasp egg.
 */
struct fly_egg {
	genome_ptr fly_genome;
	genome_ptr wasp_genome;
	insect_ptr laying_fly;
	insect_ptr laying_wasp;
};

typedef std::shared_ptr<fly_egg> fly_egg_ptr;

/** One fruit is a vector of fly eggs. */
typedef std::vector<fly_egg_ptr> fruit;

/** One branch is a vector of fruits. */
typedef std::vector<fruit> branch;

/** Shared pointer to object of type branch. */
typedef std::shared_ptr<branch> branch_ptr;

/** And a whole plant consists of branches. */
typedef std::vector<branch_ptr> plant;

class Bushworld;
typedef std::shared_ptr<Bushworld> bushworld_ptr;

/** All the percepted information an insect gets. */
struct perception {
	unsigned int fruits_in_branch; // quantity of fruits on the current branch
	double competition_pressure; // insects-to-nests ratio
	bool i_know_this_fruit; // Insect has been on this fruit before.
	bool i_know_this_branch;
	bool fruit_free; // Can the fly lay an egg in the fruit?
	unsigned int fly_eggs_in_fruit; // How many fly eggs are in this fruit?
	unsigned int wasp_eggs_in_fruit; // How many wasp eggs are in this fruit?
	unsigned int foreign_eggs_in_fruit; // How many eggs from other insects?
	unsigned int own_eggs_in_fruit; // How many eggs by this insect?
	/** Current point in time. */
	turn_counter current_time;
};		

/** Type of action an agent can do.
    This must be adjusted for every setting.*/
enum action_type {
	WAIT, // Should not be used, makes no sense for an insect in this setting.
	LAY_EGG, // Lay an egg.
	GO_TO_FRUIT, // Go to another fruit.
	GO_TO_BRANCH_WEST, // Go to another branch in direction west.
	GO_TO_BRANCH_EAST // Go to another branch in direction east.
};

/** One action an agent can do. */
struct action {
	action_type type;
	double intensity;
};

/**
 * A bushworld consists of fruits (places to lay eggs for insects) and
 * branches (containers/clusters of fruits).
 */
class Bushworld final : public World {
public:
	Bushworld(const unsigned int branches=10, const unsigned int fruits_per_branch=10);
	void set_parasitoid_max_age(turn_counter para_max_age);
	void set_host_max_age(turn_counter host_max_age);
	turn_counter get_average_branch_time(const bool for_parasitoid);
	double get_average_cluster_jumps(const std::type_info* ins_type);
	void set_parasitoid_beginning_time(const turn_counter new_beg_t);
	unsigned int get_branch_quantity() const;
	unsigned int get_fruits_per_branch() const;
	void set_bush_size(unsigned int new_branch_quantity, unsigned int fruits_per_branch);
	void set_branch_quantity(unsigned int new_branch_quantity);
	void set_fruits_per_branch(unsigned int fruits_per_branch);
	void set_insect_death_chance(double new_death_chance);
	double get_insect_death_chance() const;
	void reset_statistics() override;
	double calculate_fitness();
	void recreate_world() override;
	void add_branch_jumps(const bool for_parasitoid, unsigned int new_jumps);
	void add_branch_time(const bool for_parasitoid, double new_time);
	unsigned int get_branch_jumps(const bool for_parasitoid);
	double get_branch_time(const bool for_parasitoid);
	void collect_multithread_statistics(world_ptr tmp_world) override;
	void finish_multithread_statistics(unsigned int world_runs) override;
	double get_best_insect_jumps(const std::type_info* ins_type);
	void set_best_insect_jumps(const std::type_info* ins_type, double jumps);
	double get_best_insect_avg_branch_time(const std::type_info* ins_type);
	void set_best_insect_avg_branch_time(const std::type_info* ins_type, double avg_b_t);
	void set_nn_layers(unsigned int new_nn_layers);
	
protected:
	void make_perception(agent_ptr cooper, perception* cooper_sees);
	void execute_action(agent_ptr cooper, action coopers_action);
	agent_ptr create_agent(genome_ptr agent_genome);
	/** Average time flys stay on branches per life. */
	turn_counter fly_branch_time;
	/** Average number of fly branch changes per life. */
	unsigned int fly_branch_jumps;
	/** Average time wasps stay on branches per life. */
	turn_counter wasp_branch_time;
	/** Average number of wasps branch changes per life. */
	unsigned int wasp_branch_jumps;
	int get_flying_distance(double intensity);
	void agent_death_statistics(agent_ptr dead_agent);
	/** Latest point in time time to die for wasps. */
	turn_counter parasitoid_max_age;
	/** Latest point in time time to die for flys. */
	turn_counter host_max_age;
	
private:
	/** Chance to die for every insect per turn. */
	double insects_death_chance;
	/** The data structure which contains all branches (and fruits). */
	plant bush;
	/** Returns the randomly chosen index number of one fruit. */
	unsigned int choose_fruit(unsigned int branch_no) const;
	turn_counter get_action_duration(const action* acting_action);
	/** Point in time when wasps can start to act. */
	turn_counter parasitoid_beginning_time;
	void place_insect_randomly(insect_ptr lost_insect);
	/** Amount of moves between clusters of the best wasp. */
	double best_wasp_cluster_jumps;
	/** Amount of moves between clusters of the best fly. */
	double best_fly_cluster_jumps;
};

#endif // _BUSHWORLD_H_
