#ifndef _Symbol_H_
#define _Symbol_H_

#include <vector>
#include <string>
#include <iostream>


using namespace std;


struct Forward_entry{
	short patch;
	char type='b';
	Forward_entry *next=nullptr;
	
};



struct Symbol
{
	string name;
	string section;
	int value=0;
	char visibility='l';
	int id;

	bool defined;     
	Forward_entry* flink=nullptr;


	bool relocatable=true;
	bool equ=false;
	Symbol* und_symbol=nullptr;
	
	~Symbol(){
		und_symbol=nullptr;
		 Forward_entry* f=flink,*before;
		  while(f!=nullptr)
			{
				before=f;
				f=f->next;
				delete before;
			}	
	}
};


class Symbol_table
{
public:


	vector<Symbol*> table;

	Symbol_table(){

		Symbol *s=new Symbol();

		s->name="UND";
		s->section="UND";
		s->value=0;
		s->visibility='l';
		s->id=0;
		s->defined=true;
		s->flink=nullptr;

		table.push_back(s);
	};

	~Symbol_table(){
		for (std::vector<Symbol*>::iterator i = table.begin(); i != table.end(); ++i)
		{
				delete (*i);
		}		
	}

	bool exists_symbol(string s);
	bool is_all_defined();
	Symbol* get(string s);

	Symbol* get_by_id(int id){
		for (std::vector<Symbol*>::iterator i = table.begin(); i != table.end(); ++i)
		{
			if((*i)->id==id)
				return *i;
		}
		return nullptr;		
	}
	
};



std::ostream& operator<<(std::ostream &strm,  Forward_entry &f);
std::ostream& operator<<(std::ostream &strm,  Symbol &s);
std::ostream& operator<<(std::ostream &strm,  Symbol_table &s);

#endif
