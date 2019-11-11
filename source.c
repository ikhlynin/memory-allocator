#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<string.h>
#include<stdint.h>
#include<assert.h>

#define STACK_SIZE 64				//virtual stack size
#define HEAP_SIZE 256				//virtual heap size (Stack * 4)
#define HEADER 4					
#define u8 uint8_t					
#define u16 uint16_t

static u16 busy = 0;

typedef struct virtusal_memory
{
	u8 stack[STACK_SIZE];			//virtual stack
	u8 heap[HEAP_SIZE];				//vierual heap
	char** undef;					//unmaped memory
	struct
	{
		char** not_inizialized;		//not inizialized
		char** read_only;			//read only
		char** inizialized;			//inizialized
	}dt;
}vm;



typedef struct entity				//access to heap 
{
	u8* ptr;
	u16 size;
}et;

et ARR[40];

void LOG()
{
	printf("Our ARR\n");
	for (unsigned i = 0; i < busy; i++)
	{
		printf("Data + HEADER. (%p). Memory of our heap free: [%u]\n", ARR[i].ptr, ARR[i].size);
	}
	printf("Entities in use: [%d]\n", (sizeof(ARR) / sizeof(ARR[0]) - busy));
}
et* newEt(size_t size)				
{
	if (ARR[0].ptr == NULL && ARR[0].size == 0)
	{
		static vm newVm;
		ARR[0].ptr = newVm.heap;
		ARR[0].size = HEAP_SIZE;
		busy++;
		LOG();
	}

	et* ptr = ARR;
	for (unsigned i = 0; i < busy; i++)						//loop to minimize fragmentation
	{
		if (ARR[i].size>=size && ARR[i].size<ptr->size)
		{
			ptr = &ARR[i];
		}
	}
	return ptr;
}

void* malloc(size_t size)
{
	assert(size <= HEAP_SIZE);
	size += HEADER;

	et* free = newEt(size);
	u8* start = free->ptr;
	u8* ptr = start + HEADER;
	*start = size;				
	free->ptr += size;
	free->size -= size;
	assert(free->size >= 0);
	LOG();
	return ptr;
}

void free(void* ptr) 
{
	u8* start = (u8*)ptr - HEADER;

	et* prev = 0, * next = 0;
	for (size_t i = 0; i < busy; i++)
	{
		if (ARR[i].ptr + ARR[i].size == start)
			prev = &ARR[i];
		if (start + *start == ARR[i].ptr)
			next = &ARR[i];
	}
	if (next)
	{
		next->ptr = start;
		next->size += *start;
		*start = next->size;
		busy--;
	}
	if (prev)
	{
		prev->size += *start;
		if (next)
		{
			next->ptr = ARR[busy].ptr; 
			next->size = ARR[busy].size;
		}
		busy--;
	}
	if (!(prev || next))
	{
		ARR[busy].ptr = start;
		ARR[busy].size = *start;
	}
	busy++;
	LOG();
}

void call()
{
	typedef struct testStruct
	{
		int a;
		int b;
	}test_struct;

	test_struct* ts;

	char* test1;

	int* test2;

	ts = malloc(sizeof(ts));
	test1 = malloc(5);
	test2 = malloc(sizeof(int));

	ts->a = 10;
	ts->b = 20;
	strcpy(test1, " char*");
	memcpy(test2, &ts->a, sizeof(int));
	 
	printf("Address: (%p), data: [%d] [%d]\n", ts, ts->a,ts->b);
	printf("Address: (%p), data: [%s]\n", test1, test1);
	printf("Address: (%p), data: [%d] [%d]\n", test2, *test2);

	free(ts);
	free(test1);

	char* newTest = malloc(100 /*set 260 to overflow*/);									
	printf("Address: (%p), data: [%s]\n", newTest, newTest);
}

int main(int argc, char** argv)
{
	call();

	return 0;
}