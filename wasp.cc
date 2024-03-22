/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 */

#include "wasp.h"

Wasp::Wasp(genome_ptr mygen) : 
	Insect(mygen),
	fly_eggs_seen(0),
	empty_fruits_seen(0),
	own_eggs_seen(0),
	foreign_wasp_eggs_seen(0),
	laid_eggs(0),
	cluster_laid_eggs(0),
	bad_fruits_seen(0)
{
	agent_type = "Wasp";
}

/**
 * The wasp gets his perception and has to decide, means: has to return an object of
 * type action. The main decision is about leaving or not leaving the branch. This is
 * done by an artificial neuronal network.
 */
action Wasp::cognite(const perception* pcpt) {
	cognition_start_statistics(pcpt);
	action ret;
	
	ret.intensity = 1.0;
	if (pcpt->fruit_free) {
		++empty_fruits_seen;
	} else {
		own_eggs_seen += pcpt->own_eggs_in_fruit;
		foreign_wasp_eggs_seen += pcpt->wasp_eggs_in_fruit - pcpt->own_eggs_in_fruit;
		fly_eggs_seen += pcpt->fly_eggs_in_fruit;
	}
	
	if ((pcpt->fly_eggs_in_fruit) && (!pcpt->wasp_eggs_in_fruit)) {
		ret.type = LAY_EGG;
		++laid_eggs;
		++cluster_laid_eggs;
	} else {
		if (!pcpt->own_eggs_in_fruit)
			++bad_fruits_seen;
		// The perception vector.
		nn_signals_ptr sigs = nn_signals_ptr(new nn_signals());

		// Collected personal statistics as perceptions:
		sigs->push_back(sigmoid(foreign_wasp_eggs_seen));   // on current branch
		sigs->push_back(sigmoid(empty_fruits_seen));		// on current branch
		sigs->push_back(sigmoid(laid_eggs));				// in the whole bush
		sigs->push_back(sigmoid(cluster_laid_eggs));		// on current branch
		sigs->push_back(sigmoid(fly_eggs_seen));			// on current branch
		sigs->push_back(sigmoid(cluster_jumps));			// in the whole bush
		sigs->push_back(sigmoid(bad_fruits_seen));			// in the whole bush
		sigs->push_back(sigmoid((pcpt->current_time - birth_time) / (max_age - birth_time))); // Lifetime normalized

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
		
		if (neuronal_network(sigs)) {
			double branchtime = (pcpt->current_time - last_branch_arrival_time) * time_scaler;
			if (branchtime)
				reward_rate_sum += cluster_laid_eggs / branchtime;
			if (get_genome_ptr()->get_gene(next_gene++) < World::randone())
				ret.type = GO_TO_BRANCH_WEST;
			else
				ret.type = GO_TO_BRANCH_EAST;
			branch_hopping = true;
			ret.intensity = (int) (1.0 + get_genome_ptr()->get_gene(next_gene++) * 3.0);
			foreign_wasp_eggs_seen = 0; // Means wasp eggs on current branch.
			empty_fruits_seen = 0;
			fly_eggs_seen = 0;
			last_branch_leaving_time = pcpt->current_time;
			++cluster_jumps;
			cluster_laid_eggs = 0;
		} else {
			ret.intensity = 1.0;
			ret.type = GO_TO_FRUIT;
		}
	}
	return ret;
}

/**
 * Returns true, because this wasp is a parasitoid.
 */
bool Wasp::is_parasitoid() {
	return true;
}
