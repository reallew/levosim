/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim3
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 */

#include "genome-window.h"
#include "genome-draw-area.h"
#include "simulation-database.h"

GenomeWindow::GenomeWindow(data_set_ptr new_data_set) :
	draw_area(new_data_set, &scrl_wind)
{
	//	scrl_wind.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_NEVER);
	scrl_wind.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_NEVER);
	scrl_wind.add(draw_area);
	add(scrl_wind);
	draw_area.show();
	scrl_wind.show();
	set_title(new_data_set->title);
	set_default_size(600, 480); 
	set_size_request(200, 80); 
}


void GenomeWindow::redraw() {
	draw_area.redraw();
}
