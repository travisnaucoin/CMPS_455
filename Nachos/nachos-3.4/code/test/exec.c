// test exec()

#include "syscall.h"

int main() {

	int FileId1;
	FileId2 = Exec("../test/test1");
	Join(FileId1);

	int FileId2;
	FileId2 = Exec("../test/halt");
	Join(FileId2);
	//Halt();
	return;
}
