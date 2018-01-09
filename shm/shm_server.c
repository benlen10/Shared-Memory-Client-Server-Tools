#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "shm_header.h"

int main(int argc, char *argv[])
{
	// Override the default sig handlers
	overrideSigHandlers();

	// Create and initalize a new shared memory page
	int fd_shm = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0660);

	// Exit server if the shared memory page was unable to be created
	if (fd_shm < 0) {
		exit(1);
	}

	// Fetch the current page size
	pageSize = getpagesize();

	// Determine the max number of clients
	maxClients = (pageSize / SEG_SIZE);

	// Atempt to truncate the shared memory to fit within a single page
	if (ftruncate(fd_shm, pageSize) < 0) {
		exit(1);
	}

	// Attmpt to map the shared memory page
	stats_t * shm = (stats_t *)mmap(0, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
	close(fd_shm);
	// Verify that the shared memory page was successfully mapped
	if (shm == MAP_FAILED) {
		exit(1);
	}

	// Assign the mutex var to the first shared m
	mutex = (pthread_mutex_t *)shm;

	// If mapping was sucuessful, initialize mutex variables 
	pthread_mutexattr_init(&mutexAttribute);
	pthread_mutexattr_setpshared(&mutexAttribute, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(mutex, &mutexAttribute);

	//Intialize the counter 
	int iterationCount = 0;

	// Indefinite loop to display updated statistics for the shared memory page 
	// Refresh rate = 1 second 
	while (1)
	{
		// Iterate through the client list and display statistics for valid clients
		iterationCount++;
		for (int currentClient = 0; currentClient < maxClients; currentClient++) {
			// Check if the segment is valid and print stats
			if (shm[currentClient].valid == 1) {
				printf("%i, pid : %i, birth : %s, elapsed : %i s %f ms, %s\n", iterationCount, shm[currentClient].pid, shm[currentClient].birth, shm[currentClient].elapsed_sec, shm[currentClient].elapsed_msec, shm[currentClient].clientString);
			}
		}
		sleep(1);
	}
	return 0;
}

// Override default system sig handlers
void overrideSigHandlers(){
    
// Override the SIGINT signal handler
	if (signal(SIGINT, exit_handler) == SIG_ERR) {
		printf("Failed to override SIGINT signal handler\n");
		exit(1);
	}

	// Override the SIGTERM signal handler
	if (signal(SIGTERM, exit_handler) == SIG_ERR) {
		printf("Failed to override SIGTERM signal handler\n");
		exit(1);
	}
}

// Exit routine for the server process
void exit_handler(int sig)
{
	// Attempt to unnmap the shared memory page
	munmap(NULL, (size_t)getpagesize());

	// Attempt to unlink the shared memory page
	shm_unlink(SHM_NAME);

	// Exit gracefully with a sucuess code
	exit(0);
}
