#include "section.h"

void Section::skip(unsigned int num)
{
	for (int i = 0; i < num; ++i)
	{
		bytes.push_back('\0');
	}
	size+=num;
	return ;
}


void Section::add_byte(_byte num)
{
	bytes.push_back(num);
	size+=1;

	return ;
}

void Section::add_word(_word num)
{
	_byte b=0xFF & num;
	bytes.push_back(b);
	b=(0xFF00 & num)>>8;
	bytes.push_back(b);
	size+=2;

	return ;
}
void Section::change_byte(short location,_byte byte){
	bytes[location]+=byte;
}

void Section::change_word(short location,_word word){

	_word help=(bytes[location+1]<<8)+bytes[location];
	help+=word;


	_byte b=0xFF & help;
	bytes[location]=b;
	b=(0xFF00 & help)>>8;
	bytes[location+1]=b;
}

void Section::fix_up(Symbol_table* sym_table){
	int i=0;
	while(i!=relocation.size()){
		Symbol* s=sym_table->get_by_id(relocation[i]->value);

		if(s!=nullptr){
			if(s->relocatable){
				if(s->und_symbol==nullptr){

					if(s->visibility=='l'){
						if(relocation[i]->type==R_386_16 || relocation[i]->type==R_386_PC16){

							change_word(relocation[i]->offset,s->value);
							Symbol* sect=sym_table->get(s->section);

							if(sect==nullptr) throw 17;
							relocation[i]->value=sect->id;
						}
						else if(relocation[i]->type==R_386_8 || relocation[i]->type==R_386_PC8){
							change_byte(relocation[i]->offset,s->value);

							Symbol* sect=sym_table->get(s->section);

							if(sect==nullptr) throw 17;
							relocation[i]->value=sect->id;

						}

					}
					else if(s->visibility=='g' && relocation[i]->type==R_386_PC16 && s->section==name && name!="UND"){

						change_word(relocation[i]->offset,s->value-relocation[i]->offset);
						relocation.erase(relocation.begin() + i);
						continue;

					}	

				}
				else{
						if(relocation[i]->type==R_386_16 || relocation[i]->type==R_386_PC16){
							change_word(relocation[i]->offset,s->value);
						}
						else if(relocation[i]->type==R_386_8 || relocation[i]->type==R_386_PC8){
							change_byte(relocation[i]->offset,s->value);
						}		

						while(s->und_symbol!=nullptr)s=s->und_symbol;

						relocation[i]->value=s->id;		
				}



			}
			else{
				if(relocation[i]->type==R_386_16 || relocation[i]->type==R_386_PC16)
					change_word(relocation[i]->offset,s->value);
				else if(relocation[i]->type==R_386_8 || relocation[i]->type==R_386_PC8)
					change_byte(relocation[i]->offset,s->value);


				if(relocation[i]->type==R_386_PC16){
					change_word(relocation[i]->offset,2);
					relocation.erase(relocation.begin() + i);
				}
				else if(relocation[i]->type==R_386_PC8){
					throw 28;
				}
				else relocation.erase(relocation.begin() + i);
				continue;
			}
		}
		else throw 1;

		i++;
	}
}





std::ostream& operator<<(std::ostream &strm,  Section &s) {

	strm<<"Name:"<<s.name<<"  Size:"<<s.size<<endl;
	strm<<"Relocation:"<<endl;


	strm<<"/////////////////////////////"<<endl;
	for (std::vector<Offset_entry*>::iterator i = s.relocation.begin(); i != s.relocation.end(); ++i)
	{
		strm<<(*(*i))<<endl;
	}
	strm<<"/////////////////////////////"<<endl;

	strm<<"Bytes:"<<endl;
	int jmp=0;
	for (std::vector<_byte>::iterator i = s.bytes.begin(); i != s.bytes.end(); ++i)
	{
		jmp++;
		strm<<setfill('0') << setw(2)<<hex << (int)(*i)<<" ";
		if(jmp%4==0){
			strm<<'\t';
		}
		if(jmp==20)
		{
			jmp=0;
			strm<<"\n";
		}
	}	


  return strm;
}

std::ostream& operator<<(std::ostream &strm,  Section_Table &s) {
		strm<<"-----------------------------------------------------------"<<endl;
		for (std::vector<Section*>::iterator i = s.section_table.begin(); i != s.section_table.end(); ++i)
		{
			strm<<(*(*i))<<endl;
			strm<<"-----------------------------------------------------------"<<endl;
		}
  return strm;
}

std::ostream& operator<<(std::ostream &strm, Offset_entry &o) {
	strm<<"Offset:"<<o.offset<<" Type:";
	if(o.type==R_386_16)strm<<"R_386_16";
	else if(o.type==R_386_PC16)strm<<"R_386_PC16";
	else if(o.type==R_386_8)strm<<"R_386_8";
	else if(o.type==R_386_PC8)strm<<"R_386_PC8";
	else if(o.type==R_386_PC8_APS)strm<<"R_386_PC8_APS";
	else if(o.type==R_386_PC16_APS)strm<<"R_386_PC16_APS";

	strm<<" Value:"<<o.value;
  return strm;
}

