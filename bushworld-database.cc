/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim3
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 */

#include "bushworld-database.h"
#include "fly.h"
#include "wasp.h"


BushworldDatabase::BushworldDatabase() {
	Fly tmp_fly = Fly();
	Wasp tmp_wasp = Wasp();
	unsigned int i;
	
	data_set_ptr avg_genomes = data_set_ptr(new data_set);
	avg_genomes->title = "Average Fly Genome";
	avg_genomes->type = AVERAGE_GENOMES;
	avg_genomes->agent_name = "Fly";
	avg_genomes->agent_class_id = &typeid(Fly);
	avg_genomes->highest_value = 1.0;
	avg_genomes->max_genome_size = 0;
	for (i=0; i<40; ++i)
		avg_genomes->gene_names.push_back(tmp_fly.get_gene_description(i));
	db.push_back(avg_genomes);

	data_set_ptr best_genomes = data_set_ptr(new data_set);
	best_genomes->title = "Best Fly Genome";
	best_genomes->type = BEST_GENOMES;
	best_genomes->agent_name = "Fly";
	best_genomes->agent_class_id = &typeid(Fly);
	best_genomes->highest_value = 1.0;
	best_genomes->max_genome_size = 0;
	for (i=0; i<40; ++i)
		best_genomes->gene_names.push_back(tmp_fly.get_gene_description(i));
	db.push_back(best_genomes);

	data_set_ptr avg_wsp_genomes = data_set_ptr(new data_set);
	avg_wsp_genomes->title = "Average Wasp Genome";
	avg_wsp_genomes->type = AVERAGE_GENOMES;
	avg_wsp_genomes->agent_name = "Wasp";
	avg_wsp_genomes->agent_class_id = &typeid(Wasp);
	avg_wsp_genomes->highest_value = 1.0;
	avg_wsp_genomes->max_genome_size = 0;
	db.push_back(avg_wsp_genomes);
	
	data_set_ptr best_wasp_genomes = data_set_ptr(new data_set);
	best_wasp_genomes->title = "Best Wasp Genome";
	best_wasp_genomes->type = BEST_GENOMES;
	best_wasp_genomes->agent_name = "Wasp";
	best_wasp_genomes->agent_class_id = &typeid(Wasp);
	best_wasp_genomes->highest_value = 1.0;
	best_wasp_genomes->max_genome_size = 0;
	db.push_back(best_wasp_genomes);
	
	data_set_ptr dwell_time = data_set_ptr(new data_set);
	dwell_time->title = "Average Cluster Time";
	dwell_time->type = DWELL_TIME;
	dwell_time->highest_value = 1.0;
	dwell_time->max_genome_size = 0;
	string_ptr wasp_act = string_ptr(new std::string("Wasp Average Cluster Time"));
	string_ptr fly_act = string_ptr(new std::string("Fly Average Cluster Time"));
	dwell_time->agent_name = "Fly and Wasp";
	dwell_time->gene_names.push_back(fly_act);
	dwell_time->gene_names.push_back(wasp_act);
	db.push_back(dwell_time);

	data_set_ptr avg_jumps = data_set_ptr(new data_set);
	avg_jumps->title = "Average Cluster Jumps";
	avg_jumps->type = AVG_JUMPS;
	avg_jumps->highest_value = 1.0;
	avg_jumps->max_genome_size = 0;
	string_ptr fly_aju = string_ptr(new std::string("Fly Average Jumps"));
	string_ptr wasp_aju = string_ptr(new std::string("Wasp Average Jumps"));
	avg_jumps->agent_name = "Fly and Wasp";
	avg_jumps->gene_names.push_back(fly_aju);
	avg_jumps->gene_names.push_back(wasp_aju);
	db.push_back(avg_jumps);

	data_set_ptr best_jumps = data_set_ptr(new data_set);
	best_jumps->title = "Best Insect Cluster Jumps";
	best_jumps->type = BEST_AGENT_JUMPS;
	best_jumps->highest_value = 1.0;
	best_jumps->max_genome_size = 0;
	string_ptr fly_bestju = string_ptr(new std::string("Best Fly Jumps"));
	string_ptr wasp_bestju = string_ptr(new std::string("Best Wasp Jumps"));
	best_jumps->agent_name = "Fly and Wasp";
	best_jumps->gene_names.push_back(fly_bestju);
	best_jumps->gene_names.push_back(wasp_bestju);
	db.push_back(best_jumps);

	data_set_ptr best_dwell_time = data_set_ptr(new data_set);
	best_dwell_time->title = "Best Insect Average Cluster Time";
	best_dwell_time->type = BEST_AGENT_DWELL_TIME;
	best_dwell_time->highest_value = 1.0;
	best_dwell_time->max_genome_size = 0;
	string_ptr fly_bestdwt = string_ptr(new std::string("Best Fly Average Cluster Time"));
	string_ptr wasp_bestdwt = string_ptr(new std::string("Best Wasp Average Cluster Time"));
	best_dwell_time->gene_names.push_back(fly_bestdwt);
	best_dwell_time->gene_names.push_back(wasp_bestdwt);
	best_dwell_time->agent_name = "Fly and Wasp";
	db.push_back(best_dwell_time);
	
	data_set_ptr overall_offspring = data_set_ptr(new data_set);
	overall_offspring->title = "Average Fitness";
	overall_offspring->type = OVERALL_OFFSPRING;
	overall_offspring->highest_value = 1.0;
	overall_offspring->max_genome_size = 0;
	string_ptr fly_af = string_ptr(new std::string("Fly Average Fitness"));
	string_ptr wasp_af = string_ptr(new std::string("Wasp Average Fitness"));
	overall_offspring->agent_name = "Fly and Wasp";
	overall_offspring->gene_names.push_back(fly_af);
	overall_offspring->gene_names.push_back(wasp_af);
	db.push_back(overall_offspring);

	data_set_ptr agent_fit = data_set_ptr(new data_set);
	agent_fit->title = "Best Fitness";
	agent_fit->type = BEST_AGENT_FIT;
	string_ptr fly_bf = string_ptr(new std::string("Fly Best Fitness"));
	string_ptr wasp_bf = string_ptr(new std::string("Wasp Best Fitness"));
	agent_fit->agent_name = "Fly and Wasp";
	agent_fit->gene_names.push_back(fly_bf);
	agent_fit->gene_names.push_back(wasp_bf);
	agent_fit->highest_value = 1.0;
	agent_fit->max_genome_size = 0;
	db.push_back(agent_fit);
}

/**
 * Takes interesting data of the actual generation from the given world my_world and puts 
 * them into the database.
 */
void BushworldDatabase::collect(world_ptr my_world) {
	for (auto const& data_set : db) {
		genome_ptr new_gd;
		switch (data_set->type) {
		case AVERAGE_GENOMES: {
			genome_ptr tmp_avg_g = my_world->average_genome(*data_set->agent_class_id);
			if (tmp_avg_g)
				new_gd = genome_ptr(new Genome(*tmp_avg_g));
		}
			break;
			
		case BEST_GENOMES: {
			genome_ptr tmp_bst_g = my_world->best_genome(*data_set->agent_class_id);
			if (tmp_bst_g)
				new_gd = genome_ptr(new Genome(*tmp_bst_g));
		}
			break;
			
		case GENOMES_BY_NUMBER:	{
		}
			break;
			
		case BEST_AGENT_FIT: {
			new_gd = genome_ptr(new Genome(*data_set->agent_class_id, 2, 0.0));
			genome_ptr fly_best_genome = my_world->best_genome(typeid(Fly));
			double best_fit_pa;
			if (!fly_best_genome || !fly_best_genome->get_offspring_quantity())
				best_fit_pa = 0.0;
			else
				best_fit_pa = fly_best_genome->get_fitness() /
					(double)fly_best_genome->get_offspring_quantity();
			new_gd->set_gene(0, best_fit_pa);
				
			genome_ptr wasp_best_genome = my_world->best_genome(typeid(Wasp));
			if (!wasp_best_genome || !wasp_best_genome->get_offspring_quantity())
				best_fit_pa = 0.0;
			else
				best_fit_pa = wasp_best_genome->get_fitness() /
					(double)wasp_best_genome->get_offspring_quantity();
			new_gd->set_gene(1, best_fit_pa);
				
		}
			break;
			
		case DWELL_TIME: {
			new_gd = genome_ptr(new Genome(*data_set->agent_class_id, 2, 0.0));
			bushworld_ptr my_buw = std::static_pointer_cast<Bushworld>(my_world);
			turn_counter time_per_branch = my_buw->get_average_branch_time(false);
			new_gd->set_gene(0, time_per_branch);
			time_per_branch = my_buw->get_average_branch_time(true);
			new_gd->set_gene(1, time_per_branch);
		}
			break;
			
		case AVG_JUMPS: {
			new_gd = genome_ptr(new Genome(*data_set->agent_class_id, 2, 0.0));
			bushworld_ptr my_buw = std::static_pointer_cast<Bushworld>(my_world);
			double cl_jumps = my_buw->get_average_cluster_jumps(&typeid(Fly));
			new_gd->set_gene(0, cl_jumps);
			cl_jumps = my_buw->get_average_cluster_jumps(&typeid(Wasp));
			new_gd->set_gene(1, cl_jumps);
		}
			break;
			
		case OVERALL_OFFSPRING: {
			new_gd = genome_ptr(new Genome(*data_set->agent_class_id, 2, 0.1));
			new_gd->set_gene(0, my_world->get_average_fitness(&typeid(Fly)));
			new_gd->set_gene(1, my_world->get_average_fitness(&typeid(Wasp)));
		}
			break;
			
		case BEST_AGENT_JUMPS: {
			new_gd = genome_ptr(new Genome(*data_set->agent_class_id, 2, 0.1));
			bushworld_ptr my_buw = std::static_pointer_cast<Bushworld>(my_world);
			new_gd->set_gene(0, my_buw->get_best_insect_jumps(&typeid(Fly)));
			new_gd->set_gene(1, my_buw->get_best_insect_jumps(&typeid(Wasp)));
		}
			break;
			
		case BEST_AGENT_DWELL_TIME: {
			new_gd = genome_ptr(new Genome(*data_set->agent_class_id, 2, 0.1));
			bushworld_ptr my_buw = std::static_pointer_cast<Bushworld>(my_world);
			new_gd->set_gene(0, my_buw->get_best_insect_avg_branch_time(&typeid(Fly)));
			new_gd->set_gene(1, my_buw->get_best_insect_avg_branch_time(&typeid(Wasp)));
		}
			break;
			
		case INTERPRETED_BEST_GENOME: {
			genome_ptr best_g = my_world->best_genome(*data_set->agent_class_id);
			if (best_g) {
				new_gd = genome_ptr(new Genome(*best_g->get_type_id(), 1));
				double divider = best_g->get_gene(0) * 10;
				if (divider <= 0.0)
					divider = 0.0001;
				new_gd->set_gene(0, best_g->get_gene(2) / divider);
				new_gd->set_agents_name(best_g->get_agents_name());
			}
		}
			break;
		}

		if (new_gd) {
			for (unsigned i=0; i<new_gd->size(); ++i) {
				double gene_val = new_gd->get_gene(i);
				if (gene_val > data_set->highest_value)
					data_set->highest_value = gene_val;
			}
			if (new_gd->size() > data_set->max_genome_size)
				data_set->max_genome_size = new_gd->size();
			data_set->genomes.push_back(new_gd);
		}	   
	}
}
