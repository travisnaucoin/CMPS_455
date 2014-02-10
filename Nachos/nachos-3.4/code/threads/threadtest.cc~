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
#include "synch.h"

// Begin code changes by Travis and Xiyue Xiang

//-------------------TASK 2 GLOBALS------------------------
//---------------------------------------------------------
void EnterRoom (int);
void LeaveRoom (int);
void Eat (int);
void Thinking (int);
void GetSticks(int);
void PutSitcks (int);
void Philosopher (int);
int NUMPHIL = 0; //number of philosophers
int PINROOM = 0; //number of philosophers in room 
int NUMMEAL  = 0; //number of meals
typedef enum {THINKING,ISEATING,ISHUNGRY,STOODUP} pState; //state of philosophers
pState *philState; //array holding all philosophers states
Semaphore *stand = new Semaphore("stand",1);
Semaphore *mutex = new Semaphore("mutex",1); //mutual exclusion
Semaphore **phil;//actual philosopher
#define RIGHT(I) (I+NUMPHIL-1)%NUMPHIL
#define LEFT(I)  ((I)+1) % NUMPHIL //left right check of philosophers
bool sitting = false;
//-----------------------------------------------------------
//-----------------------------------------------------------

// Begin code changes by Bradley Milliman
// global variables to solve Dining Philosopher's problem
//int people;
int MealLeft;			
int PhilosopherSeated;
int PhilosophersReady;
int mealsEaten;
int *chopstick; 
int allPhilosopherSeated;
bool seatGrant;
bool leaveGrant;
// end code changes by Bradley Milliman

extern char * selectArgs;
int NumShout;
int P; //number of people
int S; //number of messages a person's mailbox can hold;
int M; //total number of messages
char * MsgList [] = {"Pattern0","Pattern1","Pattern2","Pattern3","Pattern4","Pattern5"};

int CheckType (char *);
void CheckInputOnly(int); 
int promptInput (int);
void Shout(int);
void PostOfficeLoop(int);
int NumMsgSent;  // Need to be protected.
bool * done; // Leaving signal for person
int * NumMsg; // Num of messages in each mailbox ---- Need to be protected.
Semaphore ** mailboxSemaphore;
Semaphore *MsgCntSemaphore = new Semaphore ("MsgCntSemaphore",1);

struct MailSlot
{
	int Who;
	char * Msg; 
};
MailSlot MailBox [128][64];

// code Changes made by Bradley Milliman begin
//----------------------------------------------------------------------
// Philosophers waiting loop for 
// 	Task 1: Dinning Philosophers Problem using busy waiting loop
//----------------------------------------------------------------------
int PhilosophersWait(int num, int CheckCnt)
 {
	if (chopstick[num] == 1)
	{
		chopstick[num] = 0;
		CheckCnt = 0;
	}
	else
	{ 
		CheckCnt ++;
		int randomnum = 2+ Random() % 4;
		for (int i=0;i<randomnum;i++)
		{	
			currentThread->Yield();
		}	
	}
	return CheckCnt; 	
 }

//----------------------------------------------------------------------
//  Philosophers waiting loop for 
// 	Task 1: Dinning Philosophers Problem using busy waiting loop
//----------------------------------------------------------------------
void DinningPhilosophers(int info)
 {
 	int ThreadNum = info;
	int RandomNum;
	int CheckCntRight = 0;
	int CheckCntLeft = 0;
	bool AbortPickup = false;

    printf("Philosopher %d has joined the room \n", ThreadNum);
 	allPhilosopherSeated--;
	
	printf("Philosopher %d is waiting to sit\n", ThreadNum);
 	while (allPhilosopherSeated > 0)
 	{	
 	    currentThread->Yield();
 	}
	
	if (allPhilosopherSeated==0 && seatGrant == false)
	{		
		printf("All philosiphers sit down at table\n"); 
		seatGrant = true;
	}
	// now all philosophers are seated
 
    while (MealLeft > 0)
    {	
 		// reaching for leftChopstick
		while (CheckCntLeft != 0)
		{
			CheckCntLeft = PhilosophersWait((ThreadNum-1) % P , CheckCntLeft);
			printf("Philosopher %d has picked up his left chopstick \n",ThreadNum);
		}
		
 		// reaching for rightChopstick	
		while (CheckCntRight != 0 && CheckCntRight < 5)
		{
			CheckCntRight = PhilosophersWait((ThreadNum % P), CheckCntRight);
			printf("Philosopher %d has picked up his right chopstick \n",ThreadNum);
		}
		if (CheckCntRight == 5) AbortPickup = true;
		
		if (AbortPickup != true) 
		{
			printf("Philosopher %d has picked up his right chopstick \n",ThreadNum);
			// begin to eat

			MealLeft--;
			printf("Philosopher %d has begun to eat (%d Philosophers ate so far) \n",ThreadNum, M-MealLeft);
			
			
			RandomNum = 2 + Random()%5;
			for (int i=1;i<=RandomNum;i++)
			{
				currentThread->Yield();
			}
			printf("Philosopher %d has finished eating  \n",ThreadNum);

			chopstick[(ThreadNum-1)%P] = 1;
			printf("Philosopher %d has dropped his left chopstick \n",ThreadNum);
			// dropping right chopstick
			chopstick[ThreadNum%P] = 1;
			printf("Philosopher %d has dropped his right chopstick \n",ThreadNum);
		}
		else {
			chopstick[(ThreadNum-1)%P] = 1;
			printf("Philosopher %d has dropped his left chopstick \n",ThreadNum);
		}
		
        // start thinking
		printf("Philosopher %d Thinking \n",ThreadNum);
		RandomNum = 2 + Random()%5;
		for (int i = 1; i<=RandomNum;i++)
		{
			currentThread->Yield();
		}
    }
	
    PhilosophersReady++;
    printf("Philosopher %d is waiting to leave\n", ThreadNum);
	
    while (PhilosophersReady < P) // waiting for all philosophers to be ready
    {
		currentThread->Yield();
    }
	
	if (PhilosophersReady == P && leaveGrant == false)
	{
	    printf(" All Philosophers have left the table \n");
		leaveGrant = true;
	}
 }

// code Changes made by Bradley Milliman end
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
	int Task = 0;
    DEBUG('t', "Entering ThreadTest");
	
	// decide which task to run
	if (CheckType(selectArgs)==1) 
	{
		Task = atoi(selectArgs);
				
		if (Task == 1){
			// check input type
			Thread *t = new Thread("forked thread");	
			t->Fork(CheckInputOnly, 1);			
		} else if (Task == 2){
			// executing shouting
			printf("Enter number of shouter: ");
			int T = promptInput(1); // integer type is desired
			printf("Shouts how many times: ");
			NumShout = promptInput(1); // integer type is desired
			for (int i=0; i<T; i++) {
				Thread *t = new Thread("forked thread");
				t->Fork(Shout, i);
			}
		// code Changes made by Bradley Milliman begin
		} else if (Task == 3){
			// executing Post Office with waiting loop
			printf("Enter number of philosophers: ");
			P = promptInput(1);
			while (P <= 1)
			{
				printf("Not enough people to begin simulation. \n");
				printf("Please reenter the number of philosophers:");
				P = promptInput(1);
			}
			printf("Enter the total number of meals to be eaten: ");
			M = promptInput(1);	
			PhilosopherSeated = P; // initializing global variable
 			// initializing the array of chopstick flags
			chopstick = new int[P+1];
			allPhilosopherSeated = P;
			seatGrant = false;
			MealLeft = M;
			leaveGrant = false;
			PhilosophersReady = 0;
			
			for (int i = 1; i<=P;i++)
			{
				chopstick[i] = 1;
			}
			for (int i = 1; i<=P; i++)
			{		
			  Thread *t = new Thread("forked thread");
			  t->Fork(DinningPhilosophers,i);
			}
		}	
// code Changes made by Bradley Milliman end
		} else if (Task == 4){
			//executing Dining Philosophers Semaphores
			printf("Enter number of philosophers: ");
			NUMPHIL = promptInput(1);
			PINROOM = NUMPHIL;
			phil = new Semaphore*[NUMPHIL];
			philState = new pState[NUMPHIL];
			printf("Enter number of meals: ");
			NUMMEAL = promptInput(1);
			//INIT
			for(int n = 0; n < NUMPHIL; n++)
			{
				phil[n] = new Semaphore("Phil", 0);
			}
			for (int p = 0; p < NUMPHIL; p++)
			{
				Thread *t = new Thread("forked thread");
				t->Fork(EnterRoom,p);
			}
		
		} else if (Task == 5){
			// executing Post Office with waiting loop
			printf("Enter number of people: ");
			P = promptInput(1);
			printf("Enter the capacity of a mailbox: ");
			S = promptInput(1); 
			printf("Enter total number of messages: ");
			M = promptInput(1);	
			NumMsgSent = 0;
			done = new bool[P];		
			mailboxSemaphore = new Semaphore*[P];
			NumMsg =  new int [P];
			
			// Initialization
			for (int j=0; j<P; j++) {
				done [j] = false;
				NumMsg[j] = 0;
				mailboxSemaphore[j] = new Semaphore ("mailboxSemaphore",1);
			}

			for (int i=0; i<P; i++) {	
				Thread *t = new Thread("forked thread");
				t->Fork(PostOfficeLoop, i);
			}		
		}	
	 else {
		printf("Error: -A is not an appropriate mode. \n");
	}	
	currentThread -> Finish(); 
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Project 2, Task 2 Dining Philosophers Begin.                       +
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------
//PutSticks
//  Make philosopher think and check adjacent ones to see if they are 
//  hungry, put down sticks
//---------------------------------------------------------------------
void
PutSticks(int i)
{
	
	mutex->P();
	philState[i] = THINKING;
	if(philState[LEFT(i)] == ISHUNGRY) phil[LEFT(i)]->V();
	printf("Philosopher %i, puts down left stick\n", i);
	if(philState[RIGHT(i)] == ISHUNGRY) phil[RIGHT(i)]->V();
	printf("Philosopher %i, puts down right stick\n", i);
	mutex->V();
}


//----------------------------------------------------------------------
//GetSticks
//  Make philosopher hungry and check adjacent sticks to see if they can 
//  eat
//---------------------------------------------------------------------
void
GetSticks(int i)
{
	philState[i] = ISHUNGRY;
	bool left = false;
	bool right = false;
	while (philState[i] == ISHUNGRY)
	{
		mutex->P();
		if(philState[i] == ISHUNGRY && philState[LEFT(i)] != ISEATING && NUMMEAL > 0) left = true;
		
		if(philState[i] == ISHUNGRY && philState[RIGHT(i)] != ISEATING && NUMMEAL > 0) right = true;
	
		if (left && !right){
			printf("Philosopher %i, picks up left stick \n",i);
			printf("Philosopher %i, puts down left stick \n",i);
			
		}
		if (!left && right){
			printf("Philosopher %i, picks up right stick \n",i);
			printf("Philosopher %i, puts down right stick \n",i);
			
		}
		if (left && right){
			printf("Philosopher %i, picks up right stick \n",i);
			printf("Philosopher %i, picks left stick \n",i);
			
		}
		
		if (philState[i] == ISHUNGRY &&
		philState[LEFT(i)] != ISEATING &&
		philState[RIGHT(i)] != ISEATING && NUMMEAL > 0)
		{ 
			philState[i] = ISEATING;
			phil[i]->V();
		}
		
		mutex->V();
		phil[i]->P();
	}
}

//----------------------------------------------------------------------
//Thinking
//  Make a philosopher think and wait for a few cycles 
//  
//---------------------------------------------------------------------
void
Thinking(int i)
{
	printf("Philosopher %i is thinking\n",i);
	int stallCycle = Random() % 3 + 2; // randomly choose between 2 and 5. 
	while (stallCycle != 0) {
		philState[i] = THINKING;
		currentThread->Yield();
		stallCycle--;
	}
}

//----------------------------------------------------------------------
//Eating
//  Make philosopher wait and eat for a few cycles
//  
//---------------------------------------------------------------------
void
Eat(int i)
{
	printf("Philosopher %i is eating\n",i);
	NUMMEAL-=1;
	printf("meals left: %i \n",NUMMEAL); //DEBUG PURPOSES
	int stallCycle = Random() % 3 + 2; // randomly choose between 2 and 5. 
	while (stallCycle != 0) {
		philState[i] = ISEATING;
		currentThread->Yield();
		stallCycle--;
	}
}

//----------------------------------------------------------------------
//Philosopher
//  Controls each philosopher and allows them to eat and think. 
//
//---------------------------------------------------------------------
void
Philosopher(int i)
{
	//bool go = true;
	while(1){
	Thinking(i);	
	GetSticks(i);
	Eat(i);
	PutSticks(i);
	}
	
	
}

//----------------------------------------------------------------------
//EnterRoom
//  Make philosopher hungry and check adjacent ones to see if they can 
//  eat
//---------------------------------------------------------------------
void
EnterRoom(int i)
{
	//
	//This is what im having trouble with, any help would be great!
	//
	
	printf("Philosopher %i, has entered\n",i);
	PINROOM--;
	while(PINROOM >0){
	mutex->P();
	if(PINROOM > 0){
		//printf("%i waits\n",i);
		
		phil[i]->V();
		
	}
	mutex->V();
	
	}
	if(PINROOM==0 && !sitting){
	printf("They all sit at the table.\n");
	sitting = true;
	}
	if(sitting){
	Philosopher(i);
	phil[i]->P();
	}
		
}

//----------------------------------------------------------------------
//EnterRoom
//  Make philosopher hungry and check adjacent ones to see if they can 
//  eat
//---------------------------------------------------------------------
void
LeaveRoom(int i)
{
	//
	//This is what im having trouble with, any help would be great!
	//
	printf("Philosopher %i, stood up\n",i);
	PINROOM++;
	while (PINROOM < NUMPHIL){
		stand->V();
		phil[i]->V();
		stand->P();
		
	}
	
	if (PINROOM == NUMPHIL){
		printf("They all left the room.\n");
		
	}
	phil[i]->P();
		
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Project 2 Task 2, Dining Philosophers End			       +
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------
// PostOfficeLoop
// 	Task 3: Post Office Problem using busy waiting loop
//----------------------------------------------------------------------

void PostOfficeLoop (int i)
{
	int iSendTo = i; // index of recipient
	char * msgSent = new char;
	int cycleStall = 0;
	bool done_temp;
	int sentCount = 0;
	bool deadlock = false;
	
	while (done[i] != true ) 
	{		
		// Step 1: Enter the post office
		printf ("Person %u enters the post office \n",i);

		// Step 2 - 4
		printf("-Person %u checks - he has %u letters in mailbox. \n",i,NumMsg[i]);
		while (NumMsg[i] != 0) 
		{ // read msg if a mailbox is not empty
			mailboxSemaphore[i]->P();
			printf("--Person %u claims Semaphore[%u] \n",i,i);
			printf("---Person %u reads %uth msg: ",i,NumMsg[i]);
			printf("%s ", MailBox[i][NumMsg[i]-1].Msg);
			printf("Sent by Person %u \n", MailBox[i][NumMsg[i]-1].Who);
			NumMsg[i]--;
			mailboxSemaphore[i]->V();
			printf("----Person %u releases Semaphore[%u] \n",i,i);
			printf("-----Person %u yields after a read \n",i);
			currentThread->Yield();	
		}
		
		MsgCntSemaphore->P();
		printf("------------Person %u is updating NumMsgSent \n",i);
		if (NumMsgSent < M)
		{
		// step 5		
			do { 
				iSendTo = Random() % P;
			} while (iSendTo == i);
			int z = Random() % 6;
			msgSent = MsgList[z];
			printf("------Person %u complied %s sending to Person %u \n",i,msgSent,iSendTo);
			
			printf("-------Person %u checks - Person %u has %u letters in mailbox. \n",i,iSendTo,NumMsg[iSendTo]);		
			// step 7		
			while (NumMsg[iSendTo] == S && deadlock == false) 
			{
				sentCount ++;
				if (sentCount == 3) {deadlock = true;}
				printf("----------Person %u yields %uth times because mailbox[%u] is full! \n",i,sentCount,iSendTo);
				currentThread->Yield();	
			}		
			
			// step 6: sent msg
			if (deadlock == false)
			{
				mailboxSemaphore[iSendTo]->P();
				printf("---------Person %u claims Semaphore[%u]\n",i,iSendTo);				
				NumMsg[iSendTo]++;		
				MailBox[iSendTo][NumMsg[iSendTo]-1].Msg = msgSent;			
				MailBox[iSendTo][NumMsg[iSendTo]-1].Who = i;
				printf("----------Person %u successfully send %s to Person %u \n",i,MailBox[iSendTo][NumMsg[iSendTo]-1].Msg,iSendTo);								
				NumMsgSent++;
				printf("--------------Person %u successfully updates the overall msg count to %u \n",i,NumMsgSent);						
				mailboxSemaphore[iSendTo]->V();
				printf("-----------Person %u release Semaphore[%u]\n",i,iSendTo);
			} else
			{
				printf("----------Person %u aborts sending attempt to Person %u due to potential deadlock! \n",i,iSendTo);
				deadlock = false;
				sentCount = 0;
			}
			
		}
		MsgCntSemaphore->V();
		printf("-------------Person %u finishes updating NumMsgSent \n",i);
		// Step 8: leave post office
		printf("---------------Person %u leaves the post office. \n",i);
		
		// Step 9: wait for 2-5 cycles
		cycleStall = Random() % 4 + 2;
		int iStall = 0;
		while (iStall != cycleStall) 
		{ 
			iStall++; 
			printf("----------------Person %u yields %u cycles \n",i,iStall);
			currentThread->Yield();
		}
			
		// check if Person i should go home
		// done[i] is true under two conditions:
		// 		1. all mails have been read.
		//		2. The amount of msg being sent is as prompted.
		done_temp = true;
		for (int j=0; j<P; j++)
		{
			done_temp = done_temp && (NumMsg[j]==0);
//			if (NumMsg[j]==0) {done_temp = done_temp;}
//			else {done_temp = false;}
		}
//		if ((done_temp == true) && (NumMsgSent == M)) {done[i] = true;}
		done[i] = done_temp && (NumMsgSent==M);			
	}
	
	printf("Person %u is free to go home and never come back! \n",i);
}

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
	int only_sign = 0; // use to detect input '-' and '+'
	const char * negZero = "-0"; // use to cmp special case '-0' 
	bool num_append = true; // has number appended after the first decimal point?
	
	i = atoi (str); // extract number only
	printf("%d \n",i);
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
				only_sign = 0;
				num_append = true;
			} 
			// check the number and the position of the decimal points
			else if (a == 46){
				if ((j == 0) || (only_sign == 1)){ // '.32' or '-.3123' or '+.1231' are all character
					flag = 1;
				} else {
					num_pt++;
					if (num_pt == 1) num_append = false;
					else if (num_pt > 1) flag = 1;
				}
			}
			// determine if "-" or "+" appears in the middle of the string
			// instead of beginning
			else if ((a == 43 || a == 45) == true) {
				if (j == 0) {
					only_sign = 1;
				}
				if (j != 0) {
					flag = 1;
				}
			}
			else {
				flag = 1;
			}
			j++;
		}
		
		// input is combination of '-', '+', and '.' 
		if (only_sign == 1 || num_append == false) {
			flag = 1;
		}
		
		if (flag == 1) {
			TYPE = 5;
		} else {
			// determine the sign of the decimal number
			if (str[0] == '-') {
				if (strcmp(str,negZero) == 0) {TYPE = 0;}
				else {TYPE = 4;}
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

//----------------------------------------------------------------------
// promptInput
// 	Prompt Input with type = InputType (1: integer)
//	InputType value refers to CheckType();
//----------------------------------------------------------------------

int promptInput (int InputType){
	char *  InputString = new char;
	scanf("%s",InputString);
	while (CheckType(InputString) != InputType){
		printf("Error: Please check the input type! \n");
		printf("Please reenter: ");
		scanf("%s",InputString);
	}
	return atoi(InputString);
	delete InputString;
}

//----------------------------------------------------------------------
// Shout
// 		A shouter shouts msg NumShout times. Each time, the msg is choosen
//		randomly from the msg pool.
//		After each shout, a shouter yields random cycles, i.e. 2-5 cycles.
//----------------------------------------------------------------------
void Shout(int which){

	int j; // msg index
	int stallCycle; // how many cycles each thread yields.
	
	for (int i=0; i<NumShout; i++) {
		
		j = Random() % 6;
		printf("Thread %i shouts for %i times: %s \n",which,i+1,MsgList[j]);

		// stall random cycles before continuing		
		stallCycle = Random() % 4 + 2; // randomly choose between 2 and 5. 
		while (stallCycle != 0) {
			currentThread->Yield();
			stallCycle--;
		}
			
	}
} 
