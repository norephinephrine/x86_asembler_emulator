#ifndef _Equ_table_H_
#define _Equ_table_H_

#include <vector>
#include "symbol.h"
#include "section.h"
#include <sstream>
#include <string>
#include <map>
#include <iostream>
#include "shared.h"

using namespace std;


typedef unsigned short _word;


class Equ_entry{
public:

	Equ_entry(string n,vector<string> s,vector<char> o,int l){
		name=n;
		line_number=l;
		for (std::vector<string>::iterator i = s.begin(); i != s.end(); ++i)
		{
			string ns=*i;
			symbols.push_back(ns);
		}
		for (std::vector<char>::iterator i = o.begin(); i != o.end(); ++i)
		{
			char ns=*i;
			operators.push_back(ns);
		}
	}

	string name;
	vector<string> symbols;
	vector<char> operators;
	int line_number;
	bool is_solved(Symbol_table*);
	void solve(Symbol_table*);
	

};

class Equ_Table{
public:
	vector<Equ_entry*> lista;
	bool check_izraz(string izraz,Symbol_table*,string);
	void solve_izraz(vector<string>,vector<char>,Symbol_table*,string);
	void solve_all(Symbol_table*);
	
	~Equ_Table(){
	  for (int i = 1; i < lista.size(); ++i)
	  {
	  	delete lista[i];
	  }		
	}



};

std::ostream& operator<<(std::ostream &strm,  Equ_entry &e);
std::ostream& operator<<(std::ostream &strm,  Equ_Table &e);


#endif