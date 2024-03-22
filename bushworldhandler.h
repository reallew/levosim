/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim3
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 */

#ifndef _BUSHWORLDHANDLER_H_
#define _BUSHWORLDHANDLER_H_

#include "worldhandler.h"
#include "bushworld.h"
#include "debug_macros.h"

/**
 * A Bushworldhandler is the link (interface) between GUI and
 * simulation for a Bushworld. 
 */
class Bushworldhandler: public Worldhandler {
public:
	Bushworldhandler();
	void init_world();
	void run_one_generation();
	simulation_database_ptr create_database();
		
protected:
	void parameter_changed_signal(world_parameter_container::iterator param);

private:
	bushworld_ptr my_bushworld;
	unsigned int wasp_quant_param_id;
	unsigned int fly_quant_param_id;
	unsigned int branch_quant_param_id;
	unsigned int fruits_per_branch_param_id;
	unsigned int mutation_rate_id;
	unsigned int mutation_intensity_id;
	unsigned int parallel_worlds_id;
	unsigned int recombi_id;
	unsigned int hiddenlayers_id;
	static const std::string wasp_dscr;
	static const std::string fly_dscr;
	static const std::string branch_dscr;
	static const std::string fruit_dscr;
	static const std::string mutate_dscr;
	static const std::string mut_inten_dscr;
	static const std::string par_worlds_dscr;
	static const std::string recombi_dscr;
	static const std::string hiddenlayers_dscr;
};

#endif // _BUSHWORLDHANDLER_H_
