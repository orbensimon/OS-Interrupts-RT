// sou3.c
#include <dos.h>
#include <stdio.h>

#define N 200

volatile int startCount;
volatile char key;

void interrupt (*int9save) (void);
void interrupt (*int8save) (void);

void Sound(int); // set channel 2
void ChangeFreq(int); // set counter of channel 0
void TurnSpeakersOn(void);
void TurnSpeakersOff(void);

int keyPressed=2; /* keyPressed=0 -> same key enterd
				     keyPressed=1 -> first key enterd waiting for second same key
					 keyPressed=2 -> waiting for first key*/
int timeArr[N]; // time between to identical preses array
int numOfKeys; // keys press counter
int index; // timeArr index
int key_rls=1;
int t;
unsigned char keyboardLayout[128] =
{
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
	'9', '0', '-', '=', '\b',	/* Backspace */
	'\t',			/* Tab */
	'q', 'w', 'e', 'r',	/* 19 */
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
	0,			/* 29   - Control */
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
	'\'', '`',   0,		/* Left shift */
	'\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
	'm', ',', '.', '/',   0,				/* Right shift */
	'*',
	0,	/* Alt */
	' ',	/* Space bar */
	0,	/* Caps lock */
	0,	/* 59 - F1 key ... > */
	0,   0,   0,   0,   0,   0,   0,   0,
	0,	/* < ... F10 */
	0,	/* 69 - Num lock*/
	0,	/* Scroll Lock */
	0,	/* Home key */
	0,	/* Up Arrow */
	0,	/* Page Up */
	'-',
	0,	/* Left Arrow */
	0,
	0,	/* Right Arrow */
	'+',
	0,	/* 79 - End key*/
	0,	/* Down Arrow */
	0,	/* Page Down */
	0,	/* Insert Key */
	0,	/* Delete Key */
	0,   0,   0,
	0,	/* F11 Key */
	0,	/* F12 Key */
	0,	/* All other keys are undefined */
};

void interrupt myISR9(void)
{
	char c;
	unsigned int scanCode = 0;
	
	int9save();			/* call original ISR9 */

	   asm{
		   IN 	AL, 60h
		MOV BYTE PTR scanCode, AL
		AND AL, 80h
        MOV BYTE PTR t, AL
        AND AL,AL
		JNZ  KEY_RELEASE
	   }
	   

	if (key_rls == 1) 
	{
		numOfKeys++;
		c = keyboardLayout[scanCode];
		if(keyPressed==2)
		{
			key=c;
			keyPressed=1;
			printf("(%c,",key);
		}
		else if(c==key)
		{
			keyPressed=0;
			printf("%c) ",key);
		}

		key_rls=0;
	}
	KEY_RELEASE:
	 if(t!=0){
            key_rls=1;
        }
		asm{ //aknowlge the system we handle and finsh handle the keybord input
			IN	AL, 61h
			OR   AL, 80h
			OUT  61h, AL
			AND  AL, 7Fh
			OUT  61h, AL
			MOV   AL, 20h
			OUT   20h, AL
		}
        
        int9save();
	
}
void interrupt newint8(void)
{
	asm{
		
		PUSHF
		CALL DWORD PTR int8save
	}
	
	if(keyPressed==1)
		startCount++;
	if(keyPressed==0)
	{
		timeArr[index]=startCount;
		index++;
		startCount=0;
		keyPressed=2;
		Sound(timeArr[index-1]);
	}

} 



	

    

       int main( void )
        {
		   int i,j,a,sum=0;	
		   ChangeFreq(700);
		   Sound(1);
		   int8save = getvect(8);
		   setvect(8,newint8);
		   int9save = getvect(9);
		   setvect(9,myISR9);

		   while(numOfKeys<20);
		   
		   printf("\ntimed:\n");
		   
		   for(i=0;i<index;i++)
			   printf("%d ",timeArr[i]);
		   
		   for (i = 0; i < index; ++i) 
		   {
 
            for (j = i + 1; j < index; ++j)
            {
 
                if (timeArr[i] > timeArr[j]) 
                {
 
                    a =  timeArr[i];
                    timeArr[i] = timeArr[j];
                    timeArr[j] = a;
 
                }
 
            }
 
           }
		   printf("\nsorted:\n");
		   
		   for(i=0;i<index;i++)
			   printf("%d ",timeArr[i]);
		   printf("\nmax time:\n%d/1069",timeArr[i-1]);
		   printf("\nmin time:\n%d/1069",timeArr[0]);
		   printf("\nmed time:\n%d/1069",timeArr[index/2]);
		   printf("\ntotal time:\n");
		   for(i=0;i<index;i++)
			   sum+=timeArr[i];
		   printf("%d/1069",sum);
		
		   TurnSpeakersOff( );
		   setvect(9,int9save);
		   setvect(8,int8save);
		   ChangeFreq(65535);
           return(0);
        } /*--main( )-------*/
		
void TurnSpeakersOn()
{
	asm{
		IN AL,61h					
		OR AL,00000011b				
		OUT 61h,AL
	}
}

void TurnSpeakersOff()
{
	asm{
		IN      AL,61h          
		AND     AL,11111100B 
		OUT		61h,AL			
	}
}
		
    void Sound( int hertz )
	{
	
	 
	 unsigned divisor = 1193180L / hertz;

	  TurnSpeakersOff();
	  asm{
		  CLI
		  MOV AL,0B6h
		  OUT 43h,AL
		  MOV BX,WORD PTR divisor
		  MOV AL,BL
		  OUT 42h,AL
		  MOV AL,BH
		  OUT 42h,AL
		  STI
	  }

		TurnSpeakersOn();

	} /*--Sound( )-----*/
	
	void ChangeFreq(int freq)
	{
		asm{
			CLI
			MOV AL,36H
			OUT 43H,AL
			MOV BX,WORD PTR freq
			MOV AL,BL
			OUT 40h,AL
			MOV AL,BH
			OUT 40h,AL
			STI
		}
		
		
		
	}
