/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 * This file contains the abstract class Agent.
 *
 */

#ifndef _AGENT_H_
#define _AGENT_H_

#include <vector>
#include "debug_macros.h"
#include "world.h"

class Genome;

class Agent;
typedef std::shared_ptr<Agent> agent_ptr;

typedef std::vector<double> nn_signals;
typedef std::shared_ptr<nn_signals> nn_signals_ptr;


/**
 * One individual beeing in the world.
 * A derived class must be used, this agent is abstract.
 */
class Agent {
	
	public:
		Agent(genome_ptr new_genome_ptr = genome_ptr());
		
		genome_ptr get_genome_ptr();
		void set_genome(genome_ptr new_genome_ptr);
		void starts_to_act(const turn_counter duration_time, const turn_counter actual_time);
		void set_death_chance(const double new_death_chance);
		void set_generation(const int new_generation);
		void set_max_age(turn_counter new_max_age);
		turn_counter get_current_action_duration() const;
		static bool compare_finishing_times(const agent_ptr a, const agent_ptr b);
		bool died(const turn_counter turns);
		bool died();
		turn_counter accomplish_action();
		turn_counter get_action_finishing_time() const;
		void set_action_finishing_time(const turn_counter new_f_t);
		static void set_duration_noise(const double new_noise);
		static double get_duration_noise();
		void print();
		std::string get_agent_type() const;
		void set_agent_type(const std::string new_type);
		bool has_same_type(agent_ptr other_agent) const;
		unsigned int get_agent_id();
		virtual action cognite(const perception* agents_personal_perception) = 0;
		virtual string_ptr get_gene_description(const unsigned int gene_no);
		unsigned int get_genes_size() const;
		turn_counter get_birth_time() const;
		turn_counter get_death_time() const;
		turn_counter get_life_span() const;
		void is_dead_now();
		void inc_personal_fitness(double add_fit);
		void set_personal_fitness(double new_fit);
		double get_personal_fitness() const;
		static void set_nn_hidden_layers(const unsigned int new_nn_layers);

	protected:
		inline double scale(double val);
		bool neuronal_network(nn_signals_ptr signals, int hidden_layer_quant);
		bool neuronal_network(nn_signals_ptr signals);
		/** Normalizes the given input to range -1..1 per sigmoid function. */
		inline double sigmoid(double inp) { return inp / (1.0 + abs(inp)); }
		
		/** Pointer to genome this agent belongs to. */
		genome_ptr my_genome;
		/** Chance to die per turn for tbool neuronal_network(nn_signals_ptr signalshis agent. */
		double death_chance;
		/** Human readable description of the agent. */
		std::string agent_type;
		/** Fitness of this agent. Used only for statistics and not for offspring 
		 creation. */
		double personal_fitness;
		/** Counter for the genes used in cognite. */
		unsigned int next_gene;
		/** The time of birth (first cognition beginning) of this agent. */
		turn_counter birth_time;
		/** The time of death of this agent. */
		turn_counter death_time;
		/** Maximum point in time before death for this individual agent. */
		turn_counter max_age;
		
	private:
		nn_signals_ptr neuronal_layer(unsigned int output_layer_size, nn_signals_ptr input_signals, bool negative_genes=false);
		void set_current_action_duration(const turn_counter new_duration);
		
		/** Amount of turns it will take until the agent can do a new action. */
		turn_counter current_action_duration;
		/** Point in time when the current action is finished. */
		turn_counter action_finishing_time;
		/** Number of previous generations. TODO: still unused */
		int generation;
		/** Maximum noise which is added to every period. */
		static double action_duration_noise;
		/** Unique identification number of this agent. */
		unsigned int agent_id;
		/** Next free (unused) id for all agents. */
		static unsigned int next_agent_id;
		/** Standard quantity of hidden layers in the neuronal networks. */
		static unsigned int hidden_layers;




};

#endif // _AGENT_H_

