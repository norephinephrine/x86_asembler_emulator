#ifndef _INS_H_
#define _INS_H_


#include "section.h"
#include "shared.h"
#include <string>
#include "equ_table.h"
#include "symbol.h"
#include <sstream>
#include <regex>
#include <map>
#include <iostream>
#include "offset.h"
using namespace std;
void process_instuction(istringstream&,string,Symbol_table*,Section* &);
int jump_param_check(string);
void jump_proccess(int,string,Symbol_table*,Section*&,int);
void non_jump_process(int,string,Symbol_table*,Section*&,int);
void init_registers();
void init_jumps();
void init_non_Jump();
int non_jump_check(string);
bool is_jump(string);
bool is_non_jump(string);


#endif