// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#ifdef HOST_SPARC
#include <strings.h>
#endif
#include "bitmap.h" // Marcus


// begin Anderson
// Memory allocation algorithm definition
extern char * MemAlgSelArgs;
extern int CheckType (char *);
// begin Marcus

BitMap *MainMemMap = new BitMap(NumPhysPages);
int     StartHere;

// begin Marcus
//-------------------------------------------------------------------------
// FirstFit
//
//
//-------------------------------------------------------------------------
int AddrSpace::FirstFit () 
{
	int 	chunk = 0;				// the size of the available contiguous memory chunk
	int 	index = 0;
	int 	startingAddress = 0;
	bool	SpaceFound = FALSE;
	bool    EndOfMem = FALSE;
	
	while (!SpaceFound && index < NumPhysPages && !EndOfMem)  // stay in loop until either a space is found or i've searched the entire memory
	{
		printf(" FirstFit: index is equal to %d \n",index);
		// if MainMemMap->Test(index) == 0 then start consecutive pages
		// or else you index and keep looking
		while (!MainMemMap->Test(index)&& !EndOfMem)	// go into and stay in this loop while indexed page is empty
		{
			if (chunk == 0)
			{
			    chunk++;
			    startingAddress = index;
			}	
			else
			{
			    chunk++;
			}
			
		  	index++;
			if (index >= NumPhysPages)
			{
				EndOfMem = TRUE;
				index = 0; // to prevent an ASSERT inside Bitmap Test Function
			}
		}
		if (chunk >= numPages) // can the new program fit into the memory chunk
		{		       // possibly more memory to check
			SpaceFound = TRUE;
			printf("I found a chunk of memory starting at frame # %d \n",startingAddress );
			return (startingAddress);
		}
		else			// if the chunk was too small, keep searching
		{
			chunk = 0;
		}
		printf("I'm done finding this memory chunk\n");

		index ++;
	}
	if (chunk >= numPages)      // can the new program fit into the memory chunk
	{			    // No more memory to check
		SpaceFound = TRUE;
	}
	if (SpaceFound)
	{
		printf("I found a chunk of memory starting at frame # %d \n",index);
		return (startingAddress);
	}
	else
	{
		printf("There was not enough contiguous memory available for the new program\n");
		return(-1);
	}
	
}
//--------------------------------------------------------------------------------------------------------------------------------------
// BestFit
//
//
//
//---------------------------------------------------------------------------------------------------------------------------------------
//begin marcus and bradley
int AddrSpace::BestFit () {
	 int 	chunk = 0;	// the size of the available contiguous memory chunk
	int  bestChunk = NumPhysPages;			
	int 	index = 0;
	int  bestStartingAddress =0;
	int 	startingAddress = 0;
	bool SpaceFound = FALSE;
	bool    EndOfMem = FALSE;
	
	while (index < NumPhysPages && !EndOfMem)  // stay in loop until either a space is found or i've searched the entire memory
	{
		printf(" BestFit: index is equal to %d \n",index);
		// if MainMemMap->Test(index) == 0 then start consecutive pages
		// or else you index and keep looking
		while (!MainMemMap->Test(index)&& !EndOfMem)	// go into and stay in this loop while indexed page is empty
		{
			if (chunk == 0)
			{
			    chunk++;
			    startingAddress = index;
			}	
			else
			{
			    chunk++;
			}
			
		  	index++;
			if (index >= NumPhysPages)
			{
				EndOfMem = TRUE;
				index = 0; // to prevent an ASSERT inside Bitmap Test Function
			}
		}
		if (chunk >= numPages) // can the new program fit into the memory chunk
		{		       // possibly more memory to check
			if(chunk <= bestChunk)
			{
				bestChunk=chunk;
				bestStartingAddress = startingAddress;
				printf("I found a smaller chunk of memory starting at frame # %d \n",startingAddress);
				SpaceFound = TRUE;
			}
			
			
		}
		chunk = 0;
		printf("I'm done finding this memory chunk\n");

		index ++;
	}
	if (SpaceFound)
	{
		printf("I found the best chunk of memory starting at frame # %d \n",bestStartingAddress );
		return (bestStartingAddress );
	}
	else
	{
		printf("There was not enough contiguous memory available for the new program\n");
		return(-1);
	}
}
//---------------------------------------------------------------------------------------------------------------------------------------
// WorstFit
//
//
//
//---------------------------------------------------------------------------------------------------------------------------------------
int AddrSpace::WorstFit () {
	int 	chunk = 0;	// the size of the available contiguous memory chunk
	int  worstChunk = 0;			
	int 	index = 0;
	int  worstStartingAddress =0;
	int 	startingAddress = 0;
	bool SpaceFound = FALSE;
	bool    EndOfMem = FALSE;
	
	while (index < NumPhysPages && !EndOfMem)  // stay in loop until either a space is found or i've searched the entire memory
	{
		printf(" worstFit: index is equal to %d \n",index);
		// if MainMemMap->Test(index) == 0 then start consecutive pages
		// or else you index and keep looking
		while (!MainMemMap->Test(index)&& !EndOfMem)	// go into and stay in this loop while indexed page is empty
		{
			if (chunk == 0)
			{
			    chunk++;
			    startingAddress = index;
			}	
			else
			{
			    chunk++;
			}
			
		  	index++;
			if (index >= NumPhysPages)
			{
				EndOfMem = TRUE;
				index = 0; // to prevent an ASSERT inside Bitmap Test Function
			}
		}
		if (chunk >= numPages) // can the new program fit into the memory chunk
		{		       // possibly more memory to check
			if(chunk > worstChunk)
			{
				worstChunk=chunk;
				worstStartingAddress = startingAddress;
				SpaceFound = TRUE;
			}
			printf("I found a bigger chunk of memory starting at frame # %d \n",startingAddress);
		}
		chunk = 0;
		printf("I'm done finding this memory chunk\n");

		index ++;
	}
	if (SpaceFound)
	{
		printf("I found the worst chunk of memory starting at frame # %d \n",worstStartingAddress );
		return (worstStartingAddress );
	}
	else
	{
		printf("There was not enough contiguous memory available for the new program\n");
		return(-1);
	}
}

// end Marcus and bradley


// Pick the desired memory allocation algorithm
// Return the starting physical address of the available memory frame
int AddrSpace::MemoryAllocation (void) {
//end Marcus
	int StartAddr = 0;
	int MemAlg;
	if (MemAlgSelArgs == NULL) {
		StartAddr = FirstFit();
	} else if (CheckType(MemAlgSelArgs)==1) {
		MemAlg = atoi (MemAlgSelArgs);
		if (MemAlg == 1) {
			StartAddr = FirstFit();
		} else if (MemAlg == 2) {
			StartAddr = BestFit();
		} else if (MemAlg == 3) {
			StartAddr = WorstFit();
		} else
			StartAddr = FirstFit();
	} else 	StartAddr = FirstFit();

	return StartAddr;
}

// end Anderson
//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{
    NoffHeader noffH;
    unsigned int i, size;

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;
// begin Marcus
    //ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);

	MainMemMap->Print();
	StartHere = MemoryAllocation();
        
	
// first, set up the translation 
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
	pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
	pageTable[i].physicalPage = i+StartHere;
	MainMemMap->Mark(i);
	pageTable[i].valid = TRUE;
	pageTable[i].use = FALSE;
	pageTable[i].dirty = FALSE;
	pageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
					// a separate page, we could set its 
					// pages to be read-only
    }
	
   printf("Here is the Bitmap after the program memory has been allocated: \n");
   MainMemMap->Print();
 // end Marcus
	
// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
    bzero(machine->mainMemory, size);

// then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
			noffH.code.virtualAddr, noffH.code.size);
        executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),
			noffH.code.size, noffH.code.inFileAddr);
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
			noffH.initData.virtualAddr, noffH.initData.size);
        executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]),
			noffH.initData.size, noffH.initData.inFileAddr);
    }

}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
   delete pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}
