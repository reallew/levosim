/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * LEvoSim3
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 */

#include <iostream>
#include <gtkmm.h>
#include <glibmm/thread.h>
#include <sigc++/functors/slot.h>
#include <unistd.h>
#include "mainwindow.h"
#include "worldhandler.h"
#include "simulation-database.h"
#include "genome-window.h"


/**
 * Configures the windows and starts the simulation threads.
 */
Mainwindow::Mainwindow(worldhandler_ptr new_world_handler) : 
	start_button("Run"),
	stop_button("Stop"),
	reset_button("Reset"), 
	save_button("Save data to file"),
	cockpit_frame("Cockpit"),
	windows_frame("Statistic Windows"),
	stat_frame("World Parameters"),
	gen_text("Generation:"),
	gen_no("0"),
	running_info("stopped"),
	world_handler(new_world_handler),
	sim_should_run(false),
	program_must_end(false),
	simulation_is_idle(true),
	main_table(4, 1),
	stat_table(1, 3, false),
	windows_button_table(1, 1)
{
	set_title("LEvoSim v1.1");
	set_size_request(280, 600); 
	sim_db = world_handler->create_database();
	stat_windows = button_container(sim_db->data_sets()->size());
	set_resizable(false);
	add(main_table);
	set_border_width(4);

	// Signal handlers for cockpit and save buttons:
	start_button.signal_clicked().connect(sigc::mem_fun(*this, 
	                                                    &Mainwindow::on_start_clicked));
	stop_button.signal_clicked().connect(sigc::mem_fun(*this, 
	                                                   &Mainwindow::on_stop_clicked));
	reset_button.signal_clicked().connect(sigc::mem_fun(*this, 
	                                                    &Mainwindow::on_reset_clicked));
	save_button.signal_clicked().connect(sigc::mem_fun(*this, 
	                                                   &Mainwindow::on_save_clicked));

	// main_operation_box contains the buttons Start, Stop and Reset. It is surrounded
	// by a frame called cockpit_frame.
	main_operation_box = Gtk::manage( new Gtk::HButtonBox() );
	main_operation_box->add(start_button);
	main_operation_box->add(stop_button);
	main_operation_box->add(reset_button);
	cockpit_frame.add(*main_operation_box);
	
	main_table.attach(cockpit_frame, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL, 4, 4);

	// The windows_frame dadd(*main_operation_box);oes not contain windows, it stores buttons for opening the
	// statistical windows.
	init_buttons(); // Initialize the buttons for the statistical windows.
	button_scrollbox.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
	button_scrollbox.add(windows_button_table);
	button_scrollbox.set_size_request(240, 220); 
	windows_frame.add(button_scrollbox);
	main_table.attach(windows_frame, 0, 1, 2, 3, Gtk::FILL, Gtk::FILL, 4, 4);

	// stat_table includes all world-parameter changing scalers and displays the 
	// actual generation number:
	stat_table.attach(gen_text, 0, 1, 0, 1);
	stat_table.attach(gen_no, 1, 2, 0, 1);
	stat_table.attach(running_info, 2, 3, 0, 1);
	init_scalers(); // makes scaler_table
	main_table.attach(stat_table, 0, 1, 1, 2, Gtk::FILL, Gtk::EXPAND, 4, 4);
	scaler_scrollbox.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
	scaler_scrollbox.set_size_request(240, 220); 
	scaler_scrollbox.add(scaler_table);
	stat_frame.add(scaler_scrollbox);
	main_table.attach(stat_frame, 0, 1, 3, 4, Gtk::FILL, Gtk::EXPAND, 4, 4);

	show_all();

	// If this signal is called everything must be repainted.
	repaint_signal.connect(sigc::mem_fun(*this, &Mainwindow::paint_stats));

	// There are two threads. simulation_thread handles the simulation (and has some sub-
	// threads maybe), and the main thread cares about the GUI.
	simulation_thread = Glib::Thread::create(sigc::mem_fun(this, &Mainwindow::run_world_loop), 
	                                     	 true);
}

/**
 * Creates the buttons for opening windows to display database data.
 * The windows are created by the buttons, Mainwindow must not care about it.
 */
void Mainwindow::init_buttons() {
	auto pos = 0;
	for (auto& but: stat_windows) {
		but = button_ptr(new OptWindowButton(sim_db->data_sets()->at(pos)));
		windows_button_table.attach(*but, 0, 1, pos, pos+1, Gtk::FILL, Gtk::FILL, 4, 4);
		++pos;
	}
	windows_button_table.attach(save_button, 0, 1, pos, pos+1, Gtk::EXPAND, Gtk::FILL, 4, 4);
}

/**
 * Gets the changable world parameters from the world_handler and connects them with
 * GUI scalers.
 */
void Mainwindow::init_scalers() {
	unsigned int rows = stat_table.property_n_rows();
	for (auto& wp: *world_handler->get_parameters()) {
		frame_ptr new_frame = frame_ptr(new Gtk::Frame(wp.first));
		scaler_ptr new_scaler = scaler_ptr(new ParamScale(wp.second->min_val, 
								  wp.second->max_val, 
								  wp.second->steps));
		new_scaler->set_value(wp.second->val);
		new_scaler->sim_parameter = wp.second;
		new_frame->add(*new_scaler);
		new_scaler->signal_value_changed().connect(sigc::bind(sigc::mem_fun(*this, &Mainwindow::parameter_change),
								      new_scaler));
		
		scaler_table.attach(*new_frame, 0, 1, rows, rows+1);
		++rows;
		scaler_frames.push_back(new_frame);
		scalers.push_back(new_scaler);
	}
}

/**
 * This is called when the user changes a scaler.
 * It writes the changes to the corresponding world_handler parameters.
 */
void Mainwindow::parameter_change(scaler_ptr chng_scaler) {
	chng_scaler->sim_parameter->val = chng_scaler->get_value();
	chng_scaler->sim_parameter->dirty = true; // This means: Note: value has changed.
}

/**
 * The destructor waits for the simulation thread to stop.
 */
Mainwindow::~Mainwindow() {
	// Do this all to get out of run_world_loop loop:
	sim_should_run = true; 
	program_must_end = true;
	start_sim_condition.signal(); 
	simulation_thread->join();
}

/**
 * Refreshes the visual information in the main window.
 * It should be called every time after a significant data change.
 */
void Mainwindow::paint_stats() {
	std::stringstream ss;
	ss << world_handler->get_generation();
	gen_no.set_text(ss.str());
	if (simulation_is_idle && !sim_should_run)
		running_info.set_text("stopped");
	else
		running_info.set_text("running");
	for (auto& but: stat_windows)
		but->redraw_window();
}

/**
 * This method is run in a sperate thread. It calculates the simulation.
 */
void Mainwindow::run_world_loop() {
	while (!program_must_end) {
		while (!sim_should_run) {
			Glib::Mutex::Lock lock(start_sim_mutex);
			start_sim_condition.wait(start_sim_mutex);
		}
		if (program_must_end)
			return;
		if (simulation_is_idle) {
			simulation_is_idle = false;
			repaint_signal();
		}
		world_handler->apply_changes();
		world_handler->run_one_generation();
		sim_db->collect(world_handler->get_world());
		simulation_is_idle = true;
		repaint_signal();
	}
}

/**
 * Starts the simulation. 
 */
void Mainwindow::on_start_clicked() {
	sim_should_run = true;
	start_sim_condition.signal();
}

/**
 * Stops the simulation's run.
 */
void Mainwindow::on_stop_clicked() {
	sim_should_run = false;
}

/**
 * Opens a window -- a file chooser dialog -- and saves all database-data to the chosen
 * file, which will be overwritten.
 */
void Mainwindow::on_save_clicked() {
	bool sim_was_running = sim_should_run;
	wait_until_idle();
	
	Gtk::FileChooserDialog dialog("Please name a file for data saving",
	                              Gtk::FILE_CHOOSER_ACTION_SAVE);
	dialog.set_transient_for(*this);
	dialog.set_create_folders();
	dialog.set_current_name(".csv");

	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

	/*
	Gtk::FileFilter filter_csv;
	filter_csv.set_name("csv files");
	filter_csv.add_mime_type("text/csv");
	dialog.add_filter(filter_csv);

	Gtk::FileFilter filter_any;
	filter_any.set_name("Any files");
	filter_any.add_pattern("*");
	dialog.add_filter(filter_any);
	*/

	if (!(dialog.run() == Gtk::RESPONSE_OK))
		return;
	
	auto save_file = dialog.get_file();
	Glib::RefPtr<Gio::Cancellable> iostack;
	auto save_stream = save_file->replace(iostack);
	sim_db->write_db(save_stream);
	save_stream->close();
	if (sim_was_running)
		on_start_clicked();
}

/**
 * Tells the simulation to stop and waits until this is done.
 */
void Mainwindow::wait_until_idle() {
	on_stop_clicked();
	// Spinlock until simulation stops.
	while (!simulation_is_idle)
		usleep(30);
}

/**
 * Re-initialiases the simulation. All calculated data is lost.
 */
void Mainwindow::on_reset_clicked() {
	bool sim_was_running = sim_should_run;
	wait_until_idle();
	world_handler->init_world();
	sim_db->clear();
	paint_stats();
	if (sim_was_running)
		on_start_clicked();
}
