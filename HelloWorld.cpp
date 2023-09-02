#include <iostream>			// For std::cout, <<, std::endl
#include <cstdio>				// For puts

// The following is UNIX-specific, not available on Windows:
#include <unistd.h>			// For write

int main()
{
  // Using the standard C++ output:
  std::cout << "Hello World via C++" << std::endl;

	// Using the LIBC output:
	puts("Hello World via LibC");

	// Using the LINIX/UNIX system call:
	constexpr char str0[] = "Hello World via SysCall\n";
	write(1, str0, sizeof(str0)-1);

	return 0;
}
