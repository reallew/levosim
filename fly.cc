/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 */
#include <cstdlib>
#include "fly.h"
#include "bushworld.h"

/**
 * Sets some variables to zero.
 */
Fly::Fly(genome_ptr mygen) : 
	Insect(mygen), 
	fruits_on_current_branch_seen_free(0),
	free_fruits_on_other_branches_seen(0),
	foreign_fly_eggs_on_current_branch_seen(0),
	own_eggs_seen(0),
	all_own_eggs_seen(0),
	laid_eggs(0),
	cluster_laid_eggs(0),
	bad_fruits_seen(0)
{ 
	agent_type = "Fly";
}

/**
 * The Fly uses this methode to decide what to do next. It gets a pointer to the flys
 * perception in this moment and has to return the flys wanted action. The World tries to
 * do that action -- but it must not succeed.
 */
action Fly::cognite(const perception* pcpt) {
	cognition_start_statistics(pcpt);
	action ret;

	if (pcpt->fruit_free) {
		ret.intensity = 1.0;
		ret.type = LAY_EGG;
		++laid_eggs;
		++cluster_laid_eggs;
		++fruits_on_current_branch_seen_free;
	} else {
		if (!pcpt->own_eggs_in_fruit)
			++bad_fruits_seen;
		foreign_fly_eggs_on_current_branch_seen += pcpt->foreign_eggs_in_fruit;
		own_eggs_seen += pcpt->own_eggs_in_fruit;
		all_own_eggs_seen += pcpt->own_eggs_in_fruit;

		nn_signals_ptr sigs = nn_signals_ptr(new nn_signals()); // The perception vector.
		fill_input_signals(sigs, pcpt);

		if (neuronal_network(sigs)) {
			static const double time_scaler = 0.05;
			double branchtime = (pcpt->current_time - last_branch_arrival_time) * time_scaler;
			if (branchtime)
				reward_rate_sum += cluster_laid_eggs / branchtime;
			leave_branch(&ret);
			last_branch_leaving_time = pcpt->current_time;
		} else { 
			ret.type = GO_TO_FRUIT;
			ret.intensity = 1.0;
		}
		
	}

	return ret;
}

/**
 * Called after the decision to leave the branch and to go to another.
 * Mainly some statistics are done here.
 */
void Fly::leave_branch(action* ret) {
	free_fruits_on_other_branches_seen += fruits_on_current_branch_seen_free;
	foreign_fly_eggs_on_current_branch_seen = 0;
	fruits_on_current_branch_seen_free = 0;
	own_eggs_seen = 0;
	cluster_laid_eggs = 0;
	branch_hopping = true;
	++cluster_jumps;
	if (get_genome_ptr()->get_gene(next_gene++) < World::randone())
		ret->type = GO_TO_BRANCH_WEST;
	else
		ret->type = GO_TO_BRANCH_EAST;
	ret->intensity = (int) (1.0 + get_genome_ptr()->get_gene(next_gene++) * 3.0);
}

/**
 * Collects some data for the neuronal network, which may hopefully be helpful there.
 */
void Fly::fill_input_signals(nn_signals_ptr sigs, const perception* pcpt) {
	// Collected personal statistics as perceptions:
	sigs->push_back(sigmoid(foreign_fly_eggs_on_current_branch_seen));  // on current branch
	// sigs->push_back(sigmoid(fruits_on_current_branch_seen_free));		// on current branch
	sigs->push_back(sigmoid(laid_eggs));								// in the whole bush
	sigs->push_back(sigmoid(cluster_laid_eggs));						// on current branch
	sigs->push_back(sigmoid(cluster_jumps));							// in the whole bush
	sigs->push_back(sigmoid((pcpt->current_time - birth_time) / (max_age - birth_time))); // Lifetime normalized
	//sigs->push_back(sigmoid(free_fruits_on_other_branches_seen));		

	// McNamara-Houston-input (reward rates):
	double average_reward_rate = cluster_jumps ? reward_rate_sum / cluster_jumps : 0.0;
	sigs->push_back(sigmoid(average_reward_rate)); // Average reward rate.
	static const double time_scaler = 0.05;
	double branchtime = (pcpt->current_time - last_branch_arrival_time) * time_scaler;
	double current_branch_reward_rate = branchtime ? cluster_laid_eggs / branchtime : 0.0;
	sigs->push_back(sigmoid(current_branch_reward_rate)); // Reward rate.

	// A static input for the neuronal network.
	// sigs->push_back(0.5);

	// A random input for the neuronal network.
	// sigs->push_back(World::randone());
}

/**
 * How to call one fly gene human readable? "Fly Gene".
 */
string_ptr Fly::get_gene_description(const unsigned int gene_no) {
	std::stringstream ss;
	ss << "Fly Gene " << gene_no;
	string_ptr ret = string_ptr(new std::string(ss.str()));	
	return ret;
}
