/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * levosim
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 * This file contains definitions of all methods of the class Bushworld.
 *
 */

#include "bushworld.h"
#include "insect.h"
#include "fly.h"
#include "wasp.h"


Bushworld::Bushworld(const unsigned int branch_quantity, const unsigned int fruits_per_branch) : 
	fly_branch_time(0.0),
	fly_branch_jumps(0),
	wasp_branch_time(0.0),
	wasp_branch_jumps(0),
	insects_death_chance(0.002),
	parasitoid_beginning_time(500.0),
	best_wasp_cluster_jumps(0),
	best_fly_cluster_jumps(0)
{
	set_bush_size(branch_quantity, fruits_per_branch);
	
	BUG_CHECK(branch_quantity != bush.size(), "Wrong size in bushworld creation");
}

/**
 * Creates a completely new bush. All insects, genomes and eggs are lost, but all
 * other parameters (world size, etc.) are kept.
 */ 
void Bushworld::recreate_world() {
	set_bush_size(get_branch_quantity(), get_fruits_per_branch());
	genepool = genepool_copy();
	population.clear();
}

/**
 * Tells you how many branches are in the world.
 */
unsigned int Bushworld::get_branch_quantity() const {
	return bush.size();
}

/**
 * Returns the quantity of cluster changes of the fittest insect of the given type.
 */
double Bushworld::get_best_insect_jumps(const std::type_info* ins_type) {
	insect_ptr best_insect = 
		std::dynamic_pointer_cast<Insect>(get_best_agent(ins_type));
	BUG_CHECK(!best_insect, "No best insect.");
	return best_insect->get_cluster_jumps();
}

/**
 * Returns the average time per cluster of the fittest insect of the given type.
 */
double Bushworld::get_best_insect_avg_branch_time(const std::type_info* ins_type) {
	insect_ptr best_insect = 
		std::dynamic_pointer_cast<Insect>(get_best_agent(ins_type));
	BUG_CHECK(!best_insect, "No best insect.");
	turn_counter avg_b_t = best_insect->get_avg_branch_time();	
	BUG_CHECK(avg_b_t < 0.0, "Average branch time below zero: " << avg_b_t);
	return avg_b_t;
}

/**
 * Gets the best (fittest) insect of the given type and sets its average branch time to
 * the given value.
 */
void Bushworld::set_best_insect_avg_branch_time(const std::type_info* ins_type, double avg_b_t) {
	insect_ptr best_insect = 
		std::dynamic_pointer_cast<Insect>(get_best_agent(ins_type));
	BUG_CHECK(!best_insect, "No best insect.");
	BUG_CHECK(avg_b_t < 0.0, "Average branch time below zero: " << avg_b_t);
	best_insect->set_avg_branch_time(avg_b_t);
}

/**
 * Sets the quantity of cluster changes of the fittest insect of the given type.
 * This is used for multithreading statistics. Normally best cluster jumps should be 
 * calculated automatically by Bushworld::agent_death_statistics.
 */
void Bushworld::set_best_insect_jumps(const std::type_info* ins_type, double jumps) {
	BUG_CHECK(agent_type_infos.find(ins_type) == agent_type_infos.end(), "Agent type info not found.");
	
	agent_ptr best_agent = get_best_agent(ins_type);
	BUG_CHECK(!best_agent, "Got no best agent. Weird.");
	insect_ptr best_insect = std::dynamic_pointer_cast<Insect>(best_agent);
	best_insect->set_cluster_jumps(jumps);
	//	set_best_agent(best_insect); // useless
}

/**
 * Tells you how many fruits are in one branch (cluster). 
 */
unsigned int Bushworld::get_fruits_per_branch() const {
	BUG_CHECK(!bush.size(), "Empty bush");
	return bush.at(0)->size();
}

/**
 * This changes the amount of branches and the amount of fruits per branch in the world.
 * It can cause a lot of trouble if you call it during a generation run (in a 
 * multithreaded environment), because it changes the world size. Don't do that!
 */
void Bushworld::set_bush_size(unsigned int new_branch_quantity, unsigned int fruits_per_branch) {
	BUG_CHECK(!new_branch_quantity || !fruits_per_branch, "creating empty bush");
	bush = plant(new_branch_quantity);
	plant::iterator branch_i = bush.begin();
	while (branch_i != bush.end()) {
		fruit empty_fruit;
		branch_ptr empty_branch = branch_ptr(new branch(fruits_per_branch, empty_fruit));
		*branch_i = empty_branch;
		++branch_i;
	}

	// The living insects must have new places because of the different world size.
	agent_container::iterator agent_i = population.begin();
	while (agent_i != population.end()) {
		insect_ptr cooper = std::dynamic_pointer_cast<Insect>(*agent_i);
		place_insect_randomly(cooper);
		++agent_i;
	}
}

/**
 * This changes the amount of branches in the world.
 * It can cause a lot of trouble if you call it during a generation run (in a 
 * multithreaded environment), because it changes the world size. Don't do that!
 */
void Bushworld::set_branch_quantity(unsigned int new_branch_quantity) {
	set_bush_size(new_branch_quantity, get_fruits_per_branch());
}

/**
 * This changes the amount of fruits per branch in the world.
 * It can cause a lot of trouble if you call it during a generation run (in a 
 * multithreaded environment), because it changes the world size. Don't do that!
 */
void Bushworld::set_fruits_per_branch(unsigned int fruits_per_branch) {
	set_bush_size(get_branch_quantity(), fruits_per_branch);
}

/**
 * Assembles the perception for one insect in the situation before the insect decides
 * what to do next.
 * The insect can not see very far. In a 'perceiving situation' it always sits on a fruit
 * on a branch. That's why it gets informations about the fruit (what kind of eggs are
 * here?).
 */
void Bushworld::make_perception(agent_ptr agent_cooper, perception* cooper_sees) {
	insect_ptr cooper = std::dynamic_pointer_cast<Insect>(agent_cooper);
	bush_position c_pos = cooper->get_position();
	
	cooper_sees->competition_pressure = 1.0; // TODO?
	BUG_CHECK(c_pos.branch >= bush.size(), "Insect sits on branch " << c_pos.branch << 
	          ", but there are only " << bush.size() << " branches in the bush.");
	cooper_sees->fruits_in_branch = bush.at(c_pos.branch)->size();
	BUG_CHECK(cooper_sees->fruits_in_branch <= c_pos.fruit, "Agent sits on fruit " << 
	          c_pos.fruit << ", but there are only " << cooper_sees->fruits_in_branch << 
	          " fruits here.");
	cooper_sees->fruit_free = bush.at(c_pos.branch)->at(c_pos.fruit).size() == 0;
	cooper_sees->fly_eggs_in_fruit = bush.at(c_pos.branch)->at(c_pos.fruit).size();
	cooper_sees->wasp_eggs_in_fruit = 0;
	cooper_sees->foreign_eggs_in_fruit = 0;
	cooper_sees->own_eggs_in_fruit = 0;
	cooper_sees->current_time = turn;

	// The insects looks at every egg here.
	for (auto const& fly_egg: bush.at(c_pos.branch)->at(c_pos.fruit)) {
		if (fly_egg->wasp_genome)
			++cooper_sees->wasp_eggs_in_fruit;
		if (fly_egg->fly_genome != agent_cooper->get_genome_ptr() && 
		    fly_egg->wasp_genome != agent_cooper->get_genome_ptr())
			++cooper_sees->foreign_eggs_in_fruit;
		else
			++cooper_sees->own_eggs_in_fruit;			
	}
}

/**
 * Returns a randomly chosen fruit from the branch with the given number.
 */
unsigned int Bushworld::choose_fruit(unsigned int branch_no) const {
	BUG_CHECK(branch_no>=bush.size(), "Branch number too high: " << branch_no);
	BUG_CHECK(!bush.size(), "Empty bush.");
	BUG_CHECK(!bush.at(branch_no)->size(), "Empty branch.");
	unsigned int ret = randone() * (double)bush.at(branch_no)->size();
	// This is a quick dirty hack, but this situation happens very rarely.
	if (ret == bush.at(branch_no)->size())
		--ret;
	BUG_CHECK(ret < 0 || ret >= bush.at(branch_no)->size(), "Fruit " << ret << 
		" chosen, but that's impossible.");
	return ret;
}


int Bushworld::get_flying_distance(double intensity) {
/*	int distance = intensity * 3.0;
	if (distance < 1)
		distance = 1; 
	return distance; */
	return (intensity);
}

/**
 * Returns the time one action consumes for an agent.
 */
turn_counter Bushworld::get_action_duration(const action* acting_action) {
	double intensity = acting_action->intensity;
	
	if (acting_action->intensity <= 0.0) {
		std::cout << "Maybe Error: Action intensity <= 0 with action " << acting_action->type <<
			std::endl;
		std::cout << "Setting intensity to 0.1 to defend infinite loops." << std::endl;
		intensity = 0.1;
	}
	
	switch(acting_action->type) {
		case LAY_EGG: {
			return 3.3;
		}
		case GO_TO_FRUIT: {
			return 3.3;
		}
		case GO_TO_BRANCH_WEST: {
			return 3 + get_flying_distance(intensity) * 3;
		}
		case GO_TO_BRANCH_EAST: {
			return 3 + get_flying_distance(intensity) * 3;
		}
		case WAIT: {
			debug_msg("BUG? Somebody's waiting for " << intensity << " hours!");
			return intensity;
		}
	}

	// If the action type is not identified (what not should happen) the agent waits
	// one turn. A waiting time of one and not zero was chosen because zero could cause 
	// an infinite loop.
	return 1.0;
}

/**
 * This implements the wanted action of one insect to the world, if possible.
 * The action can be moving around (to another fruit or to another branch) or laying
 * an egg.
 */
void Bushworld::execute_action(agent_ptr agent_cooper, action coopers_action) {
	BUG_CHECK(!agent_cooper, "Empty agent pointer.")
	insect_ptr cooper = std::dynamic_pointer_cast<Insect>(agent_cooper);
	bush_position c_pos = cooper->get_position();
	debug_msg("Folgende Action beginnt um " << turn << " und soll " << 
	          get_action_duration(&coopers_action) << " dauern.");
	cooper->starts_to_act(get_action_duration(&coopers_action), turn);
	
	switch(coopers_action.type) {
		case LAY_EGG: {
			debug_msg("Agents LAYs EGG");

			if (!cooper->is_parasitoid()) {
				if (bush.at(c_pos.branch)->at(c_pos.fruit).size()) { // if there is already a fly egg.
					debug_msg("Bug? There is already a fly egg!");
					break; // there is no chance to lay an egg.
				}
				fly_egg_ptr new_fly_egg = fly_egg_ptr(new fly_egg);
				new_fly_egg->wasp_genome = genome_ptr();
				new_fly_egg->fly_genome = cooper->get_genome_ptr();
				new_fly_egg->laying_fly = cooper;
				bush.at(c_pos.branch)->at(c_pos.fruit).push_back(new_fly_egg);
			} else {
				if (bush.at(c_pos.branch)->at(c_pos.fruit).size() < 1) { // if there is no fly egg.
					debug_msg("Bug? There is no a fly egg, but wasp wants to lay an egg!");
					break; // there is no chance to lay an egg.
				}
				fly_egg_ptr old_fly_egg = bush.at(c_pos.branch)->at(c_pos.fruit).front();
				if (old_fly_egg->wasp_genome) {
					debug_msg("Bug? There is already a wasp egg in the fly egg, but wasp "
					          << "wants to lay another.");
					break; // there is no chance to lay an egg.
				}
				old_fly_egg->wasp_genome = cooper->get_genome_ptr();
				old_fly_egg->laying_wasp = cooper;
			}
		}
		break;
		
		case GO_TO_FRUIT: {
			cooper->set_fruit_pos(choose_fruit(c_pos.branch));
			debug_msg("Agents GOES TO FRUIT " << cooper->get_position().fruit);
		}
		break;
		
		case GO_TO_BRANCH_WEST: {
			int coopers_new_branch = c_pos.branch + get_flying_distance(coopers_action.intensity);
			coopers_new_branch %= bush.size();
			cooper->set_branch_pos(coopers_new_branch);
			if (cooper->is_parasitoid()) {
				wasp_branch_time += turn - cooper->get_last_branch_arrival_time();
				++wasp_branch_jumps;
			} else {
				fly_branch_time += turn - cooper->get_last_branch_arrival_time();
				++fly_branch_jumps;
			}
			//cooper->set_last_branch_arrival_time(cooper->get_action_finishing_time());
		}
		break;

		case WAIT: {
			// A waiting insect does nothing.
		}
		break;

		case GO_TO_BRANCH_EAST: {
			int coopers_new_branch = c_pos.branch - get_flying_distance(coopers_action.intensity);
			coopers_new_branch %= bush.size();
			cooper->set_branch_pos(coopers_new_branch);
			if (cooper->is_parasitoid()) {
				wasp_branch_time += turn - cooper->get_last_branch_arrival_time();
				++wasp_branch_jumps;
			} else {
				fly_branch_time += turn - cooper->get_last_branch_arrival_time();
				++fly_branch_jumps;
			}
			//cooper->set_last_branch_arrival_time(cooper->get_action_finishing_time());
		}
		break;
	}
}

double Bushworld::get_average_cluster_jumps(const std::type_info* ins_type) {
	double offspr_quant = get_offspring_quantity(ins_type);
	double jumps;
	if (*ins_type == typeid(Wasp))
		jumps = wasp_branch_jumps;
	else if (*ins_type == typeid(Fly))
		jumps = fly_branch_jumps;
	else {
		jumps = 0.0; // Dummy to get rid of the warning.
		BUG_CHECK(true, "Can not tell cluster jumps for " << ins_type->name());
	}
	return offspr_quant ? jumps / offspr_quant : 0.0;
}

turn_counter Bushworld::get_average_branch_time(const bool for_parasitoid) {
	double b_jumps, b_time;
	if (for_parasitoid) {
		b_jumps = (double)wasp_branch_jumps + (double)get_offspring_quantity(&typeid(Wasp));
		b_time = (double)wasp_branch_time;
	} else {
		b_jumps = (double)fly_branch_jumps + (double)get_offspring_quantity(&typeid(Fly));
		b_time = (double)fly_branch_time;
	}
	return b_time / b_jumps;
}

void Bushworld::add_branch_jumps(const bool for_parasitoid, unsigned int new_jumps) {
	if (for_parasitoid)
		wasp_branch_jumps += new_jumps;
	else
		fly_branch_jumps += new_jumps;
}

void Bushworld::add_branch_time(const bool for_parasitoid, double new_time) {
	BUG_CHECK(new_time<0.0, "Negative branch time: " << new_time);
	if (for_parasitoid)
		wasp_branch_time += new_time;
	else
		fly_branch_time += new_time;
}

/**
 * Returns the sum of all branch-changes that have been done -- for wasps or for
 * flys, depending on the given flag.
 */
unsigned int Bushworld::get_branch_jumps(const bool for_parasitoid) {
	return for_parasitoid ? wasp_branch_jumps : fly_branch_jumps;
}

/**
 * Returns the sum of all time periods insects have been on branches -- for wasps or for
 * flys, depending on the given flag.
 */
double Bushworld::get_branch_time(const bool for_parasitoid) {
	return for_parasitoid ? wasp_branch_time : fly_branch_time;
}

/**
 * This method is called automatically after every insects death.
 * It puts the insects last time on the last branch to the statistics.
 */
void Bushworld::agent_death_statistics(agent_ptr dead_agent) {
	BUG_CHECK(!dead_agent, "No agent body here.");
	insect_ptr dead_insect = std::static_pointer_cast<Insect>(dead_agent);

	// Assumption: the insect dies in the middle of the time the current action takes.
	turn_counter death_time = dead_agent->get_action_finishing_time() -
					   dead_agent->get_current_action_duration() / 2.0;
	// The time the insect spent on its last branch.
	turn_counter last_branch_time = death_time - dead_insect->get_last_branch_arrival_time();

	BUG_CHECK(last_branch_time < 0.0, "Last branch time below zero.");

	if (dead_insect->is_parasitoid())
		wasp_branch_time += last_branch_time;
	else
		fly_branch_time += last_branch_time;	
}

/**
 * This is called before every generation.
 * Prepares the bushworld for generation calculation, sets several statistical variables
 * to zero.
 */
void Bushworld::reset_statistics() {
	best_fitness = 0.0;
	fly_branch_time = 0.0;
	fly_branch_jumps = 0;
	wasp_branch_time = 0.0;
	wasp_branch_jumps = 0;
	best_fly_cluster_jumps = 0;
	best_wasp_cluster_jumps = 0;
	freeze_agents(parasitoid_beginning_time, &typeid(Wasp)); // Wasps must wait a while.
	delete_agent_fitnesses_statistics();
}

/**
 * Sets the point in time when the parasitoids (wasps) start to act.
 * This is useful because often you want the parasitoids to start after all hosts have
 * finished their actions.
 */
void Bushworld::set_parasitoid_beginning_time(const turn_counter new_beg_t) {
	parasitoid_beginning_time = new_beg_t;
}

/**
 * This method counts all surviving eggs in the bushworld and uses this amout as
 * fitness value for the genomes of the insects which have layed the eggs.
 * Every lonely fly egg survives and counts as one fitness point for the genome
 * (genotype) of the responsible (laying) fly.
 * If there is a wasp egg in the fly egg, only the wasp egg survives and counts as one
 * fitness point for the wasps genome.
 */
double Bushworld::calculate_fitness() {
	// Lets set all the fitness to zero first.
	debug_msg("There are " << genepool->size() << " Genes in Genepool.");
	for (auto const& genome: *genepool)
		genome->set_fitness(0.0);
	
	for (auto const& branch: bush)
		for (auto& fruit: *branch)
			if (fruit.size()) {  // Are there fly eggs?
				fly_egg_ptr first_fly_egg = fruit.front();
				
				BUG_CHECK(!first_fly_egg->fly_genome, "Fly egg without fly genome!");
				BUG_CHECK(!first_fly_egg->laying_fly, "Fly pointer missing.");
				
				genome_ptr surviving_genome;
				insect_ptr laying_insect;
				if (first_fly_egg->wasp_genome) {
					surviving_genome = first_fly_egg->wasp_genome;
					BUG_CHECK(!first_fly_egg->laying_wasp, "Wasp pointer missing.");
					laying_insect = first_fly_egg->laying_wasp;
				} else {
					surviving_genome = first_fly_egg->fly_genome;
					laying_insect = first_fly_egg->laying_fly;
				}
				
				surviving_genome->increase_fitness(1.0);
				inc_agent_fitness_statistic(laying_insect, 1.0);
				
				if (surviving_genome->get_fitness() > best_fitness)
					best_fitness = surviving_genome->get_fitness();

				// Delete all the eggs.
				fruit.clear();
			}
	
	return best_fitness;
}

void Bushworld::collect_multithread_statistics(world_ptr tmp_world) {
	bushworld_ptr tmp_bushworld = std::dynamic_pointer_cast<Bushworld>(tmp_world);

	// Average cluster jumps.
	add_branch_jumps(true, tmp_bushworld->get_branch_jumps(true));
	add_branch_jumps(false, tmp_bushworld->get_branch_jumps(false));

	// Average time per cluster and insect.
	add_branch_time(true, tmp_bushworld->get_branch_time(true));
	add_branch_time(false, tmp_bushworld->get_branch_time(false));

	// Fitness value of the best wasp.
	double best_per_agent_fitness = tmp_world->get_best_per_agent_fitness(typeid(Wasp));
	best_per_agent_fitness += get_best_per_agent_fitness(typeid(Wasp));
	set_best_per_agent_fitness(typeid(Wasp), best_per_agent_fitness);

	// Fitness value of the best fly.
	best_per_agent_fitness = tmp_world->get_best_per_agent_fitness(typeid(Fly));
	best_per_agent_fitness += get_best_per_agent_fitness(typeid(Fly));
	set_best_per_agent_fitness(typeid(Fly), best_per_agent_fitness);

	// Amount of cluster jumps of the best wasp.
	double best_jumps = tmp_bushworld->get_best_insect_jumps(&typeid(Wasp));
	best_jumps += get_best_insect_jumps(&typeid(Wasp));
	set_best_insect_jumps(&typeid(Wasp), best_jumps);

	// Amount of cluster jumps of the best fly.
	best_jumps = tmp_bushworld->get_best_insect_jumps(&typeid(Fly));
	best_jumps += get_best_insect_jumps(&typeid(Fly));
	set_best_insect_jumps(&typeid(Fly), best_jumps);

	// Average cluster time of best wasp.
	double best_bt = tmp_bushworld->get_best_insect_avg_branch_time(&typeid(Wasp));
	best_bt += get_best_insect_avg_branch_time(&typeid(Wasp));
	set_best_insect_avg_branch_time(&typeid(Wasp), best_bt);

	// Average cluster time of best fly.
	best_bt = tmp_bushworld->get_best_insect_avg_branch_time(&typeid(Fly));
	best_bt += get_best_insect_avg_branch_time(&typeid(Fly));
	set_best_insect_avg_branch_time(&typeid(Fly), best_bt);
}

void Bushworld::finish_multithread_statistics(unsigned int world_runs) {
	double best_pa_fitness = get_best_per_agent_fitness(typeid(Wasp)) / (double)world_runs;
	set_best_per_agent_fitness(typeid(Wasp), best_pa_fitness);
	best_pa_fitness = get_best_per_agent_fitness(typeid(Fly)) / (double)world_runs;
	set_best_per_agent_fitness(typeid(Fly), best_pa_fitness);
	
	wasp_branch_jumps /= world_runs;
	fly_branch_jumps /= world_runs;
	wasp_branch_time /= (double)world_runs;
	fly_branch_time /= (double)world_runs;
	
	double best_jumps = get_best_insect_jumps(&typeid(Wasp)) / (double)world_runs;
	set_best_insect_jumps(&typeid(Wasp), best_jumps);
	best_jumps = get_best_insect_jumps(&typeid(Fly)) / (double)world_runs;
	set_best_insect_jumps(&typeid(Fly), best_jumps);

	double best_bt = get_best_insect_avg_branch_time(&typeid(Wasp)) / (double)world_runs;
	set_best_insect_avg_branch_time(&typeid(Wasp), best_bt);
	best_bt = get_best_insect_avg_branch_time(&typeid(Fly)) / (double)world_runs;
	set_best_insect_avg_branch_time(&typeid(Fly), best_bt);
}

/**
 * Sets the latest point in time to die for all parasitoids (wasps), no one could get 
 * older.
 */
void Bushworld::set_parasitoid_max_age(turn_counter new_para_max_age) {
	for (auto const& agent: population) {
		insect_ptr cooper = std::dynamic_pointer_cast<Insect>(agent);
		if (cooper->is_parasitoid())
			cooper->set_max_age(new_para_max_age);
	}
	parasitoid_max_age = new_para_max_age;
}

/**
 * Sets the latest point in time to die for all hosts (flys), no one could get older.
 */
void Bushworld::set_host_max_age(turn_counter new_host_max_age) {
	for (auto const& agent: population) {
		insect_ptr cooper = std::dynamic_pointer_cast<Insect>(agent);
		if (!cooper->is_parasitoid())
			cooper->set_max_age(host_max_age);
	}
	host_max_age = new_host_max_age;
}

/**
 * Creates an agent from a given genome.
 * The genome must have a valid type for this world (Wasp or Fly).
 */
agent_ptr Bushworld::create_agent(genome_ptr agent_genome) {
	BUG_CHECK(!agent_genome, "I want to create an agent, but there is no genome.");

	insect_ptr new_agent;
	if (agent_genome->agents_type_equals(typeid(Wasp))) {
		new_agent = wasp_ptr(new Wasp(agent_genome));
		new_agent->set_max_age(parasitoid_max_age);
	} else if (agent_genome->agents_type_equals(typeid(Fly))) {
		new_agent = fly_ptr(new Fly(agent_genome));
		new_agent->set_max_age(host_max_age);
	} else {
		BUG_CHECK(true, "Bug: Unknown genome from agent " << 
		          agent_genome->get_agents_name());
	}
	new_agent->set_death_chance(insects_death_chance);

	place_insect_randomly(new_agent);

	return new_agent;
}

/**
 * Puts the insect on a randomly chosen fruit in the Bushworld.
 */
void Bushworld::place_insect_randomly(insect_ptr lost_insect) {
	BUG_CHECK(!lost_insect, "No insect.");
	unsigned int new_branch_pos = (double)bush.size() * randone();
	BUG_CHECK(new_branch_pos >= bush.size() || bush.size() == 0, "Wrong branch");
	unsigned int new_fruit_pos = (double)bush.at(new_branch_pos)->size() * randone();
	BUG_CHECK(new_fruit_pos >= bush.at(new_branch_pos)->size() || 
	          bush.at(new_branch_pos)->size() == 0, "Wrong fruit.");
	lost_insect->set_position(new_branch_pos, new_fruit_pos);
}

/**
 * Sets the chance to die per turn (per one time unit) for every insect.
 */
void Bushworld::set_insect_death_chance(double new_death_chance) {
	BUG_CHECK(new_death_chance < 0.0 || new_death_chance > 1.0, "Death chance is " << 
	          new_death_chance << " but range is 0..1.");
	insects_death_chance = new_death_chance;
}

/**
 * Returns the chance to die per turn (per one time unit) for the insects.
 */
double Bushworld::get_insect_death_chance() const {
	return insects_death_chance;
}
