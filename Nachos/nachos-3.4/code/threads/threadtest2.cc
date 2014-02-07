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
#include "stdio.h"
//#include "stdlib.h"
#include "ctype.h"

extern void RandomInit(unsigned seed);
extern int Random();
int meals= 0;			// global variables to solve Dining Philosopher's problem
int PhilosopherSeated = 0;
int PhilosophersReady = 0;
int mealsEaten = 0;
Semaphore **chopsticks;		// Semaphore for task 2			
int *chopstick; 		// Array for flag 3
int mailboxes = 0;
int boxSize = 0;
int sendLimit = 0;
int messagesSent = 0;




//------------------------------------------------------------------------
//  struct DataPass 
//          used to pass multiple parameters to a forked thread
//-------------------------------------------------------------------------
typedef struct DataPass
{
	int x;
        int y;
	int z;
}; 

//-------------------------------------------------------------------------
//  struct Mailbox
//      used to store the data from the PostOffice Simulation
//----------------------------------------------------------------------
typedef struct Mailbox
{
     Semaphore *MailBoxSem;
     int    sender;
     int    message;
     int    MailBoxFlag;                  //  Flag used for busy wait loops in task 5
};
Mailbox **PostOffice;



// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------
void
SimpleThread(int which)
{
    int num;
    
    for (num = 1; num <= 2; num++) {
	printf("The old thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}


//---------------------------------------------------------------------
// OtherThreads
//      Part of HW # 1 solution
//	Loops 2 times and switches between threads to print data
//----------------------------------------------------------------------
void OtherThreads(int which)
{
	for (int i = 1; i<=2; i++)
	{
		printf("The new thread %d looped %d times\n", which, i);
		currentThread->Yield();
	}
}


//--------------------------------------------------------------------------
// Wait(int num)
//     Uses busy waiting loops to simulate P() function for semaphores
//     Used to solve Philosopher's problem without semaphores
//---------------------------------------------------------------------------
void Wait(int num)
{
	bool  ReadytoRun = FALSE;
        while (ReadytoRun == FALSE)
        {
           if (chopstick[num] == 1)
	   {
	      chopstick[num]--;
	      ReadytoRun = TRUE;	
	   }
	   else
	   { 
	      int randomnum = 2+ Random()%5;
	      for (int i=1;i<=randomnum;i++)
	      {
		  currentThread->Yield();
              }
	   }
        } 
	
}


//-------------------------------------------------------------------------
//  Signal(int num)
//       Raises the flag value of the chopstick flag to 1;
//-------------------------------------------------------------------------
void Signal(int num)
{
	chopstick[num]++;

}


//-------------------------------------------------------------------------
//  Task 1: 
//     carries out Randomly Shouting Threads
//-------------------------------------------------------------------------
void Task1(int data)
{

        
	DataPass *localData = (DataPass*) data;
	int ThreadNum = localData->x;
	int NumPeople = localData->y;
	int NumShouts = localData->z;
	int RandNum;
	int ShoutCount = 0;
        while  (ShoutCount < NumShouts)
	{
        	RandNum = 1 + Random()%NumPeople;// pickking which thread to run
		if (RandNum == ThreadNum)        // if current thread is chosen
		{

			RandNum = 1 + Random() %5;//choosing which statement to say
			if (RandNum == 1)
			{
				printf("Shouter %d: Howdy Ho! \n",ThreadNum);
			}
			if (RandNum == 2)
			{
				printf("Shouter %d: Fe Fi Fo Fum! \n",ThreadNum);
			}
			if (RandNum == 3)
			{
				printf("Shouter %d: Big dogg in da building! \n",ThreadNum);
			}
			if (RandNum == 4)
			{
				printf("Shouter %d: Here we go!!!!! \n",ThreadNum);
			}
			if (RandNum == 5)
			{
				printf("Shouter %d: I command you to grow! \n",ThreadNum);
			} 
			ShoutCount++;
			RandNum = 2 + Random()%5;
			// busy wait loop
			for (int i = 1; i<= RandNum; i++)
			{
		              currentThread->Yield();
			}
		}
			// if current thread is not chosen, it goes straight to yield
		currentThread->Yield();

   	}
}

//----------------------------------------------------------------------------
//  Task2 (int numPhilosophers)
//        Solves the dining philosopher's problem with semaphores
//----------------------------------------------------------------------------
void Task2(int info)
{
	DataPass *localInfo = (DataPass*)info;
	int ThreadNum = localInfo->x;
	int numPhilosophers = localInfo->y;
	int RandomNum;

        printf("Philosopher %d has joined the table \n", ThreadNum);
	PhilosopherSeated --;
	if (PhilosopherSeated == 0)
	{
	    printf("All of the philosophers have been seated \n \n \n");
	}
	else
	{
	  while (PhilosopherSeated > 0)
            {
	      currentThread->Yield();
            }	
	}
		// now all philosophers are seated

    while (meals > 0)
    {	
	
		// reaching for leftChopstick
	if(ThreadNum == 1) // philospher 1 reaching for chopstick # maxNumphilosopers
	{	
		chopsticks[numPhilosophers]->P();
		printf("     Philosopher %d has picked up his left chopstick \n",ThreadNum);
	}
	else		// other philsophers reaching for left chopstick
	{
		chopsticks[ThreadNum-1]->P();
		printf("     Philosopher %d has picked up his left chopstick \n",ThreadNum);
	}
		// reaching for rightChopstick

	chopsticks[ThreadNum]->P();
	printf("     Philosopher %d has picked up his right chopstick \n",ThreadNum);
		// begin to eat
      if (meals > 0)
      {	
		// busy loop next	
        meals--;
        mealsEaten++;
	printf("Philosopher %d has begun to eat (%d Philosophers ate so far) \n",ThreadNum, mealsEaten);
	RandomNum = 2 + Random()%5;
	for (int i=1;i<=RandomNum;i++)
	{
	    currentThread->Yield();
	}
	
	printf("     Philosopher %d has finished eating  \n",ThreadNum);
     }
		//dropping left chopstick	
	if (ThreadNum == 1)
	{
		chopsticks[numPhilosophers]->V();
	}		
	else
	{
		chopsticks[ThreadNum - 1]->V();
	}
	printf("     Philosopher %d has dropped his left chopstick \n",ThreadNum);
		// dropping right chopstick

	chopsticks[ThreadNum]->V();
	printf("     Philosopher %d had dropped his right chopstick \n",ThreadNum);
 
                // start thinking
	printf("Philosopher %d started pondering the mysteries of the universe.\n",ThreadNum);
	RandomNum = 2 + Random()%5;
	for (int i = 1; i<=RandomNum;i++)
	{
	    currentThread->Yield();
	}
    }
    PhilosophersReady++;
    while (PhilosophersReady < numPhilosophers) // waiting for all philosophers to be ready
    {
	currentThread->Yield();
    }
    printf("Philosopher %d has left the table \n",ThreadNum);
}


//----------------------------------------------------------------------------
//  Task3 (int numPhilosophers)
//        Solves the dining philosopher's problem with busy wait loops
//----------------------------------------------------------------------------
void Task3(int info)
{
	DataPass *localInfo = (DataPass*)info;
	int ThreadNum = localInfo->x;
	int numPhilosophers = localInfo->y;
	int RandomNum;	

        printf("Philosopher %d has joined the table \n", ThreadNum);
	PhilosopherSeated --;
	if (PhilosopherSeated == 0)
	{
	    printf("All of the philosophers have been seated \n \n \n");
	}
	else
	{
            while (PhilosopherSeated > 0)
            {
	      currentThread->Yield();
            }	
	}
		// now all philosophers are seated

    while (meals > 0)
    {	
		// reaching for leftChopstick
	if(ThreadNum == 1) // philospher 1 reaching for chopstick # maxNumphilosopers
	{
		Wait(numPhilosophers);
		printf("     Philosopher %d has picked up his left chopstick \n",ThreadNum);
	}
	else		// other philsophers reaching for left chopstick
	{
		Wait(ThreadNum-1);
		printf("     Philosopher %d has picked up his left chopstick \n",ThreadNum);
	}
		// reaching for rightChopstick
	currentThread->Yield();
	Wait(ThreadNum);
	printf("     Philosopher %d has picked up his right chopstick \n",ThreadNum);
		// begin to eat
      if (meals > 0)
      {	
		// busy loop next	
        meals--;
        mealsEaten++;
	printf("Philosopher %d has begun to eat (%d Philosophers ate so far) \n",ThreadNum, mealsEaten);
	RandomNum = 2 + Random()%5;
	for (int i=1;i<=RandomNum;i++)
	{
	    currentThread->Yield();
	}
	
	printf("     Philosopher %d has finished eating  \n",ThreadNum);
     }
		//dropping left chopstick	
	if (ThreadNum == 1)
	{
		Signal(numPhilosophers);
	}		
	else
	{
		Signal(ThreadNum - 1);
	}
	printf("     Philosopher %d has dropped his left chopstick \n",ThreadNum);
		// dropping right chopstick
	Signal(ThreadNum);
	printf("     Philosopher %d had dropped his right chopstick \n",ThreadNum);
 
                // start thinking
	printf("Philosopher %d started pondering the mysteries of the universe.\n",ThreadNum);
	RandomNum = 2 + Random()%5;
	for (int i = 1; i<=RandomNum;i++)
	{
	    currentThread->Yield();
	}
    }
    PhilosophersReady++;
    while (PhilosophersReady < numPhilosophers) // waiting for all philosophers to be ready
    {
	currentThread->Yield();
    }
    printf("Philosopher %d has left the table \n",ThreadNum);
}


//---------------------------------------------------------------------------
//   Message(int message) 
//        outputs what is in the PostOffice messages
//---------------------------------------------------------------------------
void Message(int message)
{
     if (message == 1)
     {
	printf("Orange \n");
     }
     else if (message == 2)
     {
	printf("Blue \n");
     }
     else if (message == 3)
     {
	printf("Red \n");
     }
     else if (message == 4)
     {
	printf("White \n");
     }
     else if (message == 5)
     {
	printf("Black \n");
     }
}


//----------------------------------------------------------------------------
// Task4(int ThreadNum)
//        runs the Post Office Simulation with semaphores
//----------------------------------------------------------------------------
void Task4(int ThreadNum)
{
    int    destination;             // recipient of outgoing message
    int    messageDetail; 	    // describing contents of message
    bool   messageSent;		    // keeps track of outgoing message
    bool   foundEmptySlot;	    // alerts when open slot is found in mailbox
    int    search;		    // keeps track of empty slot
    int    waitTime;		    // Random length of busy wait loop
    int    timesChecked = 0;        // Amount of times a mailbox was found full
 
    while (messagesSent < sendLimit)
    {
	// read messages in mailbox
	printf("Person %d is checking for new mail at the post office \n",ThreadNum);
	for (int i=boxSize;i>=1;i--)
	{
//	    printf("=========================================================\n");
	    printf("%d\n",PostOffice[ThreadNum][i].sender);
	     if (PostOffice[ThreadNum][i].sender != 0) //  message in box
	     {	
		 PostOffice[ThreadNum][i].MailBoxSem->P();
		 printf("     +Person %d is reading mail from Person %d containing message :",ThreadNum,PostOffice[ThreadNum][i].sender);
	  	 Message(PostOffice[ThreadNum][i].message);
			//emptying mailbox slot
		 PostOffice[ThreadNum][i].message = 0;
	   	 PostOffice[ThreadNum][i].sender = 0;
			//freeing up that mailbox
		 PostOffice[ThreadNum][i].MailBoxSem->V();
			//letting other people do check mailbox
		 currentThread->Yield();
 	     }
	}
		// choosing who to send a message to
	while ((destination == ThreadNum) || (destination == 0))
	{
	    destination = 1+Random()%mailboxes;
        }
	messageDetail = 1+Random()%5;
	printf("Person %d is done checking mail and now is composing a new message to person %d \n",ThreadNum,destination);
		// sending a message
	messageSent = FALSE;
	foundEmptySlot = FALSE;
		// loop to continue until message is sent
	while((messageSent == FALSE) && (messagesSent < sendLimit))  
        {
	    search = 1; 
		//searching for an empty slot
	    while(search<=boxSize && foundEmptySlot == FALSE)
            {

		if (PostOffice[destination][search].sender == 0)
	        {	//case 1: empty slot is found
			//        send message

		    foundEmptySlot = TRUE;
		    printf("1 ");
		    PostOffice[destination][search].MailBoxSem->P();
		    printf("2 ");
		    PostOffice[destination][search].message = messageDetail;
	            PostOffice[destination][search].sender = ThreadNum;
		    PostOffice[destination][search].MailBoxSem->V();
		    messageSent = TRUE;
		    messagesSent++;
		    printf("     Person %d sent a message to Person %d with a message containing: ",ThreadNum,destination);
                    Message(messageDetail);
		    printf("************************ Messages Sent = %d \n",messagesSent);
		}
		else if(PostOffice[destination][search].sender != 0)
		{	//case 2: empty slot is not found
			//   search the next spot

		    search++;

                }
            

	        if ((foundEmptySlot == FALSE) && (search >= boxSize))
			// busy waiting loop if message not sent
	        {
		   
		   timesChecked++;
                   printf("Person %d did not find any open mail slots for person %d (%d) time(s)\n",ThreadNum,destination,timesChecked);
		   if (timesChecked >= 3) // stop trying to send message if checked 3 times
		   {
		       printf("After waiting %d times, Person %d has decided to try and send his message to Person %d another time\n",timesChecked,ThreadNum,destination);
		       messageSent = TRUE;
		   }
		
       		   else
		   {	//perform busy wait loop is timesChecked < 3
		      waitTime = 2+Random()%5;
		      for (int i=1;i <= waitTime;i++)
		      {
		         currentThread->Yield();
                      }
		   }
		   if (messagesSent == sendLimit)
		   {
			printf("Person %d quit because the simulation is over \n",ThreadNum);
			messageSent =  TRUE;
		   }
            }
           }
        }
	// person going back to work
       
	printf("Person %d is leaving the post office... back to work \n",ThreadNum);
	waitTime = 2 + Random() %5;
	for(int i =1;i<=waitTime;i++)
	{
	     currentThread->Yield();
	}
        


    }
    printf("\n");
}


//--------------------------------------------------------------------------
// Wait5(int owner, int slot)
//     Uses busy waiting loops to simulate P() function for semaphores
//     Used to solve Post Office problem without semaphores
//     Used with Task 5
//---------------------------------------------------------------------------
void Wait5(int owner, int slot)
{
	bool  ReadytoRun = FALSE;
        while (ReadytoRun == FALSE)
        {
           if (PostOffice[owner][slot].MailBoxFlag == 1)
	   {
	      PostOffice[owner][slot].MailBoxFlag--;
	      ReadytoRun = TRUE;	
	   }
	   else
	   { 
	      int randomnum = 2+ Random()%5;
	      for (int i=1;i<=randomnum;i++)
	      {
		  currentThread->Yield();
              }
	   }
        } 
	
}


//-------------------------------------------------------------------------
//  Signal5(int owner, int slot)
//       Raises the flag value of the MailBox flag to 1;
//-------------------------------------------------------------------------
void Signal5(int owner, int slot)
{
	PostOffice[owner][slot].MailBoxFlag++;

}


//----------------------------------------------------------------------------
// Task 5
//     Solves the Post Office problem using busy wait loops
//----------------------------------------------------------------------------
void Task5(int  ThreadNum)
{
    int    destination;             // recipient of outgoing message
    int    messageDetail; 	    // describing contents of message
    bool   messageSent;		    // keeps track of outgoing message
    bool   foundEmptySlot;	    // alerts when open slot is found in mailbox
    int    search;		    // keeps track of empty slot
    int    waitTime;		    // Random length of busy wait loop
    int    timesChecked = 0;        // Amount of times a mailbox was found full
 
    while (messagesSent < sendLimit)
    {
	// read messages in mailbox
	printf("Person %d is checking for new mail at the post office \n",ThreadNum);
	for (int i=boxSize;i>=1;i--)
	{

	    printf("%d\n",PostOffice[ThreadNum][i].sender);
	     if (PostOffice[ThreadNum][i].sender != 0) //  message in box
	     {	
		// PostOffice[ThreadNum][i].MailBoxSem->P();
		 Wait5(ThreadNum,i);
		 printf("     +Person %d is reading mail from Person %d containing message :",ThreadNum,PostOffice[ThreadNum][i].sender);
	  	 Message(PostOffice[ThreadNum][i].message);
			//emptying mailbox slot
		 PostOffice[ThreadNum][i].message = 0;
	   	 PostOffice[ThreadNum][i].sender = 0;
			//freeing up that mailbox
		// PostOffice[ThreadNum][i].MailBoxSem->V();
		 Signal5(ThreadNum,i);
			//letting other people do check mailbox
		 currentThread->Yield();
 	     }
	}
		// choosing who to send a message to
	while ((destination == ThreadNum) || (destination == 0))
	{
	    destination = 1+Random()%mailboxes;
        }
	messageDetail = 1+Random()%5;
	printf("Person %d is done checking mail and now is composing a new message to person %d \n",ThreadNum,destination);
		// sending a message
	messageSent = FALSE;
	foundEmptySlot = FALSE;
		// loop to continue until message is sent
	while((messageSent == FALSE) && (messagesSent < sendLimit))  
        {
	    search = 1; 
		//searching for an empty slot
	    while(search<=boxSize && foundEmptySlot == FALSE)
            {

		if (PostOffice[destination][search].sender == 0)
	        {	//case 1: empty slot is found
			//        send message

		    foundEmptySlot = TRUE;
	
		    Wait5(destination,search);

		    PostOffice[destination][search].message = messageDetail;
	            PostOffice[destination][search].sender = ThreadNum;

		    Signal5(destination,search);
		    messageSent = TRUE;
		    messagesSent++;
		    printf("     Person %d sent a message to Person %d with a message containing: ",ThreadNum,destination);
                    Message(messageDetail);
		    printf("************************ Messages Sent = %d \n",messagesSent);
		}
		else if(PostOffice[destination][search].sender != 0)
		{	//case 2: empty slot is not found
			//   search the next spot
      
		    search++;

                }
            

	        if ((foundEmptySlot == FALSE) && (search >= boxSize))
			// busy waiting loop if message not sent
	        {
	
		   timesChecked++;
                   printf("Person %d did not find any open mail slots for person %d (%d) time(s)\n",ThreadNum,destination,timesChecked);
		   if (timesChecked >= 3) // stop trying to send message if checked 3 times
		   {
		       printf("After waiting %d times, Person %d has decided to try and send his message to Person %d another time\n",timesChecked,ThreadNum,destination);
		       messageSent = TRUE;
		   }
		
       		   else
		   {	//perform busy wait loop is timesChecked < 3
		      waitTime = 2+Random()%5;
		      for (int i=1;i <= waitTime;i++)
		      {
		         currentThread->Yield();
                      }
		   }
		   if (messagesSent == sendLimit)
		   {
			printf("Person %d quit because the simulation is over \n",ThreadNum);
			messageSent =  TRUE;
		   }
            }
           }
        }
	// person going back to work
       
	printf("Person %d is leaving the post office... back to work \n",ThreadNum);
	waitTime = 2 + Random() %5;
	for(int i =1;i<=waitTime;i++)
	{
	     currentThread->Yield();
	}
        


    }
    printf("\n");

}
//----------------------------------------------------------------------------
//   TaskSetup (int task)
//         Accepts user input for the different tasks in Assignment 1
//         Forks the necessary threads and attaches functions/tasks to threads
//----------------------------------------------------------------------------- 
void  TaskSetup(int task)
{
	
	DataPass *holder;
	holder = new DataPass;
	int people,shouts;

	if (task == 1)
	{
		printf("-------------------------------------------------\n");
		printf("How many people are shouting   ");
		scanf("%d", &people);

		printf("How many times will they shout? ");
		scanf("%d",&shouts);

        	for (int i = 1; i<= people; i++)
		{
		   holder = new DataPass;
		   // passing values into array for parameter passing
		   holder->x = i;
		   holder->y = people;
		   holder->z = shouts;
		   Thread *t = new Thread("forked thread");
		   t->Fork(Task1,(int) holder);
		}
		
	}
	else if (task == 2)
	{
		
		printf("----------------------------------------------------");
		printf("\n How many philosopher's will be eating?: ");
		scanf("%d",&people);
		printf("\n How many meals are available?: ");
		scanf("%d",&meals); 
	        PhilosopherSeated = people; // initializing global variable
		// initializing the semaphore array for chopsticks
		chopsticks = new Semaphore*[people+1];
		for (int j = 1; j<=(people+1);j++)
		{
		   chopsticks[j] = new Semaphore("chopstick",1);
		}

		for (int i = 1; i<=people; i++)
		{		
		   holder = new DataPass;
		   holder->x = i;
		   holder->y = people;
		   Thread *t = new Thread("Philosopher");
		   t->Fork(Task2,(int) holder);
		}
	}
	else if (task == 3)
	{
	    	printf("----------------------------------------------------");
		printf("\n How many philosopher's will be eating?: ");
		scanf("%d",&people);
		printf("\n How many meals are available?: ");
		scanf("%d",&meals); 
	        PhilosopherSeated = people; // initializing global variable
		// initializing the array of chopstick flags
		chopstick = new int[people+1];
		for (int i = 1; i<=people;i++)
		{
			chopstick[i] = 1;
		}
	

		for (int i = 1; i<=people; i++)
		{		
		   holder = new DataPass;
		   holder->x = i;
		   holder->y = people;
		   Thread *t = new Thread("Philosopher");
		   t->Fork(Task3,(int) holder);
		}
	}	
	else if (task == 4)
	{
             printf("---------------------------------------------------\n");
	     printf("How many mailboxes are there?:  ");
	     scanf("%d",&mailboxes);
	     printf("How many messages can each box hold?: ");
	     scanf("%d",&boxSize);
	     printf("How many messges can be sent before termination?: ");
	     scanf("%d",&sendLimit);
					// declares 2-d array
	     PostOffice = new Mailbox*[mailboxes+1];
             for (int i=1;i<=mailboxes;i++)
	     {		
		      PostOffice[i] = new Mailbox[boxSize +1];
             }

	     // initializing the two dimensional array
             for (int i=1;i<=mailboxes;i++)
	      {
		for (int j=1;j<=boxSize;j++)
		{
		    PostOffice[i][j].MailBoxSem = new Semaphore("mailbox",1);
		    PostOffice[i][j].sender = 0;
		    PostOffice[i][j].message = 0;
		}
              }
            	// creating the new threads
	
	     for (int i=1;i<=mailboxes;i++)
	     {
		Thread *t = new Thread("Mailbox User");
		t->Fork(Task4,i);
	     }

	}
	else if (task == 5)
	{
	    printf("---------------------------------------------------\n");
	     printf("How many mailboxes are there?:  ");
	     scanf("%d",&mailboxes);
	     printf("How many messages can each box hold?: ");
	     scanf("%d",&boxSize);
	     printf("How many messges can be sent before termination?: ");
	     scanf("%d",&sendLimit);
					// declares 2-d array
	     PostOffice = new Mailbox*[mailboxes+1];
             for (int i=1;i<=mailboxes;i++)
	     {		
		      PostOffice[i] = new Mailbox[boxSize +1];
             }

	     // initializing the two dimensional array
             for (int i=1;i<=mailboxes;i++)
	      {
		for (int j=1;j<=boxSize;j++)
		{
//		    PostOffice[i][j].MailBoxSem = new Semaphore("mailbox",1);
		    PostOffice[i][j].sender = 0;
		    PostOffice[i][j].message = 0;
		    PostOffice[i][j].MailBoxFlag = 1;
		}
              }
            	// creating the new threads
	
	     for (int i=1;i<=mailboxes;i++)
	     {
		Thread *t = new Thread("Mailbox User");
		t->Fork(Task5,i);
	     }


	}

}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
 	DEBUG('t', "Entering ThreadTest");

	int choice;
   
        if (*taskToRun == 1)
	{
		TaskSetup(1);
	}
	else if(*taskToRun == 2)
	{
		TaskSetup(2);
	}
	else if(*taskToRun == 3)
	{
		TaskSetup(3);
	}
	else if(*taskToRun == 4)
	{
		TaskSetup(4);
	}
	else
	{
 	  printf("Which task do you want to perform?\n");
	  scanf("%d", &choice);
	  int digit = isdigit(choice);
	 
	  if (choice == 1)
	  {
		TaskSetup(choice);
		
	  } 
	  else if (choice == 2)
	  {
		TaskSetup(choice);
	  }	
	  else if (choice == 3)
	  {
		TaskSetup(choice);
	  }
	  else if (choice == 4)
	  {
		TaskSetup(choice);
	  }
	  else if (choice == 5)
	  {
		TaskSetup(choice);
	  }
	}
}

