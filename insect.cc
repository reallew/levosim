/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim
 * Copyright (C)  2019 <Lew Palm>
 * 
 */

#include <iostream>
#include "insect.h"

double Insect::get_cluster_jumps() const {
	return cluster_jumps;
}

void Insect::set_cluster_jumps(double new_jumps) {
	cluster_jumps = new_jumps;
}

void Insect::set_position(const int branch_pos, const int fruit_pos) {
	insect_position.branch = branch_pos;
	insect_position.fruit = fruit_pos;
}

void Insect::set_fruit_pos(const int fruit_pos) {
	insect_position.fruit = fruit_pos;
}

/**
 * Returns the insects position (branch and fruit number) in the bush.
 */
bush_position Insect::get_position() const {
	return insect_position;
}

/**
 * Should return true if the insect is a parasitoid (Wasp) and false if it is a host
 * (Fly). Overwrite this method in a derived class. If you don't do it, it tells that 
 * your new class is a host.
 */
bool Insect::is_parasitoid() {
	return false;
}

/**
 * Sets the number of the insects branch, where it resides.
 */
void Insect::set_branch_pos(const int new_branch_pos) {
	BUG_CHECK(new_branch_pos<0, "Negative branch position.");
	insect_position.branch = new_branch_pos;
}

/**
 * Returns the last point in time when this insect landed on a fruit.
 * Returns -1.0 if the insect never did.
 */
turn_counter Insect::get_last_branch_arrival_time() const {
	return last_branch_arrival_time;
}

bool Insect::is_between_branches() {
	return branch_hopping;
}

/**
 * Returns the average time period this insect spent on branches.
 * Returns a stored value. If this value was never set or calculated, it calculates it
 * and returns it afterwards.
 */
turn_counter Insect::get_avg_branch_time() {
	if (avg_branch_time == -1.0) {
		double span = get_life_span();
		if (!span) {
			BUG_CHECK(travel_time_sum, "No life span but travelled?");
			avg_branch_time = 0.0;
			return avg_branch_time;
		}
		double branch_time = span - travel_time_sum;
		avg_branch_time = branch_time / (cluster_jumps + 1.0);
	}
	return avg_branch_time;
}

/**
 * Sets the average time this insect has been on a branch (cluster).
 * This method is useful for the multithreading environment, because there are agents
 * which store data but never lived.
 */
void Insect::set_avg_branch_time(turn_counter new_avg_b_t) {
	avg_branch_time = new_avg_b_t;
}

/**
 * Returns the sum of all time periods this insect has not been on a fruit.
 */
turn_counter Insect::get_travel_time_sum() {
	return travel_time_sum;
}

/**
 * Returns the average time span this insect used to be in the air in its life.
 * Travel time lengths can differ because of the flying distance.
 */
turn_counter Insect::get_average_travel_time() {
	BUG_CHECK(cluster_jumps && !travel_time_sum, "Cluster jumps without travel time.");
	return cluster_jumps ? travel_time_sum / (double)cluster_jumps : 0.0;
}

/** 
 * Collects some data for statistics. Used by both Fly and Wasp and should be called in 
 * the beginning of cognite method. 
 */
void Insect::cognition_start_statistics(const perception* pcpt) {
	next_gene = 0;

	// If the insect cognites the first time.
	if (birth_time==-1)
		birth_time = pcpt->current_time;
	
	// If the insect arrived just now on this branch.
	if (branch_hopping) {
		if (cluster_jumps)
			travel_time_sum += pcpt->current_time - last_branch_leaving_time;
		last_branch_arrival_time = pcpt->current_time;
		branch_hopping = false;
	}
}
