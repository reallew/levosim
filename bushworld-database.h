/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim3
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 */

#ifndef _BUSHWORLD_DATABASE_H_
#define _BUSHWORLD_DATABASE_H_

#include "simulation-database.h"
#include "debug_macros.h"

class BushworldDatabase;
typedef std::shared_ptr<BushworldDatabase> bushworld_database_ptr;

/** Types of datasets of a Bushworld. */
enum data_set_type {
	AVERAGE_GENOMES,
	BEST_GENOMES,
	GENOMES_BY_NUMBER,
	INTERPRETED_BEST_GENOME,
	DWELL_TIME,
	OVERALL_OFFSPRING,
	BEST_AGENT_FIT,
	AVG_JUMPS,
	BEST_AGENT_JUMPS,
	BEST_AGENT_DWELL_TIME
};


/**
 * BushworldDatabase is the database for worlds of type Bushworld.
 */
class BushworldDatabase: public SimulationDatabase {
	
public:
	BushworldDatabase();
	void collect(world_ptr my_world);

protected:

private:

};

#endif // _BUSHWORLD_DATABASE_H_
