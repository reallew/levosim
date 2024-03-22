/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim3
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 */

#include "bushworld.h"
#include "world.h"
#include "bushworldhandler.h"
#include "worldhandler.h"
#include "fly.h"
#include "wasp.h"
#include "bushworld-database.h"

Bushworldhandler::Bushworldhandler() {
	wasp_quant_param_id = create_new_parameter(80, 0, 501, &wasp_dscr);
	fly_quant_param_id = create_new_parameter(80, 1, 501, &fly_dscr);
	branch_quant_param_id = create_new_parameter(200, 1, 401, &branch_dscr);
	fruits_per_branch_param_id = create_new_parameter(50, 1, 401, &fruit_dscr);
	mutation_rate_id = create_new_parameter(0.40/Genome::mutation_rate_scaler, 0.0, 
	                                        1.0/Genome::mutation_rate_scaler, &mutate_dscr, 
	                                        0.002/Genome::mutation_rate_scaler);
	mutation_intensity_id = create_new_parameter(0.10, 0.0, 0.501, &mut_inten_dscr, 0.001);
	parallel_worlds_id = create_new_parameter(4, 1, 201, &par_worlds_dscr);
	recombi_id = create_new_parameter(1, 0, 2, &recombi_dscr);
	hiddenlayers_id = create_new_parameter(1, 0, 9, &hiddenlayers_dscr);
	
	init_world();
}

simulation_database_ptr Bushworldhandler::create_database() {
	return bushworld_database_ptr(new BushworldDatabase());
}

/**
 * This method should be called after a change to a bushworld parameter happens and 
 * applies these change to the bushworld.
 */
void Bushworldhandler::parameter_changed_signal(world_parameter_container::iterator wp_i) {
	BUG_CHECK(wp_i == parameters.end(), "Non-existent parameter should change.");

	const unsigned int param_id = wp_i->second->param_type_id;

	if (param_id == wasp_quant_param_id)
		my_bushworld->set_offspring_quantity(&typeid(Wasp), wp_i->second->val);
	else if (param_id == fly_quant_param_id)
		my_bushworld->set_offspring_quantity(&typeid(Fly), wp_i->second->val);
	else if (param_id == branch_quant_param_id)
		my_bushworld->set_branch_quantity(wp_i->second->val);
	else if (param_id == fruits_per_branch_param_id)
		my_bushworld->set_fruits_per_branch(wp_i->second->val);
	else if (param_id == mutation_rate_id)
		my_bushworld->set_mutation_rate(wp_i->second->val*Genome::mutation_rate_scaler);
	else if (param_id == mutation_intensity_id)
		my_bushworld->set_mutation_intensity(wp_i->second->val);
	else if (param_id == parallel_worlds_id)
		my_bushworld->set_max_generation_reiterations(wp_i->second->val);
	else if (param_id == recombi_id)
		my_bushworld->set_recombination(wp_i->second->val);
	else if (param_id == hiddenlayers_id)
		Agent::set_nn_hidden_layers(wp_i->second->val);
	else
		std::cout << "Unknown parameter changed signal." << std::endl;

	wp_i->second->dirty = false;
}

/**
 * As the name implies...
 */
void Bushworldhandler::run_one_generation() {
	World::run_generation<Bushworld>(my_bushworld);
}

/**
 * Call this if you want a complete new world (with actual parameters), 
 * the old one is deleted.
 */
void Bushworldhandler::init_world() {
	const int max_age = 1200;
	my_bushworld = bushworld_ptr(new Bushworld(get_parameter_value(&branch_dscr), 
	                                           get_parameter_value(&fruit_dscr)));
	my_bushworld->add_new_agent(&typeid(Fly), get_parameter_value(&fly_dscr)); 	
	my_bushworld->add_new_agent(&typeid(Wasp), get_parameter_value(&wasp_dscr));
	my_bushworld->get_population()->clear(); // First Agents should only bring genomes.
	my_bushworld->set_offspring_quantity(&typeid(Fly), get_parameter_value(&fly_dscr));
	my_bushworld->set_offspring_quantity(&typeid(Wasp), get_parameter_value(&wasp_dscr));
	my_bushworld->set_mutation_intensity(get_parameter_value(&mut_inten_dscr));
	my_bushworld->set_mutation_rate(get_parameter_value(&mutate_dscr));
	my_bushworld->set_max_generation_reiterations(get_parameter_value(&par_worlds_dscr));
	
	my_bushworld->set_insect_death_chance(2.0 / ((double)max_age));
	my_bushworld->set_host_max_age(max_age);
	my_bushworld->set_parasitoid_beginning_time(max_age);
	my_bushworld->set_parasitoid_max_age(max_age*2);
	my_world = my_bushworld;
}

const std::string Bushworldhandler::wasp_dscr = "Wasp Quantity";
const std::string Bushworldhandler::fly_dscr = "Fly Quantity";
const std::string Bushworldhandler::branch_dscr = "Cluster Quantity";
const std::string Bushworldhandler::fruit_dscr = "Fruits per Cluster";
const std::string Bushworldhandler::mutate_dscr = "Mutation Rate Per Gene";
const std::string Bushworldhandler::mut_inten_dscr = "Mutation Intensity";
const std::string Bushworldhandler::par_worlds_dscr = "Parallel Worlds";
const std::string Bushworldhandler::recombi_dscr = "Recombination";
const std::string Bushworldhandler::hiddenlayers_dscr = "Neuronal Network Hidden Layer";
