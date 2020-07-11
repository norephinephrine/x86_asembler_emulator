#ifndef _Shared_H_
#define _Shared_H_
#include <string>
#include "section.h"
#include "symbol.h"
class Error_detail{
public:
	Error_detail(string s,int l){
		text=s;
		line_number=l;
	}

	Error_detail(string s){
		text=s;
	}
	string text="";
	int line_number=0;
};
extern int line_number;
extern int symbol_und(string,Symbol_table*,Section*,char);
extern unsigned int is_literal(string num);
#endif
