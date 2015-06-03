
gridserver : grid server.c colors.h colors.c
	gcc -o gridserver options.c grid.c network.c server.c polygon.c queue.c colors.c energy.c -Wall `sdl-config --libs --cflags` -framework OpenGL -std=gnu99 -g

grid : grid.c vector.h polygon.c polygon.h queue.c queue.h energy.h energy.c main.c grid.h network.h network.c options.h options.c
	gcc -o grid main.c grid.c polygon.c colors.c queue.c energy.c network.c options.c -Wall `sdl-config --libs --cflags`  -framework OpenGL -std=gnu99 -g 

