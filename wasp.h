/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 */

#ifndef _WASP_H_
#define _WASP_H_

#include "insect.h"
#include "debug_macros.h"

#define GENOME_SIZE 4

class Wasp;
typedef std::shared_ptr<Wasp> wasp_ptr;


/**
 * A Wasp is an insect. This kind of wasp likes to put its eggs into the eggs of flys.
 */
class Wasp : public Insect {
	
	public:
		Wasp(genome_ptr mygen = genome_ptr(new Genome(typeid(Wasp), GENOME_SIZE)));
		action cognite(const perception* agents_personal_perception);
		bool is_parasitoid();

	protected:

	private:
		/** Quantity of found fly eggs in the current branch. */
		unsigned int fly_eggs_seen;
		/** Quantity of found empty fruits in the current branch. */
		unsigned int empty_fruits_seen;
		/** Quantity of found eggs of the own genome in the current branch. */
		unsigned int own_eggs_seen;
		/** Quantity of found eggs of a different wasp genome in the current branch. */
		unsigned int foreign_wasp_eggs_seen;
		/** All eggs this wasp laid. */
		unsigned int laid_eggs;
		/** Laid eggs in the current cluster. */
		unsigned int cluster_laid_eggs;
		/** All found fruits where no laying was possible. */
		unsigned int bad_fruits_seen;
};

#endif // _WASP_H_
