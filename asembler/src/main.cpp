#include <string>
#include <iostream>
#include <fstream>
#include <sstream>  
#include "section.h"
#include "symbol.h"
#include <regex>
#include <sstream>
#include "equ_table.h"
#include "shared.h"
#include "ins.h"
using namespace std;

typedef unsigned char _byte;
typedef unsigned short _word;

int line_number=1;




int process(string in,string out);
void ispis(Symbol_table*,Section_Table*,ostream &);


int main(int argc, char **argv) 
{ 
	
	if(argc==2){
	    string file_output ="a.o";
		string file_input(argv[1]);
		return process(file_input,file_output);		
	}
	else if(argc==4){
		

		string output_arg="-o";

		int arg_index=-1;
		int file_index;
		for(int i=1;i<3;i++){
			if(argv[i]==output_arg){
				arg_index=i;
			}
		}

		if(arg_index==-1){
			cout<<"Wrong command line arguments\n";
			return -1;
		}
		else if(arg_index==1){
			file_index=3;
		}
		else{
			file_index=1;
		}

		string file_output(argv[arg_index+1]);
		string file_input(argv[file_index]);
		return process(file_input,file_output);

	}
	else{
		printf("Wrong number of command line arguments\n");	
	}

  
return 0; 
} 





//POCETAK PROCESIRANJA FAJLA
int process(string input,string output){
		
		string line;

	    ofstream fout;
	    fout.open(output);
		ifstream fin (input);

		if (!fin.is_open()){
		  cout << "Unable to open file\n";
		  return -1;
		}


		//definicija tabela
		Symbol_table* sym_table=new Symbol_table();
		Section_Table* sect_table = new Section_Table();
		Equ_Table* equ_table=new Equ_Table();



		try{
			Section* current_section=nullptr;
			while ( getline (fin,line) ){

			  istringstream iss(line);
			  string word;


			  //POCETAK OBRADJIVANJA LINIJE

			  iss>>word;


			   //DODAVANJE NOVOG SIMBOLA SA LABELOM
			  int symbol_name_end=0;

			  if(word[0]==':')	throw new Error_detail("Label must have a name",line_number);

			  while(symbol_name_end<word.size() && word[symbol_name_end]!=':')symbol_name_end++;

			  if(symbol_name_end!=word.size()){
			  	string label=word.substr(0,symbol_name_end);

			  	if(label[0]=='.' || label[0]=='$' || isdigit(label[0])) 
			  		throw new Error_detail("Label cant start with dots or numbers or $",line_number);

				if (current_section == nullptr)throw new Error_detail("Label must be defined in section", line_number);

			  	if(sym_table->exists_symbol(label)) {
			  		Symbol* s=sym_table->get(label);

			  		
			  		if(s->defined || s->equ)
			  			throw new Error_detail("Symbol cant be defined multiple times",line_number);

			  		s->section=current_section->name;
			  		s->value=current_section->size;
			  		s->defined=true;
			  	} else if(sect_table->find_section(word) && !sym_table->exists_symbol(word)) throw new Error_detail("Name already in use",line_number);
			  	else{
					Symbol* new_symbol = new Symbol();
					new_symbol->name = label;
					new_symbol->section=current_section->name;
					new_symbol->value=current_section->size;
					new_symbol->visibility='l';
					new_symbol->id=sym_table->table.size();

					new_symbol->defined=true;
					new_symbol->flink=nullptr;
					
					
					sym_table->table.push_back(new_symbol);				  		
			  	}	


			  	if(symbol_name_end!=(word.size())-1){
			  		word=word.substr(symbol_name_end+1);

			  	}else{
			  		if(!(iss>>word))word="";
			  	}

			  }		

			 






//DIREKTIVE I OPERACIJE 
			  if (!word.empty()) {
			  

//SECTION =============================================================================================	  
				if (word == ".section") {
				  
					  

					  if(!(iss >> word)) throw new Error_detail("Bad declaration",line_number);




					  if (sect_table->find_section(word)) {
						  current_section = sect_table->get(word);
					  }
					  else if(!sect_table->find_section(word) && sym_table->exists_symbol(word)){
					  	Symbol* s=sym_table->get(word);

			  			if(s->defined || s->equ)
					  	throw new Error_detail("Name already in use",line_number);


					  	Section* sekcija = new Section();
						sekcija->name = word;


						current_section = sekcija;
						sect_table->section_table.push_back(sekcija);

						s->section = current_section->name;
						s->value = current_section->size;
						s->defined = true;

					  } 
					  else {
						  Section* sekcija = new Section();
						  sekcija->name = word;


						  current_section = sekcija;
						  sect_table->section_table.push_back(sekcija);




 						  if(word[0]=='.' || word[0]=='$' || isdigit(word[0])) throw new Error_detail("Section cant start with dots or numbers or $",line_number);
 						  if (word.find(':') != std::string::npos)throw new Error_detail("Section  cant have ':'' inside it",line_number);

						  Symbol* new_symbol = new Symbol();
						  new_symbol->name = word;
						  new_symbol->section = current_section->name;
						  new_symbol->value = current_section->size;
						  new_symbol->visibility = 'l';
						  new_symbol->id = sym_table->table.size();

						  new_symbol->defined = true;
						  new_symbol->flink = nullptr;
						  sym_table->table.push_back(new_symbol);
					  }

					  if (iss >> word) {
						  throw new Error_detail("Bad directive/instruction", line_number);
					  }

				  }
//END =============================================================================================	  
				else if(word == ".end"){

				  	if (iss >> word) {
						  throw new Error_detail("Bad directive/instruction", line_number);
					  }

					break;
				  }
//Global =============================================================================================	  
				else if(word==".global"){

					string p=iss.str();
					if(p[p.size()-1]==',') throw new Error_detail("Bad declaration",line_number);					
 
				  	while(getline(iss,word,',')){
				  		istringstream help(word);
				  		

						if(!(help>>word)) throw new Error_detail("Bad declaration",line_number);
						if(word.empty() || word==" ")throw new Error_detail("Bad directive/instruction",line_number);


				  		if (sect_table->find_section(word)) {
						 throw new Error_detail("Cant use section for global", line_number);
					  	}

					  	if(sym_table->exists_symbol(word)){
					  		Symbol* s=sym_table->get(word);

					  		if(s->section=="UND" || s->relocatable==false)
					  			throw new Error_detail("Cant define both extern and global", line_number);
					  		
					  		s->visibility='g';
					  	}
					  	else{
					  	  Symbol* new_symbol = new Symbol();
						  new_symbol->name = word;
						  new_symbol->section ="?";
						  new_symbol->value = 0;
						  new_symbol->visibility = 'g';
						  new_symbol->id = sym_table->table.size();

						  new_symbol->defined = false;
						  new_symbol->flink = nullptr;
						  sym_table->table.push_back(new_symbol);	
					  	}




				  		
				  		if (help >> word) {
						  throw new Error_detail("Bad directive/instruction", line_number);
					  }
				  	}
				  	

				  }
//EXTERN		=============================================================================================	  
				else if(word==".extern"){


					string p=iss.str();
					if(p[p.size()-1]==',') throw new Error_detail("Bad declaration",line_number);

				  	while(getline(iss,word,',')){

				  		istringstream help(word);


				  		if(!(help>>word)) throw new Error_detail("Bad declaration",line_number);
				  		if(word.empty() || word==" ")throw new Error_detail("Bad directive/instruction",line_number);

				  		if (sect_table->find_section(word)) {
						 throw new Error_detail("Cant use section for extern", line_number);
					  	}

					  	if(sym_table->exists_symbol(word)){
					  		Symbol* s=sym_table->get(word);

					  		if((s->section!="" && s->section!="UND") || s->relocatable==false || s->und_symbol!=nullptr)
					  			throw new Error_detail("Cant define both extern and global", line_number);
					  		
					  		s->visibility='g';
					  		s->section="UND";
					  		s->defined=true;
					  	}
					  	else{
					  	  Symbol* new_symbol = new Symbol();
						  new_symbol->name = word;
						  new_symbol->section ="UND";
						  new_symbol->value = 0;
						  new_symbol->visibility = 'g';
						  new_symbol->id = sym_table->table.size();

						  new_symbol->defined = true;
						  new_symbol->flink = nullptr;
						  sym_table->table.push_back(new_symbol);	
					  	}




				  		
				  		if (help >> word) {
						  throw new Error_detail("Bad directive/instruction", line_number);
					  }
				  	}
				  	

				  }
//SKIP ==========================================================================================================
				else if(word==".skip"){
					if (current_section == nullptr)throw new Error_detail("No section defined", line_number);


					
					if(!(iss>>word)) throw new Error_detail("Bad declaration",line_number);

					unsigned int res;
					try{
						res=is_literal(word);

					}
					catch(int g){
						Error_detail* e=new Error_detail("Wrong literal format",line_number);
						throw e;
					}
					

					if(res>20480)throw new Error_detail("LIteral for skipping too big", line_number);

				  	if (iss >> word) throw new Error_detail("Bad directive/instruction", line_number);

				  	current_section->skip(res);

				}
//BYTE		=======================================================================================================		
				else if(word==".byte"){
					if (current_section == nullptr)throw new Error_detail("No section defined", line_number);

					string p=iss.str();
					if(p[p.size()-1]==',') throw new Error_detail("Bad declaration",line_number);				

					while(getline(iss,word,',')){
				  		istringstream help(word);


				  		if(!(help>>word)) throw new Error_detail("Bad declaration",line_number);
						if(word.empty() || word==" ")throw new Error_detail("Bad directive/instruction",line_number);





						try{
					  		unsigned int res=is_literal(word);
					  		_byte b=res & 0xFF;
					  		current_section->add_byte(b);
				  		}
				  		catch(int g){
				  			int res=symbol_und(word,sym_table,current_section,'b');

				  			_byte number=res & 0xFF;
				  			current_section->add_byte(number);
				  		}							





				  		
				  		if (help >> word) {
						  throw new Error_detail("Bad directive/instruction", line_number);
					  }
				  	}

				}
//WORD============================================================================================================
				else if(word==".word"){
					if (current_section == nullptr)throw new Error_detail("No section defined", line_number);



					string p=iss.str();
					if(p[p.size()-1]==',') throw new Error_detail("Bad declaration",line_number);	

					while(getline(iss,word,',')){
				  		istringstream help(word);


				  		if(!(help>>word)) throw new Error_detail("Bad declaration",line_number);
						if(word.empty() || word==" ")throw new Error_detail("Bad directive/instruction",line_number);


						try{
					  		unsigned int res=is_literal(word);
					  		_word number=res & 0xFFFF;
					  		current_section->add_word(number);


				  		}
				  		catch(int g){
				  			int res=symbol_und(word,sym_table,current_section,'w');

				  			_word number=res & 0xFFFF;
				  			current_section->add_word(number);
				  		}							





				  		
				  		if (help >> word) {
						  throw new Error_detail("Bad directive/instruction", line_number);
					  }
				  	}

				}
//EQU====================================================================================				
				else if(word==".equ"){


					if(!getline(iss,word,',')) throw new Error_detail("Missing arguments", line_number);

					istringstream is2(word);
					is2 >> word;

					if(word.empty() || word==" ") throw new Error_detail("Bad directive/instruction", line_number);

					if(word[0]=='.' || word[0]=='$' || isdigit(word[0])) 
			  			throw new Error_detail("Label cant start with dots or numbers or $",line_number);
			  		if(sect_table->find_section(word) && !sym_table->exists_symbol(word)) throw new Error_detail("Name already in use",line_number);

					string hp;
					getline(iss,hp);

					if(hp.empty() || hp==" ") throw new Error_detail("Bad directive/instruction", line_number);

					//pozivanje funkcije koja treba sve da odradi
					try{
						equ_table->check_izraz(hp,sym_table,word);
					}
					catch(Error_detail*e){
						e->line_number=line_number;
						throw e;
					}
					catch(int g){
						Error_detail* e=new Error_detail(to_string(g),line_number);
						throw e;

					}

				}
				else{
					process_instuction(iss,word,sym_table,current_section);
				}

			  
			  
			 }




			  line_number++;
		    }

		    equ_table->solve_all(sym_table);
		    if(sym_table->is_all_defined()==false)throw new Error_detail("Not all symbols are defined");

		    sect_table->fix_up(sym_table);

		    ispis(sym_table,sect_table,fout);

		    //TO DO POSLE PRVOG PROLASKA			
		}
		catch(Error_detail* e){

			if(e->line_number==0) cout<<"Error:"<<e->text<<endl;
			else cout<<"Error at line "<<e->line_number<<":"<<e->text<<endl;
			fin.close();
			fout.close();
			delete sym_table;
			delete sect_table;
			delete equ_table;
			return -1;
		}
		catch(int g){

			cout<<"Error at line "<<line_number<<" number:"<<g;
			fin.close();
			fout.close();
			delete sym_table;
			delete sect_table;
			delete equ_table;
			return -1;
		}



	   fin.close();
	   fout.close();
	   delete sym_table;
	   delete sect_table;
	   delete equ_table;
	    return 0;


}
//checks if a symbol is und and returns its value if it is or it creates the symbol and returns \0
int symbol_und(string name,Symbol_table *sym_table,Section* sect,char type){


	Offset_entry*off_e=new Offset_entry();
	off_e->offset=sect->size;
	if(type=='b')off_e->type=R_386_8;
	else if(type=='w') off_e->type=R_386_16;
	else if(type=='r')off_e->type=R_386_PC16;
	else if(type=='e')off_e->type=R_386_PC8;
	

	if(sym_table->exists_symbol(name)) {

		Symbol* s=sym_table->get(name);

		off_e->value=s->id;
		sect->relocation.push_back(off_e);

		if(s->defined){
			if(type=='b')
				return '\0';
			else if(type=='w'){
				return '\0';
			}
			else if(type=='r') return -2;
			else if(type=='e') return -1;

		}else{
			if(s->flink==nullptr){
				s->flink=new Forward_entry();
				s->flink->patch=sect->size;
				s->flink->type=type;
			}else{
				Forward_entry* next=s->flink;
				while(next->next!=nullptr)next=next->next;

				next->next=new Forward_entry();
				next->next->patch=sect->size;
				next->next->type=type;
			}


			if(type=='b')
				return '\0';
			else if(type=='w'){
				return '\0';
			}
			else if(type=='r') return -2;
			else if(type=='e') return -1;

		}
	}else{
		Symbol* s=new Symbol();
		s->defined=false;
		s->name=name;
		s->section="";
		s->id=sym_table->table.size();


		s->flink=new Forward_entry();
		s->flink->patch=sect->size;
		s->flink->type=type;

		sym_table->table.push_back(s);

		off_e->value=s->id;
		sect->relocation.push_back(off_e);


		if(type=='b')
			return '\0';
		else if(type=='w'){
			return '\0';
		}
		else if(type=='r') return -2;
		else if(type=='e') return -1;
	}

	return '\0';

}

unsigned int is_literal(string name){

	if(name=="") return -1;
	int znak=0;
	if(name[0]=='-'){
		znak=1;
		if(name.size()==1) throw new Error_detail("- is not a valid literal",line_number);
	}

	short broj=0;
	for (int i = znak; i < name.size(); ++i)
	{
		broj=broj*10;
		if(isdigit(name[i]))
			broj+=name[i]-48;
		else throw 1;
	}
	if(znak==1)broj=-broj;
	return broj;
}

//WRITE 
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
		cout<<endl<<endl;	
	}	
	cout<<"============================================================================================";	
}
