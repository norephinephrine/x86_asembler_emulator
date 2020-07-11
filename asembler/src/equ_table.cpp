#include "equ_table.h"
unsigned int is_literal(string num);

bool Equ_Table::check_izraz(string izraz,Symbol_table* table,string symbol_name){
	string word;
	bool uspeh=true;

	vector<string> symbols;
	vector<char> operators;

	if(izraz[izraz.size()-1]=='+' || izraz[izraz.size()-1]=='-' || izraz[0] =='+' || izraz[0] =='-') throw new Error_detail("Bad declaration",line_number);
	int iter=0;
	while(iter!=izraz.size()){
		string cur="";

		while(iter<izraz.size() && izraz[iter]!='+' && izraz[iter]!='-'){
			if(izraz[iter]==',') throw new Error_detail("Cant use ',' in expression ",line_number);
			cur+=izraz[iter];
			iter++;
		}

		istringstream help(cur);

		if(!(help>>word)) throw new Error_detail("Bad declaration",line_number);
		if(word.empty() || word==" ") throw new Error_detail("Bad directive/instruction",line_number);

	

		symbols.push_back(word);


		try{
			is_literal(word);
		}
		catch(int g){

			if(word[0]=='.' || word[0]=='$' || isdigit(word[0]) || (word==",")) 
			  		throw new Error_detail("Cant use . $ or number for begging of symbol",line_number);			
			if(table->exists_symbol(word)){
				Symbol* s=table->get(word);
				if(s->defined==false)uspeh=false;
			}else{
				uspeh=false;

				Symbol* s=new Symbol();
				s->name=word;
				s->defined=false;
				s->section="";
				s->id=table->table.size();	
				
				table->table.push_back(s);		
			}
		}



		if(iter<izraz.size()){
			operators.push_back(izraz[iter]);
			iter++;
		}
		if (help >> word) throw new Error_detail("Bad directive/instruction", line_number);
					  

  	}

	//end while

	if(operators.size()!=(symbols.size()-1)) throw new Error_detail("Number of operands and operators doesnt match",line_number);;

	if(uspeh){
			
			solve_izraz(symbols,operators,table,symbol_name);

	}else{
		Equ_entry* e=new Equ_entry(symbol_name,symbols,operators,line_number);
		lista.push_back(e);

	  	if(table->exists_symbol(symbol_name)) {
	  		Symbol* s=table->get(symbol_name);

	  		
	  		if(s->defined || s->equ)
	  			throw 5;
	  		s->equ=true;
	  	}
	  	else{
			Symbol* new_symbol = new Symbol();
			new_symbol->name = symbol_name;
			new_symbol->section="";
			new_symbol->equ=true;
			new_symbol->id=table->table.size();



			new_symbol->defined=false;
			new_symbol->flink=nullptr;
			
			
			table->table.push_back(new_symbol);		  		
	  	}	

	}



	return uspeh;
}

void Equ_Table::solve_izraz(vector<string>operands,vector<char>operation,Symbol_table*table,string symbol_name){
	map<string,int> map;

	_word res;
	Symbol* und=nullptr;
	try{
		res=is_literal(operands[0]);
	}
	catch(int g){
		Symbol* s=table->get(operands[0]);
		map.insert(pair<string,int>(s->section,1));
		if(s->section=="UND")und=s;
		res=s->value;
	}


	
	//ivrsava operacije u vektoriima
	for (int i = 1; i < operands.size(); ++i)
	{
		if(operation[i-1]=='+'){
			try{
				res+=is_literal(operands[i]);
			}
			catch(int g){
				Symbol* s=table->get(operands[i]);

				std::map<string,int>::iterator it=map.find(s->section);
				if(it!=map.end()){
					it->second++;
					if(s->section=="UND") throw 7;
				}else{
					map.insert(pair<string,int>(s->section,1));
					if(s->section=="UND")und=s;
				}
				res+=s->value;
			}

		}else{
			try{
				res-=is_literal(operands[i]);
			}
			catch(int g){
				Symbol* s=table->get(operands[i]);

				std::map<string,int>::iterator it=map.find(s->section);
				if(s->section=="UND") throw 7;
				if(it!=map.end()){
					it->second--;
					
				}else{
					map.insert(pair<string,int>(s->section,1));
				}
				res-=s->value;
			}			
		}
	}





	string n_section="";

	//proverava jel sve ok
	for (std::map<string,int>::iterator i = map.begin(); i != map.end(); ++i)
	{
		if(i->second!=1 && i->second!=0) throw new Error_detail("Bad expression,wrong symbol sections");

		if(i->second==1 && n_section!="")throw new Error_detail("Bad expression,wrong symbol sections");
		else if(i->second==1)n_section=i->first;

	}


  	if(table->exists_symbol(symbol_name)) {
  		Symbol* s= table->get(symbol_name);

  		
  		if(s->defined || s->equ)
  			throw new Error_detail("Cant define symbol with equ if its already defined",line_number);

  		if(n_section =="" && s->visibility=='g')throw new Error_detail("Cant use .global for symbol that has apsolute relocation",line_number);
  		if(n_section =="UND" && s->visibility=='g')throw new Error_detail("Cant use .global for symbol that has UND for section",line_number);

  		if(n_section =="")s->relocatable=false;
  		else s->relocatable=true;

  		if(n_section =="UND")s->und_symbol=und;

  		s->section= n_section;
  		s->value=res;
  		s->defined=true;
  	}
  	else{
		Symbol* new_symbol = new Symbol();
		new_symbol->name = symbol_name;
		new_symbol->value=res;
		new_symbol->id=table->table.size();

  		if(n_section =="")new_symbol->relocatable=false;
  		else new_symbol->relocatable=true;

  		if(n_section =="UND")new_symbol->und_symbol=und;

  		new_symbol->section= n_section;


		new_symbol->defined=true;
		new_symbol->flink=nullptr;
		
		
		table->table.push_back(new_symbol);		  		
  	}	



}





void Equ_Table::solve_all(Symbol_table* sym_table){
	int i=0;

	while(!lista.empty()){
		Equ_entry* entry=lista[i];
		if(entry->is_solved(sym_table)){
			entry->solve(sym_table);

			lista.erase(lista.begin() + i);
			i=0;
			continue;
		}
		i++;
		if(i==lista.size()){
			throw new Error_detail("Cant solve expressions",lista[0]->line_number);
		}
	}
}



//=========================================================================================================
//EQU ENTRY FUNCTIONS
void Equ_entry::solve(Symbol_table* sym_table){
	map<string,int> map;

	_word res;
	Symbol* und=nullptr;
	try{
		res=is_literal(symbols[0]);
	}
	catch(int g){
		Symbol* s=sym_table->get(symbols[0]);
		map.insert(pair<string,int>(s->section,1));
		if(s->section=="UND")und=s;
		res=s->value;
	}


	
	//ivrsava operacije u vektoriima
	for (int i = 1; i < symbols.size(); ++i)
	{
		if(operators[i-1]=='+'){
			try{
				res+=is_literal(symbols[i]);
			}
			catch(int g){
				Symbol* s=sym_table->get(symbols[i]);

				std::map<string,int>::iterator it=map.find(s->section);
				if(it!=map.end()){
					it->second++;
					if(s->section=="UND") throw new Error_detail("Too many extern symbols used",this->line_number);
				}else{
					map.insert(pair<string,int>(s->section,1));
					if(s->section=="UND")und=s;
				}
				res+=s->value;
			}

		}else{
			try{
				res-=is_literal(symbols[i]);
			}
			catch(int g){
				Symbol* s=sym_table->get(symbols[i]);

				std::map<string,int>::iterator it=map.find(s->section);
				if(s->section=="UND")  throw new Error_detail("Cant subtract extern symbol",this->line_number);
				if(it!=map.end()){
					it->second--;
					
				}else{
					map.insert(pair<string,int>(s->section,1));
				}
				res-=s->value;
			}			
		}
	}





	string n_section="";

	//proverava jel sve ok
	for (std::map<string,int>::iterator i = map.begin(); i != map.end(); ++i)
	{
		if(i->second!=1 && i->second!=0) throw new Error_detail("Bad expression,wrong symbol sections");

		if(i->second==1 && n_section!="")throw new Error_detail("Bad expression,wrong symbol sections");
		else if(i->second==1)n_section=i->first;

	}


  	if(sym_table->exists_symbol(name)) {
  		Symbol* s= sym_table->get(name);
  		
  		if(s->defined)
  			throw new Error_detail("Cant define symbol with equ if its already defined",line_number);

  		if(n_section =="" && s->visibility=='g')throw new Error_detail("Cant use .global for symbol that has apsolute relocation",line_number);
  		if(n_section =="UND" && s->visibility=='g')throw new Error_detail("Cant use .global for symbol that has UND for section",line_number);

  		if(n_section =="")s->relocatable=false;
  		else s->relocatable=true;

  		if(n_section =="UND")s->und_symbol=und;

  		s->section= n_section;
  		s->value=res;
  		s->defined=true;
  	}
  	else throw new Error_detail("Symbol doesnt exist due to program error",this->line_number);  			
}

bool Equ_entry::is_solved(Symbol_table* sym_table){
	for (std::vector<string>::iterator i = symbols.begin(); i != symbols.end(); ++i)
	{
		try{
			is_literal(*i);
			continue;
		}
		catch(int g){

		}

		if(sym_table->exists_symbol((*i)) ) {
			Symbol* s= sym_table->get((*i));
			if(s->defined==false)return false;
		}
		else{
			throw new Error_detail("Symbol doesnt exist due to program error",this->line_number);
		}
	}
	return true;
}




std::ostream& operator<<(std::ostream &strm,  Equ_Table &e) {
		strm<<"-----------------------------------------------------------"<<endl;
		for (std::vector<Equ_entry*>::iterator i = e.lista.begin(); i != e.lista.end(); ++i)
		{
			strm<<(*(*i))<<endl;
			strm<<"-----------------------------------------------------------"<<endl;
		}
  return strm;
}

std::ostream& operator<<(std::ostream &strm,  Equ_entry &e) {
  strm<<"Symbol name:"<<e.name<<endl;
  strm<<"Expression:";
  strm<<e.symbols[0];
  for (int i = 1; i < e.symbols.size(); ++i)
  {
  	strm<<" "<<e.operators[i-1]<<" "<<e.symbols[i];
  }
  return strm;
}