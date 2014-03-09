// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include <stdio.h>        // FA98
#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "addrspace.h"   // FA98
#include "sysdep.h"   // FA98

// begin FA98

static int SRead(int addr, int size, int id);
static void SWrite(char *buffer, int size, int id);

// end FA98

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

// begin Anderson

extern int NumProcess;

ProcessList::ProcessList () {
	Head = new ProcessElement;
	Tail = new ProcessElement;
	Head = NULL;
	Tail = NULL;
}

//ProcessList * PCB = new ProcessList ();
extern ProcessList * PCB;
extern Semaphore * MutexNumProc;

bool ProcessList::IsEmpty() {
	if (Head == NULL)
		return true;
	else
		return false;
}

void ProcessList::Append(ProcessElement * Current) {
	ProcessElement * Old = new ProcessElement;
	if (IsEmpty()) {
		printf("PCB is Empty!\n");
		printf ("Trying to add process %u in PCB\n",Current->PID);
		Head = Current;
		//printf("Head PID is %d\n",(*Head).PID);
		Tail = Current;
		//printf("Head address is %d\n",(int)Head);
		//printf("Tail address is %d\n",(int)Tail);
	} else {
		printf ("Trying to add process %u in PCB\n",Current->PID);
		Old = Tail;
		//printf("Old address is %d\n",(int)Old);
		//(*Tail).Next = Current;
		Tail -> Next = Current;
		Tail = Current;
		Tail -> Previous = Old;
		//printf("Head address is %d\n",(int)Head);
		//printf("Tail address is %d\n",(int)Tail);
		//(*Tail).Previous = Old;
	}
	// delete Old;
}

void ProcessList::Remove(ProcessElement * Current) {
	
	if (IsEmpty()) 
	return;

	// Update Head and Tail ptr
	//the last node in doubly linked list are both the head and tail, not either or.
	if (Current == Head && Current == Tail){
		Head = NULL;
		Tail = NULL;
	}
	else if (Current == Head) {
		
		Head = Current->Next;
		Current->Next->Previous = NULL;
	} 
	else if (Current == Tail) {
		
		Tail = Current->Previous;
		
		Current->Previous->Next = NULL;
		
	} 
	else {
		
		Current->Previous->Next = Current->Next;
		Current->Next->Previous = Current->Previous;
	}
		
}

ProcessElement * ProcessList::Return(int PID) {
	if (IsEmpty()){ 
	
	return NULL;
	}
	
	ProcessElement * node = new ProcessElement;
	node = Head;
	
	do {
		if (node->PID == PID){ 
			
			return node;
			}
		else
			//printf("Current PID is %u\n",node->PID);
			node = node->Next;
			// if (node ==  NULL) printf("OMG\n\n");
			//printf("PID change to%u\n",node->PID);
	} while (node->PID != Tail->PID);
	
	// printf("Search for process %u in PCB.\n", PID);
	
	// check the tail node
	if (node->PID != PID){
		return NULL;
		}
	else{
		return node;
}		
		
	delete node;
	
}

/* 
void CleanupExit() {
	int PID;
	PID = currentThread->GetId();
	ProcessElement * ElementTemp = new ProcessElement;
	// Clean up PCB entry
	if (PCB->Return(PID) != NULL) {
		ElementTemp = PCB->Return(PID);
		PCB->Remove(ElementTemp);
	}
	
	// Free up memory frame.
	// currentThread->space->freeFrames();
	printf("Process %u is trying to deallocate memory space.\n", PID);
	delete currentThread->space;
	
	// Find out and wake up the parent thread.
	printf("Memory space for process %u is deleted\n", PID);
	
	int ParentPID = ElementTemp->ParentPID;;
	if (ParentPID != 0){		
		PCB->Return(ParentPID)->ProcessSemahpore->V();
		printf("Wake up\n");
	} else 
		printf("There is no parent process.\n");
	
	if (--NumProcess > 0) 
		printf("There is still process left.\n");
	else {
		printf("There is no process left.\n");		
		interrupt->Halt();   //no other processes left 		
	}
	printf("There are %u process left.\n",NumProcess);
}
*/
void processCreator (int PID) {
	printf ("Process %u will yield. \n", PID);
	currentThread->Yield();
	currentThread->space->InitRegisters();
	currentThread->space->RestoreState(); // load page table register
						
	printf ("Process %u being executed. \n", PID);
	machine->Run(); // jump to the user progam
	ASSERT(FALSE); // machine->Run never returns;	
}

void AdvancePC() {
	// Advance program counters.
	machine->registers[PrevPCReg] = machine->registers[PCReg];
	machine->registers[PCReg] = machine->registers[NextPCReg];
	machine->registers[NextPCReg] = machine->registers[NextPCReg] + 4;
}


void
ExceptionHandler(ExceptionType which)
{
	int type = machine->ReadRegister(2);

	int arg1 = machine->ReadRegister(4);
	int arg2 = machine->ReadRegister(5);
	int arg3 = machine->ReadRegister(6);
	int Result;
	int i, j;
	char *ch = new char [500];
	
	switch ( which )
	{
		case NoException :
			break;
		case SyscallException :		
			//AdvancePC();
			switch ( type )	// values are specified in syscall.h
			{
			// begin Anderson
				case SC_Exec : 
				{	
					int AddrFile = machine -> ReadRegister(4);
					char * filename = new char [128];
					int ValTemp;
					
					Thread * t = new Thread("SyscallThread");
					t->CreatId();
					int PID = t->GetId();
					printf("PID %u is created!\n", PID);
					printf("Process %u calls exec() system call.\n",currentThread->GetId());
					
					if(!machine->ReadMem(AddrFile,1,&ValTemp)) {
						printf("VA to PA translation fails when open file \n");
						return;
					}

					i=0;
					
					while( ValTemp != 0 )
					{
						filename[i]=(char)ValTemp;
						//printf("%uth letter is %s \n",i,filename[i]);
						//*ValTemp+=1;
						i++;
						AddrFile+=1;
						if(!machine->ReadMem(AddrFile,1,&ValTemp))return;
					}
					filename[i]=(char)0;
				
					OpenFile *executable = fileSystem->Open(filename);
					
					if (executable == NULL) {
						printf("Error, process %u is unable to open file: [%s]\n",currentThread->GetId(), filename);
						return;
					}
					
					printf ("Read file:\"%s\"\n",filename);
					delete filename;
					
					AddrSpace *space;
					space = new AddrSpace(executable);
					delete executable;	

					t->space = space;
					
					// Update PCB
		
					ProcessElement * ProcessTemp = new ProcessElement;
					ProcessTemp->ParentPID = currentThread->GetId();
					ProcessTemp->PID = PID;
					ProcessTemp->CurrentThread = t;
					ProcessTemp->ProcessSemahpore =  new Semaphore("ProcessSemaphore",0);
					ProcessTemp->Next = ProcessTemp;
					ProcessTemp->Previous = ProcessTemp;
					if (space->SpaceFound == true) {
						printf("Memory Allocation Succeeds. \n");
						ProcessTemp->Valid = true;						
						MutexNumProc -> P();
						++NumProcess;
						MutexNumProc -> V();
						t -> Fork(processCreator,PID);
					} else {
						ProcessTemp->Valid = false;
					}
					printf("This is the %uth Process.\n",NumProcess);
					PCB->Append(ProcessTemp);
					machine->WriteRegister(2, PID);
						
					AdvancePC();

					printf("Process %u finishes exec() system call.\n",PID);								
				
					break;
				// End Anderson
				}
				
				case SC_Join :
				{ 	
					printf("Process %u call join() system call.\n",currentThread->GetId());
					
					int ChildPID, ParentPID;
					ChildPID = machine->ReadRegister(4); // 1st parameter
						printf("Prepare to Join ChildProcess %u\n",ChildPID);
						if (PCB->Return(ChildPID) != NULL) { //process to wait for has not finished yet
							if (PCB->Return(ChildPID)->Valid != false) {
								ParentPID = (*(PCB->Return(ChildPID))).ParentPID;
								printf("Will put parent process %u to sleep\n", ParentPID);
								PCB->Return(ParentPID)->ProcessSemahpore->P();
								machine->WriteRegister(2, 0);   // 0 denotes that the process waited on it's child
							} else {
								printf("Process %u fails to join process %u because lacking of memomry.\n",currentThread->GetId(),ChildPID);
							}
						} else {
							machine->WriteRegister(2, -1);	// Child already exits.
							AdvancePC();
						}
						AdvancePC();
				
					break;
				}
				
				case SC_Exit : 
				{
					printf("Process %u call exit() to deallocate memory space.\n", currentThread->GetId());
					delete currentThread->space;
					break;
				}
				
				case SC_Yield : 
				{
					printf("Process %u call yield() system call.\n",currentThread->GetId());
					currentThread->Yield();
					AdvancePC();
					break;
				}
				
				case SC_Halt :
				{
					DEBUG('t', "Shutdown, initiated by user program.\n");
					AdvancePC();
					interrupt->Halt();
					break;
				}
					
				case SC_Read :
				{
					// Anderson: if the size (in reg5) OR OpenFileId (in reg6) is negative 
					if (arg2 <= 0 || arg3 < 0){
						printf("\nRead 0 byte.\n");
					}
					Result = SRead(arg1, arg2, arg3);
					machine->WriteRegister(2, Result);  // Anderson: the result of syscall must be put back to reg2
					DEBUG('t',"Read %d bytes from the open file(OpenFileId is %d)",
					arg2, arg3);
					AdvancePC();
					break;
				}

				case SC_Write :
				{
					for (j = 0; ; j++) {
						if(!machine->ReadMem((arg1+j), 1, &i))
							j=j-1;
						else{
							ch[j] = (char) i;
							if (ch[j] == '\0') 
								break;
						}
					}
					if (j == 0){
						printf("\nWrite 0 byte.\n");
						// SExit(1);
					} else {
						DEBUG('t', "\nWrite %d bytes from %s to the open file(OpenFileId is %d).", arg2, ch, arg3);
						SWrite(ch, j, arg3);
					}
					AdvancePC();
					break;
				}
				
				default :
				//Unprogrammed system calls end up here
				break;
	 
				//AdvancePC();
				
			}
			break;
	
		case ReadOnlyException :
			puts ("ReadOnlyException");
			if (currentThread->getName() == "main")
			ASSERT(FALSE);  //Not the way of handling an exception.
			puts ("test\n");
			
			//SExit(1);
			break;
		case BusErrorException :
			puts ("BusErrorException");
			if (currentThread->getName() == "main")
			ASSERT(FALSE);  //Not the way of handling an exception.
			//SExit(1);
			break;
		case AddressErrorException :
			puts ("AddressErrorException");
			if (currentThread->getName() == "main")
			ASSERT(FALSE);  //Not the way of handling an exception.
			//SExit(1);
			break;
		case OverflowException :
			puts ("OverflowException");
			if (currentThread->getName() == "main")
			ASSERT(FALSE);  //Not the way of handling an exception.
			//SExit(1);
			break;
		case IllegalInstrException :
			puts ("IllegalInstrException");
			if (currentThread->getName() == "main")
			AdvancePC();
			ASSERT(FALSE);  //Not the way of handling an exception.
			//SExit(1);
			break;
		case NumExceptionTypes :
			puts ("NumExceptionTypes");
			if (currentThread->getName() == "main")
			ASSERT(FALSE);  //Not the way of handling an exception.
			//SExit(1);
			break;

		default :
		//      printf("Unexpected user mode exception %d %d\n", which, type);
		//      if (currentThread->getName() == "main")
		//      ASSERT(FALSE);
		//      SExit(1);
		break;
	}
	
	delete [] ch;
}

// end Anderson



static int SRead(int addr, int size, int id)  //input 0  output 1
// Anderson's Question: what is actually returned?
{
	char buffer[size+10];
	int num,Result;

	//read from keyboard, try writing your own code using console class.
	if (id == 0)
	{
		scanf("%s",buffer);

		num=strlen(buffer);
		// Anderson: if the length of input from keyboard is more than the actual length SRead takes.
		if(num>(size+1)) {
			buffer[size+1] = '\0'; // defined the end-of-line
			Result = size+1;
		}
		else {
			buffer[num+1]='\0';
			Result = num + 1;
		}

		for (num=0; num<Result; num++)
		{  machine->WriteMem((addr+num), 1, (int) buffer[num]);
			if (buffer[num] == '\0')
			break; }
		return num;

	}
	//read from a unix file, later you need change to nachos file system.
	else
	{
		for(num=0;num<size;num++){
			Read(id,&buffer[num],1);
			machine->WriteMem((addr+num), 1, (int) buffer[num]);
			if(buffer[num]=='\0') break;
		}
		return num;
	}
}



static void SWrite(char *buffer, int size, int id)
{
	//write to terminal, try writting your own code using console class.
	if (id == 1)
	printf("%s", buffer);
	//write to a unix file, later you need change to nachos file system.
	if (id >= 2)
	WriteFile(id,buffer,size);
}
