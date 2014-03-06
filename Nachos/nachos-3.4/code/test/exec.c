// test exec()

#include "syscall.h"

int main() {

	int FileId2;
	FileId2 = Exec("../test/halt");
	Join(FileId2);

}
