#include "spis.h"
#include <pthread.h>
#include <locale.h>

pthread_t inputThread;

string makeThreeDigit(int n);

WINDOW *playButton;
WINDOW *stopButton;
WINDOW *pauseButton;

WINDOW *delayValueWindow;
WINDOW *increaseDelayButton;
WINDOW *decreaseDelayButton;

int  grid_size[2] = {4,4};
std::vector<node> grid;
vector<arrowType *> gridArrows;
int  state = OFF;
bool stop;

int selectedNode = -1;
int selectedLine = 0;
int selectedIndex = 0;

int startX;
int startY;

void initSystem(int begX, int begY) {
	startX = begX;
	startY = begY;
  int c, x, y, id;
  int max_x, max_y;
  int nID=0;
  WINDOW *menu;

  getmaxyx(stdscr, max_y, max_x);

  y = startY;
  for(int i=0;i<grid_size[0]; i++){
    x = startX + GAP_WIDTH_H;
    for(int j=0;j<grid_size[1];j++){
      grid.push_back(node(nID));
      grid[nID].w_main=new_bwin(NODE_HEIGHT, NODE_WIDTH, y, x);
      grid[nID].w_reg =newwin(NODE_HEIGHT-2,NODE_WIDTH-CODE_WIDTH-2, y+1, x+CODE_WIDTH+1);
      grid[nID].w_code=newwin(NODE_HEIGHT - 2, CODE_WIDTH - 2, y + 1 , x + 1);
      grid[nID].w_divider=new_bwin(NODE_HEIGHT, CODE_WIDTH, y, x);

      wprintw(grid[nID].w_reg, " ACC%d\n BAK%d", grid[nID].acc, grid[nID].bak);
      wrefresh(grid[nID].w_reg);
      refresh();

      if (i != 0) {
      	grid[nID].arrows[0] = grid[nID - grid_size[1]].arrows[2];
      }

      if (j != grid_size[1] - 1) {
      	grid[nID].arrows[1] = new arrow(nID, nID + 1, y + floor(NODE_HEIGHT / 2) - floor(ARROW_H_HEIGHT / 2), x + NODE_WIDTH + GAP_WIDTH_H, false);
      	gridArrows.push_back(grid[nID].arrows[1]);
      }

      if (i != grid_size[0] - 1) {
      	grid[nID].arrows[2] = new arrow(nID, nID + grid_size[0], y + NODE_HEIGHT + GAP_WIDTH_V, x + floor(NODE_WIDTH / 2) - floor(ARROW_V_WIDTH / 2), true);
	gridArrows.push_back(grid[nID].arrows[2]);
      }

      if (j != 0) {
      	grid[nID].arrows[3] = grid[nID - 1].arrows[1];
      }
      nID++;
      x = x + NODE_WIDTH + ARROW_H_WIDTH + 2 * GAP_WIDTH_H;
    }
    y=y+(NODE_HEIGHT+2*GAP_WIDTH_V+ARROW_V_HEIGHT);
  }

  x = startX + GAP_WIDTH_H;
  playButton = newwin(BUTTON_HEIGHT, BUTTON_WIDTH, y, x);
  wbkgd(playButton, COLOR_PAIR(2));
  wrefresh(playButton);
  x += BUTTON_WIDTH + GAP_WIDTH_H;

  stopButton = newwin(BUTTON_HEIGHT, BUTTON_WIDTH, y, x);
  wbkgd(stopButton, COLOR_PAIR(3));
  wrefresh(stopButton);

  x += BUTTON_WIDTH + GAP_WIDTH_H;
  pauseButton = newwin(BUTTON_HEIGHT, BUTTON_WIDTH, y, x);
  wbkgd(pauseButton, COLOR_PAIR(4));
  wrefresh(pauseButton);


  x += BUTTON_WIDTH + GAP_WIDTH_H * 2;
	decreaseDelayButton = newwin(BUTTON_HEIGHT, BUTTON_WIDTH, y, x);
	wbkgd(decreaseDelayButton, COLOR_PAIR(1));
	mvwprintw(decreaseDelayButton, 1, 2, "–");
	wrefresh(decreaseDelayButton);
	
  x += BUTTON_WIDTH + GAP_WIDTH_H;
  increaseDelayButton = newwin(BUTTON_HEIGHT, BUTTON_WIDTH, y, x);
  wbkgd(increaseDelayButton, COLOR_PAIR(1));
	mvwprintw(increaseDelayButton, 1, 2, "+");
  wrefresh(increaseDelayButton);

  x += BUTTON_WIDTH + GAP_WIDTH_H;
  delayValueWindow = newwin(1, 5, y + 1, x);
  mvwprintw(delayValueWindow, 0, 0, intToString(tickDelay).c_str());
  wrefresh(delayValueWindow);
  
  for(int i=0; i < grid.size(); i++){
		if(grid[i].inputCode.size() == 0)
			grid[i].inputCode.push_back("");
  }

  refresh();

  return;
}

// DRAWING

void drawCode(int nodeIndex) {
  node *tmp_node =&grid[nodeIndex];
  int beg_x, beg_y;
  getbegyx(tmp_node->w_code, beg_y, beg_x);
  if(state==RUNNING){
      drawHighlight(nodeIndex);
  }
  else{
    werase(tmp_node->w_code);
    for(int y = 0; y < tmp_node->inputCode.size(); y++)
      mvwprintw(tmp_node->w_code, y, 0, tmp_node->inputCode[y].c_str());
    wrefresh(tmp_node->w_code);
  }
  return;
}
void drawHighlight(int nodeIndex){
  node *tmp_node =&grid[nodeIndex];
  int beg_x, beg_y;
  if(tmp_node->pc==0){
    mvwprintw(tmp_node->w_code, tmp_node->inputCode.size()-1, 0, tmp_node->inputCode[tmp_node->inputCode.size()-1].c_str());
  }
  else{
    mvwprintw(tmp_node->w_code, tmp_node->pc-1, 0, tmp_node->inputCode[tmp_node->pc-1].c_str());
  }
  getbegyx(tmp_node->w_code, beg_y, beg_x);
  if(tmp_node->w_highlight){
    //werase(tmp_node->w_highlight);
    delwin(tmp_node->w_highlight);
  }
  tmp_node=&grid[nodeIndex];
  //start_color();
  tmp_node->w_highlight = newwin(1, CODE_WIDTH - 2, beg_y+tmp_node->pc, beg_x);
  //init_pair(1, COLOR_BLACK, COLOR_WHITE);
  wattron(tmp_node->w_highlight,COLOR_PAIR(1));
  wbkgd(tmp_node->w_highlight, COLOR_PAIR(1));
  wbkgd(tmp_node->w_code, COLOR_PAIR(5));
  mvwprintw(tmp_node->w_highlight, 0, 0, tmp_node->inputCode[tmp_node->pc].c_str());
  wrefresh(tmp_node->w_code);
  wrefresh(tmp_node->w_highlight);
}

void updateReg(int nodeIndex) {
  node *tmp_node = &grid[nodeIndex];

  mvwprintw(tmp_node->w_reg, 1, 0, makeThreeDigit(tmp_node->acc).c_str());
  mvwprintw(tmp_node->w_reg, 3, 0, makeThreeDigit(tmp_node->bak).c_str());
  wrefresh(tmp_node->w_reg);
}

void updateArrows() {
  for (int i = 0; i < gridArrows.size(); i++)
  	gridArrows[i]->updateValues();
}

void drawSystemContent() {
  for(int i = 0; i < grid.size(); i++) {
    drawCode(i);
    updateReg(i);
  }
  updateArrows();
}

// INPUT AND RUNTIME
bool systemInput(int input, MEVENT event) {
	if ((input >= 65 && input <= 90) || (input >= 97 && input <= 122)
			|| (input >= 48 && input <= 57) || input == 44
			|| input == 32 || input == 58 || input == 45) {
		if (input >= 97) {
			input -= 32;
		}
		if(selectedNode<0) {
			setCursor(false);
		  return true;
		} else if (grid[selectedNode].inputCode[selectedLine].length() < CODE_WIDTH - 3) {
			grid[selectedNode].inputChar(selectedLine, selectedIndex, static_cast<char>(input));
			selectedIndex++;
			x++;
			move(y, x);
			drawCode(selectedNode);
			setCursor(true);
		}
	} else if (input == KEY_ENTER || input == 11 || input == 10) {
		if (grid[selectedNode].inputCode.size() < NODE_HEIGHT - 2) {
			grid[selectedNode].newLine(selectedLine, selectedIndex);
			x -= selectedIndex;
			y++;
			selectedIndex = 0;
			selectedLine++;
			move(y, x);
			drawCode(selectedNode);
			setCursor(true);
		}
	} else if (input == KEY_DC || input == 127){
        int tmpLength = grid[selectedNode].inputCode[selectedLine].length();
        int status = grid[selectedNode].deleteKey(selectedLine, selectedIndex);

//selectedIndex++;
//      x++;

        drawCode(selectedNode);
        move(y, x);
        setCursor(true);

	} else if (input == KEY_BACKSPACE){
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

		drawCode(selectedNode);
		move(y, x);
		setCursor(true);

	} else if (input == KEY_LEFT) {
		if (selectedIndex == 0) {
			if (selectedLine != 0) {
				selectedLine--;
				y--;
				selectedIndex = grid[selectedNode].inputCode[selectedLine].length();
				x = getbegx(grid[selectedNode].w_code) + selectedIndex;

				move(y, x);
				setCursor(true);
			}
		} else {
			selectedIndex--;
			x--;

			move(y, x);
			setCursor(true);
		}
	} else if (input == KEY_RIGHT) {
		if (selectedIndex != grid[selectedNode].inputCode[selectedLine].length()) {
			selectedIndex++;
			x++;

			move(y, x);
			setCursor(true);
		} else if (selectedLine < grid[selectedNode].inputCode.size() - 1) {
			selectedLine++;
			y++;
			selectedIndex = 0;
			x = getbegx(grid[selectedNode].w_code);

			move(y, x);
			setCursor(true);
		}
	} else if (input == KEY_MOUSE && getmouse(&event) == OK) {

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
				setCursor(true);
				return true;
			}
		}

		if (pointInWindow(playButton, event.x, event.y)) {
			state = RUNNING;
			setCursor(false);
			return true;
		} else if (pointInWindow(stopButton, event.x, event.y)) {
			setCursor(false);
			return true;
		} else if (pointInWindow(pauseButton, event.x, event.y)) {
			state = OFF;
			setCursor(false);
			return true;
		} else if (pointInWindow(increaseDelayButton, event.x, event.y)) {
			if (tickDelay < 2048) {
				tickDelay *= 2;
				werase(delayValueWindow);
				mvwprintw(delayValueWindow, 0, 0, intToString(tickDelay).c_str());
				wrefresh(delayValueWindow);
			}
			setCursor(false);
			return true;
		} else if (pointInWindow(decreaseDelayButton, event.x, event.y)) {
				if (tickDelay > 1) {
					tickDelay /= 2;
					werase(delayValueWindow);
					mvwprintw(delayValueWindow, 0, 0, intToString(tickDelay).c_str());
					wrefresh(delayValueWindow);
				}
				setCursor(false);
				return true;
		}


	} else {
		//mvwprintw(stdscr, 0, 0, to_string(input).c_str());
		return false;
	}
}

void runtimeSystemInput(MEVENT event, int input) {
	if (getmouse(&event) == OK && pointInWindow(stopButton, event.x, event.y)) {
		state = EDIT;
	}
}

// UTILLITY

bool isNum(string str){
  //TODO
  return true;
}

void redrawSystem(int n){
  clear();
  refresh();
  for(unsigned int i=0;i<grid.size();i++){
      box(grid[i].w_main,0,0);
      box(grid[i].w_code,0,0);
      box(grid[i].w_divider,0,0);
      wrefresh(grid[i].w_main);
      wrefresh(grid[i].w_reg);
      wrefresh(grid[i].w_code);
      wrefresh(grid[i].w_divider);
      for(int j;j<4;j++){
	if(grid[i].arrows[j])
	  wrefresh(grid[i].arrows[j]->win);
      }
  }
  drawSystemContent();
  refresh();
  return;
}
