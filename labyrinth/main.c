#include<stdio.h>
#include<string.h>
#include<stdlib.h>

/*
 * Labyrinth input
 *
 * .#...#..#.#....#
 * ..#.####..##..#b
 * .........#.#..#.
 * ....#.#....#...#
 * a##.###.####....
 * ...............
 *
 * MAX_WIDTH = 100
 * MAX_HEIGHT = 100
 *
 *
 */

#define MAX_WIDTH 100
#define MAX_HEIGHT 100

struct _Labyrinth {
	char cells[MAX_HEIGHT][MAX_WIDTH];
	int in_x;
	int in_y;
	int out_x;
	int out_y;
	int width;
	int height;
};

struct _Path {
	char* cells[MAX_HEIGHT*MAX_WIDTH];
};

typedef struct _Labyrinth Labyrinth;

typedef struct _Path Path;

int find_shortest_path(Labyrinth* l, Path** p);

int read_labyrinth(Labyrinth **l);

int print_labyrinth(Labyrinth* l);


void free_labyrinth(Labyrinth* l);


int main(int argc, char* argv[]) {
	Labyrinth *l;
	Path* p;
	if ( read_labyrinth(&l) ) {
		fprintf(stderr, "Cannot create labyrinth!\n");
		return 1;
	}
	print_labyrinth(l);
	find_shortest_path(l,&p);
	print_labyrinth(l);
	free_labyrinth(l);
	return 0;
}

int check_cell(Labyrinth *l, int r, int c, int step ) {
	switch ( l->cells[r][c] ) {
	case 1:
		return 0;
	case 2:
		return 0;
	case 3:
		return 2;
	case 0:
		return 1;
	default:
		if (-l->cells[r][c] > step ) {
			return 1;
		} else {
			return 0;
		}
	}
}

typedef int (*walker)(Labyrinth* l , int c, int r, void* data);

int cell_walk(Labyrinth* l, int c, int r, walker f, void* data) {
	int dc, dr;
	for (dc = c-1; dc < c+2; dc++ ) {
		for (dr = r-1; dr < r+2; dr++ ) {
			if ( (dc-c)*(dc-c) + (dr-r)*(dr-r) == 1
				&& dc >=0 
				&& dc < l->width 
				&& dr >=0 
				&& dr < l->height ) {

				if (!f(l, dc, dr, data)) 
					return 0;
			}
		}
	}
	return 1;
}

typedef struct {
	int* step;
	char*** nf;

} FrontWalkerData;


int front_walker(Labyrinth *l, int c, int r, void* data) {
	FrontWalkerData* fwdata = (FrontWalkerData*)data; 
	int result = check_cell(l, r, c, *fwdata->step);
	if ( result == 1 ) {
		l->cells[r][c] = - *fwdata->step;
		**(fwdata->nf) = &l->cells[r][c];
		(*fwdata->nf)++;
	} else if ( result == 2 ) {
		return 0;
	}
	return 1;
}

int update_front(char* old_front[], char* new_front[], Labyrinth* l, int* step) {
	char **of = old_front;
	char **nf = new_front;
	while (*of) {
		fprintf(stderr, "Going front %p\n", *of);
		int c = (*of - (char*)l->cells) % MAX_WIDTH;
		int r = (*of - (char*)l->cells) / MAX_WIDTH;
		FrontWalkerData fwdata;
		fwdata.step = step;
		fwdata.nf = &nf;

		if (!cell_walk(l, c, r, front_walker, &fwdata))
			return 0;
		of++;
	}
	if (nf == new_front ) 
		return 0;
	*step += 1;
	return 1;
}

int update_path(Labyrinth* l, Path* p) {
	int r,c;
	int dc, dr;
	int s = 0;
	r = l->out_y;
	c = l->out_x;



	for (dc = c-1; dc < c+2; dc++ ) {
		for (dr = r-1; dr < r+2; dr++ ) {
			if ( (dc-c)*(dc-c) + (dr-r)*(dr-r) == 1
				&& dc >=0 
				&& dc < l->width 
				&& dr >=0 
				&& dr < l->height ) {
				
				/* TODO */
			}
		}
	}
}



int find_shortest_path(Labyrinth* l, Path** p) {
	int r,c;
	int step;
	char* front[2][MAX_WIDTH*MAX_HEIGHT];
	char** old_front = &front[0][0];
	char** new_front = &front[1][0];
	*p = (Path*) malloc(sizeof(Path));
	
	if ( ! *p ) {
		return -1;
	}

	memset(*p, 0, sizeof(Path));
	memset(front, 0, sizeof(front));
	fprintf(stderr, "front and path allocated\n");
	old_front[0] = &l->cells[l->in_y][l->in_x];
	step = 1;

	while ( update_front(old_front, new_front, l, &step) ) {
		fprintf(stderr, "Front updated with step %d\n", step);
		char **f = old_front;
		old_front = new_front;
		new_front = f;
		memset(new_front, 0, MAX_WIDTH*MAX_HEIGHT);
	}
	//update_path(l, p);
	return 0;	
}

int read_labyrinth(Labyrinth **l) {
	int row = 0;
	char buffer[MAX_WIDTH+1];
	
	*l = (Labyrinth*) malloc(sizeof(Labyrinth));
	
	if ( !(*l) ) {
		return -1;
	}

	memset(*l, 0, sizeof(Labyrinth));

	while ( fgets(buffer, MAX_WIDTH, stdin) && row < MAX_HEIGHT ) {

		char* s = buffer;
		int col = 0;
		
		while (*s && (*s) != '\n' && col < MAX_WIDTH) {
			if (*s == '.') {
				(*l)->cells[row][col] = 0;
			} else if ( *s == '#' ) {
				(*l)->cells[row][col] = 1;
			} else if ( *s == 'a' || *s == 'A' ) {
				(*l)->in_x = col;
				(*l)->in_y = row;
				(*l)->cells[row][col] = 2;
			} else if ( *s == 'b' || *s == 'B' ) {
				(*l)->out_x = col;
				(*l)->out_y = row;
				(*l)->cells[row][col] = 3;
			} else {
				(*l)->cells[row][col] = 0;
			}	
			s++;
			col++;
		}
		if ( col > (*l)->width )
			(*l)->width = col;
		row++;

		printf("reading buffer: '%s' at row = %d\n", buffer, row);
	}
	(*l)->height = row;
	return 0;
}


int print_labyrinth(Labyrinth* l) {
	int r,c;
	printf("width = %d, height = %d\n", l->width, l->height);
	for(r = 0; r < l->height; r++) {
		for (c = 0; c < l->width; c++ ) {
			switch(l->cells[r][c]) {
			case 0:
				printf("...");
				break;
			case 1:
				printf("###");
				break;
			case 2:
				printf(" A ");
				break;
			case 3:
				printf(" B ");
				break;
			default:
				if (l->cells[r][c] < 0) {
					printf(" %02d", - l->cells[r][c]);
				} else {
					printf("???");
				}
				break;
			}
		}
		printf("\n");
	}
	printf("\n");
	return 0;
}


void free_labyrinth(Labyrinth* l){
	if (l) {
		free(l);
	}
}

