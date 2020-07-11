#ifndef _Offset_H_
#define _Offset_H_
#include <iostream>


enum Offset_type{
	R_386_16,
	R_386_PC16,
	R_386_8,
	R_386_PC8,
	R_386_PC8_APS,
	R_386_PC16_APS
};


class Offset_entry{
public:
	short offset;
	Offset_type type;
	int value;


	
};

std::ostream& operator<<(std::ostream &strm, Offset_entry &o);
#endif