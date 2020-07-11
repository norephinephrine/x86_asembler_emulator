#include "symbol.h"

bool Symbol_table::exists_symbol(string s)
{
	for (std::vector<Symbol*>::iterator i = table.begin(); i != table.end(); ++i)
	{
		if((*i)->name==s)
			return true;
	}
	return false;
}

Symbol* Symbol_table::get(string s)
{
	for (std::vector<Symbol*>::iterator i = table.begin(); i != table.end(); ++i)
	{
		if((*i)->name==s)
			return *i;
	}
	return nullptr;
} 



bool Symbol_table::is_all_defined(){
	for (std::vector<Symbol*>::iterator i = table.begin(); i != table.end(); ++i)
	{
		if((*i)->defined==false){
			cout<<endl<<"Symbol "<<((*i)->name)<<" isn't defined"<<endl;
			return false;
		}
	}
	return true;

}








std::ostream& operator<<(std::ostream &strm,  Symbol_table &s) {
		strm<<"-----------------------------------------------------------"<<endl;
		for (std::vector<Symbol*>::iterator i = s.table.begin(); i != s.table.end(); ++i)
		{
			strm<<(*(*i))<<endl;
			strm<<"-----------------------------------------------------------"<<endl;
		}
  return strm;
}

std::ostream& operator<<(std::ostream &strm,  Forward_entry &f) {
  strm<<"Patch:"<<f.patch<<" Type:"<<f.type;
  return strm;
}

std::ostream& operator<<(std::ostream &strm,  Symbol &s) {
  strm<<"Name:"<<s.name<<" Section:"<<s.section<<" Value:"<<s.value<<" V:"<<s.visibility<<" id:"<<s.id<<" D:"<<s.defined<<endl;
  strm<<"Flink:"<<endl;

  Forward_entry* f=s.flink;
  while(f!=nullptr)
	{
		strm<<(*f)<<endl;
		f=f->next;
	}
  return strm;
}


