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


// begin Marcus
//-------------------------------------------------------------------------
// FirstFit
//
//
//-------------------------------------------------------------------------
void AddrSpace::FirstFit () 
{
	unsigned int 	chunk = 0;				// the size of the available contiguous memory chunk
	int 	index = 0;
	int 	startingAddress = 0;
	bool    EndOfMem = FALSE;
	SpaceFound = false;
	printf("First Fit is Called;\n");
	while (!SpaceFound && index < NumPhysPages)  // stay in loop until either a space is found or i've searched the entire memory
	{		
		// if MainMemMap->Test(index) == 0 then start consecutive pages
		// or else you index and keep looking
		while (!MainMemMap->Test(index)&& !EndOfMem)	// go into and stay in this loop while indexed page is empty
		{
			//printf(" memory frame %d is available\n",index);
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
		if (!EndOfMem)
			printf(" memory frame %d is occupied! \n",index);
			
		if (chunk >= numPages) // can the new program fit into the memory chunk
		{		       // possibly more memory to check
			SpaceFound = TRUE;
			printf("I found a chunk of memory starting at frame # %d for %u Frames \n",startingAddress, chunk );
			StartHere = startingAddress;
			return;
		}
		else			// if the chunk was too small, keep searching
		{
			chunk = 0;
		}
		// printf("I'm done finding this memory chunk\n");

		index ++;
	}
	if (chunk >= numPages)      // can the new program fit into the memory chunk
	{			    // No more memory to check
		SpaceFound = TRUE;
	}
	
	if (SpaceFound)
	{
		printf("I found a chunk of memory starting at frame # %d for %u Frames \n",startingAddress, chunk );
		StartHere = startingAddress;			
	}
	else
	{
		printf("There was not enough contiguous memory available for the new program\n");
	}
	return;
//	return (SpaceFound);
}
void AddrSpace::BestFit () {

}
void AddrSpace::WorstFit () {

}
/*
ExceptionType
AddrSpace::Translate(int virtAddr, int* physAddr, int size, bool writing)
{
    int i;
    unsigned int vpn, offset;
    TranslationEntry *entry;
    unsigned int pageFrame;

    DEBUG('a', "\tTranslate 0x%x, %s: ", virtAddr, writing ? "write" : "read");

// check for alignment errors
    if (((size == 4) && (virtAddr & 0x3)) || ((size == 2) && (virtAddr & 0x1))){
	DEBUG('a', "alignment problem at %d, size %d!\n", virtAddr, size);
	return AddressErrorException;
    }
    
    // we must have a page table, but not both!
    //ASSERT(pageTable == NULL);	
    //ASSERT(pageTable != NULL);	

// calculate the virtual page number, and offset within the page,
// from the virtual address
    vpn = (unsigned) virtAddr / PageSize;
    offset = (unsigned) virtAddr % PageSize;
    

	if (vpn >= numPages) {
	    DEBUG('a', "virtual page # %d too large for page table size %d!\n", 
			virtAddr, numPages);
	    return AddressErrorException;
	}
	else if (!pageTable[vpn].valid) {
	    DEBUG('a', "virtual page # %d too large for page table size %d!\n", 
			virtAddr, numPages);
	    return PageFaultException;
	}
	entry = &pageTable[vpn];
    
    if (entry->readOnly && writing) {	// trying to write to a read-only page
	DEBUG('a', "%d mapped read-only at %d in TLB!\n", virtAddr, i);
	return ReadOnlyException;
    }
    pageFrame = entry->physicalPage;

    // if the pageFrame is too big, there is something really wrong! 
    // An invalid translation was loaded into the page table. 
    if (pageFrame >= NumPhysPages) { 
	DEBUG('a', "*** frame %d > %d!\n", pageFrame, NumPhysPages);
	return BusErrorException;
    }
    entry->use = TRUE;		// set the use, dirty bits
    if (writing)
	entry->dirty = TRUE;
    *physAddr = pageFrame * PageSize + offset;
    ASSERT((*physAddr >= 0) && ((*physAddr + size) <= MemorySize));
    DEBUG('a', "phys addr = 0x%x\n", *physAddr);
    return NoException;
}
*/

void AddrSpace::LoadSegment(OpenFile *executable, int addr, int size, int inFileAddr) {
    
	int physAddr, loops;
	
	//translate for block not starting at the beginning of a page
	if (addr % PageSize) {
		Translate(addr,&physAddr,size % PageSize);
		DEBUG('z',"1.Translation from Virtual Addr 0x%x to Physical Addr = 0x%x for block of size %d\n",addr, physAddr,size % PageSize);
		executable->ReadAt(& (machine->mainMemory[physAddr]), size % PageSize, inFileAddr);
		size -=  (size % PageSize);
		addr += (size % PageSize);
		inFileAddr += (size % PageSize);
	}
	loops = size/PageSize;
	//translate for blocks starting at the beginning of a page and allocating the whole page
	for (int i=0; i<loops ; i++){
		Translate(addr,&physAddr,PageSize);
		DEBUG('z',"2.Translation from Virtual Addr 0x%x to Physical Addr = 0x%x for block of size %d\n",addr, physAddr, PageSize);
		executable->ReadAt(& (machine->mainMemory[physAddr]), PageSize, inFileAddr);
		size -= PageSize;
		addr += PageSize;
		inFileAddr += PageSize;
	}
	//translate for blocks starting at the beginning of a page but allocating only a part of it.
	if (size % PageSize) {
		Translate(addr,&physAddr,size % PageSize);
		DEBUG('z',"3.Translation from Virtual Addr 0x%x to Physical Addr = 0x%x for block of size %d\n",addr, physAddr,size % PageSize);
		executable->ReadAt(& (machine->mainMemory[physAddr]), size % PageSize, inFileAddr);
	}		
}


//taken from translate.cc  (the same function from translate.cc could be used
// but a problem came up when loading initial file with "StartProcess").
// it is either this or setting the pagetable pointer for the machine Object inside AddrSpace constructor
ExceptionType AddrSpace::Translate(int virtAddr, int * physAddr, int size) {
	unsigned int vpn, offset;
    TranslationEntry *entry;
    unsigned int pageFrame;
	
	if(virtAddr < 0) {
    	return AddressErrorException;
    }
	vpn = (unsigned) virtAddr / PageSize;
  	offset = (unsigned) virtAddr % PageSize;
	if (vpn >= numPages) {
	    DEBUG('z', "virtual page # %d too large for page table size %d!\n", virtAddr, numPages);
		return AddressErrorException;
	}
	else if (!pageTable[vpn].valid) {
	    DEBUG('z', "virtual page # %d too large for page table size %d!\n", virtAddr, numPages);
	    return PageFaultException;
	}
	entry = &pageTable[vpn];
	pageFrame = entry->physicalPage;

    // if the pageFrame is too big, there is something really wrong! 
    // An invalid translation was loaded into the page table or TLB. 
    if (pageFrame >= NumPhysPages) { 
		DEBUG('z', "*** frame %d > %d!\n", pageFrame, NumPhysPages);
		return BusErrorException;
    }
    entry->use = TRUE;		// set the use bits
    
    *physAddr = pageFrame * PageSize + offset;
    ASSERT((*physAddr >= 0) && ((*physAddr + size) <= MemorySize));
    DEBUG('z', "phys addr = 0x%x\n", *physAddr);
    return NoException;
}

/*
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
	
	while (index < NumPhysPages)  // stay in loop until either a space is found or i've searched the entire memory
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
		return(0);
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
	
	while (index < NumPhysPages)  // stay in loop until either a space is found or i've searched the entire memory
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
		return(0);
	}
}

// end Marcus and bradley
*/

// Pick the desired memory allocation algorithm
// Return the starting physical address of the available memory frame
void AddrSpace::MemoryAllocation (void) {
//end Marcus
//	int StartAddr = 0;
	int MemAlg;
	if (MemAlgSelArgs == NULL) {
		FirstFit();
	} else if (CheckType(MemAlgSelArgs)==1) {
		MemAlg = atoi (MemAlgSelArgs);
		if (MemAlg == 1) {
			FirstFit();
		} else if (MemAlg == 2) {
			BestFit();
		} else if (MemAlg == 3) {
			WorstFit();
		} else
			FirstFit();
	} else 	FirstFit();

//	return StartAddr;
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

	ExceptionType exception; 

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

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);

	MainMemMap->Print();
	MemoryAllocation();
        
	if (SpaceFound != false)  {
	// first, set up the translation 
		pageTable = new TranslationEntry[numPages];
		for (i = 0; i < numPages; i++) {
			pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
			pageTable[i].physicalPage = i+StartHere;
			MainMemMap->Mark(i+StartHere);
			pageTable[i].valid = TRUE;
			pageTable[i].use = FALSE;
			pageTable[i].dirty = FALSE;
			pageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
							// a separate page, we could set its 
							// pages to be read-only
		}
		
	   printf("Here is the Bitmap after the program memory has been allocated: \n");
	   MainMemMap->Print();
	// zero out the entire address space, to zero the unitialized data segment 
	// and the stack segment
	   // bzero(machine->mainMemory, size);

		for (i = 0; i < numPages; i++) {
			DEBUG('z', "Zeroing frame number %d starting at 0x%x\n", pageTable[i].physicalPage,
						pageTable[i].physicalPage*PageSize);
			bzero( &(machine->mainMemory[pageTable[i].physicalPage*PageSize]), PageSize);
		}		
			
		//memset(machine->mainMemory+StartHere,0,size);  // Marcus
		
		// then, copy in the code and data segments into memory
		if (noffH.code.size > 0) {
			DEBUG('z', "Initializing code segment, at 0x%x, size %d\n", 
				noffH.code.virtualAddr, noffH.code.size);
			/*executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),
				noffH.code.size, noffH.code.inFileAddr);*/
			LoadSegment(executable,noffH.code.virtualAddr, noffH.code.size, noffH.code.inFileAddr);
		}
		if (noffH.initData.size > 0) {
			DEBUG('z', "Initializing data segment, at 0x%x, size %d\n", 
				noffH.initData.virtualAddr, noffH.initData.size);
			/*executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]),
				noffH.initData.size, noffH.initData.inFileAddr);*/
			LoadSegment(executable,noffH.initData.virtualAddr, noffH.initData.size, noffH.initData.inFileAddr);
		}
	
	
		
		// begin Anderson
		/*
		exception = Translate(noffH.code.virtualAddr, &physicalAddress, size, FALSE);
		if (exception != NoException) { // if translation fail, raise exception.
			machine->RaiseException(exception, noffH.code.virtualAddr);
		} 
		else {
			// if address translation success without exception, proceed.
			printf("physical address is %u\n",physicalAddress);
			if (noffH.code.size > 0) {
				DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
					noffH.code.virtualAddr, noffH.code.size);
				executable->ReadAt(&(machine->mainMemory[physicalAddress]),
					noffH.code.size, noffH.code.inFileAddr);
				printf ("%u\n",&(machine->mainMemory[physicalAddress]));
			}
			if (noffH.initData.size > 0) {
				DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
					noffH.initData.virtualAddr, noffH.initData.size);
				executable->ReadAt(&(machine->mainMemory[physicalAddress]),
					noffH.initData.size, noffH.initData.inFileAddr);
			}				
		// end Anderson
		}
		
		*/
	}
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
   delete pageTable;
  // Anderson: what about bit map? 
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
{
//printf("HAHAHAHA\n");
}

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
