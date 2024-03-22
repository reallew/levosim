/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 */
#ifndef _GENOME_H_
#define _GENOME_H_

#include <vector>
#include <iostream>
#include <giomm.h>
#include "debug_macros.h"

class Agent;

typedef std::vector<double> gene_container;
typedef std::shared_ptr<std::string> string_ptr;
typedef std::shared_ptr<std::type_info> type_info_ptr;
class Genome;
typedef std::shared_ptr<Genome> genome_ptr;


/**
 * Represents a "genome". A genome here is a container of "genes". 
 * Sometimes it is called "chromosome" in genetic algorithms.
 * It belongs to a "genepool".
 */
class Genome {
public:
	Genome(const std::type_info& agents_t_id,	       
	       const int gene_quantity=0,
	       const double init_val=-1.0,
	       const double mut_intense=0.21);
	
	unsigned int size() const;
	//	void add_gene(double const new_gene);
	void print() const;
	double get_fitness() const;
	void set_fitness(const double new_fitness);
	void increase_fitness(const double inc_fitness);
	unsigned long get_genome_id() const;
	void set_new_id();
	void set_offspring_quantity(const int new_oq);
	unsigned int get_offspring_quantity() const;
	void set_last_offspring_quantity(const int new_oq);
	unsigned int get_last_offspring_quantity() const;		
	void dec_offspring_quantity(const int dec_oq = 1);
	void inc_offspring_quantity(const int inc_oq = 1);
	static void set_mutation_rate(const double new_mutation_rate);
	bool mutation_chance() const;
	bool mutation_chance(unsigned int genes) const;
	double gene_sum();
	void mutate();
	void createEmptyGenes(int gene_quantity = -1, double init_val = -1.0);
	void set_mutation_intensity(double new_intensity);
	double get_mutation_intensity() const;
	void write(Glib::RefPtr<Gio::OutputStream> write_stream);
	void set_agents_name(const std::string new_a_type);
	void attach_agents_name(std::string att_a_type);
	bool agents_type_equals(Agent* other_agent);
	bool agents_type_equals(Genome* other_genome);
	bool agents_type_equals(const std::type_info& other_type);
	std::string get_agents_name() const;
	const std::type_info* get_type_id();
	double get_gene(const unsigned int gene_no);
	void set_gene(const unsigned int gene_no, const double gene_value);
	void add_gene(const unsigned int gene_no, const double gene_value);
	void divide_gene(const unsigned int gene_no, const double divider);
	bool is_gene(const unsigned int gene_no);
	string_ptr get_gene_description(const unsigned int gene_no);
	void set_gene_description(const unsigned int gene_no, string_ptr new_dscr);
	void merge(genome_ptr other_genome);
	static genome_ptr recombine(genome_ptr parent_1, genome_ptr parent_2);

	Genome operator+=(Genome other_g);
	Genome operator/=(double divider);
	Genome operator*=(double multiplier);
	Genome operator-(Genome other_g);
	Genome operator-=(Genome other_g);
	Genome operator*(double multiplier);
	Genome operator/(double divider);

	static double mutation_rate_scaler;

protected:
	/** Human readable description of the agent (his type). Not important. */
	std::string my_agents_type;
	/** Unique id of genomes agents type. */
	const std::type_info* my_agents_type_id;
		
private:
	/** Container of all genes of this genome. */
	gene_container genes;
	/** Fitness of this genome (genotype). */
	double fitness;
	/** Stores the number of all genomes ever existed. */	
	static unsigned long genome_counter;
	/** Unique id of this genome. */
	unsigned long genome_id;
	/** Quantity of offspring agents from this genome in every generation. */
	unsigned int offspring_quantity;
	/** Quantity of offspring agents from this genome in the last generation. */
	unsigned int last_offspring_quantity;
	/** Chance to mutate the genome for every offspring agent. */
	static double mutation_rate;
	/** Maximum change of gene values in a mutation. */
	double mutation_max_intensity;
	/** Minimum value for genes. Should be 0 here. */
	static double min_gene_val;
	/** Maximum value for genes. Should be 1 here. */
	static double max_gene_val;
	/** Container of human readable gene descriptions. */
	std::vector<string_ptr> gene_descriptions;
			
};

bool operator<(Genome a, Genome b);
bool operator>(Genome a, Genome b);
Genome operator+(Genome a, Genome b);
std::ostream& operator<<(std::ostream& stream, Genome g);


#endif // _GENOME_H_
