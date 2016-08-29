#include "spis.h"
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

#define NODE_HEIGHT 11
#define ARROW_WIDTH 1
#define ARROW_HEIGHT 1
#define GAP_WIDTH_H 1
#define GAP_WIDTH_V 0
#define NODE_WIDTH 25
#define CODE_WIDTH 20
#define BUTTON_WIDTH 5
#define BUTTON_HEIGHT 3


pthread_t inputThread;

void drawContent();
void inputLoop();
void runtimeLoop();
bool pointInWindow(WINDOW *win, int x, int y);
WINDOW *new_bwin(int height, int width, int starty, int startx);

WINDOW *playButton;
WINDOW *stopButton;
WINDOW *pauseButton;

int  grid_size[2] = {4,4};
std::vector<node> grid;
int  state = 0;
bool stop;

int main(int argc, char *argv[]){
  io tmp_io;
  int c,x=GAP_WIDTH_H, y=0, id;
  int max_x, max_y;
  node *tmp_node;
  int nID=0;
  WINDOW *menu;
  std::ifstream file;
  /*    if(argc!=2){
	printf("No file specified! Exiting.\n");
	return 1;
	}*/
  file.open(argv[1]);
  
  /*if(!file.is_open()){
    printf("Couldn't open file! Exiting.\n");
    return 1;
    }*/
  initscr();
  start_color();
  signal(SIGWINCH, NULL);
  getmaxyx(stdscr, max_y, max_x);
  y=0;
  refresh();
  for(int i=0;i<grid_size[0]; i++){
    x=GAP_WIDTH_H;
    for(int j=0;j<grid_size[1];j++){
      free(tmp_node);
      tmp_node = new node(nID);
      nID++;
      tmp_node->w_main=new_bwin(NODE_HEIGHT, NODE_WIDTH, y, x);
      tmp_node->w_code=newwin(NODE_HEIGHT - 2, CODE_WIDTH - 2, y + 1 , x + 1);
      new_bwin(NODE_HEIGHT, CODE_WIDTH, y, x);
      tmp_node->w_reg =newwin(NODE_HEIGHT-2,NODE_WIDTH-CODE_WIDTH-2, y+1, x+CODE_WIDTH+1);
      grid.push_back(*tmp_node);

      wprintw(tmp_node->w_reg, "ACC%d\nBAK%d", tmp_node->acc, tmp_node->bak);
      wrefresh(tmp_node->w_reg);
      x=x+NODE_WIDTH+ARROW_WIDTH+2*GAP_WIDTH_H;
      refresh();
            
      if((int) i<grid_size[0]){
	tmp_node->arrows[0]=new arrow(nID, nID-grid_size[0]);
	tmp_node->arrows[0]->win=newwin(ARROW_HEIGHT,ARROW_WIDTH,y+NODE_HEIGHT+GAP_WIDTH_V,x+NODE_WIDTH/2);
      }
      if((i%grid_size[0])!=0){
	tmp_node->arrows[3]=new arrow(nID, nID-1);
	tmp_node->arrows[3]->win=newwin(ARROW_HEIGHT,ARROW_WIDTH,y+NODE_HEIGHT/2,x-GAP_WIDTH_H-NODE_WIDTH);
            }
      if((i/grid_size[0]-1)<grid_size[1]){
	tmp_node->arrows[2]=new arrow(nID, nID+grid_size[0]);
	tmp_node->arrows[2]->win=newwin(ARROW_HEIGHT,ARROW_WIDTH,y+NODE_HEIGHT/2,x-GAP_WIDTH_H-NODE_WIDTH);
            }
      if(((i+1)%grid_size[0])!=0){
	tmp_node->arrows[1]=new arrow(nID, nID+1);
                tmp_node->arrows[1]->win=newwin(ARROW_HEIGHT,ARROW_WIDTH,y+NODE_HEIGHT/2,x-GAP_WIDTH_H-NODE_WIDTH);
      }
    }
    y=y+(NODE_HEIGHT+2*GAP_WIDTH_V+ARROW_WIDTH);
  }

  x=GAP_WIDTH_H;
  playButton = newwin(BUTTON_HEIGHT, BUTTON_WIDTH, y, x);
  init_pair(2, COLOR_BLACK, COLOR_GREEN);
  wbkgd(playButton, COLOR_PAIR(2));
  wrefresh(playButton);
  x += BUTTON_WIDTH + GAP_WIDTH_H;
  
  stopButton = newwin(BUTTON_HEIGHT, BUTTON_WIDTH, y, x);
  init_pair(3, COLOR_BLACK, COLOR_RED);
  wbkgd(stopButton, COLOR_PAIR(3));
  wrefresh(stopButton);
  
  x += BUTTON_WIDTH + GAP_WIDTH_H;
  pauseButton = newwin(BUTTON_HEIGHT, BUTTON_WIDTH, y, x);
  init_pair(4, COLOR_BLACK, COLOR_YELLOW);
  wbkgd(pauseButton, COLOR_PAIR(4));
  wrefresh(pauseButton);
 
  if(get_code(&file, grid)){
    printf("%d\n", get_code(&file, grid));
    endwin();
    clear();
    return 1;
  }

  for(int i=0; i < grid.size(); i++){
		if(grid[i].inputCode.size() == 0)
				grid[i].inputCode.push_back("");
  }

  drawContent();
  state = 1;

  noecho();    //no russian
	cbreak();
	mousemask(ALL_MOUSE_EVENTS, NULL);
	keypad(stdscr, TRUE);

  while (state != 0) {
  	switch(state) {
  		case 1:
  			inputLoop();
  			break;
  		case 2:
  			runtimeLoop();
  			break;
  		default:
  			state = 1;
  	}
  }
  inputs.push_back(tmp_io);
  inputs[0].arr = new arrow(-1, 0);
  grid[0].arrows[0]=inputs[0].arr;
  outputs.push_back(tmp_io);
  grid.back().arrows[2]= new arrow(-2, grid.size()-1);
  outputs[0].arr=grid.back().arrows[2];
  inputs[0].get();
  outputs[0].put();
  endwin();
  return 0;
}

// DRAWING

void drawNode(int nodeIndex) {
  node *tmp_node =&grid[nodeIndex];
  int beg_x, beg_y;
  getbegyx(tmp_node->w_code, beg_y, beg_x);
  
  werase(tmp_node->w_code);
  for(int y = 0; y < tmp_node->inputCode.size(); y++) {
    if(y==grid[nodeIndex].pc&&state==2)
      continue;
      else
      mvwprintw(tmp_node->w_code, y, 0, tmp_node->inputCode[y].c_str());
  }
  wrefresh(tmp_node->w_code);
    if(state==2){
    tmp_node=&grid[nodeIndex];
    start_color();
    tmp_node->w_highlight = newwin(1, CODE_WIDTH, beg_y+tmp_node->pc, beg_x-1);
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    wattron(tmp_node->w_highlight,COLOR_PAIR(1));
    wbkgd(tmp_node->w_highlight, COLOR_PAIR(1));
    mvwprintw(tmp_node->w_highlight, 0, 0, tmp_node->inputCode[tmp_node->pc].c_str());
    wrefresh(tmp_node->w_highlight);
    }
}

void drawContent() {
  for(int i = 0; i < grid.size(); i++) {
    drawNode(i);
  }
}

// INPUT AND RUNTIME
void inputLoop() {
	MEVENT event;

	int selectedNode = 0;
	int selectedLine = 0;
	int selectedIndex = 0;

	curs_set(1);

	int y, x = 0;
	getbegyx(grid[selectedNode].w_code, y, x);

	move(y, x);

	while(true) {
	  int input = getch();
		if ((input >= 65 && input <= 90) || (input >= 97 && input <= 122)
				|| (input >= 48 && input <= 57) || input == 44
				|| input == 32) {
			if (input >= 97) {
				input -= 32;
			}

			if (grid[selectedNode].inputCode[selectedLine].length() < 17) {
				grid[selectedNode].inputChar(selectedLine, selectedIndex, static_cast<char>(input));
				selectedIndex++;
				x++;
				move(y, x);
				drawNode(selectedNode);
			}
		} else if (input == KEY_ENTER || input == 11 || input == 10) {
			if (grid[selectedNode].inputCode.size() < NODE_HEIGHT - 2) {
				grid[selectedNode].newLine(selectedLine, selectedIndex);
				x -= selectedIndex;
				y++;
				selectedIndex = 0;
				selectedLine++;
				move(y, x);
				drawNode(selectedNode);
			}
		} else if(input == KEY_BACKSPACE || input == 127) {
			// BACKSPACE
			int tmpLength = grid[selectedNode].inputCode[selectedLine].length();
		  int status = grid[selectedNode].backspace(selectedLine, selectedIndex);


		  switch(status) {
		  case 1:
		  	selectedIndex--;
		  	x--;
		  	break;
		  case 2:
		  	selectedLine--;
		  	y--;
				selectedIndex = grid[selectedNode].inputCode[selectedLine].length() - tmpLength;
				x = getbegx(grid[selectedNode].w_code) + selectedIndex;
				break;
		  }

		  drawNode(selectedNode);
		  move(y, x);

		} else if (input == KEY_LEFT) {
			if (selectedIndex == 0) {
				if (selectedLine != 0) {
					selectedLine--;
					y--;
					selectedIndex = grid[selectedNode].inputCode[selectedLine].length();
					x = getbegx(grid[selectedNode].w_code) + selectedIndex;

					move(y, x);
				}
			} else {
				selectedIndex--;
				x--;

				move(y, x);
			}
		} else if (input == KEY_RIGHT) {
			if (selectedIndex != grid[selectedNode].inputCode[selectedLine].length()) {
				selectedIndex++;
				x++;

				move(y, x);
			} else if (selectedLine < grid[selectedNode].inputCode.size() - 1) {
				selectedLine++;
				y++;
				selectedIndex = 0;
				x = getbegx(grid[selectedNode].w_code);

				move(y, x);
			}
		} else if (input == KEY_UP) {
			if (selectedLine > 0) {
				selectedLine--;
				y--;
				if (selectedIndex > grid[selectedNode].inputCode[selectedLine].length()) {
					selectedIndex = grid[selectedNode].inputCode[selectedLine].length();
					x = getbegx(grid[selectedNode].w_code) + selectedIndex;
				}
				move(y, x);
			}
		} else if (input == KEY_DOWN) {
			if (selectedLine < grid[selectedNode].inputCode.size() - 1) {
				selectedLine++;
				y++;
				if (selectedIndex > grid[selectedNode].inputCode[selectedLine].length()) {
					selectedIndex = grid[selectedNode].inputCode[selectedLine].length();
					x = getbegx(grid[selectedNode].w_code) + selectedIndex;
				}
				move(y, x);
			}
		} else if (input == KEY_MOUSE && getmouse(&event) == OK) {
			bool pointFound = false;

		  for (int i = 0; i < grid.size(); i++) {
		    if (pointInWindow(grid[i].w_code, event.x, event.y)) {
		      int begY, begX = 0;
				  getbegyx(grid[i].w_code, begY, begX);
				  
				  selectedNode = i;
				  selectedLine = event.y - begY;
				  selectedIndex = event.x - begX;
				  
				  if (selectedLine >= grid[i].inputCode.size()) {
				    selectedLine = grid[i].inputCode.size() - 1;
				    
				  }
				  
				  if (selectedIndex >= grid[i].inputCode[selectedLine].length()) {
				    selectedIndex = grid[i].inputCode[selectedLine].length();
				  }
				  
				  y = selectedLine + begY;
				  x = selectedIndex + begX;
				  
				  move(y, x);
				  pointFound = true;
				  break;
		    }
			}

		  if (!pointFound && pointInWindow(playButton, event.x, event.y)) {
		  	state = 2;
		  	return;
		  } else if (!pointFound && pointInWindow(pauseButton, event.x, event.y)) {
		  	state = 0;
		  	return;
		  }


		} else {
			//mvwprintw(stdscr, 0, 0, to_string(input).c_str());
			//return 0;
		}
	}
}

void *runtimeInputLoop(void *ptr) {
	MEVENT event;

	curs_set(0);

	while (true) {
	  int input = getch();
	  if (getmouse(&event) == OK && pointInWindow(stopButton, event.x, event.y)) {
	    state = 1;
	    return NULL;
	  }
	}
}

void run_setup(){
  for(unsigned int i=0;i<grid.size();i++){
    grid[i].code.clear();
    grid[i].pc=0;
    if(grid[i].w_highlight){
      werase(grid[i].w_highlight);
      delwin(grid[i].w_highlight);
    }
    for(unsigned int j=0;j<grid[i].inputCode.size();j++)
      grid[i].code.push_back(grid[i].inputCode[j]);
  }
  return;
}

void runtimeLoop() {
  pthread_t *thread = new pthread_t;
  int err = pthread_create(thread, NULL, &runtimeInputLoop, NULL);
  
  if(err!=0){
    return;
  }
  run_setup();
  while (state == 2) {
    compute_tick();
    drawContent();
  }
  
  pthread_cancel(*thread);
  printf("Done");
  return;
}

// UTILLITY

WINDOW *new_bwin(int height, int width, int starty, int startx){
  WINDOW *win;
  win=newwin(height, width, starty, startx);
  box(win,0,0);
  wrefresh(win);
  return win;
}

bool pointInWindow(WINDOW *win, int x, int y) {
	int begX, begY, maxX, maxY = 0;

	getbegyx(win, begY, begX);
	if (x < begX || y < begY) {
		return false;
	}

	getmaxyx(win, maxY, maxX);
	if (x > begX + maxX || y > begY + maxY) {
		return false;
	}

	return true;
}
