#include "cpu.h"


unsigned char* MemBase;
Register registri[8];
_word csip;
bool running=false;
std::chrono::time_point<std::chrono::system_clock> start;
bool bad_code=false;
volatile  char c=0;//for keyboard
volatile  bool keyboard_interrupt;


//THIS IS NEEDED TO ACTIVATE RAW MODE INPUT
static struct termios old, new1;


void resetTermios(void) {
    tcsetattr(0, TCSANOW, &old);
}


void initTermios(int echo) {
    tcgetattr(0, &old); 
    new1 = old;
    new1.c_lflag &= ~ICANON; 
    new1.c_lflag &= echo ? ECHO : ~ECHO; 
    //new1.c_cc[VTIME] = 0; 
    //new1.c_cc[VMIN] = 0;

    tcsetattr(0, TCSANOW, &new1);

    atexit(resetTermios);
}



void emule_init(){
	for (int i = 0; i < 8; ++i)
	{
		registri[i].byte[0]=0;
		registri[i].byte[1]=0;
	}
	psw.byte[0]=0;
	psw.byte[1]=0;

	registri[PC].byte[0]=MemBase[0];
	registri[PC].byte[1]=MemBase[1];
	running=true;
	bad_code=false;
    initTermios(0);
    keyboard_interrupt=false;


    pthread_t  thread;
    int rc= pthread_create(&thread, NULL, keyboard_thread, NULL);
      
    if (rc) {
         cout << "UNABLE TO CREATE THREAD. ENDING PROGRAM" << rc << endl;
         exit(-1);
    }

	start = std::chrono::system_clock::now(); 

}


void set_bad_code(){
	if(bad_code==true){
		cout<<endl<<"FOUND BAD CODE WHILE IN INTERRUPT FOR BAD CODE."<<endl<<"Ending program..";
		exit(-1);
	}
	bad_code=true;	
}

void inc_pc(_word& csip){
	csip++;
	if(csip==64*1024){
		cout<<endl<<endl<<"PC ADDRES WENT OVER MAXIMUM MEMORY SIZE";
		exit(-1);
	}
}

void emulate(){
	
	emule_init();
	bool l_h_set,l_h_set_prvi;
	_byte address_info;	
	int type,type_prvi;
	_word jump_address;
	_word operand1,operand2;
	_word dest1=0;
	_word dest2=0;
	_word result=0;
	short signed_result,signed_operand1,signed_operand2;
	_byte reg_number,reg_number_prvi;
	while(running){


		csip=registri[PC].word();
		_byte info=mem_read_b(csip);
		
		_byte opcode=(info & 0xF8)>>3;
		bool size=((info & 0x04)!=0);

		bool wrong_code=((info & 0x03)!=0);




		if(wrong_code==false){
			switch(opcode) {
			  case 0: //HALT
			  	running=false;
			    break;
			  case 1: //IRET

			  	if(size==1){
						set_bad_code();  		
			  	}else{
					_word w=pop_w();
					psw.set_word(w);

					w=pop_w();
					registri[PC].set_word(w);		  		
			  	}

	    


			    break;
//===================================================================================================================================
			  case 2: //RET
			  	if(size==1){
						set_bad_code();  		
			  	}else{
					_word w=pop_w();
					registri[PC].set_word(w);				  		
			  	}


			  break;
//===================================================================================================================================
			  case 3: //INT
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size))set_bad_code();
			  	else{
			  		operand1=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);

					push_w(csip);
					push_w(psw.word());
					psw.set_I(1);			  		
			  		registri[PC].set_word(mem_read_w((operand1%8)*2));		  		

			  	}

			  
			  break;
//===================================================================================================================================			  
			  case 4: //CALL
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size))set_bad_code();
			  	else{
			  		operand1=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);

					push_w(csip);			  		
			  		registri[PC].set_word(operand1);		  		

			  	}			  
			  
			  break;

//===================================================================================================================================
			  case 5: //JMP
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size))set_bad_code();
			  	else{
			  		operand1=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);	
	
			  		registri[PC].set_word(operand1);		  		

			  	}			  
			  
			  break;	

//===================================================================================================================================
			  case 6: //JEQ
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size))set_bad_code();
			  	else{
			  		operand1=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);		

			  		if(psw.is_Z())  		
			  			registri[PC].set_word(operand1);		  		
			  		else registri[PC].set_word(csip);	 
			  	}			  
			  
			  break;
//===================================================================================================================================
			  case 7: //JNE
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size))set_bad_code();
			  	else{
			  		operand1=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);		

			  		if(!psw.is_Z())  		
			  			registri[PC].set_word(operand1);		  		
			  		else registri[PC].set_word(csip);	 
			  	}			  
			  
			  break;
//===================================================================================================================================
			  case 8: //JGT
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size))set_bad_code();
			  	else{
			  		operand1=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);		

			  		if(!(psw.is_O()!=psw.is_N()) && !psw.is_Z())  		
			  			registri[PC].set_word(operand1);	
			  		else registri[PC].set_word(csip);	  		

			  	}			  
			  
			  break;
//===================================================================================================================================
			  case 9: //PUSH
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size))set_bad_code();
			  	else{
			  		operand1=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);		

					if(size==1)push_w(operand1);
					else push_b(operand1 && 0xFF);		  		

					registri[PC].set_word(csip);
			  	}			  
			  
			  break;	

//===================================================================================================================================
			  case 10: //POP
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size) ||type==0)set_bad_code();
			  	else{

					if(size==1)operand1=pop_w();
					else operand1=pop_w();	

					get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);	  


					reg_number=(address_info & 0x1E)>>1;
					registri[PC].set_word(csip);

					if(type!=1){
						mem_set_w(dest1,operand1);
					}else
					{
						if(size)
							registri[reg_number].set_word(operand1);
						else{
							if(l_h_set)registri[reg_number].byte[1]=operand1 & 0xFF;
							else registri[reg_number].byte[0]=operand1 & 0xFF;
						}
					}

							

			  	}			  
			  
			  break;
//===================================================================================================================================		  
			  case 11: //XCHG
			  	//CITANJE PRVOG OPERANDA
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size) ||type==0){
			  		set_bad_code();
			  		break;
			  	}
			  	else{

					operand1=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);	
					reg_number=(address_info & 0x1E)>>1;  
			  	}	

			  	type_prvi=type;
			  	l_h_set_prvi=l_h_set;
			  	reg_number_prvi=reg_number;


			  	//Citanje drugog operanda
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size) ||type==0){
			  		set_bad_code();
			  		break;
			  	}
			  	else{
					operand2=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest2);	
					reg_number=(address_info & 0x1E)>>1;    
			  	}	


			  	registri[PC].set_word(csip);
			  	//ZAMENA PRVOG OPERANDA
				

				if(type_prvi!=1){
					mem_set_w(dest1,operand2);
				}else
				{
					if(size)
						registri[reg_number_prvi].set_word(operand2);
					else{
						if(l_h_set_prvi)registri[reg_number_prvi].byte[1]=operand2 & 0xFF;
						else registri[reg_number_prvi].byte[0]=operand2 & 0xFF;
					}
				}

				//ZAMENA DRUGOG OPERANDA
				if(type!=1){
					mem_set_w(dest2,operand1);
				}else
				{
					if(size)
						registri[reg_number].set_word(operand1);
					else{
						if(l_h_set)registri[reg_number].byte[1]=operand1 & 0xFF;
						else registri[reg_number].byte[0]=operand1 & 0xFF;
					}
				}
			  
			  break;

//===================================================================================================================================
			  case 12: //MOV
			  	//CITANJE PRVOG OPERANDA
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size)){
			  		set_bad_code();
			  		break;
			  	}
			  	else{

					operand1=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);	
					reg_number=(address_info & 0x1E)>>1;  
			  	}	




			  	//Citanje drugog operanda
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size) ||type==0){
			  		set_bad_code();
			  		break;
			  	}
			  	else{
					operand2=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest2);	
					reg_number=(address_info & 0x1E)>>1;    
			  	}	


			  	registri[PC].set_word(csip);
				

				if(type!=1){
					mem_set_w(dest2,operand1);
				}else
				{
					if(size)
						registri[reg_number].set_word(operand1);
					else{
						if(l_h_set)registri[reg_number].byte[1]=operand1 & 0xFF;
						else registri[reg_number].byte[0]=operand1 & 0xFF;
					}
				}

				//SET FLAGS
				if(operand1<0)psw.set_N(1);
				else  psw.set_N(0);

				if(operand1==0)psw.set_Z(1);
				else  psw.set_Z(0);
			  
			  break;
//===================================================================================================================================
			  case 13: //ADD
			  	//CITANJE PRVOG OPERANDA
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size) ){
			  		set_bad_code();
			  		break;
			  	}
			  	else{

					operand1=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);	
					reg_number=(address_info & 0x1E)>>1;  
			  	}	



			  	//Citanje drugog operanda
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size) ||type==0){
			  		set_bad_code();
			  		break;
			  	}
			  	else{
					operand2=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest2);	
					reg_number=(address_info & 0x1E)>>1;    
			  	}	


			  	registri[PC].set_word(csip);
				

			  	result=operand1+operand2;
				if(type!=1){
					mem_set_w(dest2,result);
				}else
				{
					if(size)
						registri[reg_number].set_word(result);
					else{
						if(l_h_set)registri[reg_number].byte[1]=result & 0xFF;
						else registri[reg_number].byte[0]=result & 0xFF;
					}
				}


				signed_result=result;
				signed_operand2=operand2;
				signed_operand1=operand1;
				//SET FLAGS
				if(signed_result<0)psw.set_N(1);
				else  psw.set_N(0);

				if(signed_result==0)psw.set_Z(1);
				else  psw.set_Z(0);

				if((signed_result>= 0 && signed_operand1<0 && signed_operand2<0) &&  (signed_result< 0 && signed_operand1>0 && signed_operand2>0))psw.set_O(1);
				else psw.set_O(0);

			   if((signed_operand1<0 && signed_operand2<0) ||
			    (signed_operand1>0 && signed_operand2<0 && signed_result>=0) || (signed_operand2>0 && signed_operand1<0 && signed_result>=0))psw.set_C(1);
				else psw.set_C(0);
			  
			  break;
//===================================================================================================================================
			  case 14: //SUB
			  	//CITANJE PRVOG OPERANDA
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size) ){
			  		set_bad_code();
			  		break;
			  	}
			  	else{

					operand1=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);	
					reg_number=(address_info & 0x1E)>>1;  
			  	}	



			  	//Citanje drugog operanda
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size) ||type==0){
			  		set_bad_code();
			  		break;
			  	}
			  	else{
					operand2=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest2);	
					reg_number=(address_info & 0x1E)>>1;    
			  	}	


			  	registri[PC].set_word(csip);
				

			  	result=operand2-operand1;
				if(type!=1){
					mem_set_w(dest2,result);
				}else
				{
					if(size)
						registri[reg_number].set_word(result);
					else{
						if(l_h_set)registri[reg_number].byte[1]=result & 0xFF;
						else registri[reg_number].byte[0]=result & 0xFF;
					}
				}

				signed_result=result;
				signed_operand2=operand2;
				signed_operand1=operand1;
				//SET FLAGS
				if(signed_result<0)psw.set_N(1);
				else  psw.set_N(0);

				if(signed_result==0)psw.set_Z(1);
				else  psw.set_Z(0);


				if((signed_result< 0 && signed_operand1<0 && signed_operand2>0) &&  (signed_result>= 0 && signed_operand1>0 && signed_operand2<0))psw.set_O(1);
				else psw.set_O(0);

			   if((signed_operand1<0 && signed_operand2>0) || (signed_operand2>0 && signed_operand1>0 && signed_result<0) 
			   	|| (signed_operand2<0 && signed_operand1<0 && signed_result>=0))psw.set_C(1);
				else psw.set_C(0);
			  
			  break;		  
//===================================================================================================================================
			  case 15: //MUL
			  	//CITANJE PRVOG OPERANDA
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size)){
			  		set_bad_code();
			  		break;
			  	}
			  	else{

					operand1=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);	
					reg_number=(address_info & 0x1E)>>1;  
			  	}	



			  	//Citanje drugog operanda
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size) ||type==0){
			  		set_bad_code();
			  		break;
			  	}
			  	else{
					operand2=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest2);	
					reg_number=(address_info & 0x1E)>>1;    
			  	}	


			  	registri[PC].set_word(csip);
				

			  	result=operand2*operand1;
				if(type!=1){
					mem_set_w(dest2,result);
				}else
				{
					if(size)
						registri[reg_number].set_word(result);
					else{
						if(l_h_set)registri[reg_number].byte[1]=result & 0xFF;
						else registri[reg_number].byte[0]=result & 0xFF;
					}
				}

				signed_result=result;
				signed_operand2=operand2;
				signed_operand1=operand1;
				//SET FLAGS
				if(signed_result<0)psw.set_N(1);
				else  psw.set_N(0);

				if(signed_result==0)psw.set_Z(1);
				else  psw.set_Z(0);
			  
			  break;

//===================================================================================================================================
			  case 16: //DIV
			  	//CITANJE PRVOG OPERANDA
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size)){
			  		set_bad_code();
			  		break;
			  	}
			  	else{

					operand1=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);	
					reg_number=(address_info & 0x1E)>>1;  
			  	}	



			  	//Citanje drugog operanda
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size) ||type==0){
			  		set_bad_code();
			  		break;
			  	}
			  	else{
					operand2=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest2);	
					reg_number=(address_info & 0x1E)>>1;    
			  	}	


			  	registri[PC].set_word(csip);
				

			  	result=operand2/operand1;
				if(type!=1){
					mem_set_w(dest2,result);
				}else
				{
					if(size)
						registri[reg_number].set_word(result);
					else{
						if(l_h_set)registri[reg_number].byte[1]=result & 0xFF;
						else registri[reg_number].byte[0]=result & 0xFF;
					}
				}

				signed_result=result;
				signed_operand2=operand2;
				signed_operand1=operand1;
				//SET FLAGS
				if(signed_result<0)psw.set_N(1);
				else  psw.set_N(0);

				if(signed_result==0)psw.set_Z(1);
				else  psw.set_Z(0);
			  
			  break;
//===================================================================================================================================
			  case 17: //CMP
			  	//CITANJE PRVOG OPERANDA
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size)){
			  		set_bad_code();
			  		break;
			  	}
			  	else{

					operand1=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);	
					reg_number=(address_info & 0x1E)>>1;  
			  	}	



			  	//Citanje drugog operanda
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size)){
			  		set_bad_code();
			  		break;
			  	}
			  	else{
					operand2=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest2);	
					reg_number=(address_info & 0x1E)>>1;    
			  	}	


			  	registri[PC].set_word(csip);
				

			  	result=operand2-operand1;



				signed_result=result;
				signed_operand2=operand2;
				signed_operand1=operand1;
				//SET FLAGS
				if(signed_result<0)psw.set_N(1);
				else  psw.set_N(0);

				if(signed_result==0)psw.set_Z(1);
				else  psw.set_Z(0);


				if((signed_result< 0 && signed_operand1<0 && signed_operand2>0) &&  (signed_result>= 0 && signed_operand1>0 && signed_operand2<0))psw.set_O(1);
				else psw.set_O(0);

			   if((signed_operand1<0 && signed_operand2>0) || (signed_operand2>0 && signed_operand1>0 && signed_result<0) 
			   	|| (signed_operand2<0 && signed_operand1<0 && signed_result>=0))psw.set_C(1);
				else psw.set_C(0);
			  
			  break;

//===================================================================================================================================
			  case 18: //NOT
			  	//CITANJE PRVOG OPERANDA
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size)){
			  		set_bad_code();
			  		break;
			  	}
			  	else{

					operand1=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);	
					reg_number=(address_info & 0x1E)>>1;  
			  	}	



			  	//Citanje drugog operanda
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size) ||type==0){
			  		set_bad_code();
			  		break;
			  	}
			  	else{
					operand2=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest2);	
					reg_number=(address_info & 0x1E)>>1;    
			  	}	


			  	registri[PC].set_word(csip);
				

			  	result=~operand1;
				if(type!=1){
					mem_set_w(dest2,result);
				}else
				{
					if(size)
						registri[reg_number].set_word(result);
					else{
						if(l_h_set)registri[reg_number].byte[1]=result & 0xFF;
						else registri[reg_number].byte[0]=result & 0xFF;
					}
				}

				signed_result=result;
				signed_operand2=operand2;
				signed_operand1=operand1;
				//SET FLAGS
				if(signed_result<0)psw.set_N(1);
				else  psw.set_N(0);

				if(signed_result==0)psw.set_Z(1);
				else  psw.set_Z(0);
			  
			  break;

//===================================================================================================================================
			  case 19: //AND
			  	//CITANJE PRVOG OPERANDA
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size) ){
			  		set_bad_code();
			  		break;
			  	}
			  	else{

					operand1=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);	
					reg_number=(address_info & 0x1E)>>1;  
			  	}	



			  	//Citanje drugog operanda
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size) ||type==0)set_bad_code();
			  	else{
					operand2=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest2);	
					reg_number=(address_info & 0x1E)>>1;    
			  	}	


			  	registri[PC].set_word(csip);
				

			  	result=operand1 & operand2;
				if(type!=1){
					mem_set_w(dest2,result);
				}else
				{
					if(size)
						registri[reg_number].set_word(result);
					else{
						if(l_h_set)registri[reg_number].byte[1]=result & 0xFF;
						else registri[reg_number].byte[0]=result & 0xFF;
					}
				}

				signed_result=result;
				signed_operand2=operand2;
				signed_operand1=operand1;
				//SET FLAGS
				if(signed_result<0)psw.set_N(1);
				else  psw.set_N(0);

				if(signed_result==0)psw.set_Z(1);
				else  psw.set_Z(0);
			  
			  break;
//===================================================================================================================================
			  case 20: //OR
			  	//CITANJE PRVOG OPERANDA
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size)){
			  		set_bad_code();
			  		break;
			  	}
			  	else{

					operand1=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);	
					reg_number=(address_info & 0x1E)>>1;  
			  	}	



			  	//Citanje drugog operanda
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size) ||type==0){
			  		set_bad_code();
			  		break;
			  	}
			  	else{
					operand2=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest2);	
					reg_number=(address_info & 0x1E)>>1;    
			  	}	


			  	registri[PC].set_word(csip);
				

			  	result=operand1 | operand2;
				if(type!=1){
					mem_set_w(dest2,result);
				}else
				{
					if(size)
						registri[reg_number].set_word(result);
					else{
						if(l_h_set)registri[reg_number].byte[1]=result & 0xFF;
						else registri[reg_number].byte[0]=result & 0xFF;
					}
				}

				signed_result=result;
				signed_operand2=operand2;
				signed_operand1=operand1;
				//SET FLAGS
				if(signed_result<0)psw.set_N(1);
				else  psw.set_N(0);

				if(signed_result==0)psw.set_Z(1);
				else  psw.set_Z(0);
			  
			  break;


//===================================================================================================================================
			  case 21: //XOR
			  	//CITANJE PRVOG OPERANDA
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size)){
			  		set_bad_code();
			  		break;
			  	}
			  	else{

					operand1=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);	
					reg_number=(address_info & 0x1E)>>1;  
			  	}	



			  	//Citanje drugog operanda
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size) ||type==0){
			  		set_bad_code();
			  		break;
			  	}
			  	else{
					operand2=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest2);	
					reg_number=(address_info & 0x1E)>>1;    
			  	}	


			  	registri[PC].set_word(csip);
				

			  	result=operand1 ^ operand2;
				if(type!=1){
					mem_set_w(dest2,result);
				}else
				{
					if(size)
						registri[reg_number].set_word(result);
					else{
						if(l_h_set)registri[reg_number].byte[1]=result & 0xFF;
						else registri[reg_number].byte[0]=result & 0xFF;
					}
				}

				signed_result=result;
				signed_operand2=operand2;
				signed_operand1=operand1;
				//SET FLAGS
				if(signed_result<0)psw.set_N(1);
				else  psw.set_N(0);

				if(signed_result==0)psw.set_Z(1);
				else  psw.set_Z(0);
			  
			  break;

//===================================================================================================================================
			  case 22: //TEST
			  	//CITANJE PRVOG OPERANDA
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size)){
			  		set_bad_code();
			  		break;
			  	}
			  	else{

					operand1=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);	
					reg_number=(address_info & 0x1E)>>1;  
			  	}	



			  	//Citanje drugog operanda
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size)){
			  		set_bad_code();
			  		break;
			  	}
			  	else{
					operand2=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest2);	
					reg_number=(address_info & 0x1E)>>1;    
			  	}	


			  	registri[PC].set_word(csip);
				

			  	result=operand1 & operand2;

				signed_result=result;
				signed_operand2=operand2;
				signed_operand1=operand1;
				//SET FLAGS
				if(signed_result<0)psw.set_N(1);
				else  psw.set_N(0);

				if(signed_result==0)psw.set_Z(1);
				else  psw.set_Z(0);
			  
			  break;

//===================================================================================================================================
			  case 23: //Shl
			  	//CITANJE PRVOG OPERANDA
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size)){
			  		set_bad_code();
			  		break;
			  	}
			  	else{

					operand1=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);	
					reg_number=(address_info & 0x1E)>>1;  
			  	}	



			  	//Citanje drugog operanda
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size) ||type==0){
			  		set_bad_code();
			  		break;
			  	}
			  	else{
					operand2=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest2);	
					reg_number=(address_info & 0x1E)>>1;    
			  	}	


			  	registri[PC].set_word(csip);
				

				
				signed_operand1=operand1;

			  	if(signed_operand1>32 || signed_operand1<0){
			  		set_bad_code();
			  		break;
			  	}

			  	result=operand2 << signed_operand1;


				if(type!=1){
					mem_set_w(dest2,result);
				}else
				{
					if(size)
						registri[reg_number].set_word(result);
					else{
						if(l_h_set)registri[reg_number].byte[1]=result & 0xFF;
						else registri[reg_number].byte[0]=result & 0xFF;
					}
				}

				signed_result=result;
				signed_operand2=operand2;
				//SET FLAGS
				if(signed_result<0)psw.set_N(1);
				else  psw.set_N(0);

				if(signed_result==0)psw.set_Z(1);
				else  psw.set_Z(0);
			  	
			  	if(signed_operand1>0 && signed_operand1<17 && ((1<<(16-signed_operand1)) & signed_operand2))psw.set_C(1);
			  	else psw.set_C(0);

			  break;


//===================================================================================================================================
			  case 24: //Shr
			  	//CITANJE PRVOG OPERANDA
			  	inc_pc(csip);
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size)  ||type==0){
			  		set_bad_code();
			  		break;
			  	}
			  	else{

					operand1=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest1);	
					reg_number=(address_info & 0x1E)>>1;  
			  	}	

			  	type_prvi=type;
			  	l_h_set_prvi=l_h_set;
			  	reg_number_prvi=reg_number;

			  	//Citanje drugog operanda
			  	address_info=mem_read_b(csip);

			  	l_h_set=((address_info & 0x01)!=0);
			  	type=check_op(address_info);

			  	if(type==-1 || (l_h_set && size)){
			  		set_bad_code();
			  		break;
			  	}
			  	else{
					operand2=get_operand(csip,type,size,(address_info & 0x1E)>>1,l_h_set,dest2);	
					reg_number=(address_info & 0x1E)>>1;    
			  	}	


			  	registri[PC].set_word(csip);
				
			  	signed_operand2=operand2;
			  	signed_operand1=operand1;
			  	if(operand2>32 || operand2<0){
			  		set_bad_code();
			  		break;
			  	}

			  	result=(signed_operand1) >>signed_operand2;

				if(type_prvi!=1){
					mem_set_w(dest1,result);
				}else
				{
					if(size)
						registri[reg_number_prvi].set_word(result);
					else{
						if(l_h_set_prvi)registri[reg_number_prvi].byte[1]=result & 0xFF;
						else registri[reg_number_prvi].byte[0]=result & 0xFF;
					}
				}

				signed_result=result;
				

				//SET FLAGS
				if(signed_result<0)psw.set_N(1);
				else  psw.set_N(0);

				if(signed_result==0)psw.set_Z(1);
				else  psw.set_Z(0);
			  	
			  	if(signed_operand2>0 && signed_operand2<17 && ((1<<(signed_operand2-1)) & signed_operand1))psw.set_C(1);
			  	else psw.set_C(0);



			  break;
			  default:
			  	set_bad_code();  	
			   
			}

		}else{
			set_bad_code();
		}



		check_interrupts();


	}
}













// MEMORY DEFINITIONS
void init_mem() {
	MemBase = new unsigned char[64*1024];
		if (!MemBase) 
		{
			throw new Error_detail("Cant allocate memory");
		}
		memset((void*)MemBase,0,64*1024);
}

//BYTE OPERATIONS
 _byte mem_read_b(_word location){
	return MemBase[location];	
 }
 void mem_set_b(_word location,_byte byte){
 	MemBase[location]=byte;
 }
 void mem_add_b(_word location,_byte byte){
 	MemBase[location]+=byte;
 }


 _word mem_read_w(_word location){

	if(location==(64*1024-1)){
		cout<<endl<<endl<<"Trying to read memory over MEMORY_MAX";
		exit(-1);	
	}


	_word help=(MemBase[location+1]<<8)+MemBase[location];
	return help;
}


 void mem_set_w(_word location,_word num){

	if(location==(64*1024-1)){
		cout<<endl<<endl<<"Trying to read memory over MEMORY_MAX";
		exit(-1);	
	}



	_byte b=0xFF & num;
	MemBase[location]=b;
	b=(0xFF00 & num)>>8;
	MemBase[location+1]=b;

}
void mem_add_w(_word location,_word word){

	if(location==(64*1024-1)){
		cout<<endl<<endl<<"Trying to read memory over MEMORY_MAX";
		exit(-1);	
	}

	_word help=(MemBase[location+1]<<8)+MemBase[location];
	help+=word;


	_byte b=0xFF & help;
	MemBase[location]=b;
	b=(0xFF00 & help)>>8;
	MemBase[location+1]=b;	
}


// STACK OPERATIONS
void push_b(_byte b){
	_word word=0;
	if(b & 0x80)word=(0xFF)<<8 + b;
	else word=b;

	if(registri[SP].word()==1 || registri[SP].word()==0){
		cout<<endl<<endl<<"STACK UNDERFLOW";
		exit(-1);
	}

	registri[SP].add_word(-2);

	mem_set_w(registri[SP].word(),word);


}
void push_w(_word w){
	if(registri[SP].word()==1 || registri[SP].word()==0){
		cout<<endl<<endl<<"STACK UNDERFLOW";
		exit(-1);
	}

	registri[SP].add_word(-2);

	mem_set_w(registri[SP].word(),w);	
}

_word pop_b(){

	_byte b=mem_read_b(registri[SP].word());

	_word word=0;
	if(b & 0x80)word=(0xFF)<<8 + b;
	else word=b;

	if(registri[SP].word()==(64*1024-1) || registri[SP].word()==(64*1024-2)){
		cout<<endl<<endl<<"STACK OVERFLOW";
		exit(-1);
	}

	registri[SP].add_word(2);

	return word;

}
_word pop_w(){
	_word word=mem_read_w(registri[SP].word());

	if(registri[SP].word()==(64*1024-1) || registri[SP].word()==(64*1024-2)){
		cout<<endl<<endl<<"STACK OVERFLOW";
		exit(-1);
	}

	registri[SP].add_word(2);

	return word;
}





//IO DEFINITIONS


//CHECKS TIMER CONFIG
bool check_timer(std::chrono::duration<double, std::milli> elapsed_seconds){

	_byte b=0x07 & MemBase[TIMER_CFG];
	int time;
	if(b==0) time=500;
	else if(b==1)time=1000;
	else if(b==2)time=1500;
	else if(b==3)time=2000;
	else if(b==4)time=5000;
	else if(b==5)time=10000;
	else if(b==6)time=30000;
	else if(b==7)time=60000;



	if(elapsed_seconds>=std::chrono::milliseconds(time))
		return true;
	return false;
}

void check_interrupts(){

	//CHECKS IF THERE IS A ERROR


	if(bad_code){
		push_w(registri[PC].word());
		push_w(psw.word());
		psw.set_I(1);

		registri[PC].set_word(mem_read_w(2));
		if(mem_read_w(2)==0)running=false;
		bad_code=false;

	}


	std::chrono::time_point<std::chrono::system_clock>  end; 
	end = std::chrono::system_clock::now(); 
	std::chrono::duration<double, std::milli> elapsed_seconds = end - start;  

	if(check_timer(elapsed_seconds)){
		start=end;

		if(!(psw.is_I() || psw.is_Tr())){
			push_w(registri[PC].word());
			push_w(psw.word());
			psw.set_I(1);

			registri[PC].set_word(mem_read_w(4));

		}

	}


	//CHECKS IF THERE IS A KEYBOARD INTERRUPT
	if(keyboard_interrupt){

		if(!(psw.is_I() || psw.is_Tl())){
			MemBase[DATA_IN]=c;
			c=0;

			keyboard_interrupt=false;
			push_w(registri[PC].word());
			push_w(psw.word());
			psw.set_I(1);

			registri[PC].set_word(mem_read_w(6));

		}		
	}


	//CHECKS IF THERE IS A CHARACTER TO WRITE TO TERMINAL
	if(MemBase[DATA_OUT]!=0 || MemBase[DATA_OUT]!=0){


		char c1=(char)((MemBase[DATA_OUT]));
		char c2=(char)((MemBase[DATA_OUT+1]));
		MemBase[DATA_OUT]=0;
		MemBase[DATA_OUT+1]=0;

		cout<<c1;
		cout.flush();

	}


}




//CHEKS FOR OPERATION OPERANDS
int check_op(_byte address_info){

	_byte address_style=(address_info & 0xE0)>>5;
	bool l_h_set=((address_info & 0x01)!=0);

	_byte register_numb=(address_info & 0x1E)>>1;
	if(register_numb>7 && register_numb<15)return -1;


	switch(address_style) {
	    case 0 :
	    	if(l_h_set || register_numb!=0) return -1;


	   		return 0;
	   		break; 
	    case 1  :
	    	return 1;
	   		break; 

	    case 2  :
	    	if(l_h_set) return -1;

	    	return 2;
	   		break;
	   	case 3  :
	   		if(l_h_set) return -1;

	    	return 3;
	   		break;
	   	case 4  :
	   		if(l_h_set || register_numb!=0) return -1;
	    	return 4;
	   		break;		

	  
	    default : 
	    return -1;
	}
}

_word get_operand(_word& csip,int type,bool size,_byte register_numb,bool low_high,_word& dest1){

	_byte byte_low,byte_high;
	_word offset=0;

	int dodatak=0;
	if(low_high)dodatak=1;
	_word word=0;

	switch(type) {
	    case 0 : //IMM
	    	inc_pc(csip);
	    	byte_low=mem_read_b(csip);


	    	if(size){
	    		inc_pc(csip);
	    		byte_high=mem_read_b(csip);
	    		word=(byte_high<<8) + byte_low;

	    	}else{
				if(byte_low & 0x80)word=(0xFF)<<8 + byte_low;
				else word=byte_low;

	    	}

	    	inc_pc(csip);
	   		return word;
	   		break; 
	    case 1  ://REG DIR
	    	inc_pc(csip);
	    	if(size){
	    		if(register_numb==7)word=csip;
	    		else word=registri[register_numb].word();

	    	}else{
	    		if(register_numb==7)byte_low=csip & 0xFF;
	    		else byte_low=registri[register_numb].byte[dodatak];

				if(byte_low & 0x80)word=(0xFF)<<8 + byte_low;
				else word=byte_low;

	    	}
	    	
	   		return word;
	   		break; 

	    case 2  : //REG IND
	    	inc_pc(csip);
	    	if(size){
	    		if(register_numb==7)word=csip;
	    		else word=registri[register_numb].word();

	    		dest1=word;
	    		word=mem_read_w(word);

	    	}else{
	    		if(register_numb==7)word=csip;
	    		else word=registri[register_numb].word();

	    		dest1=word;
	    		byte_low=mem_read_b(word);

				if(byte_low & 0x80)word=(0xFF)<<8 + byte_low;
				else word=byte_low;


	    	}
	    	
	    	
	   		return word;
	   		break;
	   	case 3  : //REG IND + POMERAJ
	    	inc_pc(csip);
	    	inc_pc(csip);

	    	offset=(mem_read_b(csip)<<8)+mem_read_b(csip-1);

	    	inc_pc(csip);
	    	if(size){
	    		if(register_numb==7)word=csip;
	    		else word=registri[register_numb].word();

	    		dest1=word+offset;
	    		word=mem_read_w(word+offset);

	    	}else{
	    		if(register_numb==7)word=csip;
	    		else word=registri[register_numb].word();


	    		dest1=word+offset;
	    		byte_low=mem_read_b(word+offset);

				if(byte_low & 0x80)word=(0xFF)<<8 + byte_low;
				else word=byte_low;
	    		

	    	}
	    	
	    	
	   		return word;
	   		break;
	   	case 4  : //MEM DIR
	    	inc_pc(csip);
	    	inc_pc(csip);

	    	offset=(mem_read_b(csip)<<8)+mem_read_b(csip-1);
	    	inc_pc(csip);

	    	dest1=offset;
	    	if(size){
	    		word=mem_read_w(offset);

	    	}else{

	    		byte_low=mem_read_b(offset);
				if(byte_low & 0x80)word=(0xFF)<<8 + byte_low;
				else word=byte_low;
	    		

	    	}


	    	return word;
	   		break;		

	  
	    default : 
	    return word;
	}

}

bool lessBits(_word w1,_word w2) 
{ 
    unsigned int count1 = 0; 
    unsigned int count2=0;
    while (w1) { 
        count1 += w1 & 1; 
        w1 >>= 1; 
    } 

    while (w2) { 
        count2 += w2 & 1; 
        w2 >>= 1; 
    } 
    return count1<count2; 
} 

void* keyboard_thread(void *threadid) 
{
	while(true){
	 	while(keyboard_interrupt);

	 	_byte b=0;
	 	read(0, &b, 1);
	 	c=b;
	 	keyboard_interrupt=true;		
	}
	return NULL;

}