/* readyear.c */

#include <stdio.h>
#include<math.h>
#include<dos.h>


typedef struct Reminder {
	int ReminderHour;
	int ReminderMin;
	int ReminderSec;
}Reminder ;

Reminder ReminderArray[30];

volatile char key=' ';
char str[16];
char str2[16];
int specialFeature=0;
int index=0;

void interrupt (*int0x70save) (void);

void TerrminateProc(unsigned int, unsigned int, unsigned int, unsigned int);
void interrupt NewTimer(void);
int convert_to_binary(int x);
void readclk(char str[]);


void main()
{
	
  int zero_flag;
  unsigned int masterOldMask;
  unsigned int slaveOldMask;
  unsigned int rtcOldAMask;
  unsigned int rtcOldBMask;
  int0x70save= getvect(0x70);
  asm{
	  // PROGRAMING MASTER PIC TO SET INT 8,9 AND SLAVE PIC
	  CLI
	  IN AL,21h
	  MOV BYTE PTR masterOldMask,AL
	  MOV AL,11111000b
	  OUT 21h,AL
	  // PROGRAMING SLAVE PIC TO SET INT 70h
	  IN AL,0A1h
	  MOV BYTE PTR slaveOldMask,AL
	  MOV AL,11111110b
	  OUT 0A1h,AL
	  // PROGRAMING A REGISTER TO WORK WITH THE STANDART RATE
	  IN AL,70h
	  MOV AL,0Ah
	  OUT 70h,AL
	  MOV AL,8Ah
	  OUT 70h,AL
	  IN AL,71h
	  MOV BYTE PTR rtcOldAMask,AL
	  XOR AX,AX
	  OR AL,0110b
	  OR AL,00100000b
	  OUT 71h,AL
	  IN AL,71h
	  // PROGRAMING B REGISTER TO WORK IN PERIODIC MODE AND ALARM MODE
	  MOV AL,0Bh
	  OUT 70h,AL
	  MOV AL,8Bh
	  OUT 70h,AL
	  IN AL,71h
	  MOV BYTE PTR rtcOldBMask,AL
	  XOR AX,AX
	  OR AL,01100000b
	  OUT 71h,AL
	  IN AL,71h
	  // READING C REGISTER VALUES
	  MOV AL,0Ch
	  OUT 70h,AL
	  IN AL,71h
	  // READING D REGISTER VALUES
	  MOV AL,0Dh
	  OUT 70h,AL
	  IN AL,71h
	  
	  STI
  }
  setvect(0x70,NewTimer);
  
 
  while(key!='q')
  {
    putchar(13);
    readclk(str);
    printf(str);
   asm{
		   PUSH AX
	       MOV AH, 1     /* BIOS read char from buffer option */
	       INT 16h       /* BIOS read char from buffer        */
	       MOV key, AL     /* Transfer char to program          */
		   PUSHF
		   POP AX
		   AND AX,64  	 /* zero flag */
		   MOV zero_flag,AX
		   POP AX
	   }

	if (zero_flag == 0) 
	{
		
		if(key=='D' || key=='d' || key=='n' || key=='A')
			specialFeature=1;
		else if(key=='q')
			TerrminateProc(masterOldMask,slaveOldMask,rtcOldAMask,rtcOldBMask);
		asm{
			PUSH AX
			MOV AH,0
			INT 16h
			POP AX
		}
	}
	
	

  } // while



}  // main


void TerrminateProc(unsigned int master,unsigned int slave,unsigned int rtca,unsigned int rtcb)
{
	// ORIGINAL RTC VALUES
	asm{
		CLI
		// ORIGINAL A REGISTER VALUE 
		MOV AL,0Ah
		OUT 70h,AL
		
		MOV AL,8Ah
		OUT 70h,AL
		
		MOV AL,BYTE PTR rtca
		OUT 71h,AL
		
		IN AL,71h
		// ORIGINAL B REGISTER VALUES
		MOV AL,0Bh
		OUT 70h,AL
		
		MOV AL,8Bh
		OUT 70h,AL
		
		MOV AL,BYTE PTR rtcb
		OUT 71h,AL
		
		IN AL,71h
		// READING C REGISTER
		MOV AL,0Ch
		OUT 70h,AL
		IN AL,71h
		// READING D REGISTER
		MOV AL,0Dh
		OUT 70h,AL
		IN AL,71h
		
		// ORIGINAL SLAVE PIC VALUES
		MOV AL,BYTE PTR slave
		OUT 0A1h,AL
		
		//ORIGINAL MASTER PIC VALUES
		MOV AL,BYTE PTR master
		OUT 21h,AL
		
		STI
	}
	// SET 70h INTERRUPT TO THE ORIGINAL ISR
	setvect(0x70,int0x70save);
	
}

void interrupt NewTimer(void)
{
	
	int dayNum;
	int monthNum;
	int yearNum;
	int hourNum;
	int minutesNum;
	int secNum;
	if(key=='d')
	{
		asm{
			MOV AL,6
			OUT 70h,AL
			IN AL,71h
			MOV BYTE PTR dayNum,AL
		}
		dayNum=abs(convert_to_binary(dayNum));
		sprintf(str2," d=%d    ",dayNum);
	}
	else if(key=='D')
	{
		asm{
			// DAY NUMBER
			MOV AL,7
			OUT 70h,AL
			IN AL,71h
			MOV BYTE PTR dayNum,AL
			// MONTH NUMBER
			MOV AL,8
			OUT 70h,AL
			IN AL,71h
			MOV BYTE PTR monthNum,AL
			// YEAR NUMBER
			MOV AL,9
			OUT 70h,AL
			IN AL,71h
			MOV BYTE PTR yearNum,AL
		}
		dayNum=abs(convert_to_binary(dayNum));
		monthNum=abs(convert_to_binary(monthNum));
		yearNum=abs(convert_to_binary(yearNum));
		sprintf(str2," %d.%d.%d",dayNum,monthNum,yearNum);
	}
	else if(key=='n')
		sprintf(str2,"         ");
		
	else if(key=='A')
	{
		key='n';
		if(index == 30)
			printf("Full!!");
		else
		{
			printf("\nPlz enter time for alarm (FORMAT: hour:min:sec)\n");
			scanf("%d%*c%d%*c%d",&hourNum,&minutesNum,&secNum);// the %* expresion skips the ':' in the input
			ReminderArray[index].ReminderHour=hourNum;
			ReminderArray[index].ReminderMin=minutesNum;
			ReminderArray[index].ReminderSec=secNum;
			index++;
			printf("\nAlarm recived!\n");
		}
		
		
		
	}
		
	// END OF INTERRUPT ROUTINE PROTOCOL
	asm{
		IN AL,70h
		MOV BX,AX
		MOV AL,0Ch
		OUT 70h,AL
		MOV AL,8Ch
		OUT 70h,AL
		IN AL,71h
		MOV AX,BX
		OUT 70h,AL
		MOV AL,20h
		OUT 0A0h,AL
		OUT 020h,AL
	}
	
	
}

int  convert_to_binary(int x)
{
 int i;
 int temp, scale, result;
 

  temp =0;
  scale = 1;
  for(i=0; i < 4; i++)
   {
     temp = temp + (x % 2)*scale;
     scale *= 2;
     x = x >> 1;
   } // for

  result = temp;
  temp = 0;

  scale = 1;
  for(i=0; i < 4; i++)
   {
     temp = temp + (x % 2)*scale;
     scale *= 2;
     x = x >> 1;
   } // for

  temp *= 10;
  result = temp + result;
  return result;

} // convert_to_binary

void readclk(char str[])
{
  int i,j;
  int hour, min, sec;


  hour = min = sec = 0;

  asm {
   PUSH AX
   MOV AL,0
   OUT 70h,AL
   IN AL,71h
   MOV BYTE PTR sec,AL
;
   MOV AL,2
   OUT 70h,AL
   IN AL,71h
   MOV BYTE PTR min,AL
;
   MOV AL,4
   OUT 70h,AL
   IN AL,71h
   MOV BYTE PTR hour,AL
;
   POP AX
  } // asm

  sec = convert_to_binary(sec);
  min = convert_to_binary(min);
  hour = convert_to_binary(hour);
  for(j=0;j<index;j++)
  {
	  if(hour==ReminderArray[j].ReminderHour&&min==ReminderArray[j].ReminderMin&&sec==ReminderArray[j].ReminderSec)
	  {
		  printf("\nALARM ALARM ALARM!!\n");
		  sleep(2);
	  }
  }
  sprintf(str,"%2d:%2d:%2d", hour, min, sec);
  if(specialFeature==1)
	  sprintf(str,"%2d:%2d:%2d %s", hour, min, sec, str2);
  
} // readclk

