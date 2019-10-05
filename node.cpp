#include "spis.h"
node::node(uint8_t nId){
  nodeId = nId;
  acc = 0;
  bak = 0;
  pc=0;
  no_code=FALSE;
  
  for (int i = 0; i < 4; i++)
  	arrows[i] = NULL;
}

bool node::runline(){
  if(clear_arrows&(1<<4)){
    for(int i=0;i<4;i++){
      if(arrows[i]){
	if(clear_arrows&(1<i)){
	    int index=arrows[i]->getNodeIndex(nodeId);
	    arrows[i]->status[index]=WAIT;
	  }
	}
      }
      clear_arrows=0;
  }
  if(code.size()==0)
    return true;
  if(is_whitespace(code[pc]) || code[pc].length() == 0){
    pc++;
    if(pc>=code.size()-1)
      pc=0;
    return true;
  }
  string line = code[pc];
  string operation = line.substr(0,line.find(" "));
  if(!strcmp("NOP", operation.c_str())){
    pc++;
    return true;
  } else if(!strcmp("MOV", operation.c_str())){
    int c;
    std::string src, dst;
    pair<int8_t, uint16_t> p;
    int16_t input;
    arrowType *a;

    string operands = line.substr(4);
    c = operands.find_first_of(' ');
    src = operands.substr(0,c);
    dst = operands.substr(c + 1);
    p = getFromSrc(src);
  
    if(!p.first){
      return true;
    }
    input = p.second;
    if(!strcmp(dst.c_str(), "ACC")) {
      acc = input;
    } else if(!strcmp(dst.c_str(), "NIL")) {
	
    } else if(!strcmp(dst.c_str(),"ANY")){
	bool nextLine=false;
	for(int i=0;i<4;i++){
	  if(arrows[i]){
	    if(arrows[i]->setRequest(nodeId)){
	      clear_arrows=clear_arrows|(1<<i);
	      arrows[i]->nodeSet(nodeId, input);
	      arrows[i]->updateValues();
	      nextLine=true;
	    }
	  }
	}
	if(nextLine)
	  pc++;
	return true;
      }else if(!strcmp(dst.c_str(), "LAST")){
	int num;
	if(last==4)
	  num=0;
	else
	  num=last;
	if(arrows[num]->setRequest(nodeId)){
	  arrows[num]->nodeSet(nodeId, input);
	  arrows[num]->updateValues();
	  pc++;
	}
	return true;
      }else{
      unsigned int a_num;
      if(!strcmp("LEFT", dst.c_str())){
	a_num=3;
      } else if(!strcmp(dst.c_str(), "RIGHT")) {
	a_num=1;;
      } else if(!strcmp(dst.c_str(), "UP")) {
	a_num=0;
      } else if(!strcmp(dst.c_str(),"DOWN")) {
    	a_num=2;
      }      
      else
	return false;
      a=arrows[a_num];
      if(!a)
	return false;
      if (a->setRequest(nodeId)){
	a->nodeSet(nodeId, input);
	a->updateValues();
      }
      else
	pc--;
    }
    pc++;
    return true;
  }
    else if(!strcmp("SWP", operation.c_str())){
    int16_t tmpAcc = acc;
    acc = bak;
    bak = tmpAcc;
    pc++;
    return true;
    
  } else if(!strcmp("SAV", operation.c_str())){
    bak = acc;
    pc++;
    return true;

    }else if(!strcmp("ADD", operation.c_str())){
    string src = line.substr(4);
    pair<bool, int16_t> p = getFromSrc(src);
    acc += p.second;
    pc++;
    return true;
    } else if(!strcmp("SUB", operation.c_str())){
    string src = line.substr(4);
    pair<int8_t, int16_t> p = getFromSrc(src);
    int16_t input;
 
    if (!p.first) {
      return true;
    }
   
    else{
      input = p.second;
      acc -= input;
      pc++;
      return true;
    }
    
  } else if(!strcmp("NEG", operation.c_str())){
    acc = -acc;
    pc++;
    return true;   
    }
  else if(!strncmp("J",operation.c_str(),1)){
    unsigned int p;
    if(!strcmp("JRO", operation.c_str())){
      string operands = line.substr(4);
      string src = operands.substr(0);
      pair<int8_t, int16_t> p = getFromSrc(src);
      if(!p.first)
	 return true;
      else
	pc=pc+p.second;
      return true;
    }
    else{
      p=getLine(line.substr(4));
      if(p<0)
	return false;
      if(strcmp("JGZ", operation.c_str()) && strcmp("JLZ", operation.c_str())
	 && strcmp("JEZ", operation.c_str()) && strcmp("JNZ", operation.c_str())&& strcmp("JEZ", operation.c_str()) && strcmp("JMP", operation.c_str()))
	return false;
      
      else if(strcmp("JGZ", operation.c_str())&&this->acc>0)
	pc=p;
      else if(!strcmp("JLZ", operation.c_str()) &&this->acc<0)
	pc=p;
      else if(!strcmp("JEZ", operation.c_str())&&this->acc==0)
	pc=p;
      else if(!strcmp("JNZ", operation.c_str())&&this->acc!=0)
	pc=p;
      else if(!strcmp("JMP", operation.c_str()))
	pc=p;
      else
	pc++;
      return true;
    }
  }
  else
    return false;
}
  
bool node::runPrepare(){
  std::vector<int> labelLines;
  labels.clear();
  code.resize(inputCode.size());
  for (int8_t i = 0; i < inputCode.size(); i++){
    bool label=false;
    for(unsigned int j=0;j<inputCode.size();j++){
      string line=inputCode[i];
      char ch=line[j];
      string tmp;      
      if(ch==':'){
	std::pair<std::string, uint8_t> p;
	int c;
	p.first=line.substr(0,j);
	p.second=i;
	labels.push_back(p);
	label=true;
	c=line.substr(j+1).find_first_not_of(" ");
	code[i]=line.substr(c+j+1);
	break;
      }
    }
    if(label==false){
      code[i]=inputCode[i];
    }
  }
  for(int i=0; i<labelLines.size();i++){
    string operand;
    bool label_exists=false;
    operand=code[labelLines[i]].substr(4);
    for(list<pair<string, uint8_t> >::iterator j=labels.begin();j!=labels.end();j++){
      if(j->first==operand){
	label_exists=true;
	break;
      }
    }
    if(label_exists==false)
      return false;
  }
  for(unsigned int i=0;i<code.size();i++){
    bool space=false;
    int  remove_count=0;
    for(int j=0; j<code[i].length();j++){
      char ch=code[i].c_str()[j];
      if(ch == ' '||ch==','){
	if(space){
	  code[i].erase(j, 1);              //get rid of whitespace
	  j--;
	}
	space=true;
      }
      if(ch==':')
	return false;
      else
	space=false;
    }
    for(int j=code[i].length()-1;j>=0;j--){
      char ch=code[i].c_str()[j];
      if(ch==' '||ch==',')
	remove_count++;
      else{
	code[i].erase(j+1, remove_count);
	break;
      }
    }
  }
  return true;
}

void node::inputChar(int line, int index, char ch) {
  if (inputCode.size() > line) {
    if (inputCode[line].length() < MAX_LINE_LENGTH) {
      inputCode[line] = inputCode[line].substr(0, index) + ch + inputCode[line].substr(index);
      //inputCode[line] += ch;
    }
  } else {
    inputCode[line] = ch;
  }
}

void node::newLine(int line, int index) {
  inputCode.insert(inputCode.begin() + line, inputCode[line].substr(0, index));
  inputCode[line + 1] = inputCode[line + 1].substr(index);
}

int node::backspace(int line, int index) {
  if (index > 0) {
    if (inputCode[line].length() > index) {
      inputCode[line] = inputCode[line].substr(0, index - 1) + inputCode[line].substr(index);
    } else if (inputCode[line].length() == index) {
      inputCode[line] = inputCode[line].substr(0, index - 1);
    }
    return 1;
  } else if(line > 0 && inputCode[line - 1].length() + inputCode[line].length() <= MAX_LINE_LENGTH && index == 0) {
    inputCode[line - 1] += inputCode[line];
    inputCode.erase(inputCode.begin() + line);
    return 2;
  }
  
  return 0;
}

int node::deleteKey(int line, int index) {
    if (index >= 0) {
        if (inputCode[line].length() > index) {
            inputCode[line] = inputCode[line].substr(0, index) + inputCode[line].substr(index + 1);
        } else if (inputCode[line].length()<= index) {
            // do nothing..for now

            }
        return 1;
    } else if(line > 0 && inputCode[line - 1].length() + inputCode[line].length() <= MAX_LINE_LENGTH && index == 0) {
        inputCode[line - 1] += inputCode[line];
        inputCode.erase(inputCode.begin() + line);
        return 2;
    }

    return 0;
}



pair<int8_t, int16_t> node::getFromSrc(string src) {
  if(src.find_first_not_of("0123456789") == std::string::npos) {
    // Then src is just a number
    pair<bool, int16_t> p;
    p.first = READY;
    try{
      p.second = atoi(src.c_str());
    }
    catch(...){
      p.first = INVALID;
      return p;
    }
    return p;
  } else {
    if(src=="ANY"){
      pair<bool, int16_t> p;
      for(int i=0;i<4;i++){
	int num=abs((i+3)%4-(int)floor((i+1)/2)%2);            //Ask Tyler
	if (arrows[num]->getRequest(nodeId)){
	  p.first = true;
	  p.second = arrows[num]->nodeGet(nodeId);
	  return p;
	}
      }
      p.first=false;
      return p;
    }
    if(src=="LAST"){
      if(last==5){
	pair<bool, int16_t> p;
	p.first=true;
	p.second=0;
	return p;
      }
      else{
	if(last==0)
	  src="UP";
	else if(last==1)
	  src="RIGHT";
	else if(last==2)
	  src="DOWN";
	else if(last==3)
	  src="LEFT";
      }
    }
    if(src == "ACC") {
      pair<bool, int16_t> p;
      p.first = true;
      p.second = this->acc;
      return p;
    } else if (src ==  "NIL") {
      pair<bool, int16_t> p;
      p.first = true;
      p.second = 0;
      return p;
    } else if (src ==  "LEFT") {
      arrowType *a = arrows[3];
      pair<bool, int16_t> p;

      if (a->getRequest(nodeId)) {
        p.first = true;
        p.second = a->nodeGet(nodeId);
      } else {
        p.first = false;
      }
      a->updateValues();
      return p;
    } else if (src ==  "RIGHT") {
    	arrowType *a = arrows[1];
      pair<bool, int16_t> p;

      if (a->getRequest(nodeId)) {
        p.first = true;
        p.second = a->nodeGet(nodeId);
      } else {
        p.first = false;
      }
      a->updateValues();
      return p;
    } else if (src ==  "UP") {
    	arrowType *a = arrows[0];
	pair<bool, int16_t> p;

      if (a->getRequest(nodeId)){
        p.first = true;
        p.second = a->nodeGet(nodeId);
      } else {
        p.first = false;
      }
      a->updateValues();
      return p;
    } else if (src ==  "DOWN") {
    	arrowType *a = arrows[2];
      pair<bool, int16_t> p;

      if (a->getRequest(nodeId)) {
        p.first = true;
        p.second = a->nodeGet(nodeId);
      } else {
        p.first = false;
      }
      a->updateValues();
      return p;
    }
  }
  // ANY and LAST are for the future (hopefully not)
}

void node::first_instruction(){
  no_code=false;
  for(int i=0;i<inputCode.size();i++){
    if(!is_whitespace(inputCode[i])){
      pc=i;
      return;
    }
  }
  pc=0;
  no_code=true;
  return;
}

void node::reset(){
  acc=0;
  bak=0;
  first_instruction();
}

int node::getLine(std::string label){
  for(list<pair<string, uint8_t> >::iterator i=labels.begin(); i!=labels.end();i++){
    if(i->first==label)
      return i->second;
  }
  return -1;
}
