/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim3
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 *
 */

#ifndef _SIMULATION_DATABASE_H_
#define _SIMULATION_DATABASE_H_

#include <vector>
#include <giomm.h>
#include "genome.h"
#include "debug_macros.h"
#include "world.h"


typedef std::vector<genome_ptr> genome_data_container;

struct data_set {
	std::string title;
	std::string agent_name;
	const std::type_info* agent_class_id;
	genome_data_container genomes;
	int type;
	double highest_value;
	unsigned int max_genome_size;
	std::vector<string_ptr> gene_names;
};

typedef std::shared_ptr<data_set> data_set_ptr;
typedef std::vector<data_set_ptr> data_set_container;

class SimulationDatabase;
typedef std::shared_ptr<SimulationDatabase> simulation_database_ptr;


/**
 * This class contains all logged data of one simulation run.
 * It logs one generation via the method collect and writes all data to a file
 * stream using write_db.
 */
class SimulationDatabase {

public:
	virtual void collect(world_ptr my_world) = 0;
	void print();
	void write_db(Glib::RefPtr<Gio::OutputStream> write_stream);
	void clear();
	data_set_container* data_sets();

protected:
	/** This is the data structure where all statistics are stored. */
	data_set_container db;

private:
};

#endif // _SIMULATION_DATABASE_H_
