// progtest.cc 
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.  
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "synch.h"
#include "thread.h"

//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------
int NumProcess;
ProcessList * PCB;
BitMap *MainMemMap;
int MemAll = 0;	// Glb variable for selecting memory allocation algorithm.
extern char * MemAlgSelArgs;
extern int CheckType (char *);
Semaphore * MutexNumProc;

int MemAlloAlgSelect (void) {
	int MemAlg;
	int Select = 0;
	
	if (MemAlgSelArgs == NULL) {
		Select = 0;
		printf("First-fit algorithm is selected for memory allocation.\n");
	} else if (CheckType(MemAlgSelArgs)==1) {
		MemAlg = atoi (MemAlgSelArgs);
		if (MemAlg == 1) {
			Select = 0;
			printf("First-fit algorithm is selected for memory allocation.\n");
		} else if (MemAlg == 2) {
			Select = 1;
			printf("Best-fit algorithm is selected for memory allocation.\n");
		} else if (MemAlg == 3) {
			Select = 2;
			printf("Worst-fit algorithm is selected for memory allocation.\n");
		} else {
			Select = 0;
			printf("Wrong value for -M option.\n");
			printf("Will select default first-fit algorithm.\n");
		}
	} else 	{
		Select = 0;
		printf("Wrong type for value of -M option.\n");
		printf("Will select default first-fit algorithm.\n");
	}
	return Select;
}

void
StartProcess(char *filename)
{	
	MemAll = MemAlloAlgSelect();
	
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;
    if (executable == NULL) {
		printf("Unable to open file %s\n", filename);
		return;
    }
	
	currentThread->CreatId();
	int PID = currentThread->GetId();
	printf("Process %u is created. \n",PID);
	
	MainMemMap = new BitMap(NumPhysPages);
	
    space = new AddrSpace(executable);    
    currentThread->space = space; 
		
	// Create and Update PCB;
	NumProcess = 0;
	PCB = new ProcessList ();
	printf("PCB is created.\n");
	
	ProcessElement * ProcessTemp = new ProcessElement;
	ProcessTemp->ParentPID = 0;
	ProcessTemp->PID = PID;
	ProcessTemp->CurrentThread = currentThread;
	ProcessTemp->ProcessSemahpore =  new Semaphore("ProcessSemaphore",0);
	ProcessTemp->Next = ProcessTemp;
	ProcessTemp->Previous = ProcessTemp;
	ProcessTemp->Valid = true;	
	PCB->Append(ProcessTemp);
	MutexNumProc = new Semaphore("MutexNumProc", 1);
	MutexNumProc -> P();
	++NumProcess;
	printf("This is the %uth Process.\n",NumProcess);
    MutexNumProc -> V();
	delete executable;			// close file

    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register

    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void 
ConsoleTest (char *in, char *out)
{
    char ch;

    console = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    
    for (;;) {
	readAvail->P();		// wait for character to arrive
	ch = console->GetChar();
	console->PutChar(ch);	// echo it!
	writeDone->P() ;        // wait for write to finish
	if (ch == 'q') return;  // if q, quit
    }
}
