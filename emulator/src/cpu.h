#ifndef _CPU_H_
#define _CPU_H_

#include "shared.h"
#include <iostream>
#include <stdio.h>
#include <string>
#include <string.h>
#include <chrono>

#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <pthread.h>
using namespace std;


typedef unsigned char _byte;
typedef unsigned short _word;

#define DATA_OUT	0xFF00
#define DATA_IN		0xFF02
#define TIMER_CFG 	0xFF10
#define PC 	7
#define SP 	6


//REGISTERS START=================================================================================
struct Register {
	_byte byte[2];
	_word word(){
		_word w=(byte[1]<<8)+byte[0];
		return w;
	}
	_word set_word(_word num){
		_byte b=0xFF & num;
		byte[0]=b;
		b=(0xFF00 & num)>>8;
		byte[1]=b;
	}
	_word add_word(_word word){

	_word help=(byte[1]<<8)+byte[0];
	help+=word;


	_byte b=0xFF & help;
	byte[0]=b;
	b=(0xFF00 & help)>>8;
	byte[1]=b;	
	}		


};




//PSW
struct{

	_byte byte[2];
	_word word(){
		_word w=(byte[1]<<8)+byte[0];
		return w;
	}
	_word set_word(_word num){
		_byte b=0xFF & num;
		byte[0]=b;
		b=(0xFF00 & num)>>8;
		byte[1]=b;
	}
	_word add_word(_word word){

	_word help=(byte[1]<<8)+byte[0];
	help+=word;


	_byte b=0xFF & help;
	byte[0]=b;
	b=(0xFF00 & help)>>8;
	byte[1]=b;	
	}	


	bool is_I(){
		return (byte[1] & 0x80)!=0;
	}

	bool is_Tl(){
		return (byte[1] & 0x40)!=0;
	}

	bool is_Tr(){
		return (byte[1] & 0x20)!=0;
	}
	bool is_N(){
		return (byte[0] & 0x8)!=0;
	}

	bool is_C(){
		return (byte[0] & 0x4)!=0;
	}

	bool is_O(){
		return (byte[0] & 0x2)!=0;
	}
	bool is_Z(){
		return (byte[0] & 0x1)!=0;
	}



	void set_I(int val){
		if(val==0) byte[1]&=~(1UL << 7);
		else byte[1]|= 1UL << 7;
	}

	void set_Tl(int val){
		if(val==0) byte[1]&=~(1UL << 6);
		else byte[1]|= 1UL << 6;
	}

	void set_Tr(int val){
		if(val==0) byte[1]&=~(1UL << 5);
		else byte[1]|= 1UL << 5;
	}
	void set_N(int val){
		if(val==0) byte[0]&=~(1UL << 3);
		else byte[0]|= 1UL << 3;
	}

	void set_C(int val){
		if(val==0) byte[0]&=~(1UL << 2);
		else byte[0]|= 1UL << 2;
	}

	void set_O(int val){
		if(val==0) byte[0]&=~(1UL << 1);
		else byte[0]|= 1UL << 1;
	}
	void set_Z(int val){
		if(val==0) byte[0]&=~(1UL << 0);
		else byte[0]|= 1UL << 0;
	}	


}psw;



//CODE READING

int check_op(_byte);

_word get_operand(_word&,int,bool,_byte,bool,_word&);




//IO START=================================================================================
bool check_timer(std::chrono::duration<double, std::milli> elapsed_seconds);
void emule_init();
void check_interrupts();
void* keyboard_thread(void *threadid);








//MEMORY PART=================================================================================
extern void emulate();
extern unsigned char* MemBase;

void init_mem();

extern _byte mem_read_b(_word location);
extern void mem_set_b(_word location,_byte);
extern void mem_add_b(_word location,_byte);


extern _word mem_read_w(_word location);
extern void mem_set_w(_word location,_word);
extern void mem_add_w(_word location,_word);

extern void push_b(_byte);
extern void push_w(_word);

extern _word pop_b();
extern _word pop_w();


bool lessBits(_word,_word);

#endif
