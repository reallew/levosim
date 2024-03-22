/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim3
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 */

#ifndef _WORLDHANDLER_H_
#define _WORLDHANDLER_H_

#include <map>
#include "world.h"
#include "simulation-database.h"
#include "debug_macros.h"

class Worldhandler;
typedef std::shared_ptr<Worldhandler> worldhandler_ptr;

/**
 * Contains informations about _one_ parameter of a world.
 * This is mainly for communication with a GUI.
 */
struct world_parameter {
	double min_val;
	double max_val;
	double val;
	double steps;
	unsigned int param_type_id;
	bool dirty; // Means non-applied changes are waiting until generation finishes.
};
typedef std::shared_ptr<world_parameter> world_parameter_ptr;
typedef std::pair<std::string, world_parameter_ptr> parameter_key_pair;
typedef std::map<std::string, world_parameter_ptr> world_parameter_container;

/**
 * The class Worldhandler defines an interface between Mainwindow and a World.
 * With a derived class of Worldhandler, you can use any kind of World with Mainwindow
 * without changing the GUI code.
 */
class Worldhandler {
	public:
		virtual void init_world() = 0;
		world_parameter_container* get_parameters();
		world_ptr get_world();
		bool extincted();
		virtual void run_one_generation() = 0;
		double get_parameter_value(const std::string* param_name);
		void set_parameter_value(std::string* param_name, double new_val);
		void apply_changes();
		unsigned int create_new_parameter(double val, double min_val, double max_val, 
		                  				  const std::string* param_name, double stepping=1);
		unsigned int get_generation();
		virtual simulation_database_ptr create_database() = 0;
		
	protected:
		world_ptr my_world;
		world_parameter_container parameters;
		virtual void parameter_changed_signal(world_parameter_container::iterator param);
		
	private:
		static unsigned int highest_param_type_id;

};

#endif // _WORLDHANDLER_H_
