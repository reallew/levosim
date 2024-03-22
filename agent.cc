/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 */

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <limits>
#include "agent.h"
#include "world.h"

/**
 * Creates an agent. Sets some statistical variables to zero or other reasonable values 
 * and gives the agent a unique agent_id.
 * If you write a simulation you normally don't create agents by hand, you should use
 * World::add_new_agent.
 */
Agent::Agent(genome_ptr new_genome_ptr) : 
	death_chance(0.0),
	personal_fitness(0.0),
	birth_time(-1.0),
	death_time(-1.0),
	current_action_duration(0.0),
	action_finishing_time(0.0)
{
	agent_type = "Agent";
	if (next_agent_id == std::numeric_limits<unsigned int>::max()) {
		std::clog << "Maximum number of agents reached. Program full stop." << std::endl;
		std::exit(1);
	}
	agent_id = next_agent_id++;
	
	if (!new_genome_ptr) {
		new_genome_ptr = genome_ptr(new Genome(typeid(*this)));
		debug_msg("Agent makes a new genome!");
	}
	my_genome = new_genome_ptr;

	max_age = std::numeric_limits<turn_counter>::max(); // Somewhere near infinity (on Linux).
	
	// Every agent starts an action of duration 0 at time 0 in his life. Useless?
	// It's because a maybe existing action duration noise works here and moves
	// all agents randomly to little different places in time.
	set_action_finishing_time(0.0);
}

/**
 * Returns the unique id number of this agent.
 */
unsigned int Agent::get_agent_id() {
	return agent_id;
}

/**
 * Returns the amount of genes this agents genome has.
 */
unsigned int Agent::get_genes_size() const {
	return my_genome->size();
}

/**
 * Returns the point in time when the current agents action will be finished.
 */
turn_counter Agent::get_action_finishing_time() const {
	return action_finishing_time;
}

/**
 * Returns a pointer to the agents genome.
 */
genome_ptr Agent::get_genome_ptr() {
	return my_genome;
}

/**
 * Sets the agents genome (pointer).
 */
void Agent::set_genome(genome_ptr new_genome_ptr) {
	BUG_CHECK(!new_genome_ptr, "Genome pointer to nowhere.");
	my_genome = new_genome_ptr;
}

/**
 * Prints some informations to std::cout. Depricated. This method should be removed.
 */
void Agent::print() {
	std::cout << "Agent " << this << ", Genome " << my_genome->get_genome_id() << std::endl;
}

/**
 * Changes the value of the agents personal fitness by the given parameter.
 * Personal fitness is only for statistical use. Offspring creation is done by the 
 * genomes fitness.
 */
void Agent::inc_personal_fitness(double add_fit) {
	personal_fitness += add_fit;
}

/**
 * Sets the value of the agents personal fitness by the given parameter.
 * Personal fitness is only for statistical use. Offspring creation is done by the 
 * genomes fitness.
 */
void Agent::set_personal_fitness(double new_fit) {
	personal_fitness = new_fit;
}

/**
 * Returns the value of the agents personal fitness.
 * Personal fitness is only for statistical use. Offspring creation is done by the 
 * genomes fitness.
 */
double Agent::get_personal_fitness() const {
	return personal_fitness;
}

/**
 * Finishes the agents current action in time.
 * This means that his time counters are set to zero.
 * Returns the exact finishing time.
 */
turn_counter Agent::accomplish_action() {
	turn_counter time_finished = action_finishing_time;
	action_finishing_time = 0.0;
	current_action_duration = 0.0;
	return time_finished;
}

/**
 * Decides if the agent dies in the given time period. Returns true if the agent will die.
 * This method uses the agents death_chance (chance to die per time unit) and a dice.
 */
bool Agent::died(const turn_counter turns) {
	BUG_CHECK(death_chance < 0.0 || death_chance > 1.0, "Death chance out of range.");
	double survive_chance = pow(1.0 - death_chance, turns);
	BUG_CHECK(survive_chance < 0.0 || survive_chance > 1.0, "Survive chance out of range.");
	bool is_dead = survive_chance < World::randone() || action_finishing_time > max_age;
	if (is_dead)
		is_dead_now();
	return is_dead;
}

/**
 * This does only one thing: it calculates an approximated death point in time and stores
 * it for statistcal use. If an agent dies because of the method call Agent::died this
 * method is called automatically and you don't have to care about it.
 * But if you let the agent die elsewhere, you may have to call this method if you want
 * to use death time statistics.
 */
void Agent::is_dead_now() {
	death_time = action_finishing_time - current_action_duration / 2.0;
}

/**
 * Decides if the agent dies during his current action.
 */
bool Agent::died() {
	return died(current_action_duration);
}

/**
 * Gives the birth point in time.
 * Returns -1.0 if the agent was never born (that whould be very sad and should not 
 * happen normally).
 */
turn_counter Agent::get_birth_time() const {
	return birth_time;
}

/**
 * Gives the death point in time.
 * Don't call this for unborn agents.
 */
turn_counter Agent::get_death_time() const {
	BUG_CHECK(death_time<0, "Agent never died.");
	return death_time;
}

/**
 * Returns the length of the agents life. Must be called only after the agents death or 
 * before his birth (then this returns zero).
 */
turn_counter Agent::get_life_span() const {
	BUG_CHECK(birth_time>=0.0 && death_time<0.0, "No life span: Agent is born and alive.");
	BUG_CHECK(birth_time > death_time, "Agent was born after his death.");
	return death_time - birth_time;
}

/**
 * Sets the point in time after that this agent must die. He could die earlier, but not
 * later.
 */
void Agent::set_max_age(turn_counter new_max_age) {
	BUG_CHECK(new_max_age < 0.0, "Negative max age: " << new_max_age);
	max_age = new_max_age;
}

/**
 * Sets the time span the current action of this agent will take.
 */
void Agent::set_current_action_duration(const turn_counter new_duration) {
	BUG_CHECK(new_duration < 0.0, "Negative action duration.");
	current_action_duration = new_duration;
}

/**
 * Sets the point in time when the current agents action will be finished.
 */
void Agent::set_action_finishing_time(const turn_counter new_f_t) {
	action_finishing_time = new_f_t + World::randone() * action_duration_noise;
}

/**
 * Tells the agent that he starts an action now. action_duration is the time span the 
 * action takes und actual_time is the action beginning time.
 * This method calculates the finishing time (with some noise / looseness maybe) and
 * stores it.
 */
void Agent::starts_to_act(const turn_counter action_duration, const turn_counter actual_time) {
	BUG_CHECK(action_duration < 0.0, "Negative action duration.");
	set_current_action_duration(action_duration);
	set_action_finishing_time(current_action_duration + actual_time);
}

/**
 * Sets the probability of death per time unit (per 'turn') for this agent.
 */
void Agent::set_death_chance(const double new_death_chance) {
	BUG_CHECK(new_death_chance < 0.0 || new_death_chance > 1.0, "Death chance out of " <<
	          "range 0..1. It is: " << new_death_chance);
	death_chance = new_death_chance;
}

/**
 * Sets the amount of ancestors of this agent.
 * Unused at the moment.
 */
void Agent::set_generation(const int new_generation) {
	generation = new_generation;
}

/**
 * Returns the number of time units the agents current action takes.
 */
turn_counter Agent::get_current_action_duration() const {
	return current_action_duration;
}

/* Maximum noise value added to every action duration. */
double Agent::action_duration_noise = 0.001;

/**
 * Returns true when the first given agent (a) will finish his action earlier than b.
 */
bool Agent::compare_finishing_times(const agent_ptr a, const agent_ptr b) {
	return a->action_finishing_time < b->action_finishing_time;
}

/**
 * Sets the maximum noise value added to every action duration.
 */
void Agent::set_duration_noise(const double new_noise) {
	BUG_CHECK(new_noise < 0.0, "Negative noise makes no sense here.");
	action_duration_noise = new_noise;
}

/**
 * Returns the maximum noise value added to every action duration.
 */
double Agent::get_duration_noise() {
	return action_duration_noise;
}

/**
 * Returns a human readable description of the agents type.
 */
std::string Agent::get_agent_type() const {
	return agent_type;
}

/**
 * Sets a human readable description of the agents type.
 */ 
void Agent::set_agent_type(const std::string new_type) {
	agent_type = new_type;
}

/**
 * Compares the human readable description of this agents type with the type of the
 * given other_agent. Returns true if they are the same.
 */
bool Agent::has_same_type(agent_ptr other_agent) const {
	return agent_type == other_agent->get_agent_type();
}

/**
 * Returns a pointer to a human readable description of the given gene number gene_no.
 * Tells only a simple Text like "Gene 3". Can be overwritten by a childclass to give
 * more interesting information about the use of the gene.
 */
string_ptr Agent::get_gene_description(const unsigned int gene_no) {
	BUG_CHECK(gene_no < 0, "Gene number negative.");
	BUG_CHECK(gene_no > 100000, "Gene number very high (" << gene_no << "). Are you sure "
	          << "this is right? Then change this bug-check.");
	std::stringstream ss;
	ss << "Gene " << gene_no;
	string_ptr tmp_gene_dscr = string_ptr(new std::string(ss.str()));
	return tmp_gene_dscr;
}

/**
 * Scale a value of range 0..1 to range -1..1.
 */
inline double Agent::scale(double val) {
	return (val - 0.5) * 2.0;
}

/**
 * Computes one layer in a simulated neuronal network.
 * It sums up the weighted input signals for every output signal.
 * This method is used by Agent::neuronal_network. Read the comment there if you want to
 * know more about it.
 */
nn_signals_ptr Agent::neuronal_layer(unsigned int output_sigs_size, nn_signals_ptr input_signals, bool negative_genes) {
	nn_signals_ptr output_sigs = nn_signals_ptr(new nn_signals(output_sigs_size, 0.0));
	BUG_CHECK(output_sigs->size()!=output_sigs_size, "Wrong output sigs size.");

	if (negative_genes)
		for (auto& output: *output_sigs) {
			double signal_sum = 0.0;
			for (auto& input: *input_signals)
				signal_sum += input * scale(get_genome_ptr()->get_gene(next_gene++));
			output = (signal_sum > get_genome_ptr()->get_gene(next_gene++));			
		}
	else
		for (auto& output: *output_sigs) {
			double signal_sum = 0.0;
			for (auto& input: *input_signals)
				signal_sum += input * get_genome_ptr()->get_gene(next_gene++);
			output = (signal_sum > get_genome_ptr()->get_gene(next_gene++));
		}
	
	return output_sigs;
}

/**
 * Does a binary decision by computing a simulated neuronal network. As thresholds and
 * weightings genes from the agents genome are taken. The agents variable next_gene is
 * used incremented for every used gene. You have to set next_gene back to something
 * (maybe zero) before you call this method.
 * The parameters are the input signals (a vector of doubles) and the amount of hidden
 * layers, which may be zero.
 * The genes must have the range 0..1, but for the weightings they are scaled to -1..1. 
 * In this neuronal network connections can have negative value.
 */
bool Agent::neuronal_network(nn_signals_ptr signals, int hidden_layer_quant) {
	BUG_CHECK(!signals->size(), "Empty input signals container makes no sense.");
	BUG_CHECK(hidden_layer_quant>100, "Too many hidden layers in neuronal network.");
	unsigned int output_sigs_size = signals->size();

	while (hidden_layer_quant >= 0) {
		if (!hidden_layer_quant)
			output_sigs_size = 1;
		signals = neuronal_layer(output_sigs_size, signals, true);
		--hidden_layer_quant;
	} 

	BUG_CHECK(signals->size() != 1, "Wrong output size of nn: " << signals->size());
	return signals->at(0) > 0.5;
}

bool Agent::neuronal_network(nn_signals_ptr signals) {
	return neuronal_network(signals, hidden_layers);
}

void Agent::set_nn_hidden_layers(const unsigned int new_nn_layers) {
	hidden_layers = new_nn_layers;
}

unsigned int Agent::next_agent_id = 0;
unsigned int Agent::hidden_layers = 1;
