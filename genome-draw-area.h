/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim3
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 */

#ifndef _GENOME_DRAW_AREA_H_
#define _GENOME_DRAW_AREA_H_

#include <gtkmm.h>
#include "simulation-database.h"
#include "debug_macros.h"

/**
 * This class renders data from a SimulationDatabase (-childclass) on a statistical chart.
 * It must be part of a Gdk::Window.
 */
class GenomeDrawArea : public Gtk::DrawingArea {
public:
	GenomeDrawArea(data_set_ptr new_data_set, Gtk::ScrolledWindow* new_wind);
	void redraw();

protected:
	bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
	void draw_gene(const Cairo::RefPtr<Cairo::Context>& cr, const unsigned int gene_no, unsigned int first_generation, 
		       unsigned int last_generation, int height, int left_wind_border);
	void draw_grid(const Cairo::RefPtr<Cairo::Context>& cr, unsigned int from_x, unsigned int to_x, int window_height, 
		       int right_wind_border);
	std::string gene_names;

private:
	data_set_ptr my_data_set;
	static constexpr double X_STRETCH = 4.0;
	Gtk::ScrolledWindow* my_window;
	Glib::RefPtr<Gtk::Adjustment> my_adjustment;
	void set_colorcycle_color(const Cairo::RefPtr<Cairo::Context>& cr, unsigned int no, unsigned int colors_quantity);
	bool stick_right;
	double last_left_val;
};

#endif // _GENOME_DRAW_AREA_H_
