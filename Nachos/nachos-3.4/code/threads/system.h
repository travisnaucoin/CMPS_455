// system.h
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H
 
#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"

// begin Anderson
#include "synch.h"
// end Anderson

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

#ifdef USER_PROGRAM
#include "machine.h"
#include "bitmap.h" // Marcus
extern Machine* machine;	// user program memory and registers
extern BitMap *MainMemMap;
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif

// begin Anderson
extern char * selectArgs; 		// select which task is running in threadtest.cc git test
extern char * MemAlgSelArgs;
extern int CheckType (char *);
extern int MemoryAllocation (void);
extern int ProcessId;
extern Semaphore * MutexNumProc;
extern int NumProcess;
extern int MemAll;

struct ProcessElement {
	
		ProcessElement * Next;
		ProcessElement * Previous;
		
		Thread * CurrentThread;
		int PID;
		bool Valid;
		int ParentPID;
		Semaphore * ProcessSemahpore;
};

class ProcessList {
	public: 
		ProcessList();
		
		bool IsEmpty();
		void Append(ProcessElement *);
		void Remove(ProcessElement *);
		ProcessElement * Return(int PID); // if process exit, return true;
		
	private:
		ProcessElement * Head;
		ProcessElement * Tail;
};

// end Anderson
extern ProcessList * PCB;

#endif // SYSTEM_H
