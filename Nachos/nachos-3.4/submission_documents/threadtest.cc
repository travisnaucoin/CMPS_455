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

extern char * selectArgs;
// Some Variables are shared amount different tasks for conciseness.
// ------- Global Variables Declaration -----------
// ---- For Shouting Task ----
int NumShout;
int P; //number of people
int S; //number of messages a person's mailbox can hold;
int M; //total number of messages
char * MsgList [] = {"Pattern0","Pattern1","Pattern2","Pattern3","Pattern4","Pattern5"};

// ---- For Dining Philosopher (busy waiting) ----
int MealLeft;			
int PhilosopherSeated;
int PhilosopherLeaveRdy;
int *chopstick; 
int NumPhilNotIn; // Number of philosphers haven't got in
bool SeatGrant;
bool LeaveGrant;

// ---- For Dining Philosopher (Semaphore)
int * PStanding;
typedef enum {THINKING,ISEATING,ISHUNGRY} pState; //state of philosophers
pState *PhilState; //array holding all philosophers states
Semaphore * mutex; 
// mutex is used to guarantee only one philosopher can get or drop the chopstick at any moment;
Semaphore * EnterMutex; // protect NumPhilNotIn;
Semaphore * LeaveMutex;	// protect NumRdyLeave;
Semaphore * SitSemaphore;
Semaphore * LeaveSemaphore;
Semaphore * MealLeftMutex; // protect GLB variable MealLeft
Semaphore ** PhilGrantSemaphore;  // grant philosopher to sit or pick up chopsticks
int NumRdyLeave;
int NumGrantLeave;
bool noMeal;

// ---- For Post Office Simulation ---- //
int MsgSentCnt;  // Need to be protected.
bool * done; // Leaving signal for person
int * MsgPtr; // Num of messages in each mailbox ---- Need to be protected.
Semaphore ** mailboxSemaphore;
Semaphore ** freeSpaceSemaphore;
Semaphore *MsgCntSemaphore = new Semaphore ("MsgCntSemaphore",1);

struct MailSlot
{
	int Who;
	char * Msg; 
};
MailSlot MailBox [128][64];

// Function declaration
int CheckType (char *);
void CheckInputOnly(int); 
int promptInput (int);
void Shout(int);
void DinPhilBusyWait(int);
void DinPhilSemaphore(int);
void PostOfficeLoop(int);
void PostOfficeSemaphore(int);
bool ReachChopStick (int);
void Eat (int);
void GetSticks(int);
void PutSticks (int);
void LeaveRoom (int);
void BusyWaitingLoop(void);

//----------------------------------------------------------------------
// ThreadTest
//		Select task and create threads
//		Fork functions to threads
//
//		task is selected based on global variable selectArgs.
//		which is defined in system.cc
//----------------------------------------------------------------------
void ThreadTest()
{
	int Task = 0;
    DEBUG('t', "Entering ThreadTest");
	if (selectArgs == NULL)
		printf("No Value is assigned to -A option. \n");
	// decide which task to run
	else if (CheckType(selectArgs)==1) 
	{
		Task = atoi(selectArgs);
				
		if (Task == 1)
		{
			// check input type
			Thread *t = new Thread("forked thread");	
			t->Fork(CheckInputOnly, 1);			
		} 
		else if (Task == 2)
		{
			// executing shouting
			printf("Enter number of shouter: ");
			int T = promptInput(1); // integer type is desired
			printf("Shouts how many times: ");
			NumShout = promptInput(1); // integer type is desired
			for (int i=0; i<T; i++) {
				Thread *t = new Thread("forked thread");
				t->Fork(Shout, i);
			}
		} 
		else if (Task == 3)
		{
			// Prompt the input
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
			chopstick = new int[P];
			NumPhilNotIn = P;
			SeatGrant = false;
			LeaveGrant = false;
			MealLeft = M;
			PhilosopherLeaveRdy = 0;
			
			for (int i=0; i<P;i++)
				chopstick[i] = 1;
			
			for (int i=0; i<P; i++)
			{		
			  Thread *t = new Thread("forked thread");
			  t->Fork(DinPhilBusyWait,i);
			}
		} 
		else if (Task == 4)
		{
			//executing Dining Philosophers Semaphores
			printf("Enter number of philosophers: ");
			P = promptInput(1);
			while (P <= 1)
			{
				printf("Not enough people to begin simulation. \n");
				printf("Please reenter the number of philosophers:");
				P = promptInput(1);
			}
			printf("Enter number of meals: ");
			M = promptInput(1);
			
			PhilGrantSemaphore = new Semaphore*[P];
			PhilState = new pState[P];
			done = new bool [P];
			
			// Initialization
			NumPhilNotIn = P;
			MealLeft = M;
			LeaveGrant = false;
			NumRdyLeave = 0;
			NumGrantLeave = 0;
			noMeal = false;
			
			mutex = new Semaphore("mutex",1);
			MealLeftMutex = new Semaphore ("MealLeftMutex",1);
			EnterMutex = new Semaphore("EnterMutex",1);
			LeaveMutex = new Semaphore("LeaveMutex",1);
			SitSemaphore = new Semaphore("SitTogether",0);
			LeaveSemaphore = new Semaphore("LeaveTogether",0);
			for(int i = 0; i < P; i++)
			{
				PhilGrantSemaphore[i] = new Semaphore("Phil", 0);
				PhilState[i] = THINKING;
				done[i] = false;
			}
			
			for (int i = 0; i < P; i++)
			{
				Thread *t = new Thread("forked thread");
				t->Fork(DinPhilSemaphore,i);
			}		
		} 
		else if (Task == 5)
		{
			// executing Post Office with waiting loop
			printf("Enter number of people: ");
			P = promptInput(1);
			while (P<=1) {
				printf("Not enough people for proper simulation! \n");
				printf("Please reenter the number of people: ");
				P = promptInput(1);
			}
			printf("Enter the capacity of a mailbox: ");
			S = promptInput(1); 
			printf("Enter total number of messages: ");
			M = promptInput(1);	
			MsgSentCnt = 0;
			done = new bool[P];		
			mailboxSemaphore = new Semaphore*[P];
			MsgPtr =  new int [P];
			
			// Initialization
			for (int j=0; j<P; j++) {
				done [j] = false;
				MsgPtr[j] = 0;
				mailboxSemaphore[j] = new Semaphore ("mailboxSemaphore",1);
			}

			for (int i=0; i<P; i++) {	
				Thread *t = new Thread("forked thread");
				t->Fork(PostOfficeLoop, i);
			}			
		} 
		else if (Task == 6)
		{
			// executing Post Office with waiting loop
			printf("Enter number of people: ");
			P = promptInput(1);
			while (P<=1) {
				printf("Not enough people for proper simulation! \n");
				printf("Please reenter the number of people: ");
				P = promptInput(1);
			}
			printf("Enter the capacity of a mailbox: ");
			S = promptInput(1); 
			printf("Enter total number of messages: ");
			M = promptInput(1);	
			MsgSentCnt = 0;
			done = new bool[P];		
			mailboxSemaphore = new Semaphore*[P];
			freeSpaceSemaphore = new Semaphore*[P];
			MsgPtr =  new int [P];
			
			// Initialization
			for (int j=0; j<P; j++) {
				done [j] = false;
				MsgPtr[j] = 0;
				mailboxSemaphore[j] = new Semaphore ("mailboxSemaphore",1);
				freeSpaceSemaphore[j] = new Semaphore ("freeSpaceSemaphore",S);
			}

			for (int i=0; i<P; i++) {	
				Thread *t = new Thread("forked thread");
				t->Fork(PostOfficeSemaphore, i);
			}		
		}	
	} 
	else	printf("Error: Wrong input type for -A Option. \n");
	
	currentThread -> Finish(); 
}

//----------------------------------------------------------------------
//  	Task 1: Dining Philosophers (waiting loop) 
// 	Dinning Philosophers Problem using only busy waiting loop
//----------------------------------------------------------------------

// Begin code changes by Bradley Milliman
void DinPhilBusyWait(int i)
 {
	bool AbortPickup = false;
	bool PickupLeft = false;
	bool PickupRight = false;

    printf("-Philosopher %d has joined the room \n", i);
 	NumPhilNotIn--;
	
	printf("--Philosopher %d is waiting to sit\n", i);
 	while (NumPhilNotIn > 0)	
 	    currentThread->Yield();
	
	if (NumPhilNotIn==0 && SeatGrant == false)
	{		
		printf("---All philosiphers sit down at table\n"); 
		SeatGrant = true;
	}
	// now all philosophers are seated
 
    while (MealLeft != 0)
    {	
		// Pickup Order: Philosopher will try to pickup the left chopstick first.
		// 					If succeed, he will then try to reach the right one.
		// Abort: Whenever failing for 5 times, he will abort pickup.
		//			If he can't even pickup the left one, he simply abort pickup.
		//			If he has already picked up the left one, but fails to pickup 
		//				the right one, he has to abort the pickup attempt and then
		//				drop the left one which has already being picked up.
		printf("----Philosopher %d is trying to pick up chopstick [%u] \n",i,i%P);
		PickupLeft = ReachChopStick(i%P);
		if (PickupLeft == true)
		{
			printf("-----Philosopher %d has picked up chopstick [%u] \n",i,i%P);
			printf("----Philosopher %d is trying to pick up chopstick [%u] \n",i,(i+1)%P);
			PickupRight = ReachChopStick((i+1)%P);
			if (PickupRight == true)
				printf("------Philosopher %d has picked up chopstick [%u] \n",i,(i+1)%P);
			else
			{
				AbortPickup = true;
				printf("-------Philosopher %u abort picking up chopstick [%u] after 5 trials to prevent deadlock! \n",i,(i+1)%P);
				chopstick[i%P] = 1;
				printf("----------Philosopher %d drops chopstick [%u] \n",i,i%P);
			}
		}		
		else
			{
				AbortPickup = true;
				printf("-------Philosopher %u abort picking up chopstick [%u] after 5 trials to prevent deadlock! \n",i,i%P);
			}
		
		// Proceed if both chopsticks are picked up successfully.
		if (AbortPickup == false) 
		{
			// need to check how many meals left here because process may get kicked out when using -rs option.
			if (MealLeft == 0)
				printf("----------All meals have been eaten! \n");
			else
			{
				// begin to eat
				MealLeft--;
				printf("-------Philosopher %d begins to eat (%d meals have been eaten so far) \n",i, M-MealLeft);
				
				BusyWaitingLoop();
				
				printf("--------Philosopher %d finishes eating  \n",i);
			}
				
			chopstick[i%P] = 1;
			printf("---------Philosopher %d drops chopstick [%u] \n",i,i%P);
			// dropping right chopstick
			chopstick[(i+1)%P] = 1;
			printf("----------Philosopher %d drops chopstick [%u] \n",i,(i+1)%P);
		}
		
		AbortPickup = false;
		PickupLeft = false;
		PickupRight = false;
		
        // start thinking
		printf("-----------Philosopher %d Thinking \n",i);
		BusyWaitingLoop();
    }
	
    PhilosopherLeaveRdy++;
    printf("------------Philosopher %d is waiting to leave\n", i);
	
    while (PhilosopherLeaveRdy < P) // waiting for all philosophers to be ready
		BusyWaitingLoop();
	
	// All philosopher leave together.
	// The following snippet is only for printing the message once.
	if (PhilosopherLeaveRdy == P && LeaveGrant == false)
	{
	    printf("$$$$$ All Philosophers leave the table together. $$$$$ \n$$$$$World Peace! $$$$$ \n");
		LeaveGrant = true;
	}
 }

// ******************************* //
//		ReachChopStick
//	Philosopher attemps to pickup 
//	the chop sticks
// ******************************* //

bool ReachChopStick (int i)
{
	int CheckCnt = 0;
	bool Pickup = false;
	
	// reaching for Chopstick
	while (CheckCnt != 5 && Pickup == false)
	{
		if (chopstick[i] == 1)
		{
			chopstick[i] = 0;
			Pickup = true;	
		}
		else 
		{
			CheckCnt ++;	
			BusyWaitingLoop();
		}
	}
	return Pickup;
}

// ******************************* //
//		Busy Waiting Loop
// ******************************* //
void BusyWaitingLoop(void)
{
	int stallCycle = Random() % 4 + 2; // randomly choose between 2 and 5. 
	while (stallCycle != 0) {
		currentThread->Yield();
		stallCycle--;
	}
}
// End code changes by Bradley Milliman

//----------------------------------------------------------------------
//  				Task 2: Dining Philosophers (Semaphore) 
//---------------------------------------------------------------------- 

// Begin code changes by Travis Aucoin

// ******************************* //
//		DinPhilSemaphore 
// main function for task 2
// ******************************* //

void DinPhilSemaphore(int i)
{
	// Guarantee that no one will sit until everyone is present
	EnterMutex->P();
	printf("-Philosopher %i, has entered. \n",i);
	NumPhilNotIn--;
	if (NumPhilNotIn == 0) {
		printf ("--All philosophers are present \n");
		for (int j=0; j<P; j++) SitSemaphore -> V(); // grant P sit requests
	}
	EnterMutex -> V();
	SitSemaphore -> P();
	printf("---Philosopher %u sits at the table.\n",i);
	
	// Dinner Start
	while (done[i] != true) 
	{
		GetSticks(i);
		Eat(i);
		PutSticks(i);
		BusyWaitingLoop();	
	}
	
	// All meals have been eaten.
	// Philosopher will leave together.
	LeaveRoom(i);	
}

// ******************************* //
//				GetSticks
//  Check the status of adjacent philosophers
//  to see if they can eat
// ******************************* //
void GetSticks(int i)
{
	mutex->P();	
	PhilState[i] = ISHUNGRY;
	bool left;
	bool right;
	
	while (PhilState[i] == ISHUNGRY)
	{
		left = false;
		right = false;
		
		if(PhilState[(i-1+P) % P] != ISEATING) left = true;
		
		if (P>2)
		{
			if(PhilState[(i+1) % P] != ISEATING) right = true;
		}
		else if (P==2)	right =true; // In case of 2 philosopher, there is no neighbor on the right.
				
		if (left)	printf("----Philosopher %u's LEFT chopstick is available \n",i);
		else	printf("----Philosopher %u's LEFT chopstick is NOT available \n",i);
			
		if (right)	printf("----Philosopher %u's RIGHT chopstick is available \n",i);
		else printf("----Philosopher %u's RIGHT chopstick is NOT available \n",i);
		
		
		if ((PhilState[i] == ISHUNGRY &&
		PhilState[(i-1+P) % P] != ISEATING &&
		PhilState[(i+1) % P] != ISEATING && P > 2) || 
		(PhilState[i] == ISHUNGRY &&
		PhilState[(i+1) % P] != ISEATING && P == 2))
		{ 
			PhilState[i] = ISEATING;
			printf("-----Philosopher %u gets a pair of chopsticks \n",i);
			PhilGrantSemaphore[i]->V();	// skip PhilGrantSemaphore[i] -> P() if it is executed
		}
		
		mutex->V(); 
		PhilGrantSemaphore[i]->P(); // only be exected if no fork has been acquired.
	}
}


// ******************************* //
//				Eating
//  Make philosopher eat for a few cycles
// ******************************* //
void Eat(int i)
{
	MealLeftMutex -> P();
	if (MealLeft != 0)
	{
		printf("------Philosopher %i is eating.\n",i);
		MealLeft--;
		printf("-------Meals left: %i \n",MealLeft); //DEBUG PURPOSES
	} else 
	{
		printf("-------No meal is left for Philosoper %u \n",i);
		done[i] = true;
	}	
	MealLeftMutex -> V();
	BusyWaitingLoop();
}


// ******************************* //
//				PutSticks
//  Check adjacent ones to see if they are 
//  hungry, put down sticks and force his 
//	neighbor to check the state for avoiding
//	deadlock
// ******************************* //
void PutSticks(int i)
{	
	mutex->P();	
	// forcefully wake up its left neighbor to check the state to avoid deadlock
	if(PhilState[(i-1+P) % P] == ISHUNGRY) PhilGrantSemaphore[(i-1+P) % P]->V(); 
	printf("--------Philosopher %i puts down left stick\n", i);
	if (P > 2) // In case of 2 philosopher, there is nobody on the right hand side.
	{
		// forcefully wake up its right neighbor to check the state to avoid deadlock
		if(PhilState[(i+1) % P] == ISHUNGRY) PhilGrantSemaphore[(i+1) % P]->V();
		printf("---------Philosopher %i puts down right stick\n", i);
	}
	PhilState[i] = THINKING;
	printf("----------Philosopher %u start thinking. \n",i);
	mutex->V();
}


// ******************************* //
//			LeaveRoom
//  Make all philosophers leave room   
// ******************************* //
void LeaveRoom(int i)
{
	LeaveMutex -> P();
	NumRdyLeave++;
	printf("-----------Philosopher %u is ready to leave the table. \n",i);
	if (NumRdyLeave == P) 
	{
		printf ("------------All philosophers are ready to leave the table. \n");
		for (int j=0; j<P; j++) LeaveSemaphore -> V(); // grant P leave requests
	}
	LeaveMutex -> V();
	LeaveSemaphore -> P();
	LeaveMutex -> P();
	if (NumGrantLeave < P-1) 
	{	
		NumGrantLeave ++;
		printf("-------------Philosopher %u is granted to leave the table. \n",i);
		LeaveMutex -> V();
	} 
	else 
	{
		if (LeaveGrant == false) 
		{
			printf("-------------Philosopher %u is granted to leave the table. \n",i);
			LeaveGrant = true;
			printf("$$$$$$ All philosophers are now leaving the table together. $$$$$\n$$$$$ World Peace! $$$$$\n");
		}
	}	
}

// End code changes by Travis Aucoin
 
//----------------------------------------------------------------------
// PostOfficeLoop
// 	Task 3: Post Office Problem using busy waiting loop
//----------------------------------------------------------------------

// Start code changes by Xiyue Xiang

void PostOfficeLoop (int i)
{
	int iSendTo = i; // index of recipient
	char * msgSent = new char;
	int cycleStall = 0;
	bool done_temp;
	bool all_done; // print "simulation completes" msg
	int sentCount = 0;
	bool deadlock = false;
	
	while (done[i] != true ) 
	{		
		// Step 1: Enter the post office
		printf ("Person %u enters the post office \n",i);

		// Step 2 - 4
		printf("-Person %u checks - he has %u letters in mailbox. \n",i,MsgPtr[i]);
		while (MsgPtr[i] != 0) 
		{ // read msg if a mailbox is not empty
			mailboxSemaphore[i]->P();
			printf("--Person %u claims mailboxSemaphore[%u] \n",i,i);
			printf("---Person %u reads %uth msg: ",i,MsgPtr[i]);
			printf("%s ", MailBox[i][MsgPtr[i]-1].Msg);
			printf("Sent by Person %u \n", MailBox[i][MsgPtr[i]-1].Who);
			MsgPtr[i]--;
			mailboxSemaphore[i]->V();
			printf("----Person %u releases mailboxSemaphore[%u] \n",i,i);
			printf("-----Person %u yields after a read \n",i);
			currentThread->Yield();	
		}
		printf("------Person%u's mailbox is empty. \n",i);
		MsgCntSemaphore->P();
		if (MsgSentCnt < M)
		{
		// step 5		
			do { 
				iSendTo = Random() % P;
			} while (iSendTo == i);
			int z = Random() % 6;
			msgSent = MsgList[z];
			printf("------Person %u is trying sending %s to Person %u \n",i,msgSent,iSendTo);
			
			printf("-------Person %u checks - Person %u has %u letters in mailbox. \n",i,iSendTo,MsgPtr[iSendTo]);		
			// step 7		
			while (MsgPtr[iSendTo] == S && deadlock == false) 
			{
				sentCount ++;
				if (sentCount == 3) {deadlock = true;}
				printf("--------Person %u yields %uth times because mailbox[%u] is full! \n",i,sentCount,iSendTo);
				currentThread->Yield();	
			}		
			
			// step 6: sent msg
			if (deadlock == false)
			{
				mailboxSemaphore[iSendTo]->P();
				printf("---------Person %u claims mailboxSemaphore[%u]\n",i,iSendTo);				
				MsgPtr[iSendTo]++;		
				MailBox[iSendTo][MsgPtr[iSendTo]-1].Msg = msgSent;			
				MailBox[iSendTo][MsgPtr[iSendTo]-1].Who = i;
				printf("----------Person %u successfully send %s to Person %u \n",i,MailBox[iSendTo][MsgPtr[iSendTo]-1].Msg,iSendTo);								
				MsgSentCnt++;
				printf("-----------Person %u successfully sends the %uth message.\n" ,i,MsgSentCnt);						
				mailboxSemaphore[iSendTo]->V();
				printf("------------Person %u release mailboxSemaphore[%u]\n",i,iSendTo);
			} else
			{
				printf("------------Person %u aborts sending attempt to Person %u due to potential deadlock! \n",i,iSendTo);
				deadlock = false;
				sentCount = 0;
			}
			
		} 
		else	printf ("------------ All required messages have been sent! Abort sending attempt! \n"); 
		MsgCntSemaphore->V();
		
		// Step 8: leave post office
		printf("-------------Person %u leaves the post office. \n",i);
		
		// Step 9: wait for 2-5 cycles
		cycleStall = Random() % 4 + 2;
		int iStall = 0;
		while (iStall != cycleStall) 
		{ 
			iStall++; 
			printf("--------------Person %u yields %u cycles \n",i,iStall);
			currentThread->Yield();
		}
			
		// check if Person i should go home
		// done[i] is true under two conditions:
		// 		1. all mails have been read.
		//		2. The amount of msg being sent is as prompted.
		done_temp = true;
		for (int j=0; j<P; j++)
		{
			done_temp = done_temp && (MsgPtr[j]==0);
		}
		done[i] = done_temp && (MsgSentCnt==M);			
	}
	printf("****** Person %u is free to go home and never come back! ******\n",i);

	// Print the message if every message has been read.
	// Thread is terminated safely.
	all_done =  true;
	for (int j=0; j<P; j++)	all_done = done[j] && all_done;
	if (all_done == true) 
	{
		printf("$$$$$$$$$$ All %u messages have been sent and read. $$$$$$$$$$\n",M);
		printf("$$$$$$$$$$ All %u people have left the post office. $$$$$$$$$$\n",P);
		printf("$$$$$$$$$$ Exit simulation successfully! World peace! $$$$$$$$$$\n");
	}	
}

// End code changes by Xiyue Xiang

//----------------------------------------------------------------------
// PostOfficeSemaphore
// 	Task 4: Post Office Problem using Semphore to check if the mailbox has available space
//----------------------------------------------------------------------

// Start code changes by Marcus Amos

void PostOfficeSemaphore (int i)
{
	int iSendTo = i; // index of recipient
	char * msgSent = new char;
	int cycleStall = 0;
	bool done_temp;
	bool all_done;
	
	while (done[i] != true ) 
	{		
		// Step 1: Enter the post office
		printf ("Person %u enters the post office \n",i);

		// Step 2 - 5
		printf("-Person %u checks - he has %u letters in mailbox. \n",i,MsgPtr[i]);
		while (MsgPtr[i] != 0) 
		{ // read msg if a mailbox is not empty
			mailboxSemaphore[i]->P();
			printf("--Person %u claims mailboxSemaphore[%u] \n",i,i);
			printf("---Person %u reads %uth msg: ",i,MsgPtr[i]);
			printf("%s ", MailBox[i][MsgPtr[i]-1].Msg);
			printf("Sent by Person %u \n", MailBox[i][MsgPtr[i]-1].Who);
			MsgPtr[i]--;
			freeSpaceSemaphore[i]->V();
			printf("----Person %u releases and increases freeSpaceSemaphore[%u] by 1 \n",i,i);
			mailboxSemaphore[i]->V();
			printf("-----Person %u releases mailboxSemaphore[%u] \n",i,i);
			printf("------Person %u yields after a read \n",i);
			currentThread->Yield();	
		}
		printf("-------Person%u's mailbox is empty. \n",i);
		
		// step 6
		do { 
			iSendTo = Random() % P;
		} while (iSendTo == i);
		int z = Random() % 6;
		msgSent = MsgList[z];
		printf("--------Person %u is trying sending %s to Person %u \n",i,msgSent,iSendTo);
									
		// step 7-8: sent msg
		printf("---------Person %u tries to claim freeSpaceSemaphore[%u]. \n",i,iSendTo);
		freeSpaceSemaphore[iSendTo]->P();
		printf("----------Person %u gets and decreases freeSpaceSemaphore[%u] by 1 \n",i,iSendTo);
		mailboxSemaphore[iSendTo]->P();
		printf("-----------Person %u claims mailboxSemaphore[%u]\n",i,iSendTo);	
		
		MsgCntSemaphore->P();
		printf("------------Person %u claims MsgCntSemaphore \n",i);
		
		if (MsgSentCnt < M)
		{		
			MsgPtr[iSendTo]++;		
			MailBox[iSendTo][MsgPtr[iSendTo]-1].Msg = msgSent;			
			MailBox[iSendTo][MsgPtr[iSendTo]-1].Who = i;
			printf("-------------Person %u successfully send %s to Person %u \n",i,MailBox[iSendTo][MsgPtr[iSendTo]-1].Msg,iSendTo);								
			MsgSentCnt++;
			printf("--------------Person %u successfully updates the overall msg count to %u \n",i,MsgSentCnt);
		} else {
			printf("-------------Person %u aborts sending request because all msg have been sent!\n",i);
			freeSpaceSemaphore[iSendTo]->V(); // if MsgSentCnt is not updated because all msgs have been sent, freeSpaceSemaphore[iSendTo] needs to be release and # of free space should not be consumed.
			printf("--------------Person %u releases and increases freeSpaceSemaphore[%u] by 1 \n",i,iSendTo);			
		}		
		mailboxSemaphore[iSendTo]->V();
		printf("---------------Person %u release Semaphore[%u]\n",i,iSendTo);
		MsgCntSemaphore->V();
		printf("----------------Person %u release MsgCntSemaphore \n",i);
		
		// Step 9: leave post office
		printf("-----------------Person %u leaves the post office. \n",i);
		
		// Step 10: wait for 2-5 cycles
		cycleStall = Random() % 4 + 2;
		int iStall = 0;
		while (iStall != cycleStall) 
		{ 
			iStall++; 
			printf("------------------Person %u yields the %uth cycles \n",i,iStall);
			currentThread->Yield();
		}
			
		// check if Person i should go home or continue to step 11
		// done[i] is true under two conditions:
		// 		1. all mails have been read.
		//		2. The amount of msg being sent is as prompted.
		done_temp = true;
		for (int j=0; j<P; j++)
		{
			done_temp = done_temp && (MsgPtr[j]==0);
		}
		done[i] = done_temp && (MsgSentCnt==M);			
	}	
	printf("****** Person %u is free to go home and never come back! ******\n",i);
	
	all_done =  true;
	for (int j=0; j<P; j++)	all_done = done[j] && all_done;
	if (all_done == true) 
	{
		printf("$$$$$$$$$$ All %u messages have been sent and read. $$$$$$$$$$\n",M);
		printf("$$$$$$$$$$ All %u people have left the post office. $$$$$$$$$$\n",P);
		printf("$$$$$$$$$$ Exit simulation successfully! World peace! $$$$$$$$$$\n");
	}
}
// End code changes by Marcus Amos

// ---------------------------- Project 1 ------------------------------
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
