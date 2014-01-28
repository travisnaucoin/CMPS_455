// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

// Begin code changes by Travis and Xiyue Xiang

extern int selectArgs;
int NumShout;

//----------------------------------------------------------------------
// CheckType
// 	Check the type of the input string
//  TYPE = 0-zero; 1-pos int; 2-neg int; 3-pos dec; 4-neg dec; 5-character

//----------------------------------------------------------------------

int CheckType (char * str) {
	int i;
	int TYPE; // 0-zero; 1-pos int; 2-neg int; 3-pos dec; 4-neg dec; 5-character
	char * i_str = new char;
	int flag = 0; // if flag == 1, the input is a character string.
				// otherwise, it is a decimal number
	int num_pt = 0;
	int j = 0;
		
	i = atoi (str); // extract number only
	sprintf(i_str,"%d",i); // load into str type for cmp
	if (strcmp(str,i_str) == 0){
		if ((*i_str) == '-'){
			TYPE = 2;
		}
		else if ((*str) == '0') {
			TYPE = 0;
		}
		else {
			TYPE = 1;
		}
	}
	else {
		// scan each of the str			
		// if the str has only number with only a signle point, it is a decimal otherwise it is a character		
		while (str[j] != '\0') {
			int a = str[j];
			// Use ASCII code to determine if the current digit is a number
			if ((a >= 48) && (a <= 57) == true) {
				flag = flag;
			} 
			// check the number and the position of the decimal points
			else if (a == 46){
				if (j == 0) {
					flag = 1;
				} else {
					num_pt++;
					if (num_pt > 1) {
						flag = 1;
					}
				}
			}
			// determine if "-" or "+" appears in the middle of the string
			// instead of beginning
			else if ((a == 43 || a == 45) == true) {
				if (j != 0) {
					flag = 1;
				}
			}
			else {
				flag = 1;
			}
			j++;
		}
		if (flag == 1) {
			TYPE = 5;
		} else {
			// determine the sign of the decimal number
			if (str[0] == '-') {
				TYPE = 4;
			} else {
				TYPE = 3;
			}
		}
	}
	delete i_str;
	return TYPE;
}

//----------------------------------------------------------------------
// CheckInputOnly
//		Print out the input type; 	
//----------------------------------------------------------------------
void CheckInputOnly(int i) {
	char * str = new char;
	printf ("Enter something: ");
	scanf("%s",str);
	int TYPE = CheckType(str);
	switch (TYPE) {
		case 0: printf ("Input type: Integer 0 \n"); // 0 is unsigned.
		break;
		case 1:	printf ("Input type: Positive Integer \n"); // upper bound of integer = 2^31-1
		break;
		case 2: printf ("Input type: Negative Integer \n");	// lower bound of integer = -2^31
		break;
		case 3:	printf ("Input type: Pos Decimal \n");
		break;
		case 4:	printf ("Input type: Neg Decimal \n");
		break;
		case 5: printf ("Input type: Character \n");
		break;		
		default: printf ("Invalid input type! \n");
	}
	delete str;
}

// End code changes by Travis Aucoin and Xiyue Xiang

// Begin code changes by Bradley Milliman
//----------------------------------------------------------------------
// NumShouter
// 	Prompt number of shouter
// 	The input string is checked to obtain an positive integer.
//----------------------------------------------------------------------
int NumShouter (){
	char * T = new char;
	printf("Enter number of shouter: ");
	scanf("%s",T);
	while (CheckType(T) != 1){
		printf("Ërror: Please check the input type! \n");
		printf("Please reenter the number of shouter: ");
		scanf("%s",T);
	}
	return atoi(T);
	delete T;
}

//----------------------------------------------------------------------
// Occurrence
// 	Prompt how many time each thread shouts.
// 	The input string is checked to obtain an positive integer.
//----------------------------------------------------------------------
int Occurrence (){
	char * S = new char;
	printf("Shouts how many times: ");
	scanf("%s",S);
	while (CheckType(S) != 1){
		printf("Ërror: Please check the input type! \n");
		printf("Please reenter how many times: ");
		scanf("%s",S);
	}
	return atoi(S);
	delete S;
}

//----------------------------------------------------------------------
// Shout
// 		A shouter shouts msg NumShout times. Each time, the msg is choosen
//		randomly from the msg pool.
//		After each shout, a shouter yields random cycles, i.e. 2-5 cycles.
//----------------------------------------------------------------------
void Shout(int which){
	char msg[5][64];
	strcpy(msg[0],"Pattern0");
	strcpy(msg[1],"Pattern1");
	strcpy(msg[2],"Pattern2");
	strcpy(msg[3],"Pattern3");
	strcpy(msg[4],"Pattern4");	
	int j; // msg index
	int stallCycle; // how many cycles each thread yields.
	
	for (int i=0; i<NumShout; i++) {
		
		j = Random() % 5;
		printf("Thread %i shouts for %i times: %s \n",which,i+1,msg[j]);

		// stall random cycles before continuing		
		stallCycle = Random() % 3 + 2; // randomly choose between 2 and 5. 
		while (stallCycle != 0) {
			currentThread->Yield();
			stallCycle--;
		}
			
	}
} 
// End code changes by Bradley Milliman

// Begin code changes by Marcus Amos
//----------------------------------------------------------------------
// ThreadTest
//		Select task and create threads
//		Fork functions to threads
//
//		task is selected based on global variable selectArgs.
//		which is defined in system.cc
//----------------------------------------------------------------------
void
ThreadTest()
{
    DEBUG('t', "Entering ThreadTest");
	
	// decide which task to run
	if (selectArgs == 0){
		// no operation
		printf("Opps! \n");		
	} else if (selectArgs == 1){
		// check input type
		Thread *t = new Thread("forked thread");	
		t->Fork(CheckInputOnly, 1);			
	} else if (selectArgs == 2){
		// executing shouting
		int T = NumShouter();
		NumShout = Occurrence();
		for (int i=0; i<T; i++) {
			Thread *t = new Thread("forked thread");
			t->Fork(Shout, i);
		}
	}	
			
	currentThread -> Finish(); 
}
// End code changes by Marcus Amos

