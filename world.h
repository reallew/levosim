/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 */

#ifndef _WORLD_H_
#define _WORLD_H_

#include <list>
#include <map>
#include <random>
#include "debug_macros.h"
#include "genome.h"


/** Turns on population dynamics if it is used via  
    set_offspring_quantity(DYNAMIC_OFFSPRING_QUANTITY) */
#define DYNAMIC_OFFSPRING_QUANTITY -1

/** There must be a definition of struct perception in every child of world. */
struct perception;
/** There must be a definition of struct action in every child of world. */
struct action;

class World;
/** Shared pointer to a World. */
typedef std::shared_ptr<World> world_ptr;

class Agent;
/** Shared pointer to an Agent. */
typedef std::shared_ptr<Agent> agent_ptr;

/** This type of variable is used to store turn-numbers. */
typedef double turn_counter;

typedef std::list<genome_ptr> genome_container;
typedef std::shared_ptr<genome_container> genome_container_ptr;

typedef std::list<agent_ptr> agent_container;
typedef std::shared_ptr<agent_container> agent_container_ptr;

typedef std::shared_ptr<std::string> string_ptr;


/**
 * Parameters of one class of agents. For every type (class) of agents which occurs one 
 * time or more often in the world there will be exactly one agent_type_parameter object.
 * Its main use is to store the quantity of offspring for this class, the rest is for 
 * statistics and efficent calculation (the list of genomes of this type).
 */
struct agent_type_parameter {
	/** How many offspring individuals must be created next generation? This is only used
	    if dynamic_offspring is false, otherwise it means the quantity of offspring of the
	    last created generation. */
	unsigned int offspring_quantity;
	/** If this is true, for every point of fitness of one genome there is one orphane
	    created in the next generation. */
	bool dynamic_offspring;
	/** Average gene values of the last generation. Only for statistics. */
	genome_ptr last_average_genome;
	/** Pointer the the fittest genome. */
	agent_ptr best_agent;
	/** Fitness values of the best genome. */
	double best_genomes_fitness;
	/** Container of pointers to all genomes of this type in genepool. This is calculated
	    (updated) by World::calculate_offspring. */
	genome_container_ptr genomes;
};
typedef std::pair<const std::type_info*, agent_type_parameter> info_agent_pair;
typedef std::map<const std::type_info*, agent_type_parameter> agent_type_parameter_container;

/**
 * The universe for the simulated agents.
 * This class is abstract and offers functions every world needs. It keeps the data
 * structures for agents and genomes.
 * A working implementation must be a child of World.
 */
class World  {
public:
	World();
	
	void randomize_genes();
	static void stochastic_universal_sampling(genome_container_ptr g_list, 
											  unsigned int g_quant);
	void set_offspring_quantity(const std::type_info* agent_type, 
								const unsigned int new_quant);
	void set_dynamic_offspring_quantity(const std::type_info* agent_type, bool dynam);
	unsigned int get_offspring_quantity(const std::type_info* agent_type);
	void set_standard_offspring_quantity(const unsigned int new_standard_quant);
	unsigned int get_different_agent_type_number() const;
	int get_population_size() const;
	int get_population_size(const std::type_info* agent_type);
	agent_container* get_population();
	genome_container_ptr get_genepool();
	void freeze_agents(turn_counter end_time, const std::type_info* agent_type=NULL);
	int get_generation() const;
	bool run();
	void kill_agent(agent_ptr cooper);
	void set_max_turns(const turn_counter new_max_turns);
	double get_best_fitness() const;
	static double get_collective_fitness(genome_container_ptr g_list);
	static void set_genome_offspring(genome_container_ptr g_list, unsigned int new_offspring);
	genome_ptr average_genome(const std::type_info& average_agents_type);
	genome_ptr best_genome(const std::type_info& best_agents_type);

	/** Returns a random value between 0 and 1. */
	inline static double randone() {
		static std::default_random_engine _engine;
		std::uniform_real_distribution<double> distribution(0.0, 1.0);
		return distribution(_engine);
	}
	void set_mutation_intensity(double new_inten);
	void set_mutation_rate(double new_rate);
	void set_mutation_intensity(const std::type_info* agent_type, double new_inten);
	void set_mutation_rate(const std::type_info* agent_type, double new_rate);
	void set_max_age(turn_counter new_max_age);
	genome_container_ptr get_genomes_by_type(const std::type_info& agents_t_id);
	void add_new_agent(genome_ptr agent_genome=genome_ptr(), unsigned int quantity=1);
	void add_new_agent(const std::type_info* agents_t_id, unsigned int quantity=1);
	double get_average_fitness(const std::type_info* agents_type);
	double get_best_per_agent_fitness(const std::type_info& agents_type);
	void set_best_per_agent_fitness(const std::type_info& agents_type, double new_fit);
	void kill_all_agents();
	void set_max_generation_reiterations(const unsigned int max_reitr);
	genome_container_ptr genepool_copy();
	void set_time(turn_counter new_time);
	void create_offspring();
	void calculate_offspring();
	unsigned int get_max_reiterations() const;
	void inc_current_generation(int new_gens=1);
	void set_all_fitnesses(double new_fit);
	agent_ptr get_best_agent(const std::type_info* agent_type);
	//	void set_best_agent(agent_ptr new_best_agent);
	void recombine_all_genomes();
	void set_recombination(bool new_recomb);
	bool does_recombination();

	virtual void reset_statistics();
	virtual void recreate_world();
	virtual void collect_multithread_statistics(world_ptr tmp_world) = 0;
	virtual void finish_multithread_statistics(unsigned int world_runs) = 0;

	/**
	 * Calculates one or more generations for the given world.
	 * Generations can be calculated in parallel. This means that every generation 
	 * is computed more than one time to get rid of stochastical effect (noise),
	 * because fitness average values are taken. If you compile this with OpenMP, 
	 * all processor cores are used for that.
	 */
	template<class World_type> static void run_generation(std::shared_ptr<World_type> rel_world,
														  unsigned int generations=1) {
		while (generations > 0) {
			rel_world->calculate_offspring();
			rel_world->delete_unused_genomes(); // Delete all genomes without offspring.
			if (rel_world->does_recombination())
				rel_world->recombine_all_genomes();
			rel_world->mutate_genomes(); // Mutate some genomes with offspring.
			rel_world->set_all_fitnesses(0.0);
			rel_world->reset_statistics();
			rel_world->delete_agent_fitnesses_statistics();
				
			unsigned int max_reiterations = rel_world->get_max_reiterations();
				
#pragma omp parallel for		
			for (unsigned para_generation=0; para_generation<max_reiterations; ++para_generation) {
				auto tmp_world = std::shared_ptr<World_type>(new World_type(*rel_world));
				tmp_world->recreate_world();
				tmp_world->create_offspring();
				tmp_world->reset_statistics();
				tmp_world->set_time(0.0);
				while (tmp_world->get_population_size() && tmp_world->run());
				tmp_world->kill_all_agents();
				tmp_world->calculate_fitness();
				genome_container::iterator tmp_gen_i = tmp_world->get_genepool()->begin();
				genome_container::iterator rel_gen_i = rel_world->get_genepool()->begin();
				BUG_CHECK(tmp_world->get_genepool()->size() != rel_world->get_genepool()->size(),
						  "Different genepool sizes.");
				while (rel_gen_i != rel_world->get_genepool()->end()) {
#pragma omp critical (genome_merging) 
					(*rel_gen_i)->merge(*tmp_gen_i);
					BUG_CHECK((*tmp_gen_i)->size() > (*rel_gen_i)->size(), 
							  "Original genome too small.");
#pragma omp critical (average_fit_change) 
					(*rel_gen_i)->increase_fitness((*tmp_gen_i)->get_fitness());
					++tmp_gen_i;
					++rel_gen_i;
				}
#pragma omp critical (collect_statistics)
				rel_world->collect_multithread_statistics(tmp_world);
			}

			rel_world->finish_multithread_statistics(max_reiterations);
			for (auto const& rel_gen: *rel_world->get_genepool())
				rel_gen->set_fitness(rel_gen->get_fitness() / max_reiterations);
			rel_world->inc_current_generation();
			--generations;
		}
	}

protected:
	unsigned int offspring_from_fitness(genome_container_ptr gcp);
	virtual void agent_death_statistics(agent_ptr dead_agent);
	void delete_agent_fitnesses_statistics();
	void inc_agent_fitness_statistic(agent_ptr cooper, double add_fit = 1.0);
		
	/** make_perception shall create the chunk of data an agent percieves 
	    every round. Must be implemented by every world. */
	virtual void make_perception(agent_ptr cooper, perception* cooper_sees) = 0;
	virtual void execute_action(agent_ptr cooper, action coopers_action) = 0;
	virtual double calculate_fitness() = 0;
	virtual agent_ptr create_agent(genome_ptr agent_genome) = 0;
		
	/** Container where all genomes are stored. */
	genome_container_ptr genepool;
	/** Container where all agents are stored. */
	agent_container population;
	/** Fitness of best genome. NOT of best agent. */
	double best_fitness;
	/** Current point in time. This is a real number value. */
	turn_counter turn;
	/** How many times in parallel is one generation computed. */
	unsigned int max_redundant_generation_reiterations;
	/** Container with one information entry for every kind of agent which appears
	    in this world. Stores statistical informations and the offspring quantity. */
	agent_type_parameter_container agent_type_infos;
	/** The standard dummy parameter package for kinds of agents that appear here at
	    their first time. */
	agent_type_parameter standard_agent_type_parameter;
		
		
private:
	void create_agents_from_genomes(genome_container_ptr genome_list);
	void delete_unused_genomes();
	bool create_agent_type(const std::type_info* agent_type);
	void mutate_genomes();
	genome_ptr get_fortune_wheel_genome(agent_type_parameter* atp);
	genome_ptr recombine(genome_ptr parent1, genome_ptr parent2);

	/** Number of current living generation. */
	int current_generation;
	/** After this point in time everything stops and all agents die. */
	turn_counter max_turns_per_generation;
	/** This flag turns genetic recombination on or off. */
	bool recombination;
		
};

#endif // _WORLD_H_
