/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim3
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 *
 */

#include <giomm.h>
#include "simulation-database.h"
#include "genome.h"


/**
 * Wipes out all saved genomes.
 */
void SimulationDatabase::clear() {
	data_set_container::iterator data_set_i = db.begin();
	while (data_set_i != db.end()) {
		(*data_set_i)->genomes.clear();
		(*data_set_i)->highest_value = 1.0;
		(*data_set_i)->max_genome_size = 0;
		++data_set_i;
	}
}

/**
 * Returns a pointer to the whole database data.
 */
data_set_container* SimulationDatabase::data_sets()  {
	return &db;
}

/**
 * Writes all data to the given stream in csv formatting.
 */
void SimulationDatabase::write_db(Glib::RefPtr<Gio::OutputStream> write_stream) {
	if (db.size()<1) {
		write_stream->write("empty database\n");
		return;
	}
	
	// Header
	/*
	write_stream->write("Generation, Genome Name, Fitness");
	unsigned int i;
	for (i=0; i<(*timeline.begin())->best_genome->size(); ++i) {
		write_stream->write(", Gene ");
		write_stream->write(boost::lexical_cast<Glib::ustring> (i));
	}
	write_stream->write("\n");
	*/

	// Data
	data_set_container::iterator data_set_i = db.begin();
	while (data_set_i != db.end()) {
		genome_data_container::iterator gen_data_i = (*data_set_i)->genomes.begin();
		unsigned int generation_no = 1;
		while (gen_data_i != (*data_set_i)->genomes.end()) {
			std::stringstream ss;
			ss << generation_no;
			write_stream->write(ss.str());					// No. of generation.
			write_stream->write(", ");
			write_stream->write((*data_set_i)->title);		// Title of dataset.
			write_stream->write(", ");
			write_stream->write((*data_set_i)->agent_name); // Agent type.
			write_stream->write(", ");
			(*gen_data_i)->write(write_stream);				// Data itself.
			write_stream->write("\n");
			++gen_data_i;
			++generation_no;
		}
		++data_set_i;
	}
}
