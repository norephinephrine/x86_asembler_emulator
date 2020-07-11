#ifndef _Section_H_
#define _Section_H_

#include <vector>
#include <string>
#include "offset.h"
#include <iostream>
#include <iomanip>
#include "symbol.h"
using namespace std;

typedef unsigned char _byte;
typedef unsigned short _word;

class Section
{
	public:


		vector<Offset_entry*> relocation;
		vector<_byte> bytes;
		string name;
		
		int size;


		Section() {
			size = 0;
		};



		void add_word(_word);
		void add_byte(_byte);
		void skip(unsigned int number);
		void change_byte(short location,_byte);
		void change_word(short location,_word);
		void fix_up(Symbol_table*);

		~Section(){
			bytes.clear();
			for (std::vector<Offset_entry*>::iterator i = relocation.begin(); i != relocation.end(); ++i)
			{
				delete(*i);
			}			
		};


};



class Section_Table {

public:
	vector<Section*> section_table;

	Section_Table(){
		Section*s =new Section();
		s->name="UND";
		section_table.push_back(s);
		

	}

	bool find_section(string name) {
		for (std::vector<Section*>::iterator i = section_table.begin(); i != section_table.end(); ++i)
		{
			if ((*i)->name == name)
				return true;
		}
		return false;
	}

	Section* get(string word) {
		for (std::vector<Section*>::iterator i = section_table.begin(); i != section_table.end(); ++i)
		{
			if ((*i)->name == word)
				return *i;
		}
		return nullptr;
	}


	void fix_up(Symbol_table* sym_table){
		for (std::vector<Section*>::iterator i = section_table.begin(); i != section_table.end(); ++i)
		{
			(*i)->fix_up(sym_table);
		}		
	}
	
	~Section_Table(){
			for (std::vector<Section*>::iterator i = section_table.begin(); i != section_table.end(); ++i)
			{
				delete (*i);
			}
	};



};
	std::ostream& operator<<(std::ostream &strm,  Section &s);
	std::ostream& operator<<(std::ostream &strm,  Section_Table &s);
#endif
