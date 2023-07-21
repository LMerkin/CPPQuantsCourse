#include <iostream>			// For std::cout, <<, std::endl
//#include <stdio.h>
#include <cstdio>				// For puts

// The following is UNIX-specific, not available on Windows:
#include <unistd.h>			// For write

//#include <string.h>
#include <cstring>			// For strlen

int main()
{
  // Using the standard C++ output:
  std::cout << "Hello World via C++" << std::endl;

	// Using the LIBC output:
	puts("Hello World via LIBC");

	// Using the LINIX/UNIX system call:
  char   str0[] = "Hello World via SysCall (0)\n";

  constexpr char const* const str1   = "Hello World via SysCall (1)\n";
//const char* str1   = "Hello World via SysCall (1)\n";

	constexpr size_t len   = strlen(str1);
	write(1, str1, len);

	return 0;
}
