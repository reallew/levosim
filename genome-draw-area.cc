/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim3
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 */

#include <gtkmm.h>

#include <cairomm/context.h>
#include <iomanip>
#include <limits>
#ifdef __INTEL_COMPILER
#include <mathimf.h>
#else
#include <math.h>
#endif 
#include "genome-draw-area.h"
#include "simulation-database.h"
#include "genome.h"

GenomeDrawArea::GenomeDrawArea(data_set_ptr new_data_set, Gtk::ScrolledWindow* new_wind) : 
	my_data_set(new_data_set),
	my_window(new_wind),
	my_adjustment(my_window->get_hadjustment()),
	stick_right(true),
	last_left_val(0.0)
{
	// This must be done for redrawing the 'moving' text.
	// After every adjustment change the whole area is redrawed.
	my_adjustment->signal_value_changed().connect(sigc::mem_fun(*this,&GenomeDrawArea::redraw)); 
}

/**
 * Calculates color spreading around the chromatic circle for an amount of numbers.
 * It sets the drawing color.
 */
void GenomeDrawArea::set_colorcycle_color(const Cairo::RefPtr<Cairo::Context>& cr, unsigned int no, unsigned int cycle_colors) {
	const int mix_colors = 3;
	if (cycle_colors>2) // UGLY: is a WORKAROUND
		cycle_colors+=2;
	unsigned int colorcycle = no%cycle_colors;
	double red, green, blue, change;
	unsigned int mix_color_snibble = cycle_colors / mix_colors;
	if (colorcycle < mix_color_snibble) {
		change = (double)colorcycle / (double)mix_color_snibble;
		red = 1.0 - change;
		green = change;
		blue = 0.0;
	} else if (colorcycle < mix_color_snibble*2) {
		change = (double)(colorcycle-mix_color_snibble) / (double)mix_color_snibble;
		red = 0.0;
		green = 1.0 - change;
		blue = change;
	} else {
		change = (double)(colorcycle-mix_color_snibble*2) / (double)mix_color_snibble;
		red = change;
		green = 0.0;
		blue = 1.0 - change;
	}
	cr->set_source_rgba(red, green, blue, 1.0);
}

void GenomeDrawArea::draw_gene(const Cairo::RefPtr<Cairo::Context>& cr, const unsigned int gene_no, unsigned int from_x, 
                               unsigned int to_x, int window_height, int left_wind_border) {
	unsigned int generation = from_x / X_STRETCH;
	unsigned int last_generation = to_x / X_STRETCH;
								   
	// Write gene title.
	int font_size = window_height / 25;
	if (font_size<8)
		font_size = 8;
	else if (font_size>16)
		font_size = 16;

	set_colorcycle_color(cr, gene_no, my_data_set->max_genome_size);

	cr->move_to(left_wind_border+4, font_size * (gene_no + 1));
	Cairo::RefPtr<Cairo::ToyFontFace> font =
		Cairo::ToyFontFace::create("Bitstream Charter",
		                           Cairo::FONT_SLANT_NORMAL,
		                           Cairo::FONT_WEIGHT_NORMAL);
	cr->set_font_face(font);
	cr->set_font_size(font_size);

	if (my_data_set->gene_names.size() <= gene_no) {
		std::stringstream ss;
		ss << my_data_set->agent_name << " Gene " << gene_no;
		cr->show_text(ss.str());
	} else {
		cr->show_text(*my_data_set->gene_names.at(gene_no));
	}

	if (generation >=  my_data_set->genomes.size())
		return;

	// Draw gene graph.
	double from_y = window_height;
   	if (my_data_set->genomes.at(generation)->is_gene(gene_no))
		from_y -= window_height - (window_height * my_data_set->genomes.at(generation)->get_gene(gene_no));
								   
	cr->move_to(from_x, from_y);

	bool visible = true;
	while ((generation < my_data_set->genomes.size()) && (generation <= last_generation)) {
		BUG_CHECK(my_data_set->highest_value <= 0.0, "Highest value below zero.");
		double normalised_gene_val;
		if (!my_data_set->genomes.at(generation)->is_gene(gene_no))
			normalised_gene_val = 0.0;
		else
			normalised_gene_val = my_data_set->genomes[generation]->get_gene(gene_no);
		if (visible) {
			if (std::isnan(normalised_gene_val)) {
				visible = false;
			} else {
				normalised_gene_val /= my_data_set->highest_value;
				BUG_CHECK(normalised_gene_val<0.0 || normalised_gene_val>1.0, 
				          "Normalised gene for draw gene graph out of range 0..1" << std::endl
				          << "Normalised gene value is " << normalised_gene_val << std::endl
				          << "Highest value is " << my_data_set->highest_value);
				cr->line_to(generation * X_STRETCH, window_height - 
				            (window_height * normalised_gene_val));
			}
		} else {
			if (!std::isnan(normalised_gene_val)) {
				visible = true;
				normalised_gene_val /= my_data_set->highest_value;
				BUG_CHECK(normalised_gene_val<0.0 || normalised_gene_val>1.0, 
				          "Normalised gene for draw gene graph out of range 0..1" << std::endl
				          << "Normalised gene value is " << normalised_gene_val << std::endl
				          << "Highest value is " << my_data_set->highest_value);
				cr->move_to(generation * X_STRETCH, window_height - 
				            (window_height * normalised_gene_val));
			}
		}
		++generation;
	}

	cr->stroke();
}

/**
 * Redraw the whole drawing area.
 * This 'invalidates' the drawing area, which motivates GTK+ to call 
 * GenomeDrawArea::on_draw
 */
void GenomeDrawArea::redraw() {
	Glib::RefPtr<Gdk::Window> window = get_window();
	if (window) {
		Gtk::Allocation allocation = get_allocation();
		window->invalidate_rect(Gdk::Rectangle(my_adjustment->get_value(), 0, 
						       my_adjustment->get_page_size()+my_adjustment->get_value(), 
						       allocation.get_height()), TRUE);		
	}
}

/**
 * Draws some light grey bars in the background of the drawing area.
 */
void GenomeDrawArea::draw_grid(const Cairo::RefPtr<Cairo::Context>& cr, unsigned int from_x, unsigned int to_x, int window_height, int window_width) {
	unsigned int x_counter;
	from_x = from_x - (from_x%(unsigned int)(10*X_STRETCH)) + 10*X_STRETCH;
	
	cr->set_source_rgba(0, 0, 0, 0.50);
	for (x_counter=from_x; x_counter < to_x; x_counter+=10*X_STRETCH) {
		double line_width;
		if (x_counter%(unsigned int)(100*X_STRETCH)) {
			line_width = 0.05;	
		} else {
			if (x_counter%(unsigned int)(1000*X_STRETCH))
				line_width = 0.2;	
			else
				line_width = 0.4;	
		}
		cr->set_line_width(line_width*X_STRETCH);	
		cr->move_to(x_counter, 0);
		cr->line_to(x_counter, window_height);
		cr->stroke();
	}

	// Draw horizontal bars.
	cr->set_line_width(0.1);
	Cairo::RefPtr<Cairo::ToyFontFace> font =
		Cairo::ToyFontFace::create("Bitstream Charter",
		                           Cairo::FONT_SLANT_NORMAL,
		                           Cairo::FONT_WEIGHT_NORMAL);
	cr->set_font_face(font);
	cr->set_font_size(10);
	
	double value_of_bar = 0.0;
	double y_steps = 30; // All 30 pixels a bar.
	double quantity_of_steps = (double)window_height / y_steps;
	double h_bar_distance = my_data_set->highest_value / quantity_of_steps;
	double y_counter = window_height;

	while (y_counter > 0) {
		cr->move_to(0, y_counter);
		cr->line_to(to_x, y_counter);

		std::stringstream ss;
		if (my_data_set->highest_value > 9)
			ss << std::setprecision(3);
		else
			ss << std::setprecision(2);
		ss << value_of_bar;

		int sidemove = ss.str().length() * 6;
		cr->move_to(window_width-sidemove, y_counter-2);
		cr->show_text(ss.str());
		
		value_of_bar += h_bar_distance;
		
		y_counter -= y_steps;
	}
	cr->stroke();
}

/**
 * This is called when the system wants the draw area redrawed.
 * Here the whole drawing is done.
 */
bool GenomeDrawArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
	if (!my_data_set->genomes.size())
		return true;
	double upper_val = my_adjustment->get_upper();
	double left_val = my_adjustment->get_value();
	double page_size = my_adjustment->get_page_size();
	double current_right_val = left_val + page_size;
	double end_rel = current_right_val / upper_val;

	if (left_val < last_left_val)
		stick_right = false;
	else if ((left_val > last_left_val) && end_rel > 0.99)
		stick_right = true;
	else if (!left_val)
		if (upper_val > my_data_set->genomes.size()*X_STRETCH)
			stick_right = true;
	last_left_val = left_val;	
	if (stick_right && (current_right_val < upper_val)) {
		my_adjustment->set_value(upper_val - page_size);
		my_adjustment->value_changed();
	}

	Glib::RefPtr<Gdk::Window> window = get_window();
	if (window) {
		set_size_request(my_data_set->genomes.size() * X_STRETCH, 20);
		Gtk::Allocation allocation = get_allocation();
	        int width = allocation.get_width() + X_STRETCH;
		int height = allocation.get_height();
		for (unsigned i=0; i<my_data_set->max_genome_size; ++i)
			draw_gene(cr, i, 0, width, height, left_val);
		draw_grid(cr, 0, width, height, current_right_val);
	}

	return true;
}
