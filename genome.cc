/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
	 * 
 */
#include <iostream>
#include <giomm.h>
#include <cstdlib>
#include <cmath>
#include "genome.h"
#include "agent.h"

/**
 * Creates a genome containing a number of <gene_quantity> genes.
 * The new genes will have a the value init_val.
 * If init_val is not given the value will be randomly chosen.
 */

Genome::Genome(const std::type_info& agents_t_id,
			   const int gene_quantity,
			   const double init_val,
			   const double max_mut_intensity) :
	my_agents_type("Unknown Agent"),
	my_agents_type_id(&agents_t_id),
	fitness(0.0),
	offspring_quantity(0),
	last_offspring_quantity(0),
	mutation_max_intensity(max_mut_intensity)
{
	debug_msg("Genome: Constructor with " << gene_quantity << " genes.");
	createEmptyGenes(gene_quantity, init_val);
	set_new_id();
}

/**
 * Returns true if there is a gene with the given gene number.
 */
bool Genome::is_gene(const unsigned int gene_no) {
	return gene_no < genes.size();
}

/**
 * Returns the value of the gene with the given gene number.
 * If the gene does not exist it is created and randomly initialised. This is also true
 * for all genes between the old maximum gene number and the new one.
 * Example: there are 3 genes (numbered 0, 1, 2). Then you call this method and ask for 
 * gene 5. Without batting an eye it creates the genes numbered 3, 4, 5 and fills them
 * with random values of range 0..1. Then it delivers the value of gene no 5 to you.
 */ 
double Genome::get_gene(const unsigned int gene_no) {
	BUG_CHECK(gene_no>100000, "Maybe too high gene number: " << gene_no);
	
	if (gene_no>=genes.size()) {
		unsigned int old_genes_size = genes.size();
		genes.resize(gene_no+1);
		for (unsigned i=old_genes_size; i<(gene_no+1); ++i)
			genes.at(i) = World::randone();
	}

	return genes.at(gene_no);
}

/**
 * Returns a (pointer to a) text string with a human readable description of the given 
 * gene number.
 */
string_ptr Genome::get_gene_description(const unsigned int gene_no) {
	if (gene_descriptions.size()>gene_no)
		return gene_descriptions[gene_no];
	else
		return string_ptr(new std::string("Unknown Gene"));
}

/**
 * Sets a human readable textual description of a gene. The gene must not exist for this.
 */
void Genome::set_gene_description(const unsigned int gene_no, string_ptr new_dscr) {
	BUG_CHECK(gene_no>100000, "Maybe too big gene number: " << gene_no);
	if (gene_no >= gene_descriptions.size())
		gene_descriptions.resize(gene_no+1);
	gene_descriptions.at(gene_no) = new_dscr;
}

/**
 * Adds a value to a gene value (increases it).
 * If the gene does not exist it and maybe some others are created and initialised. Look
 * at the comment of Genome::get_gene for further explanation.
 * The gene value can get higher than 1.
 */
void Genome::add_gene(const unsigned int gene_no, const double gene_value) {
	BUG_CHECK(gene_no>100000, "Maybe too big gene number: " << gene_no);
	if (gene_no >= genes.size()) {
		std::cout << "add_gene: Gen " << gene_no << " ist kleiner als Genpoolgröße " << genes.size() << " – die wird vergrößert." << std::endl;
		unsigned int old_genes_size = genes.size();
		genes.resize(gene_no+1);
		for (unsigned i=old_genes_size; i<(gene_no+1); ++i)
			genes.at(i) = World::randone();
	}
	genes.at(gene_no) += gene_value;
}

/**
 * Set a gene value.
 * If the gene does not exist it and maybe some others are created and initialised. Look
 * at the comment of Genome::get_gene for further explanation.
 * The gene value can be higher than 1.
 */
void Genome::set_gene(const unsigned int gene_no, const double gene_value) {
	BUG_CHECK(gene_no>100000, "Maybe too big gene number: " << gene_no);
	if (gene_no>=genes.size()) {
		unsigned int old_genes_size = genes.size();
		genes.resize(gene_no+1);
		for (unsigned i=old_genes_size; i<(gene_no); ++i)
			genes.at(i) = World::randone();
	}
	genes.at(gene_no) = gene_value;
}

/**
 * Divides a gene value by the given divider.
 * If the gene does not exist it and maybe some others are created and initialised. Look
 * at the comment of Genome::get_gene for further explanation.
 */
void Genome::divide_gene(const unsigned int gene_no, const double divider) {
	BUG_CHECK(gene_no>100000, "Maybe too big gene number: " << gene_no);
	if (gene_no>=genes.size()) {
		unsigned int old_genes_size = genes.size();
		genes.resize(gene_no+1);
		for (unsigned i=old_genes_size; i<(gene_no); ++i)
			genes.at(i) = World::randone();
	}
	genes.at(gene_no) /= divider;
}

/**
 * Returns the typeid of the to this genome belonging agents.
 */
const std::type_info* Genome::get_type_id() {
	return my_agents_type_id;
}

/**
 * This genome gets a new unique ID.
 */
void Genome::set_new_id() {
	genome_id = genome_counter++;
}

/**
 * Creates a number of <gene_quantity> new genes.
 * If init_val is -1, the value is randomly chosen between 0 and 1.
 * If gene_quantity is -1, the size of the already existing gene container is taken.
 */
void Genome::createEmptyGenes(int gene_quantity, double init_val) {
	if (gene_quantity < 0)
		gene_quantity = genes.size();
	genes = gene_container(gene_quantity);
	if (init_val == -1.0)
		for (auto& gene: genes)
			gene = World::randone();
	else
		for (auto& gene: genes)
			gene = init_val;
}

/**
 * Returns the number of genes in this genome.
 */
unsigned int Genome::size() const {
	return genes.size();
}

/**
 * Adds another gene to this gene pool.
 */
/*void Genome::add_gene(double const new_gene)  {
	genes.push_back(new_gene);
	}*/

/**
 * Returns the fitness of this genome.
 * Must be calculated via World.calculate_fitness() first.
 */
double Genome::get_fitness() const {
	return fitness;
}

/**
 * Sets a new fitness for this genome.
 */
void Genome::set_fitness(const double new_fitness) {
	fitness = new_fitness;
}

/**
 * Merges another genome into this one.
 * If this genome is bigger (more genes) or of the same size, nothing happens.
 * If the other genome has more genes, the in this genome non-existent genes are copied.
 * Does not change any existing gene values of any genome, but could increase the amount
 * of genes of this genome by copying some from the other.
 */
void Genome::merge(genome_ptr other_genome) {
	unsigned int my_old_size = size();
	unsigned int other_size = other_genome->size();
	if (other_size <= my_old_size)
		return;
	genes.resize(other_size);
	while (other_size > my_old_size) {
		--other_size;
		genes.at(other_size) = other_genome->get_gene(other_size);
	}
}

/**
 * Increases the fitness for this genome with <inc_fitness>.
 */
void Genome::increase_fitness(const double inc_fitness) {
	fitness += inc_fitness;
}

/**
 * Returns the unique id of this genome.
 */
unsigned long Genome::get_genome_id() const {
	return genome_id;
}

/**
 * Sets the quantity of offspring agents of this genome, which should be created every
 * generation.
 */
void Genome::set_offspring_quantity(const int new_oq) {
	offspring_quantity = new_oq;
}

/**
 * Sets the quantity of offspring this genome had last generation. This is only for
 * statistical storage and you should not think about it too much.
 */ 
void Genome::set_last_offspring_quantity(const int new_loq) {
	last_offspring_quantity = new_loq;
}

/**
 * Decreases the quantity of offspring agents of this genome, which should be created every
 * generation. If you try to push it below zero it gets zero.
 */
void Genome::dec_offspring_quantity(const int dec_oq) {
	if (offspring_quantity-dec_oq < 0)
		offspring_quantity = 0;
	else
		offspring_quantity -= dec_oq;
}

/**
 * Increases the quantity of offspring agents of this genome, which should be created every
 * generation.
 */
void Genome::inc_offspring_quantity(const int inc_oq) {
	offspring_quantity += inc_oq;
}

/**
 * Returns the amount of agents which are created every generation.
 */
unsigned int Genome::get_offspring_quantity() const {
	return offspring_quantity;
}

/**
 * Returns the amount of agents which were created in the last generation. This is only
 * a statistical variable, which has to be set by Genome::set_last_offspring_quantity. If
 * you are not sure that you have done this don't touch this method.
 */
unsigned int Genome::get_last_offspring_quantity() const {
	return last_offspring_quantity;
}

/**
 * Sets the maximum value change of a gene in a mutation for this genome.
 */
void Genome::set_mutation_rate(const double new_mutation_rate) {
	mutation_rate = new_mutation_rate;
}

/**
 * Determines if there should be a mutation randomly.
 */
bool Genome::mutation_chance() const {
	return mutation_chance(genes.size());
}

/**
 * Determines if there should be a mutation randomly.
 */
bool Genome::mutation_chance(unsigned int gene_quant) const {
	if (!gene_quant)
		return false;
	double chance_no_gene_mutates = pow(1.0-(mutation_rate/mutation_rate_scaler), gene_quant);
	return World::randone() > chance_no_gene_mutates;
}

/** Chance per mutated gene to mutate (randomize) it totally. */
#define STRONG_MUTATION_CHANCE 0.05

/**
 * Mutates one or more of the genes.
 */
void Genome::mutate() {
	unsigned int gene_quant = genes.size();

	do {
		if (!gene_quant)
			return;
		unsigned int mut_gene_no = World::randone() * (double)genes.size();
		BUG_CHECK(mut_gene_no>=genes.size(), "Outer space gene should mutate.");
		if (mut_gene_no >= genes.size()) // HACK
			mut_gene_no = genes.size() - 1;
		if (World::randone() < STRONG_MUTATION_CHANCE)
			set_gene(mut_gene_no, World::randone() * max_gene_val);
		else 
			add_gene(mut_gene_no, World::randone() * mutation_max_intensity - mutation_max_intensity / 2.0);
		if (genes[mut_gene_no] < min_gene_val)
			genes[mut_gene_no] = min_gene_val;
		if (genes[mut_gene_no] > max_gene_val)
			genes[mut_gene_no] = max_gene_val;
		--gene_quant;
	} while (mutation_chance(gene_quant));
}

/**
 * Returns the sum of all gene values of this genome.
 */
double Genome::gene_sum() {
	double sum = 0.0;
	for (auto& gene: genes)
		sum += gene;
	return sum;
}

/**
 * Sets the maximum mutation value for this genome.
 */
void Genome::set_mutation_intensity(double new_intensity) {
	//	std::cout << "Set mut intense from " << mutation_max_intensity << " to " << new_intensity << std::endl;
	mutation_max_intensity = new_intensity;
}

double Genome::get_mutation_intensity() const {
	return mutation_max_intensity;
}

/**
 * Writes the fitness and values of the genes as ustring comma seperated to the stream.
 */
void Genome::write(Glib::RefPtr<Gio::OutputStream> write_stream) {
	std::stringstream ss;
	ss << get_fitness();
	write_stream->write(ss.str());
	for (auto const& gene: genes) {
		write_stream->write(", ");
		std::stringstream gene_ss;
		gene_ss << gene;
		write_stream->write(gene_ss.str());
	}
}

/**
 * Sets a human readable text string that should describe the type of agents belonging
 * to this genome.
 */
void Genome::set_agents_name(const std::string new_a_type) {
	my_agents_type = new_a_type;
}

/**
 * Attaches a human readable text string to the already existing agents type description
 * string. The attachment is done in front of the old text.
 * If the old agents type is 'chicken' you can attach 'big ' using this method and then
 * the genome has the type of 'big chicken'.
 */
void Genome::attach_agents_name(std::string att_a_type) {
	my_agents_type = att_a_type + my_agents_type;
}

/**
 * Returns a human readable text string that should describe the type of agents belonging
 * to this genome.
 */
std::string Genome::get_agents_name() const {
	return my_agents_type;
}

/**
 * Checks if this genome belongs to the same types of agents like another agent.
 */
bool Genome::agents_type_equals(Agent* other_agent) {
	return agents_type_equals(&*other_agent->get_genome_ptr());
}

/**
 * Checks if this genome belongs to the same types of agents like another genome.
 */
bool Genome::agents_type_equals(Genome* other_genome) {
	return *other_genome->get_type_id() == *get_type_id();
}

/**
 * Checks if this genome belongs to the same types of agents like another typeid.
 */
bool Genome::agents_type_equals(const std::type_info& other_type) {
	return other_type == *get_type_id();
}

/** Stores the amount of all genomes ever existed. */
unsigned long Genome::genome_counter = 0;

double Genome::mutation_rate = 0.01; 
double Genome::min_gene_val = 0.0;
double Genome::max_gene_val = 1.0;

/**
 * Puts the fitness value and the gene valus of the given genome comma seperated to
 * a stream. This is for humans to read and for saving in a CSV file.
 */
std::ostream& operator<<(std::ostream& stream, Genome g) {
	stream << g.get_fitness();
	for (unsigned i=0; i<g.size(); i++)
		stream << ", " << g.get_gene(i);
	return stream;
}

/**
 * Returns true if the first genome has a lower fitness than the other.
 */
bool operator<(Genome a, Genome b) {
	return a.get_fitness() < b.get_fitness();
}

/**
 * Returns true if the first genome has a higher fitness than the other.
 */
bool operator>(Genome a, Genome b) {
	return a.get_fitness() > b.get_fitness();
}

/**
 * Sums both genomes up and returns a new one which contains the sum. 
 * They are added gene by gene. Works with genomes of different sizes, then missing
 * genes are treated like zero.
 */
Genome operator+(Genome a, Genome b) {
	Genome bigger_g = Genome(*a.get_type_id());
	Genome* smaller_g;
	if (a.size() > b.size()) {
		bigger_g = a;
		smaller_g = &b;
	} else {
		bigger_g = b;
		smaller_g = &a;
	}
	for (unsigned i=0; i<smaller_g->size(); i++)
		bigger_g.add_gene(i, smaller_g->get_gene(i));
	return bigger_g;
}

Genome Genome::operator+=(Genome other_g) {
	return *this = *this + other_g;
}

Genome Genome::operator/=(double divider) {
	return *this = *this / divider;
}

/**
 * Multiplies all genes values of the genome by the given multiplier.
 */
Genome Genome::operator*=(double multiplier) {
	return *this = *this * multiplier;
}

/**
 * Multiplies all genes values of this genome by the given multiplier.
 */
Genome Genome::operator*(double multiplier) {
	Genome m_g = Genome(*this);
	for (unsigned i=0; i<genes.size(); ++i)
		m_g.genes.at(i) *= multiplier;
	return m_g;
}

/**
 * Divides all genes values of this genome by the given divider.
 */
Genome Genome::operator/(double divider) {
	return *this * (1.0 / divider);
}

/**
 * Returns a new Genome, which is the result of the subtracion of the two given genomes.
 * The gene containers are treated like mathematical vectors here. Works with genomes of
 * different sizes (amount of genes). Then a missing gene is handled like a zero.
 */
Genome Genome::operator-(Genome other_g) {
	unsigned int shorter_size = genes.size() < other_g.genes.size() ? genes.size() : other_g.size();
	unsigned int longer_size = genes.size() > other_g.genes.size() ? genes.size() : other_g.size();
	Genome difference = Genome(*other_g.get_type_id(), longer_size, 0.0);
	for (unsigned i = 0; i < longer_size; i++)
		if (i < shorter_size)
			difference.genes[i] = this->genes[i] - other_g.genes[i];
		else
			difference.genes[i] = (genes.size() > other_g.genes.size()) ? this->genes[i] : -other_g.genes[i];
	return difference;
}

Genome Genome::operator-=(Genome other_g) {
	return *this = *this - other_g;
}

genome_ptr Genome::recombine(genome_ptr parent_1, genome_ptr parent_2) {
	BUG_CHECK(!parent_1 || !parent_2, "Parent missing.");
	unsigned genome_size = parent_1->size() > parent_2->size() ? parent_1->size() : parent_2->size();
	genome_ptr child = genome_ptr(new Genome(*parent_1->get_type_id(), genome_size));
	child->set_mutation_intensity(parent_1->get_mutation_intensity());
	if (!genome_size)
		return child;
	double cut_point = (double)genome_size * World::randone();
	BUG_CHECK(cut_point>=genome_size, "Cut point out of range: " << cut_point);
	for (unsigned gene_i=0; gene_i<genome_size; ++gene_i)
		if ((double)gene_i < cut_point)
			child->set_gene(gene_i, parent_1->get_gene(gene_i));
		else
			child->set_gene(gene_i, parent_2->get_gene(gene_i));
	return child;
}

double Genome::mutation_rate_scaler = 20;
