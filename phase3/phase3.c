#include <process.h>  // Windows multithreading functions
#include <stdio.h>    // needed to print to the screen
#include <stdlib.h>   // ascii to integer conversion
#include <windows.h>  // for Sleep function
#include <time.h>     // gets the time
#include <synchapi.h> // Used for WaitForSingleObject to detect when threads are done

#define MAXTHREADS 10 // Hard-coded upper limit on the number of threads 

// When creating a new thread, only one "thing" can be passed to it. However, if you make a struct with multiple fields,
// you can send one thing that has as many different fields as you need. 
typedef struct s_thread_t{
	int tid;					// numerical ID of the thread, for easier human reference
	int r;						// randomly generated integer, just so there is some data to pass to the thread for it to use
	int handle;					// OS assigned thread handle - address used to keep track of thread, we'll use this in WaitForSingleObject
	struct s_thread_t *next;	// Used for linked list, points to the next item
} thread_t;

thread_t *head = NULL;			// Points to the beginning of the list of threads
thread_t *newThread;			// Needed to add a new thread_t to the list
thread_t *oldThread;			// Needed to delete an old thread_t from the list

// function prototypes
void printThread(void *myID);

int main(int argc, char *argv[])
{	
	int i;
	int nThreads;
	
	if (argc == 2)  
	{
		if (atoi(argv[1]) <=0)
		{
			printf("main: The command line argument must be a postive integer.\n");
			return;
		}
	}
	else
	{
		printf("main: This program requires one integer as a command line argument.\n");
		return;
	}
	
	nThreads = atoi(argv[1]) > MAXTHREADS ? MAXTHREADS : atoi(argv[1]);  // "condition ? value if true : value if false"
 	  
	printf("main: Using %d threads.\n", nThreads);

	srand((int)time(NULL)); // cheap way of generating a different random seed each time

	for (i = 0; i < nThreads; i++)
	{
		// Allocate a chunk of memory for a thread_t object and append it to the front of the linked list.
		if ((newThread = malloc(sizeof(thread_t))) == NULL)
		{
			return; // memory allocation failed, scream and die}
		} 
		else
		{ // add the new thread to the beginning of the list
			newThread->next = head; // add the new thread to the beginning of the list
			head = newThread;       // ...THEN move the pointer to the list to the newly added thread_t
			newThread->tid = i;
			newThread->r = rand();
			printf("main: Launching thread %d...\n", newThread->tid);
			newThread->handle = _beginthread(printThread, 0, newThread);   // name of function, stack size (0 for default), pointer to passed parameter
		}
	}
	
	// All threads have been created, so now we wait in the main program until they are all done.
	// Although there is a WaitForMultipleObjects that could be used without a loop, the WaitForSingleObject function
	// will be more useful in the final project.

	while (head != NULL) // head will equal NULL when all the threads have been deallocated.
	{
		WaitForSingleObject((HANDLE)head->handle, INFINITE);  // Using the OS's name for the thread, wait for the thread to end
		printf("main: Detected that thread %d has finished.\n", head->tid);
		oldThread = head;  // get ready to deallocate this thread_t since its associated thread is finished
		head = head->next; // move head to next thread_t in the list
		free(oldThread);
	}

	printf("main: All threads have finished.\n");
}

void printThread(void *pMyID) // pointer to void let's you pass one pointer to absolutely anything...however, since the type is "void",
							  // the compiler has no way to treat it as anything more useful, like a struct or array, so we will assign the 
							  // pointer to void to a pointer to an object of the correct type, and now the compiler can access the 
							  // fields. This is a little cleaner than type-casting the void pointer EVERY time it is used.
{               
	thread_t* MyID = pMyID;   // somewhat clean way to convert the void input to a typed input
	printf("printThread: Thread %d has started...\n", MyID->tid);
	Sleep( (MyID->r % 100) * 50);
	printf("printThread: Thread %d has finished...\n", MyID->tid);
}