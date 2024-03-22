/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 */

#include <list>
#include <limits>
#include <cstdlib>
#include <sys/time.h>
#include <unistd.h>

#include "world.h"
#include "genome.h"
#include "agent.h"
#include "insect.h"


/**
 * Contructs a new World without agents.
 */
World::World() : 
	best_fitness(0.0), 
	turn(0.0), 
	max_redundant_generation_reiterations(1),
	current_generation(0),
	max_turns_per_generation(std::numeric_limits<turn_counter>::max()),
	recombination(true)
{
	genepool = genome_container_ptr(new genome_container);
	
	standard_agent_type_parameter.offspring_quantity = 50;
	standard_agent_type_parameter.dynamic_offspring = false;
	standard_agent_type_parameter.best_agent = agent_ptr();
	standard_agent_type_parameter.best_genomes_fitness = 0.0;


	// std::default_random_engine _engine;
	// Getting a really unpredictable random seed:
	struct timeval tv;
	gettimeofday(&tv, 0);
	srand(tv.tv_usec * getpid());
}

/**
 * Does nothing here.
 * Should be overwritten by a childclass. This method is called one time before every
 * generation computation. You can use it to set your worlds special statistics to zero
 * or whatever.
 */
void World::reset_statistics() {
}

/**
 * Deletes all per-agent-fitness-statistics. These statistics are used only for your 
 * information and have no influence on the events in the world.
 */
void World::delete_agent_fitnesses_statistics() {
	for (auto& ainfo: agent_type_infos) {
		//ainfo_i->second.agent_fitnesses.clear();
		//ainfo_i->second.best_agents_fitness = 0.0;
		ainfo.second.best_agent = agent_ptr();
		ainfo.second.best_genomes_fitness = 0.0;
	}
}

/**
 * Increases the fitness value for one agent.
 * This is only for statistics and has no influence on events in the world or offspring.
 * Offspring fitness is calculated by another mechanism; therefore fitness is stored by
 * genome (genotype) and not by agent.
 */
void World::inc_agent_fitness_statistic(agent_ptr cooper, double add_fit) {
	BUG_CHECK(!cooper, "Agent pointer to nowhere.");
	agent_type_parameter_container::iterator ainfo_i = agent_type_infos.find(&typeid(*cooper));
	BUG_CHECK(ainfo_i == agent_type_infos.end(), "Agent type info not found.");
	cooper->inc_personal_fitness(add_fit);
	if (!ainfo_i->second.best_agent)
		ainfo_i->second.best_agent = cooper;
	else if (cooper->get_personal_fitness() > ainfo_i->second.best_agent->get_personal_fitness())
		ainfo_i->second.best_agent = cooper;
}

/**
 * Sets how many parallel worlds are computed - how many parallel generations are running.
 * You need more worlds to get rid of stochastics effects, which means you can bear down
 * the bad effects of noise in your world.
 */
void World::set_max_generation_reiterations(const unsigned int max_reitr) {
	BUG_CHECK(max_reitr < 1, "I can not do zero generation reiterations.");
	BUG_CHECK(max_reitr > 1000000, "More than one million reiterations seems too much.");
	
	max_redundant_generation_reiterations = max_reitr;
}

/**
 * Returns the fittest agent of given type agent_type.
 */
agent_ptr World::get_best_agent(const std::type_info* agents_type) {
	agent_type_parameter_container::iterator ainfo_i = agent_type_infos.find(agents_type);
	BUG_CHECK(ainfo_i == agent_type_infos.end(), "Agent type info not found.");
	if (!ainfo_i->second.best_agent) // TODO: Warum werden hier keine Agenten gefunden?
		ainfo_i->second.best_agent = create_agent(genome_ptr(new Genome(*agents_type)));
	BUG_CHECK(!ainfo_i->second.best_agent, "Best agent does not exist.");
	return ainfo_i->second.best_agent;
}

/*
  void World::set_best_agent(agent_ptr new_best_agent) {
  agent_type_parameter_container::iterator ainfo_i = 
  agent_type_infos.find(&typeid(new_best_agent));
  }
*/

/**
 * Sets the standard value for the number of agents created in every generation.
 * This value is only used for (in future) new types off agents. Old agent types stay
 * with their offspring quantity and are not touched by this!
 */
void World::set_standard_offspring_quantity(const unsigned int new_standard_quant) {
	standard_agent_type_parameter.offspring_quantity = new_standard_quant;
}

/**
 * Creates a new agent_type parameter set in agent_type_infos with standard values if it 
 * is non-existent.
 * When new created this method returns true, when there was such an agent_type before it
 * returns false.
 */
bool World::create_agent_type(const std::type_info* agent_type) {
	if (agent_type_infos.find(agent_type) == agent_type_infos.end()) {
		agent_type_infos.insert(info_agent_pair(agent_type, standard_agent_type_parameter));
		return true;
	}
	return false;
}

/**
 * Sets the fitnesses of all genomes in the World to the given value.
 */
void World::set_all_fitnesses(double new_fit) {
	for (auto const& genome: *genepool)
		genome->set_fitness(new_fit);
}

/**
 * Returns the average fitness value per agent for the given agents_type.
 */
double World::get_average_fitness(const std::type_info* agents_type) {
	double fitness_amount = 0.0;
	unsigned int agent_amount = 0;
	
	for (auto const& genome: *genepool)
		if (genome->agents_type_equals(*agents_type)) {
			fitness_amount += genome->get_fitness();
			agent_amount += genome->get_offspring_quantity();
		}

	return agent_amount ? fitness_amount /  (double) agent_amount : 0.0;
}

/**
 * Returns the fitness of the best agent of the given type.
 * This fitness is not calculated here. This method returns only a stored value. The value
 * can be set directly by calling World::set_best_per_agent_fitness or will be calculated 
 * automatically if you call World::inc_agent_fitness_statistic for the agents.
 */
double World::get_best_per_agent_fitness(const std::type_info& agents_type) {
	agent_type_parameter_container::iterator ainfo_i = agent_type_infos.find(&agents_type);
	BUG_CHECK(ainfo_i == agent_type_infos.end(), "Agent type info not found.");
	if (!ainfo_i->second.best_agent)
		return 0.0;
	else
		return ainfo_i->second.best_agent->get_personal_fitness();
}

/**
 * Sets the best fitness for the given agent type directly.
 * Don't use this if you don't really know what you do. Normally the best-per-agent-fitness
 * is calculated automatically if you use World::inc_agent_fitness_statistic for all
 * agents. 
 */
void World::set_best_per_agent_fitness(const std::type_info& agents_type, double new_fit) {
	agent_type_parameter_container::iterator ainfo_i = agent_type_infos.find(&agents_type);
	BUG_CHECK(ainfo_i == agent_type_infos.end(), "Agent type info not found.");
	agent_ptr tmp_agent = get_best_agent(&agents_type);
	BUG_CHECK(!tmp_agent, "Got no best agent.");
	tmp_agent->set_personal_fitness(new_fit);
	ainfo_i->second.best_agent = tmp_agent;
}

/**
 * Sets the amount of offspring agents created every generation for the given agent type.
 */
void World::set_offspring_quantity(const std::type_info* agent_type, 
                                   const unsigned int new_quant) {
	create_agent_type(agent_type);
	agent_type_infos[agent_type].offspring_quantity = new_quant;
}

/**
 * Turns dynamic offspring for the given type on or off (using bool dynam as switch).
 * Dynamic offspring on means that any information about a offspring quantity (set using
 * World::set_offspring_quantity) is ignored, but not deleted.
 */
void World::set_dynamic_offspring_quantity(const std::type_info* agent_type, bool dynam) {
	create_agent_type(agent_type);
	agent_type_infos[agent_type].dynamic_offspring = dynam;
}

/**
 * Set the mutation rate for the given agent type. 
 */
void World::set_mutation_rate(const std::type_info* agent_type, double new_rate) {
	for (auto const& genome: *genepool)
		if (genome->agents_type_equals(*agent_type))
			genome->set_mutation_rate(new_rate);
}

/**
 * Set the mutation intensity for the given agent type. 
 */
void World::set_mutation_intensity(const std::type_info* agent_type, double new_intense) {
	for (auto const& genome: *genepool)
		if (genome->agents_type_equals(*agent_type))
			genome->set_mutation_intensity(new_intense);
}

/**
 * Set the mutation intensity for all agents.
 */
void World::set_mutation_intensity(double new_inten) {
	std::cout << "Set mut int for all folks to " << new_inten << std::endl;
	for (auto const& genome: *genepool)
		genome->set_mutation_intensity(new_inten);
}	

/**
 * Set the mutation rate for all agents.
 */
void World::set_mutation_rate(double new_rate) {
	for (auto const& genome: *genepool)
		genome->set_mutation_rate(new_rate);
}

/**
 * Does nothing.
 * Should be overwritten by a childclass if necessary. This method is mainly used for
 * resetting a temporary world in multihreading and should delete all agents, genomes
 * and create a new world structure.
 */
void World::recreate_world() {

}

/**
 * Returns the number of offspring agents created every generation for the given type.
 */
unsigned int World::get_offspring_quantity(const std::type_info* agent_type) {
	auto atp_i = agent_type_infos.find(agent_type);
	return atp_i == agent_type_infos.end() ? 0 : atp_i->second.offspring_quantity;
}

/**
 * Returns the number of different agent types seen until now in this world.
 */
unsigned int World::get_different_agent_type_number() const {
	return agent_type_infos.size();
}

/**
 * Sets the 'seed' value for the used random-value-creation-technique. 
 * You can use this method to have the same series of 'random' values in every run.
 * Therefore create your world(s) first, and then run this method one time. Every world
 * constructor sets a different and unpredictable seed, that's why you have to run this
 * method afterwards.
 * This does not work and makes no sense in a multithreaded environment, which means you
 * have compiled LEvoSim with OpenMP and have set parallel world computations 
 * (vie World::set_max_generation_reiterations) to more than 1.
 */
//void World::set_random_seed(const unsigned int new_seed) {
//	srand(new_seed);
//}

/**
 * Sets the point in time after that every agent dies.
 * If you don't set this it is somewhere near infinity.
 */
void World::set_max_turns(const turn_counter new_max_turns) {
	max_turns_per_generation = new_max_turns;
}

/**
 * Every agent gets completly new genes with random values.
 */
void World::randomize_genes() {
	for (auto const& agent: population)
		agent->get_genome_ptr()->createEmptyGenes();
}

/**
 * Finds and deletes one agent from the population.
 * Erases the pointer from population-list and maybe frees the objects memory.
 */
void World::kill_agent(agent_ptr cooper) {
	for (auto agent_i = population.begin(); agent_i != population.end(); agent_i++)
		if (*agent_i == cooper) {
			population.erase(agent_i);
			break;
		}   
}

/**
 * This method will deep-freeze agents. With only one parameter given, it freezes all 
 * agents. The second parameter can be the type-id of the agents to freeze.
 * Deep-freeze means that they will sleep until time has reached the given point in time,
 * and there is no chance to die for the agents during freezing (would be unfair).
 */
void World::freeze_agents(turn_counter end_time, const std::type_info* agent_type) {
	for (auto const& agent: population)
		if (agent_type == NULL)
			agent->set_action_finishing_time(end_time);
		else if (*agent_type == typeid(*agent))
			agent->set_action_finishing_time(end_time);
}

/**
 * Returns the quantity of actual living agents.
 */
int World::get_population_size() const {
	return population.size();
}

/**
 * Returns the quantity of current living agents in this world.
 */
int World::get_population_size(const std::type_info* agent_type) {
	int ret = 0;
	for (auto const& agent: population)
		if (agent->get_genome_ptr()->agents_type_equals(*agent_type))
			++ret;
	return ret;
}

/**
 * Sets the latest point in time for death for all current living agents.
 */
void World::set_max_age(turn_counter new_max_age) {
	for (auto const& agent: population)
		agent->set_max_age(new_max_age);
}

/**
 * This is called afters every agents death in the simulation.
 * It does nothing, but can be overwritten in orphane classes to collect 
 * data for statistics or whatever.
 */
void World::agent_death_statistics(agent_ptr dead_agent) {
}

/**
 * One agent can act one time in a run().
 * Returns true if there is time left for more runs of other agents.
 */
bool World::run() {
	// The list of living agents is sorted by the lengths of time they need to complete 
	// their current action. 
	population.sort(Agent::compare_finishing_times);

	// current_agent points to the next (in time) ready agent.
	agent_container::iterator current_agent = population.begin();

	// Bug-check for nullpointer.
	BUG_CHECK(!(*current_agent), "Agent pointer to nowhere.");
	
	// In the case of death the agent is deleted from the population-list and 
	// from memory (if there is no other shared pointer to this agent).
	// This run() is stopped with <true> because there could be other active agents 
	// in this generation.
	if ((*current_agent)->died()) {
		debug_msg("Agent " << *current_agent << " is DEAD.");
		agent_death_statistics(*current_agent);
		population.erase(current_agent);
		return true;
	}
	
	// We 'wait' until the agent has done everything and is ready to do the next.
	// The 'clock' <turn> is set to the finishing time.
	turn = (*current_agent)->accomplish_action();

	// If the time for this generation is over we stop everything.
	if (turn>max_turns_per_generation)
		return false;

	// The world must arrange a data struture for this agent, which contains all facts
	// he should know to decide what he wants to do.
	perception agents_perception;
	make_perception(*current_agent, &agents_perception);

	// The agent gets his perception, ponders, and gives back what he decided to do next.
	action what_he_does = (*current_agent)->cognite(&agents_perception);

	// The world tries to 'execute' the agents will. There is no certainity that this 
	// action can be done. World is responsible to decide about the effects of the agents
	// ambition.
	execute_action(*current_agent, what_he_does);
	
	return true;
}

/**
 * Does some death statistics for all living agents and deletes them afterwards.
 * Call this method if they should be dead will not act anymore.
 */
void World::kill_all_agents() {
	for (auto const& agent: population) {
		agent->is_dead_now();
		agent_death_statistics(agent);
	}
	population.clear();
}

/**
 * Create a new genome container, which contains copies of all genomes of the genepool.
 */
genome_container_ptr World::genepool_copy() {
	genome_container_ptr dest_gp = genome_container_ptr(new genome_container);
	for (auto const& genome: *genepool)
		dest_gp->push_back(genome_ptr(new Genome(*genome)));
	return dest_gp;
}

/**
 * Sets the current point in time (called 'turn').
 */
void World::set_time(turn_counter new_time) {
	turn = new_time;
}

/**
 * Increase the current_generation counter by the given parameter new_gens.
 */
void World::inc_current_generation(int new_gens) {
	current_generation += new_gens;
}

/**
 * Returns the number of the current generation.
 */
int World::get_generation() const {
	return current_generation;
}

/**
 * Returns the value of the fitness of the fittest agent.
 */
double World::get_best_fitness() const {
	return best_fitness;
}

/**
 * Returns a pointer to a copy of the genome with the highest fitness.
 */
genome_ptr World::best_genome(const std::type_info& best_agents_type) {
	BUG_CHECK(!genepool->size(), "There is no genepool.");
	genome_ptr best_g;
	for (auto const& genome: *genepool)
		if (genome->agents_type_equals(best_agents_type)) {
			if (!best_g) {
				best_g = genome;
			} else {
				if (genome->get_fitness() > best_g->get_fitness())
					best_g = genome;
			}
		}
	if (!best_g)
		return best_g;
	genome_ptr copy_of_best_g = genome_ptr(new Genome(*best_g));
	copy_of_best_g->attach_agents_name("Best ");
	return copy_of_best_g;
}

/**
 * Gives back a genome with arithmetic mean gene values.
 * For the calculation only genomes with average_agents_type are taken.
 * The gene values are multiplied by their genomes offspring quantity. Thus the average genome
 * is arithmetic mean of the agents of the next generation.
 */
genome_ptr World::average_genome(const std::type_info& average_agents_type) {
	auto atp_i = agent_type_infos.find(&average_agents_type);
	BUG_CHECK(atp_i == agent_type_infos.end(), "Can not find agents_type.");
	if (!atp_i->second.offspring_quantity)
		return atp_i->second.last_average_genome;
	genome_ptr avg_g;
	unsigned int individuals = 0;

	// Sum all the appropriate genomes.
	for (auto const& genome: *genepool) 
		if (genome->agents_type_equals(average_agents_type)  && 
		    genome->get_offspring_quantity()) {
			if (!avg_g)				
				avg_g = genome_ptr(new Genome(*genome->get_type_id(), genome->size(), 0.0));
			*avg_g += *genome * (double)genome->get_offspring_quantity(); 
			avg_g->increase_fitness(genome->get_fitness());
			individuals += genome->get_offspring_quantity();
		} 

	// If no genomes were found return an empty pointer.
	if (!avg_g) { 
		debug_msg("Bug? No genomes found!");
		return avg_g; 
	}

	// If no agents were found return an empty pointer.
	if (!individuals) {
		genome_ptr null_pointer_genome;
		return null_pointer_genome;
	}
	
	// Divide all sums by the amount of accumulated agents.
	(*avg_g) /= (double)individuals;
	avg_g->set_fitness(avg_g->get_fitness() / (double)individuals);
	//avg_g->add_fitness_to_average();

	// Add an "Average" to the describing type string.
	avg_g->attach_agents_name("Average ");

	// Store a pointer to this freshly created average genome in the agent type parameters.
	atp_i->second.last_average_genome = avg_g;
	
	return avg_g;
}

/**
 * Returns the sum of fitness of all genomes in g_list.
 */
double World::get_collective_fitness(genome_container_ptr g_list) {
	double ret_fit = 0.0;
	for (auto const& genome: *g_list)
		ret_fit += genome->get_fitness();
	return ret_fit;
}

/**
 * Sets the offspring quantity to new_offspring for all genomes in the given list.
 */
void World::set_genome_offspring(genome_container_ptr g_list, unsigned int new_offspring) {
	for (auto const& genome: *g_list)
		genome->set_offspring_quantity(new_offspring);
}

void store_last_offspring_quantity(genome_container_ptr g_list) {
	for (auto const& genome: *g_list)
		genome->set_last_offspring_quantity(genome->get_offspring_quantity());
}

unsigned int World::get_max_reiterations() const {
	return max_redundant_generation_reiterations;
}

/**
 * Calculates the amount of offspring for each genome from fitness using the 'Stochastic 
 * Universal Sampling' algorithm from James Barker.
 */
void World::stochastic_universal_sampling(genome_container_ptr g_list, unsigned int g_quant) {
	// First set all offspring to zero.
	set_genome_offspring(g_list, 0);
	// If there should be no offspring there is need need for further calculations.
	if (!g_quant)
		return;
	double dist_pointers = 1.0 / g_quant;
	// All genomes are equal when they all have no fitness.
	bool all_are_equal = false;
	double collective_fitness = get_collective_fitness(g_list);
	if (collective_fitness == 0.0) {
		collective_fitness = g_list->size();
		all_are_equal = true;
	}
	unsigned int cur_pointer = 0;
	genome_container::iterator cur_genome = g_list->begin();
	if (cur_genome == g_list->end())
		return;
	double right_fitness_border;
	if (all_are_equal)
		right_fitness_border = 1.0 / collective_fitness;
	else
		right_fitness_border = (*cur_genome)->get_fitness() / collective_fitness;
	double first_pointer_shift = randone() * dist_pointers;
	
	while (cur_pointer < g_quant) {
		double cur_pointer_pos = first_pointer_shift + dist_pointers * (double)cur_pointer;
		if (cur_pointer_pos < right_fitness_border) {
			(*cur_genome)->inc_offspring_quantity();
			++cur_pointer;
		} else {
			++cur_genome;   
			if (all_are_equal)
				right_fitness_border += 1.0 / collective_fitness;
			else
				right_fitness_border += (*cur_genome)->get_fitness() / collective_fitness;
		}
	}
}

/**
 * Returns a dynamic pointer to a fresh-made container of pointers to all genomes which
 * are belonging to agents of type agents_t_id.
 */
genome_container_ptr World::get_genomes_by_type(const std::type_info& agents_t_id) {
	genome_container_ptr ret_gc = genome_container_ptr(new genome_container);
	for (auto const& genome: *genepool)
		if (genome->agents_type_equals(agents_t_id))
			ret_gc->push_back(genome);
	return ret_gc;
}

/**
 * Returns a pointer to the population container.
 */
agent_container* World::get_population() {
	return &population;
}

/**
 * Returns a pointer to the genepool (container of all genomes).
 */
genome_container_ptr World::get_genepool() {
	return genepool;
}

/**
 * Sets the offspring quantity of all genomes in the given list like their fitness values.
 * Returns the sum of all offsprings.
 */
unsigned int World::offspring_from_fitness(genome_container_ptr gcp) {
	unsigned int offsp_sum = 0;
	for (auto const& genome: *gcp) {
		genome->set_offspring_quantity(genome->get_fitness());
		offsp_sum += genome->get_offspring_quantity();
	}
	return offsp_sum;
}

/**
 * Deletes all genomes without offspring from the genepool.
 * Exception: it does not delete offspring-free genomes when the offspring quantity for
 * the whole belonging agent type is set to zero.
 */
void World::delete_unused_genomes() {
	auto genome_i = genepool->begin();
	while (genome_i!=genepool->end()) {
		auto atp_i = agent_type_infos.find((*genome_i)->get_type_id());
		if (!(*genome_i)->get_offspring_quantity() && (atp_i->second.offspring_quantity))
			genome_i = genepool->erase(genome_i);
		else
			++genome_i;
	}
}

/**
 * Adds an amount of quantity agents to the population.
 * For every agent the virtual method create_agent called one time. This method must
 * be defined somewhere in your implementation and should initialise the agent.
 */ 
void World::add_new_agent(genome_ptr agent_genome, unsigned int quantity) {
	for (unsigned i=0; i<quantity; ++i) {
		agent_ptr fresh_agent = create_agent(agent_genome);
		BUG_CHECK(!fresh_agent->get_genome_ptr(), "New agent has no genome.");
		agent_genome->set_agents_name(fresh_agent->get_agent_type());
		population.push_back(std::move(fresh_agent));
	}
}

void World::add_new_agent(const std::type_info* agents_t_id, unsigned int quantity) {
	for (unsigned i=0; i<quantity; ++i) {
		genome_ptr new_genome = genome_ptr(new Genome(*agents_t_id));
		add_new_agent(new_genome);
		genepool->push_back(std::move(new_genome));
	}
}

void World::create_agents_from_genomes(genome_container_ptr genome_list) {
	for (auto& genome: *genome_list)
		add_new_agent(genome, genome->get_offspring_quantity());
}

/**
 * Mutates (depending on random) the genome of every living agent in population.
 * In case one agent is chosen for mutation a new genome of his / her old one is created
 * (a copy is made) and the copy is mutated.
 */
void World::mutate_genomes() {
	for (auto const& genome: *genepool)
		for (unsigned offspring_i=0; offspring_i<genome->get_offspring_quantity(); ++offspring_i)
			if (genome->mutation_chance()) {
				// An exact copy of the old genome is made.
				genome_ptr mutated_genome = genome_ptr(new Genome(*genome));
				// The copy must get its own unique ID.
				mutated_genome->set_new_id();
				// Exactly one agent has this genome.
				mutated_genome->set_offspring_quantity(1);
				// The new genotype gets a little of the old one's fitness.
				unsigned int old_g_offspring = genome->get_offspring_quantity();
				double old_g_fitness = genome->get_fitness();
				double f_carryover = old_g_offspring ? old_g_fitness / (double)old_g_offspring : 0.0;
				mutated_genome->set_fitness(f_carryover);			
				// The old genotype looses some fitness.
				genome->set_fitness(old_g_fitness - f_carryover);
				// The old genome has 1 offspring lost to the mutated one, but must have
				// more than 0 now. Check for that:
				BUG_CHECK(genome->get_offspring_quantity() < 1, "Genome has only " << 
				          genome->get_offspring_quantity() <<
				          " offspring but should have 1 or more.");
				genome->dec_offspring_quantity(1);
				// And now the most important: the new genome must mutate.
				mutated_genome->mutate();
				// It goes to be part of the official genepool.
				genepool->push_back(std::move(mutated_genome));
			}
}

/**
 * Calculates offspring agents from the genomes.
 * Every Genome has a fitness. This is used for offspring_quantity calculation. No new 
 * agents are created.
 */
void World::calculate_offspring() {
	// Compute offspring for every agent type.
	for (auto& atp: agent_type_infos) {
		atp.second.genomes = get_genomes_by_type(*atp.first);
		store_last_offspring_quantity(atp.second.genomes);
		if (atp.second.dynamic_offspring)
			atp.second.offspring_quantity = offspring_from_fitness(atp.second.genomes);
		else
			stochastic_universal_sampling(atp.second.genomes, 
			                              atp.second.offspring_quantity);
	}
}

genome_ptr World::get_fortune_wheel_genome(agent_type_parameter* atp) {
	BUG_CHECK(atp->offspring_quantity<1, "No offspring wanted.");
	BUG_CHECK(!atp->genomes->size(), "Empty genome list.");
	unsigned int agent_no = (double)atp->offspring_quantity * randone();
	BUG_CHECK(agent_no>=atp->offspring_quantity, "Agent no out of range: " << agent_no);

	genome_container::iterator genome_i = atp->genomes->begin();
	unsigned int agent_pointer = (*genome_i)->get_offspring_quantity();
	while (agent_pointer<=agent_no) {
		++genome_i;
		agent_pointer += (*genome_i)->get_offspring_quantity();
	}

	BUG_CHECK(genome_i==atp->genomes->end(), "Genome pointer over the top.");
	return *genome_i;
}

/**
 *
 */
void World::recombine_all_genomes() {
	genome_container_ptr new_genepool = genome_container_ptr(new genome_container);

	for (auto& atp: agent_type_infos)
		if (atp.second.genomes->size()) {
			genome_container_ptr special_pool;
			if (atp.second.offspring_quantity)
				special_pool = genome_container_ptr(new genome_container);
			else {
				special_pool = get_genomes_by_type(*atp.first);
				for (auto const& genome: *special_pool)
					new_genepool->push_back(genome);
			}
			for (unsigned offsp_i=0; offsp_i<atp.second.offspring_quantity; ++offsp_i) {
				genome_ptr child = Genome::recombine(get_fortune_wheel_genome(&atp.second),
													 get_fortune_wheel_genome(&atp.second));
				child->set_offspring_quantity(1);
				new_genepool->push_back(child);
				special_pool->push_back(child);
			}
			atp.second.genomes = special_pool;
		}

	genepool = new_genepool;
}

void World::set_recombination(bool new_recomb) {
	recombination = new_recomb;
}

bool World::does_recombination() {
	return recombination;
}

void World::create_offspring() {
	BUG_CHECK(population.size(), "There are living agents before creation.");
	for (auto const& genome: *genepool)
		add_new_agent(genome, genome->get_offspring_quantity());
}
