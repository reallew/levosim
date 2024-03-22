/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim3
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 * This file contains two classes: the complete definition of OptWindowButton and the
 * interface of Mainwindow.
 * OptWindowButton is a minor class which belongs to Mainwindow, this is why it hangs
 * around in the same file.
 *
 */

#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <glibmm/thread.h>
#include <gtkmm.h>
#include <vector>
#include "simulation-database.h"
#include "worldhandler.h"
#include "genome-window.h"
#include "debug_macros.h"

/**
 * A special button that owns a window for displaying genomes.
 */
class OptWindowButton : public Gtk::Button {
public:
	OptWindowButton(data_set_ptr my_data_set) : 
		Gtk::Button(my_data_set->title) {
		my_window = new GenomeWindow(my_data_set);
	}
	~OptWindowButton() {
		delete my_window;
	}
	
	void redraw_window() {
		if (my_window->get_visible())
			my_window->redraw();
	}
protected:
	virtual void on_clicked() {
		if (window_mutex.trylock()) {
			Gtk::Main::run(*my_window);
			window_mutex.unlock();
		}
	}
	
	Glib::Mutex window_mutex;
	GenomeWindow* my_window;
};


using button_ptr = std::unique_ptr<OptWindowButton>;
using button_container = std::vector<button_ptr>;

typedef std::shared_ptr<Gtk::Frame> frame_ptr;
typedef std::list<frame_ptr> frame_container;

/**
 * Very small wrapper class for HScale. It enhances HScale to store a pointer
 * to 'his' world parameter set.
 */
class ParamScale : public Gtk::HScale {
public:
	ParamScale(double a, double b, double c) : Gtk::HScale(a,b,c) {}
	world_parameter_ptr sim_parameter;
};

typedef std::shared_ptr<ParamScale> scaler_ptr;
typedef std::list<scaler_ptr> scaler_container;


/**
 * The LEvoSim user interface-window. From this other windows are opened and
 * controlled. It interact via a Worldhandler with the simulation and has a
 * SimulationDatabase object to store statistical data.
 */
class Mainwindow : public Gtk::Window {
	
public:
	Mainwindow(worldhandler_ptr new_world_handler);
	~Mainwindow();
	
protected:
	Gtk::Button start_button, stop_button, reset_button, save_button;
	Gtk::HButtonBox* main_operation_box;
	Gtk::Frame cockpit_frame, windows_frame, stat_frame;
	Gtk::Label gen_text, gen_no;
	Gtk::Label running_info;
	void on_start_clicked();
	void on_stop_clicked();
	void on_reset_clicked();
	void on_save_clicked();
	void paint_stats();
	void run_world_loop();
	void parameter_change(scaler_ptr wpar);
	
private:
	void init_scalers();
	void init_buttons();
	frame_container scaler_frames;
	scaler_container scalers;
	void wait_until_idle();
	worldhandler_ptr world_handler;
	Glib::Thread* simulation_thread;
	bool sim_should_run;
	bool program_must_end;
	bool simulation_is_idle;
	Glib::Dispatcher repaint_signal;
	Glib::Cond start_sim_condition;
	Glib::Mutex start_sim_mutex;
	simulation_database_ptr sim_db;
	Gtk::Table main_table, stat_table, scaler_table, windows_button_table;
	button_container stat_windows;
	Gtk::ScrolledWindow scaler_scrollbox, button_scrollbox;
};

#endif // _MAINWINDOW_H_
