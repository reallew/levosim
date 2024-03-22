BIN = levosim
OBJS = agent.o bushworld-database.o fly.o genome-draw-area.o insect.o mainwindow.o wasp.o worldhandler.o bushworld.o bushworldhandler.o genome.o genome-window.o main.o simulation-database.o world.o
CC = g++
CFLAGS = -Wall -O2 -fopenmp -march=native -mtune=native -fmax-errors=1
LIBSUSED = `pkg-config gtkmm-3.0 --cflags --libs gthread-2.0`
LDFLAGS = -s

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BIN) $(OBJS) $(LIBSUSED)

main.o: main.cc
	$(CC) $(CFLAGS) -o main.o -c main.cc $(LIBSUSED)

agent.o: agent.cc
	$(CC) $(CFLAGS) -o agent.o -c agent.cc $(LIBSUSED)

bushworld-database.o: bushworld-database.cc
	$(CC) $(CFLAGS) -o bushworld-database.o -c bushworld-database.cc $(LIBSUSED)

fly.o: fly.cc
	$(CC) $(CFLAGS) -o fly.o -c fly.cc $(LIBSUSED)

genome-draw-area.o: genome-draw-area.cc
	$(CC) $(CFLAGS) -o genome-draw-area.o -c genome-draw-area.cc $(LIBSUSED)

insect.o: insect.cc
	$(CC) $(CFLAGS) -o insect.o -c insect.cc $(LIBSUSED)

mainwindow.o: mainwindow.cc
	$(CC) $(CFLAGS) -o mainwindow.o -c mainwindow.cc $(LIBSUSED)

wasp.o: wasp.cc
	$(CC) $(CFLAGS) -o wasp.o -c wasp.cc $(LIBSUSED)

worldhandler.o: worldhandler.cc
	$(CC) $(CFLAGS) -o worldhandler.o -c worldhandler.cc $(LIBSUSED)

bushworld.o: bushworld.cc
	$(CC) $(CFLAGS) -o bushworld.o -c bushworld.cc $(LIBSUSED)

bushworldhandler.o: bushworldhandler.cc
	$(CC) $(CFLAGS) -o bushworldhandler.o -c bushworldhandler.cc $(LIBSUSED)

genome.o: genome.cc
	$(CC) $(CFLAGS) -o genome.o -c genome.cc $(LIBSUSED)

genome-window.o: genome-window.cc
	$(CC) $(CFLAGS) -o genome-window.o -c genome-window.cc $(LIBSUSED)

simulation-database.o: simulation-database.cc
	$(CC) $(CFLAGS) -o simulation-database.o -c simulation-database.cc $(LIBSUSED)

world.o: world.cc
	$(CC) $(CFLAGS) -o world.o -c world.cc $(LIBSUSED)

clean:
	rm -f $(BIN) $(OBJS)
