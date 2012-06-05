#ifndef H_SERVER_MEMORY_DUMP_GUARD
#define H_SERVER_MEMORY_DUMP_GUARD

#include <stdio.h>

void read_memory_dump();
void read_logins_dump(FILE *logins);
void read_games_dump(FILE *games);

void create_memory_dump();
void create_logins_dump(FILE *logins);
void create_games_dump(FILE *games);

void* DumpsThread(void* arg);
void create_dump_thread(int dump_creation_frequency);

#endif

