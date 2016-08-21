#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <stdint.h>
#include <ncurses.h>


const string SIM_OPS[] = { "NOP", "SWP", "SAV", "NEG" };  // Operations with no parameters
const string SRC_OPS[] = { "ADD", "SUB", "JRO" };  // Operations with only SRC as a parameter
const string SD_OPS[] = { "MOV" };  // Operations with both SRC and DST as parameters
const string LAB_OPS[] = { "JMP", "JEZ", "JNZ", "JGZ", "JLZ"}; // Operations with only SRC as a parameter


enum STATUS{
  SET,
  WAIT,
  INVALID
};

using namespace std;

class arrow{
  uint8_t getNodeIndex(int8_t nodeId);

public:
  int8_t nodeId[2];
  int8_t status[2];
  int16_t value[2];

  WINDOW *win;
  arrow(int8_t nodeId1, int8_t nodeId2);
  bool nodeRequest(int8_t nodeId);
  int16_t nodeGet(int8_t nodeId);
  void nodeSet(int8_t nodeId, int16_t number);
};

class io{
 public:
  arrow Arrow; 
  std::vector<string> io;
};

class node{
  uint8_t nodeId;
  pair<int8_t, int16_t> getFromSrc(string src);
 public:
  WINDOW *w_main;
  WINDOW *w_code;
  WINDOW *w_reg;
  node(uint8_t nId);
  int16_t acc;
  int16_t bak;
  int8_t  pc;
  std::vector<string> code;
  std::vector<string> inputCode;
  bool runline();
  bool runPrepare();
  std::map<uint8_t, uint16_t> labels;
  arrow *arrows[4];
};
