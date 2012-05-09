#ifndef H_SERVER_DYNAMIC_ARRAY_GUARD
#define H_SERVER_DYNAMIC_ARRAY_GUARD

#define DYNAMIC_ARRAY_SIZE_INC 10

typedef struct dynamic_array{
	char **data;
	int size;
	int size_of_data;
	pthread_mutex_t locking_mutex;
} dynamic_array;

dynamic_array* init_dynamic_array(int size_of_data);
void dynamic_array_add(dynamic_array* arr, void* data);
void dynamic_array_delete_at(dynamic_array* arr, int n);

#endif
