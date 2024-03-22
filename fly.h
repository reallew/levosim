/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 */

#ifndef _FLY_H_
#define _FLY_H_

#include "insect.h"
#include "debug_macros.h"

#define GENOME_SIZE 4

class Fly;
typedef std::shared_ptr<Fly> fly_ptr;

/**
 * The Fly is an Agent and an Insect. It was made to live in the Bushworld.
 */
class Fly: public Insect 
{
	public:
		Fly(genome_ptr mygen = genome_ptr(new Genome(typeid(Fly), GENOME_SIZE)));

		action cognite(const perception* agents_personal_perception);
		virtual string_ptr get_gene_description(const unsigned int gene_no);

	protected:

	private:
		/** Unused (egg-free) fruits seen on the current branch by this fly. This value
		 means also 'laid eggs in this branch'. */
		unsigned int fruits_on_current_branch_seen_free;
		/** Free fruits on other branches, actually unused variable. */
		unsigned int free_fruits_on_other_branches_seen;
		/** Egg from other flys seen on the current branch by this fly. */
		unsigned int foreign_fly_eggs_on_current_branch_seen;
		/** Own eggs found on the current branch by this fly. */
		unsigned int own_eggs_seen;
		/** Own eggs found until now in the whole world by this fly. */
		unsigned int all_own_eggs_seen;
		/** All eggs this fly laid. */
		unsigned int laid_eggs;
		/** Laid eggs in the current cluster. */
		unsigned int cluster_laid_eggs;
		/** All found fruits where no laying was possible since birth. */
		unsigned int bad_fruits_seen;
		
		void fill_input_signals(nn_signals_ptr sigs, const perception* pcpt);
		void leave_branch(action* ret);
};

#endif // _FLY_H_
