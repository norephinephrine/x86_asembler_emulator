#include <string>
#include <iostream>
#include <fstream>
#include <sstream>  
#include "section.h"
#include "symbol.h"
#include <regex>
#include "shared.h"
#include <map>
#include "symbol.h"
#include "section.h"
#include "offset.h"
#include "cpu.h"

using namespace std;

map<string,_word> place_map;
map<_word,string> place_map_reverse;


vector<_word> placement_array;
vector<Symbol_table*> symbol_table_vector;
vector<Section_Table*> section_table_vector;

Section_Table* combine_section_table=new Section_Table();
Symbol_table* combine_symbol_table=new Symbol_table();

void process_file(ifstream&);
void ispis(Symbol_table* sym_table,Section_Table* sect_table,ostream &cout);
int main(int argc, char **argv) 
{ 



    ifstream fin;
   


   smatch matches;
   regex str_expr("^-place=([^$.0-9][^:]*)@0x([0-9ABCDEF]{4})$");
// GOES THROUGH ALL ARGUMENTS
	for (int i = 1; i < argc; ++i)
	{
		string param(argv[i]);
	    
		if(regex_search(param, matches, str_expr)){
			istringstream iss (matches[2].str());
			iss.flags(std::ios::hex);
			_word j;
			iss >> j;
			
			std::map<string,_word>::iterator it;
			it=place_map.find(matches[1].str());
			if(it!=place_map.end()){
				
				cout<<endl<<"Cant define place two times for same section "<<matches[1].str()<<endl;
				return -1;
			}			

			place_map.insert (pair<string,_word>(matches[1].str(),j));

			place_map_reverse.insert (pair<_word,string>(j,matches[1].str()));
			placement_array.push_back(j);


		}
		else{
			string param(argv[i]);
			fin.open(param);
			if (!fin.is_open()){
			  cout << "Unable to open file "<<param<<'\n';
			  return -1;
			}

			try{
				process_file(fin);
			}
			catch(Error_detail* e){

				if(e->line_number==0) cout<<"Error:"<<e->text<<endl;
				else cout<<"Error:"<<e->text<<endl;
				fin.close();
				for(vector<Symbol_table *>::iterator it = symbol_table_vector.begin(); it != symbol_table_vector.end(); ++it) {
				  delete *it;
				}
				for(vector<Section_Table *>::iterator it = section_table_vector.begin(); it != section_table_vector.end(); ++it) {
				  delete *it;
				}


				return -1;
			}
			catch(int g){

				cout<<"Error number:"<<g;

				fin.close();
				for(vector<Symbol_table *>::iterator it = symbol_table_vector.begin(); it != symbol_table_vector.end(); ++it) {
				  delete *it;
				}
				for(vector<Section_Table *>::iterator it = section_table_vector.begin(); it != section_table_vector.end(); ++it) {
				  delete *it;
				}
				return -1;
			}

			fin.close();


		}
	}

	sort(placement_array.begin(),placement_array.end());


//CHECKS IF PLACEMENTS EXIST--------------------------------------------------------------------
	int location=0;
	try{
		
		init_mem();


//GO THROUGH ALL PLACEMENTS
		for (int i = 0; i < placement_array.size(); ++i)
		{

			if(i!=0 && location>placement_array[i]){
				throw new Error_detail("There is overlap between sections");
			}

			location=placement_array[i];
			string name=place_map_reverse[placement_array[i]];

			bool  exists=false;

			Symbol* new_symbol=new Symbol();
			new_symbol->name=name;
			new_symbol->section=name;
			new_symbol->value=placement_array[i];
			new_symbol->visibility='l';
			new_symbol->id=combine_symbol_table->table.size();
			new_symbol->defined=true;
			new_symbol->flink=nullptr;	

			combine_symbol_table->table.push_back(new_symbol);

			Section*new_section=new Section();
			new_section->name=name;
			new_section->size=0;

			for (int j=0;j<section_table_vector.size();j++)
			{
					Section*s=section_table_vector[j]->get(name);
					if(s!=nullptr){
						exists=true;
						
						Symbol* symbol=symbol_table_vector[j]->get(name);
						symbol->value=location;

						for (std::vector<_byte>::iterator it = s->bytes.begin(); it != s->bytes.end(); ++it)
						{

							if(location>=64*1024)throw new Error_detail("Section is too big to be put in memory");
							mem_set_b(location,*it);
							location++;
							new_section->size++;
							
						}
					}
			}

			if(exists==false)throw new Error_detail("Section "+name+" that is used for place argument doesnt exist");
			combine_section_table->section_table.push_back(new_section);

		}
//GO THROUGH OTHER SECTIONS
		for (int i=0;i<section_table_vector.size();i++)
		{
			for (std::vector<Section*>::iterator j = section_table_vector[i]->section_table.begin(); j !=  section_table_vector[i]->section_table.end(); ++j)
			{
				
				string name=(*j)->name;

				Symbol* new_symbol=combine_symbol_table->get(name);

				if(new_symbol==nullptr){

					new_symbol=new Symbol();
					new_symbol->name=name;
					new_symbol->section=name;
					new_symbol->value=location;
					new_symbol->visibility='l';
					new_symbol->id=combine_symbol_table->table.size();
					new_symbol->defined=true;
					new_symbol->flink=nullptr;	

					combine_symbol_table->table.push_back(new_symbol);

					Section*new_section=new Section();
					new_section->name=name;
					new_section->size=0;

					for (int z=i;z<section_table_vector.size();z++){
						Section*s=section_table_vector[z]->get(name);
						if(s!=nullptr){
							
							Symbol* symbol=symbol_table_vector[z]->get(name);
							symbol->value=location;

							for (std::vector<_byte>::iterator it = s->bytes.begin(); it != s->bytes.end(); ++it)
							{
								if(location>=64*1024)throw new Error_detail("Section is too big to be put in memory");
								mem_set_b(location,*it);
								location++;
								new_section->size++;

							}
						}
					}

			
					combine_section_table->section_table.push_back(new_section);

				}


			}
		}

//GO THROUGH DEFINED SYMBOLS ===========================================================================================
		for (int i=0;i<symbol_table_vector.size();i++)
		{
			for (std::vector<Symbol*>::iterator j = symbol_table_vector[i]->table.begin(); j != symbol_table_vector[i]->table.end(); ++j)
			{
				string name=(*j)->name;
				Symbol* new_symbol=combine_symbol_table->get(name);

				if((*j)->section!="UND" && new_symbol!=nullptr && new_symbol->name!=new_symbol->section)throw new Error_detail("Cant define symbol multiple times.Symbol: "+name);
				else if((*j)->section!="UND" &&  new_symbol==nullptr){
					new_symbol=new Symbol();
					new_symbol->name=(*j)->name;
					new_symbol->section=(*j)->section;
					new_symbol->value=(*j)->value;

					Symbol* s=symbol_table_vector[i]->get((*j)->section);
					if(s==nullptr)throw new Error_detail("Program error missing section: "+(*j)->name+" .For symbol: "+name);

					new_symbol->value+=s->value;


					new_symbol->visibility='g';
					new_symbol->id=combine_symbol_table->table.size();
					new_symbol->defined=true;
					new_symbol->flink=nullptr;	

					combine_symbol_table->table.push_back(new_symbol);					
				}
			}
		}		

//GO CHECK IF ALL SYMBOLS ARE DEFINED ===========================================================================================
		for (int i=0;i<symbol_table_vector.size();i++)
		{
			for (std::vector<Symbol*>::iterator j = symbol_table_vector[i]->table.begin(); j != symbol_table_vector[i]->table.end(); ++j)
			{
				string name=(*j)->name;
				Symbol* new_symbol=combine_symbol_table->get(name);

				if((*j)->section=="UND" &&  (*j)->name!="UND" && new_symbol==nullptr) throw new Error_detail("Program error missing definition for symbol: "+(*j)->name);
			}
		}	

//GO THROUGH ALL OFFSETS AND CHANGE MEMORY
		for (int i=0;i<section_table_vector.size();i++)
		{
			for (std::vector<Section*>::iterator j = section_table_vector[i]->section_table.begin(); j !=  section_table_vector[i]->section_table.end(); ++j)
			{
				for (std::vector<Offset_entry*>::iterator z = (*j)->relocation.begin(); z != (*j)->relocation.end(); ++z)
				{
					Symbol* sect=symbol_table_vector[i]->get((*j)->name);


					Symbol* s;

					//AKO JE 0 ZNAM DA SE RADI O R_386_PC16_APS
					if((*z)->value!=0){
						s=symbol_table_vector[i]->get_by_id((*z)->value);
						if(s==nullptr)  throw new Error_detail("Bad relocation value for relocation: "+to_string((*z)->offset)+"  "+to_string((*z)->value));	


						if(s->name != s->section )
						{
							s=combine_symbol_table->get(s->name);
							if(s==nullptr) throw new Error_detail("Bad relocation value for relocation: "+to_string((*z)->offset)+"  "+to_string((*z)->value));						
						}						
					}

					

					if((*z)->type==R_386_16){
						if(sect->value+(*z)->offset==(64*1024-1)) throw new Error_detail("Bad relocation entry will overflow");
						mem_add_w(sect->value+(*z)->offset,s->value);
					}
					else if((*z)->type==R_386_8){
						mem_add_b(sect->value+(*z)->offset,s->value);
					}
					else if((*z)->type==R_386_PC16){
						if(sect->value+(*z)->offset==(64*1024-1)) throw new Error_detail("Bad relocation entry will overflow");
						mem_add_w(sect->value+(*z)->offset,s->value-(sect->value+(*z)->offset));
					}
					else if((*z)->type==R_386_PC16_APS){
						if(sect->value+(*z)->offset==(64*1024-1)) throw new Error_detail("Bad relocation entry will overflow");
						mem_add_w(sect->value+(*z)->offset,2);
					}
					else 	throw new Error_detail("Bad relocation type: "+to_string((*z)->offset)+"  "+to_string((*z)->value));						
				}
			}
		}	



	}
	catch(Error_detail* e){
		if(e->line_number==0) cout<<"Error:"<<e->text<<endl;
		else cout<<"Error:"<<e->text<<endl;
		fin.close();
		for(vector<Symbol_table *>::iterator it = symbol_table_vector.begin(); it != symbol_table_vector.end(); ++it) {
		  delete *it;
		}
		for(vector<Section_Table *>::iterator it = section_table_vector.begin(); it != section_table_vector.end(); ++it) {
		  delete *it;
		}


		return -1;
	}


	//ispis(combine_symbol_table,combine_section_table,cout);



	
	cout<<endl;

	//END FREE MEMORY
	for(vector<Symbol_table *>::iterator it = symbol_table_vector.begin(); it != symbol_table_vector.end(); ++it) {
	  delete *it;
	}
	for(vector<Section_Table *>::iterator it = section_table_vector.begin(); it != section_table_vector.end(); ++it) {
	  delete *it;
	}

	emulate();

}











//=====================================================================================================================
//PROCES FILE
void process_file(ifstream& fin){
	string word;
	for (int i = 0; i < 4; ++i)
	{
		getline(fin,word);
	}

	Section_Table* sect_table=new Section_Table();
	Symbol_table* symbol_table=new Symbol_table();



	int number;

	string name;
	while(word!="============================================================================================"){

		Symbol* symbol=new Symbol();
		istringstream help(word);
		string line=word;

		if(!(help>>word)) throw new Error_detail("Missing info in line: "+line);
		symbol->name=word;
		
		name=symbol->name;

		if(!(help>>word)) throw new Error_detail("Missing info in line: "+line);
		symbol->section=word;

		if(!(help>>word)) throw new Error_detail("Missing info in line: "+line);
		symbol->value= stoi (word,nullptr,16);

		if(!(help>>word)) throw new Error_detail("Missing info in line: "+line);
		symbol->visibility=word[0];

		if(!(help>>word)) throw new Error_detail("Missing info in line: "+line);		

		symbol->id= stoi (word,nullptr,16);

		if(help>>word){
			Section*sect=new Section();

			sect->size= stoi (word,nullptr,16);
			sect->name=name;


			sect_table->section_table.push_back(sect);
		}


		symbol_table->table.push_back(symbol);


		getline(fin,word);
	}



	//skips UND and goes to
	for (int i = 0; i < 5; ++i)
	{
		getline(fin,word);
	}

	while(word!="============================================================================================"){


		smatch matches;
	    regex str_expr("^#ret\\.([^$.0-9][^:]*)$");

	    Section* s=nullptr;
	    if(regex_search(word, matches, str_expr)){

	   		s=sect_table->get(matches[1].str());
	   		if(s==nullptr) throw new Error_detail("No section exists:"+word);
	    }
	    else throw new Error_detail("Bad form in line:"+word);


	    getline(fin,word);
	    getline(fin,word);
		
		while(word!=""){

			Offset_entry* off=new Offset_entry();

			istringstream help(word);
			string line=word;

			

			if(!(help>>word)) throw new Error_detail("Missing info in line: "+line);
			off->offset= stoi (word,nullptr,16);
			

			if(!(help>>word)) throw new Error_detail("Missing info in line: "+line);

			if(word=="R_386_16")off->type=R_386_16;
			else if(word=="R_386_PC16")off->type=R_386_PC16;
			else if(word=="R_386_8")off->type=R_386_8;
			else if(word=="R_386_PC8")off->type=R_386_PC8;
			else if(word=="R_386_PC8_APS")off->type=R_386_PC8_APS;
			else if(word=="R_386_PC16_APS")off->type=R_386_PC16_APS;			



			if(!(help>>word)) throw new Error_detail("Missing info in line: "+line);
			off->value= stoi (word,nullptr,16);	

			s->relocation.push_back(off);

			getline(fin,word);
		}	
		getline(fin,word);

	}

	//BYTES -------------------------------------------------------------------------------------------------------
	//skips UND and goes to Bytes
	for (int i = 0; i < 5; ++i)
	{
		getline(fin,word);
	}

	while(word!="============================================================================================"){


		smatch matches;
	    regex str_expr("^#([^$.0-9][^:]*)$");

	    Section* s=nullptr;
	    if(regex_search(word, matches, str_expr)){

	   		s=sect_table->get(matches[1].str());
	   		if(s==nullptr) throw new Error_detail("No section exists:"+word);
	    }
	    else throw new Error_detail("Bad form in line:"+word);


	    getline(fin,word);

	    
	    if(word!=""){
	    	while(word!="")
	    	{
		    	istringstream help(word);

		    	while(help>>word){
		    		s->bytes.push_back(stoi (word,nullptr,16));
		    	}
		    	getline(fin,word);	    		
	    	}
	    	getline(fin,word);
			//if(word=="")getline(fin,word);
	    }else{
	    	getline(fin,word);
	    	getline(fin,word);
	    }


	}


	section_table_vector.push_back(sect_table);
	symbol_table_vector.push_back(symbol_table);


}












void ispis(Symbol_table* sym_table,Section_Table* sect_table,ostream &cout){
	cout<<"#tabela simbola"<<endl;
	cout<<left<<setw(20)<<"#ime"<<setw(20)<<"sek"<<setw(6)<<right<<"vr  "<<setw(1)<<"vid"<<setw(3)<<" "<<setw(5)<<"r.b."<<setw(3)<<" "<<setw(10)<<"size"<<endl;
	for (std::vector<Symbol*>::iterator i = sym_table->table.begin(); i != sym_table->table.end(); ++i)
	{
		Section* sect=sect_table->get((*i)->name);
		if(sect!=nullptr)
		cout<<setw(20)<<(*i)->name<<setw(20)<<(*i)->section<<setw(6)<<hex <<(*i)->value<<"   "<<setw(1)<<(*i)->visibility<<"   "<<setw(5)
	<<(*i)->id<<"   "<<setw(10)<<hex<<sect->size<<endl;
	}

	for (std::vector<Symbol*>::iterator i = sym_table->table.begin(); i != sym_table->table.end(); ++i)
	{
		Section* sect=sect_table->get((*i)->name);
		if(sect==nullptr && ((*i)->visibility)=='g')
		cout<<setw(20)<<(*i)->name<<setw(20)<<(*i)->section<<setw(6)<<hex <<(*i)->value<<"   "<<setw(1)<<(*i)->visibility<<"   "<<setw(5)
	<<(*i)->id<<endl;
	}	
	cout<<"============================================================================================";
	cout<<right<<endl<<"Offsets"<<endl;
	for (std::vector<Section*>::iterator i = sect_table->section_table.begin(); i != sect_table->section_table.end(); ++i)
	{
		cout<<"#ret."<<(*i)->name<<endl;
		cout<<"#offset\t\ttip\t\t\tvr"<<endl;
		for (std::vector<Offset_entry*>::iterator j = (*i)->relocation.begin(); j != (*i)->relocation.end(); ++j)
		{
			cout<<setfill('0') << setw(4)<<hex <<(*j)->offset<<"\t\t";
			if((*j)->type==R_386_16)cout<<"R_386_16";
			else if((*j)->type==R_386_PC16)cout<<"R_386_PC16";
			else if((*j)->type==R_386_8)cout<<"R_386_8";
			else if((*j)->type==R_386_PC8)cout<<"R_386_PC8";
			else if((*j)->type==R_386_PC8_APS)cout<<"R_386_PC8_APS";
			else if((*j)->type==R_386_PC16_APS)cout<<"R_386_PC16_APS";
			cout<<"\t\t"<<(*j)->value<<endl;			
		}		
		cout<<endl;
	}	

	cout<<"============================================================================================";
	cout<<endl<<"Bytes"<<endl;
	for (std::vector<Section*>::iterator i = sect_table->section_table.begin(); i != sect_table->section_table.end(); ++i)
	{
		cout<<"#"<<(*i)->name<<endl;
		int jmp=0;
		for (std::vector<_byte>::iterator j = (*i)->bytes.begin(); j !=  (*i)->bytes.end(); ++j)
		{
			jmp++;
			cout<<setfill('0') << setw(2)<<hex << (int)(*j)<<" ";
			if(jmp%4==0){
				cout<<'\t';
			}
			if(jmp==20)
			{
				jmp=0;
				cout<<"\n";
			}
		}
		cout<<endl;	
	}	
	cout<<"============================================================================================";	
	cout<<endl<<"Bytes:"<<endl;
	int jmp=0;
	for (int i =0x4000; i <0x4080; ++i)
	{
		jmp++;
		cout<<setfill('0') << setw(2)<<hex << (int)MemBase[i]<<" ";
		if(jmp%4==0){
			cout<<'\t';
		}
		if(jmp==20)
		{
			jmp=0;
			cout<<"\n";
		}
	}	

}