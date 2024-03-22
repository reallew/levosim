/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim3
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 */

#include "worldhandler.h"

world_ptr Worldhandler::get_world() {
	return my_world;
}

/**
 * Returns a pointer the list of world parameters.
 */
world_parameter_container* Worldhandler::get_parameters() {
	return &parameters;
}

/**
 * Returns true if there is nobody alive.
 */
bool Worldhandler::extincted() {
	return my_world->get_population_size() == 0;
}

/**
 * Returns the number of the worlds actual generation.
 */
unsigned int Worldhandler::get_generation() {
	return my_world->get_generation();
}

/**
 * Gets the value of the parameter which is described by the given string. The
 * parameter must exist.
 */
double Worldhandler::get_parameter_value(const std::string* param_name) {
	world_parameter_container::iterator wp_i = parameters.find(*param_name);
	BUG_CHECK(wp_i == parameters.end(), "Can not find parameter name.");
	return  wp_i->second->val;
}

/**
 * Changes the value of the parameter which is described by the given string. The
 * parameter must exist.
 */
void Worldhandler::set_parameter_value(std::string* param_name, double new_val) {
	world_parameter_container::iterator wp_i = parameters.find(*param_name);
	BUG_CHECK(wp_i == parameters.end(), "Can not find parameter name.");
	
	if (new_val > wp_i->second->max_val) 
		new_val = wp_i->second->max_val;
	if (new_val < wp_i->second->min_val) 
		new_val = wp_i->second->min_val;
	
	double old_val = wp_i->second->val;
	wp_i->second->val = new_val;
	wp_i->second->dirty = old_val != new_val;
}

/**
 * Applies all waiting parameter changes to the world.
 * This should be called between two generation runs.
 */
void Worldhandler::apply_changes() {
	world_parameter_container::iterator wp_i = parameters.begin();
	while (wp_i != parameters.end()) {
		if (wp_i->second->dirty)
			parameter_changed_signal(wp_i);
		++wp_i;
	}
}

/**
 * Creates a new parameter.
 * If there exists a parameter of the given type it will be overwritten. Returns the 
 * unique id of the new parameter.
 */
unsigned int Worldhandler::create_new_parameter(double val, double min_val, double max_val, 
                                    			const std::string* param_name, double stepping) {
	world_parameter_ptr new_param = world_parameter_ptr(new world_parameter);
	if (val > max_val)
		val = max_val;
	if (val < min_val)
		val = min_val;
	new_param->val = val;
	new_param->steps = stepping;											
	new_param->min_val = min_val;
	new_param->max_val = max_val;
	new_param->dirty = false; // or should this be true?
	new_param->param_type_id = highest_param_type_id++;
	parameters[*param_name] = new_param;
	return new_param->param_type_id;
}

/**
 * In Worldhandler this does nothing. It should be overwritten by a child class. This 
 * method is called after a change of one world parameter and should apply the change to 
 * the world.
 */
void Worldhandler::parameter_changed_signal(world_parameter_container::iterator param) {
}

unsigned int Worldhandler::highest_param_type_id = 0;
