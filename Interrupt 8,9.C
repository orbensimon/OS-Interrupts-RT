#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <time.h>
#include <string.h>

void interrupt (*interr9save) (void);
volatile int passTime=0; //time to press password
volatile int totalTime=0; //total time of the program
volatile int slowTime=18; //delay time in interrupts unit
volatile char chBuffer; //read char
int passFlag=0;
int passLen=0;
char* globPass; //password
int passValidation=0;
int passEnterd=0; //to exit after password validation
int slowECounter=1; //number of slowE iteration
int prevTime=0; //last slowE time iteration 
unsigned int flag; //to check if the buffer contain char
int firstSlowEIteration=0; //to initial interrupts
//pointer to save original interrupts 
long int int9save;
long int int8save;

void interrupt myInt8 (void)
{
	asm{
	PUSH AX
	MOV AL,20h
	OUT 20h,AL
	POP AX
	}
	totalTime++; // slowE iteration time
	slowTime--;
	asm{
		STI
	}
	while(slowTime)printf(".");
	passTime--;
	
}

void interrupt myInt9(void)
{
	asm{
		PUSHF
		CALL DWORD PTR interr9save
		PUSH AX
		MOV AH,1
		INT 16
		PUSHF
		POP AX
		AND AX,64h
		MOV flag,AX
		POP AX
	}

	if(flag==0)
	{
		_asm{
			MOV AH,0
			INT 16h
			MOV chBuffer,AL
			
		}
		if(chBuffer==globPass[passValidation])
		{
			passValidation++;
			printf("%c ",chBuffer);
			if(passValidation==passLen)
				passFlag=1;
		}
		else
			passValidation=0;
			
	}
}

void slowD(char passwd[], int size){
	//strcpy(globPass,passwd);
	int i;
	printf("in slowD\n");
	for(i=0;i<size;i++)
			globPass[i]=passwd[i];
	if(passValidation==size)
	{
		_asm{
			PUSH AX
			CLI
			MOV AX,0
			MOV ES,AX
			MOV AX,WORD PTR int9save
			MOV ES:[4*9],AX
			MOV AX,WORD PTR int9save+2
			MOV ES:[4*9 +2],AX
		
			MOV AX,WORD PTR int8save
			MOV ES:[4*8],AX
			MOV AX,WORD PTR int8save+2
			MOV ES:[4*8 +2],AX
			STI
			POP AX
		}
	}

}

int slowE(char pass[],int size, int time)
{
	int i;
	if(passEnterd==1)
		exit(0);
	if(firstSlowEIteration==0)
	{
		interr9save = getvect(9);
	// SAVE INT9, INT8
	_asm{
		MOV AX,0
		MOV ES,AX
		MOV AX,ES:[4*9]
		MOV WORD PTR int9save,AX
		MOV AX,ES:[4*9 +2]
		MOV WORD PTR int9save+2,AX

		MOV AX,ES:[4*8]
		MOV WORD PTR int8save,AX
		MOV AX,ES:[4*8 +2]
		MOV WORD PTR int8save+2,AX
	}	
			//setvect(9,myInt9)
			// setvect(8,myInt8)
		_asm{
		PUSH AX
		MOV AX,0
		MOV ES,AX
		CLI
		MOV WORD PTR ES:[4*8],OFFSET myInt8
		MOV WORD PTR ES:[4*8 +2],SEG myInt8
		
		MOV WORD PTR ES:[4*9],OFFSET myInt9
		MOV WORD PTR ES:[4*9 +2],SEG myInt9
		
		STI
		POP AX
		}
		
		firstSlowEIteration=1;
		globPass=(char*)malloc(size*sizeof(char)+1);
		//srtcpy(globPass,pass);
		
		for(i=0;i<size;i++)
			globPass[i]=pass[i];
		passLen=size;
	}
	slowECounter == 16 ? 16 : slowECounter*2;
	passTime=time*18;
	if(slowECounter==16)
		slowTime=288;
	else
		slowTime*=2;
	
	if(passTime>0)
	{
		if(passFlag==1)
		{
			prevTime=(totalTime/18)-(prevTime/18);
			printf("\nPassword enterd!\ntime elapsed since last function = %d\n",prevTime);
			passValidation=0;
			slowD(pass,size);
			passEnterd=1;
			return slowECounter == 16 ? 16 : slowECounter;
		}
		
	}
	prevTime=(totalTime/18)-(prevTime/18);
	printf("\nTime out\ntime elapsed since last function = %d\n",prevTime);
	return slowECounter == 16 ? 16 : slowECounter;
}


	
int main()
{
	
	unsigned long int i, j;
	int counter = 0 ; 
	time_t t1,t2;
	
	i = j = 0;
	printf("in main\n");
	while (1)
	{
		printf("in while loop\n");
		t1= time(NULL);
		if (counter++ == 100)
			slowD("aaaabb" , 6);
		printf("speed : 1 / %d \n", slowE("aaaabb" , 6 ,  5));
		j=0;
		while(j < 10)
		{
			i++;
			if ((i % 10000000) == 0)
			j++;
		}// while
		    t2= time(NULL);
			printf("\nTotal  Iteration  Time -  = %d secondsn\n", (int)(t2-t1));
	}
	
}
