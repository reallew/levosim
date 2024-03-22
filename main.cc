/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.cc
 * Copyright (C) Lew Palm 2019 <lp@lew-palm.de>
 * 
 */


/**
 *
 * @author Lew Palm 
 * @mainpage LEvoSim Reference Manual
 * @image html rose.jpg "A Rose" width=301px
 * @image latex rose.pdf "A Rose" width=10cm
 */

#include "mainwindow.h"
#include "bushworldhandler.h"

/**
 * Main function of LEvoSim evolution simulation program.
 */
int main(int argc, char *argv[]) {
	if(!Glib::thread_supported()) 
		Glib::thread_init();

	Gtk::Main kit(argc, argv);

	worldhandler_ptr rosebush_handler = worldhandler_ptr(new Bushworldhandler());

	Mainwindow main_window(rosebush_handler);

	Gtk::Main::run(main_window);

	return 0;
}
