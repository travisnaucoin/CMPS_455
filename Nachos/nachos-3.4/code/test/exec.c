// test exec()

#include "syscall.h"

int main() {

	int FileId1;
	int i = 1;
	int j = 1;
	int FileId2;
	int FileId3;
	
	FileId1 = Exec("../test/test1"); // PID=2
	FileId2 = Exec("../test/test2"); // PID=3

	Join(FileId1);
	FileId3 = Exec("../test/test3"); // PID=4
	
	Join(FileId2);	// Proc 2 is done;
	Join(FileId3);
	
	
	
	
/*
	while (j < 10)
	{
	   i++;
	   FileId2 = Exec("../test/test1");
	   Join(FileId1);
	}
*/	
	//FileId2 = Exec("../test/halt");
	//Join(FileId2);
	Halt();
	//Halt();
	return;
}
