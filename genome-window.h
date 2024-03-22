/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim3
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 */

#ifndef _GENOME_WINDOW_H_
#define _GENOME_WINDOW_H_

#include <gtkmm.h>
#include "genome-draw-area.h"
#include "debug_macros.h"

/**
 * In this window the development of one or more genes is displayed.
 */
class GenomeWindow : public Gtk::Window {

 public:
	GenomeWindow(data_set_ptr new_data_set);
	void redraw();
		
 protected:
	Gtk::ScrolledWindow scrl_wind;
	GenomeDrawArea draw_area;

 private:

};

#endif // _GENOME_WINDOW_H_
