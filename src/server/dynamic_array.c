#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "dynamic_array.h"

dynamic_array* init_dynamic_array(int size_of_data){
	dynamic_array* arr;
	if( (arr = malloc(sizeof(dynamic_array))) == NULL ){
		perror("malloc");
		exit(1);
	}
	arr->size = 0;
	arr->size_of_data = size_of_data;
	if( (arr->data = malloc(sizeof(char*)*DYNAMIC_ARRAY_SIZE_INC)) == NULL ){
		perror("malloc");
		exit(1);
	}
	pthread_mutex_init(&arr->locking_mutex, NULL);
	return arr;
}

void dynamic_array_add(dynamic_array* arr, void* data){
	if(arr->size % 10 == 9){
		if( (arr->data = realloc(arr->data, sizeof(char*)*(arr->size+DYNAMIC_ARRAY_SIZE_INC))) == NULL){
			perror("realloc");
			exit(1);
		}
	}
	if( (arr->data[arr->size] = malloc(arr->size_of_data)) == NULL){
		perror("malloc");
		exit(1);
	}
	memcpy(arr->data[arr->size++], data, arr->size_of_data);
}

void dynamic_array_delete_at(dynamic_array* arr, int n){
	printf("ololo %d\n", arr->size);
	memmove(arr->data[n],
		arr->data[--(arr->size)], arr->size_of_data);
	printf("ololo1 %d\n", arr->size);
	if(arr->size % 10 == 0 && arr->size){
		if( (arr->data = realloc(arr->data, sizeof(char*)*(arr->size-DYNAMIC_ARRAY_SIZE_INC))) == NULL){
			perror("realloc");
			exit(1);
		}
	}
}
