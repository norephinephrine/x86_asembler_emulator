#include "ins.h"

map<string,int> registers_map;


void init_registers(){
	for(int i=0;i<8;i++){
		registers_map.insert(pair<string,int>("r"+to_string(i),i));
	}
	registers_map.insert(pair<string,int>("pc",7));
	registers_map.insert(pair<string,int>("sp",6));
	registers_map.insert(pair<string,int>("psw",15));
}

map<string,int>jump_map;

void init_jumps(){

	jump_map.insert(pair<string,int>("int",3));
	jump_map.insert(pair<string,int>("call",4));
	jump_map.insert(pair<string,int>("jmp",5));
	jump_map.insert(pair<string,int>("jeq",6));
	jump_map.insert(pair<string,int>("jne",7));
	jump_map.insert(pair<string,int>("jgt",8));

}
bool is_jump(string param){
		std::map<string,int>::iterator it;

		if(param[param.size()-1]=='w' || param[param.size()-1]=='b'){
			it=jump_map.find(param.substr(0,param.size()-1));
			if(it!=jump_map.end()){
				return true;
			}			
		}


		it=jump_map.find(param);
		if(it!=jump_map.end()){
			return true;
		}
		else return false;
}


map<string,int>non_jump_map;

void init_non_jump(){

	non_jump_map.insert(pair<string,int>("xchg",11));non_jump_map.insert(pair<string,int>("mov",12));
	non_jump_map.insert(pair<string,int>("add",13));non_jump_map.insert(pair<string,int>("sub",14));
	non_jump_map.insert(pair<string,int>("mul",15));non_jump_map.insert(pair<string,int>("div",16));
	non_jump_map.insert(pair<string,int>("cmp",17));non_jump_map.insert(pair<string,int>("not",18));
	non_jump_map.insert(pair<string,int>("and",19));non_jump_map.insert(pair<string,int>("or",20));
	non_jump_map.insert(pair<string,int>("xor",21));non_jump_map.insert(pair<string,int>("test",22));
	non_jump_map.insert(pair<string,int>("shl",23));non_jump_map.insert(pair<string,int>("shr",24));
}
bool is_non_jump(string param){
		std::map<string,int>::iterator it;

		if(param[param.size()-1]=='w' || param[param.size()-1]=='b'){
			it=non_jump_map.find(param.substr(0,param.size()-1));
			if(it!=non_jump_map.end()){
				return true;
			}			
		}


		it=non_jump_map.find(param);
		if(it!=non_jump_map.end()){
			return true;
		}
		else return false;	
}

void process_instuction(istringstream& iss,string word,Symbol_table* sym_table,Section* & current_section){

	init_registers();
	init_jumps();
	init_non_jump();
//HALT ==============================================================================================
	if(word=="halt"){
			current_section->add_byte('\0');
		 	if (iss >> word) throw new Error_detail("Instruction has no arguments", line_number);
	}
//IRET ==============================================================================================
	else if(word=="iret"){
		current_section->add_byte(1<<3);
		 if (iss >> word) throw new Error_detail("Instruction has no arguments", line_number);
	}
//RET ==============================================================================================	
	else if(word=="ret"){
		current_section->add_byte(2<<3);
		 if (iss >> word) throw new Error_detail("Instruction has no arguments", line_number);
	}
//INT ==============================================================================================
//CAll
//JMP-ovi
	else if(is_jump(word)){
		if (current_section == nullptr)throw new Error_detail("No section defined", line_number);
		int broj_bajtova=0;
		std::map<string,int>::iterator it;
		if(word[word.size()-1]=='w' || word[word.size()-1]=='b'){
			it=jump_map.find(word.substr(0,word.size()-1));
			if(it!=jump_map.end()){

				if(word[word.size()-1]=='w')broj_bajtova=2;
				else broj_bajtova=1;

				word=word.substr(0,word.size()-1);
			}			
		}

		int size_bajt=0;
		if(broj_bajtova==2 || broj_bajtova==0)size_bajt=1;
		int code;
		it=jump_map.find(word);
		if(it!=jump_map.end()){
			code=it->second;
		}
		else throw new Error_detail("Jump error occurred",line_number);
		if(!(iss >> word)) throw new Error_detail("Argument missing",line_number);

		current_section->add_byte((_byte)((code<<3) +(size_bajt<<2)));

		int res=jump_param_check(word);
		jump_proccess(res,word,sym_table,current_section,broj_bajtova);

	    if (iss >> word)throw new Error_detail("Too many arguments", line_number);

	}
//POP ==========================================================================================	
	else if(word=="pop" || word=="popw" || word=="popb"){
		
		if (current_section == nullptr)throw new Error_detail("No section defined", line_number);
		int broj_bajtova=0;
		if(word[word.size()-1]=='w')broj_bajtova=2;
		else if(word[word.size()-1]=='b')broj_bajtova=1;

		if(!(iss >> word)) throw new Error_detail("Argument missing",line_number);
		int size_bajt=0;
		if(broj_bajtova==2 || broj_bajtova==0)size_bajt=1;
		current_section->add_byte((_byte)((10<<3) +(size_bajt<<2)));

		int res=non_jump_check(word);
		if(res==1 || res==2)throw new Error_detail("Cant use immediate for destination argument",line_number);
		non_jump_process(res,word,sym_table,current_section,broj_bajtova);

	    if (iss >> word)throw new Error_detail("Too many arguments", line_number);
	}
//PUSH ==========================================================================================	
	else if(word=="push" || word=="pushw" || word=="pushb"){
		
		if (current_section == nullptr)throw new Error_detail("No section defined", line_number);
		int broj_bajtova=0;
		if(word[word.size()-1]=='w')broj_bajtova=2;
		else if(word[word.size()-1]=='b')broj_bajtova=1;

		if(!(iss >> word)) throw new Error_detail("Argument missing",line_number);
		int size_bajt=0;
		if(broj_bajtova==2 || broj_bajtova==0)size_bajt=1;
		current_section->add_byte((_byte)((9<<3) +(size_bajt<<2)));

		int res=non_jump_check(word);
		non_jump_process(res,word,sym_table,current_section,broj_bajtova);

	    if (iss >> word)throw new Error_detail("Too many arguments", line_number);
	}
//Non_Jump =============================================================================================

	else if(is_non_jump(word)){
		bool is_shr=false; if(word=="shr" || word=="shrb" || word=="shrw")is_shr=true;
		bool is_xchg=false; if(word=="xchg" || word=="xchgb" || word=="xchgw")is_xchg=true;
		bool is_cmp=false;  if(word=="cmp" || word=="cmpb" || word=="cmpw")is_cmp=true;
		bool is_test=false; if(word=="test" || word=="testb" || word=="testw")is_test=true;

		

		if (current_section == nullptr)throw new Error_detail("No section defined", line_number);
		int broj_bajtova=0;
		std::map<string,int>::iterator it;
		if(word[word.size()-1]=='w' || word[word.size()-1]=='b'){
			it=non_jump_map.find(word.substr(0,word.size()-1));
			if(it!=non_jump_map.end()){

				if(word[word.size()-1]=='w')broj_bajtova=2;
				else broj_bajtova=1;

				word=word.substr(0,word.size()-1);
			}			
		}

		int size_bajt=0;
		if(broj_bajtova==2 || broj_bajtova==0)size_bajt=1;
		int code;
		it=non_jump_map.find(word);
		if(it!=non_jump_map.end()){
			code=it->second;
		}
		else throw new Error_detail("Non_jump error occurred",line_number);

		current_section->add_byte((_byte)((code<<3) +(size_bajt<<2)));

		
//CHECK STUFF IF ITS OK-------------------------------------------------

		string p=iss.str();
		if(p[p.size()-1]==',') throw new Error_detail("Bad declaration",line_number);		
//FIRST PARAMETER---------------------------------------------------------
		if(!getline(iss,word,',')){
			throw new Error_detail("First argument missing",line_number);
		}

  		istringstream help(word);


  		if(!(help>>word)) throw new Error_detail("First argument missing",line_number);
  		if(word.empty() || word==" ")throw new Error_detail("Argument empty",line_number);
		

		
		int res=non_jump_check(word);
		if((res==1 || res==2) && (is_shr || is_xchg))throw new Error_detail("Cant use immediate for destination argument",line_number);
		non_jump_process(res,word,sym_table,current_section,broj_bajtova);

  		if (help >> word)  throw new Error_detail("First argument is badly formated", line_number);
//SECOND PARAMETER ---------------------------------------------------
		if(!getline(iss,word,',')){
			throw new Error_detail("Second argument missing",line_number);
		}

  		istringstream help1(word);


  		if(!(help1>>word)) throw new Error_detail("Second argument missing",line_number);
  		if(word.empty() || word==" ")throw new Error_detail("Argument empty",line_number);
		

		
		res=non_jump_check(word);
		if((res==1 || res==2) && is_shr==false && is_cmp==false && is_test==false)throw new Error_detail("Cant use immediate for destination argument",line_number);
		non_jump_process(res,word,sym_table,current_section,broj_bajtova);

  		if (help1 >> word)  throw new Error_detail("Second argument is badly formated", line_number);

//CHECK IF OK END -----------------------------------------------------------------------------------------
  		if(getline(iss,word,',')){
  			throw new Error_detail("Too many arguments", line_number);
  		}
	    if (iss >> word)throw new Error_detail("Too many arguments", line_number);
	}

	else if(word!=""){
		 throw new Error_detail("Cant recognize instruction/declaration", line_number);
	}
}




void jump_proccess(int type,string param,Symbol_table* sym_table,Section*&current_section,int broj_bajtova){
	if(type==0) throw new Error_detail("This shouldnt happen",line_number);
	if(type==1){
		unsigned int res=stoi(param);
		current_section->add_byte('\0');
		if(broj_bajtova==2 || broj_bajtova==0){

		_word number=res & 0xFFFF;
  		current_section->add_word(number);				
		}
		else{
		_byte number=res & 0xFF;
  		current_section->add_byte(number);		
		}
	}
	else if(type==2){
		
		current_section->add_byte('\0');
		if(broj_bajtova==2 || broj_bajtova==0){
			int res=symbol_und(param,sym_table,current_section,'w');
			_word number=res & 0xFFFF;
			current_section->add_word(number);			
		}
		else{
			int res=symbol_und(param,sym_table,current_section,'b');
			_byte number=res & 0xFF;
			current_section->add_byte(number);		
		}



	}
	else if(type==3){

	   smatch matches;
	   regex str_expr("^\\*%(r[0-7]|pc|sp|psw)([hl])?$");
	   regex_search(param, matches, str_expr);

		std::map<string,int>::iterator it=registers_map.find(matches[1].str());
		if(it!=registers_map.end()){
			_byte res=it->second;
			int dodatak=0;
			if(matches.size()==3){
				if(matches[2].str()!="" && (broj_bajtova==2 || broj_bajtova==0))throw new Error_detail("Cant define low or high bit if not using the 'b with operation",line_number);
				if(matches[2].str()[0]=='h')dodatak=1;
			}

			current_section->add_byte((res<<1)+(1<<5)+dodatak);
			
		}else{
			throw new Error_detail("Register doesnt exist",line_number);
		}
	}
	else if(type==4){
	   smatch matches;
	   regex str_expr("^\\*\\(%(r[0-7]|pc|sp|psw)\\)$");
	   regex_search(param, matches, str_expr);

		std::map<string,int>::iterator it=registers_map.find(matches[1].str());
		if(it!=registers_map.end()){
			_byte res=it->second;
			current_section->add_byte((res<<1)+(2<<5));
			
		}else{
			throw new Error_detail("Register doesnt exist",line_number);
		}
	}	
	else if(type==5){
		   smatch matches;
		   regex str_expr("^\\*([-]?[0-9]+)\\(%(r[0-7]|pc|sp|psw)\\)$");
		   regex_search(param, matches, str_expr);

			std::map<string,int>::iterator it=registers_map.find(matches[2].str());
			if(it!=registers_map.end()){
				int broj=0;
				/*
				if(matches[2].str()=="pc" || matches[2].str()=="r7"){

					Offset_entry*off_e=new Offset_entry();
					off_e->offset=current_section->size+1;
					off_e->type=R_386_PC16_APS;
					off_e->value=0;	
					current_section->relocation.push_back(off_e);	
					
					broj=2;			
				}*/




				_byte res=it->second;
				current_section->add_byte((res<<1)+(3<<5));

				_word number=stoi(matches[1].str());
				current_section->add_word(number-broj);

				
			}else{
				throw new Error_detail("Register doesnt exist",line_number);
			}


		}
	else if(type==6){
		   smatch matches;
		   regex str_expr("^\\*([^$.0-9][^:]*)\\(%(r[0-6]|sp|psw)\\)$");
		   regex_search(param, matches, str_expr);

			std::map<string,int>::iterator it=registers_map.find(matches[2].str());
			if(it!=registers_map.end()){


				_byte res=it->second;
				current_section->add_byte((res<<1)+(3<<5));


				int hp=symbol_und(matches[1].str(),sym_table,current_section,'w');
				_word number=hp & 0xFFFF;
				current_section->add_word(number);
				
			}else{
				throw new Error_detail("Register doesnt exist",line_number);
			}
		}
	else if(type==7){
		   smatch matches;
		   regex str_expr("^\\*([^$.0-9][^:]*)\\(%(r7|pc)\\)$");
		   regex_search(param, matches, str_expr);

			std::map<string,int>::iterator it=registers_map.find(matches[2].str());
			if(it!=registers_map.end()){


				_byte res=it->second;
				current_section->add_byte((res<<1)+(3<<5));


				int hp=symbol_und(matches[1].str(),sym_table,current_section,'r');
				_word number=hp & 0xFFFF;
				current_section->add_word(number);
				
			}else{
				throw new Error_detail("Register doesnt exist",line_number);
			}
		}
	else if(type==8){
	
	    smatch matches;
		regex str_expr("^\\*([-]?[0-9]+)$");
		regex_search(param, matches, str_expr);
		int res =stoi(matches[1].str());
		_word number=res & 0xFFFF;
		current_section->add_byte(4<<5);
		current_section->add_word(number);
	}
	else if(type==9){

	    smatch matches;
		regex str_expr("^\\*([^$.0-9][^:]*)$");
		regex_search(param, matches, str_expr);
		
		current_section->add_byte(4<<5);
		int res=symbol_und(matches[1].str(),sym_table,current_section,'w');
		_word number=res & 0xFFFF;
		current_section->add_word(number);
	}					

}




int jump_param_check(string param){

   regex str_expr("^[-]?[0-9]+$");
   smatch matches;
   if (regex_search(param, matches, str_expr)){
     return 1;
   }


   str_expr ="^\\*%(r[0-7]|sp|pc|psw)[hl]?$";
   
   if (regex_search(param, matches, str_expr)){
     return 3;
   }

   str_expr= "^\\*\\(%(r[0-7]|pc|sp|psw)\\)$";
   if (regex_search(param, matches, str_expr)){
     return 4;
   }

    str_expr= "^\\*[-]?[0-9]+\\(%(r[0-7]|pc|sp|psw)\\)$";
   if (regex_search(param, matches, str_expr)){
     return 5;
   }


   str_expr= "^\\*[^$.0-9][^:]*\\(%(r[0-6]|sp|psw)\\)$";
   if (regex_search(param, matches, str_expr)){
     return 6;
   }

   str_expr= "^\\*[^$.0-9][^:]*\\(%(r7|pc)\\)$";
   if (regex_search(param, matches, str_expr)){
     return 7;
   }

   str_expr= "^\\*[-]?[0-9]+$";
   if (regex_search(param, matches, str_expr)){
     return 8;
   }

   str_expr= "^\\*[^$.0-9][^:]*$";
   if (regex_search(param, matches, str_expr)){
     return 9;
   }

	
   str_expr="^[^$.0-9][^:]*$";
   if (regex_search(param, matches, str_expr)){
     return 2;
   }
	throw new Error_detail("Badly defined argument",line_number);
	return 0;
}


int non_jump_check(string param){

   regex str_expr("^\\$[-]?[0-9]+$");
   smatch matches;
   if (regex_search(param, matches, str_expr)){
     return 1;
   }


   str_expr ="^%(r[0-7]|sp|pc|psw)[hl]?$";
   
   if (regex_search(param, matches, str_expr)){
     return 3;
   }

   str_expr= "^\\(%(r[0-7]|sp|pc|psw)\\)$";
   if (regex_search(param, matches, str_expr)){
     return 4;
   }

    str_expr= "^[-]?[0-9]+\\(%(r[0-7]|sp|pc|psw)\\)$";
   if (regex_search(param, matches, str_expr)){
     return 5;
   }


   str_expr= "^[^$.0-9][^:]*\\(%(r[0-6]|sp|psw)\\)$";
   if (regex_search(param, matches, str_expr)){
     return 6;
   }

   str_expr= "^[^$.0-9][^:]*\\(%(r7|pc)\\)$";
   if (regex_search(param, matches, str_expr)){
     return 7;
   }

   str_expr= "^[-]?[0-9]+$";
   if (regex_search(param, matches, str_expr)){
     return 8;
   }

   str_expr= "^[^$.0-9][^:]*$";
   if (regex_search(param, matches, str_expr)){
     return 9;
   }

	
   str_expr="^\\$[^$.0-9][^:]*$";
   if (regex_search(param, matches, str_expr)){
     return 2;
   }
	throw new Error_detail("Badly defined argument",line_number);
	return 0;
}

void non_jump_process(int type,string param,Symbol_table* sym_table,Section*&current_section,int broj_bajtova){
	if(type==0) throw new Error_detail("This shouldnt happen",line_number);
	if(type==1){

		unsigned int res=stoi(param.substr(1));
		current_section->add_byte('\0');
		if(broj_bajtova==2 || broj_bajtova==0){

		_word number=res & 0xFFFF;
  		current_section->add_word(number);				
		}
		else{
		_byte number=res & 0xFF;
  		current_section->add_byte(number);		
		}
	}
	else if(type==2){
		
		current_section->add_byte('\0');
		if(broj_bajtova==2 || broj_bajtova==0){
			int res=symbol_und(param.substr(1),sym_table,current_section,'w');
			_word number=res & 0xFFFF;
			current_section->add_word(number);			
		}
		else{
			int res=symbol_und(param.substr(1),sym_table,current_section,'b');
			_byte number=res & 0xFF;
			current_section->add_byte(number);		
		}



	}
	else if(type==3){

	   smatch matches;
	   regex str_expr("^%(r[0-7]|pc|sp|psw)([hl])?$");
	   regex_search(param, matches, str_expr);

		std::map<string,int>::iterator it=registers_map.find(matches[1].str());
		if(it!=registers_map.end()){
			_byte res=it->second;
			int dodatak=0;
			if(matches.size()==3){
				if(matches[2].str()!="" && (broj_bajtova==2 || broj_bajtova==0))throw new Error_detail("Cant define low or high bit if not using the 'b with operation",line_number);
				if(matches[2].str()[0]=='h')dodatak=1;
			}

			current_section->add_byte((res<<1)+(1<<5)+dodatak);
			
		}else{
			throw new Error_detail("Register doesnt exist",line_number);
		}
	}
	else if(type==4){
	   smatch matches;
	   regex str_expr("^\\(%(r[0-7]|pc|sp|psw)\\)$");
	   regex_search(param, matches, str_expr);

		std::map<string,int>::iterator it=registers_map.find(matches[1].str());
		if(it!=registers_map.end()){
			_byte res=it->second;
			current_section->add_byte((res<<1)+(2<<5));
			
		}else{
			throw new Error_detail("Register doesnt exist",line_number);
		}
	}	
	else if(type==5){
		   smatch matches;
		   regex str_expr("^([-]?[0-9]+)\\(%(r[0-7]|pc|sp|psw)\\)$");
		   regex_search(param, matches, str_expr);

			std::map<string,int>::iterator it=registers_map.find(matches[2].str());
			if(it!=registers_map.end()){

				int broj=0;
				/*
				if(matches[2].str()=="pc" || matches[2].str()=="r7"){

					Offset_entry*off_e=new Offset_entry();
					off_e->offset=current_section->size+1;
					off_e->type=R_386_PC16_APS;
					off_e->value=0;	
					current_section->relocation.push_back(off_e);

					broj=2;			
				}*/



				_byte res=it->second;
				current_section->add_byte((res<<1)+(3<<5));

				_word number=stoi(matches[1].str());
				current_section->add_word(number-broj);



				
			}else{
				throw new Error_detail("Register doesnt exist",line_number);
			}
		}
	else if(type==6){
		   smatch matches;
		   regex str_expr("^([^$.0-9][^:]*)\\(%(r[0-6]|sp|psw)\\)$");
		   regex_search(param, matches, str_expr);

			std::map<string,int>::iterator it=registers_map.find(matches[2].str());
			if(it!=registers_map.end()){


				_byte res=it->second;
				current_section->add_byte((res<<1)+(3<<5));


				int hp=symbol_und(matches[1].str(),sym_table,current_section,'w');
				_word number=hp & 0xFFFF;
				current_section->add_word(number);
				
			}else{
				throw new Error_detail("Register doesnt exist",line_number);
			}
		}
	else if(type==7){
		   smatch matches;
		   regex str_expr("^([^$.0-9][^:]*)\\(%(r7|pc)\\)$");
		   regex_search(param, matches, str_expr);

			std::map<string,int>::iterator it=registers_map.find(matches[2].str());
			if(it!=registers_map.end()){


				_byte res=it->second;
				current_section->add_byte((res<<1)+(3<<5));


				int hp=symbol_und(matches[1].str(),sym_table,current_section,'r');
				_word number=hp & 0xFFFF;
				current_section->add_word(number);
				
			}else{
				throw new Error_detail("Register doesnt exist",line_number);
			}
		}
	else if(type==8){
	
	    smatch matches;
		regex str_expr("^([-]?[0-9]+)$");
		regex_search(param, matches, str_expr);
		int res =stoi(matches[1].str());
		_word number=res & 0xFFFF;
		current_section->add_byte(4<<5);
		current_section->add_word(number);
	}
	else if(type==9){

	    smatch matches;
		regex str_expr("^([^$.0-9][^:]*)$");
		regex_search(param, matches, str_expr);
		
		current_section->add_byte(4<<5);
		int res=symbol_und(matches[1].str(),sym_table,current_section,'w');
		_word number=res & 0xFFFF;
		current_section->add_word(number);
	}					

}