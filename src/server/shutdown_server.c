#include <stdlib.h>
#include <stdio.h>

#include "memory_dump.h"

/* There will be all the memory cleanup functions in future */

void shutdown(){
	create_memory_dump();
	exit(1);
}

