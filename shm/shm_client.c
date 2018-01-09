#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/mman.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "shm_header.h"

// Represents the current segment with shared memory page
int currentSeg;

// The name of the current client process
char * clientString;

struct timeval start_time;

struct timeval end_time;

int main(int argc, char *argv[])
{
    // Overrride the default sig handlers
    overrideSigHandlers();

	// Verify argument count
	if (argc != 2)
	{
		fprintf(stderr, "Invalid arg count\n");
		exit(1);
	}

	// Verify argument #2 length and save as clientString
	if (strlen(argv[1]) > 10)
	{
		fprintf(stderr, "ClientString cannot exceed 9 chars\n");
		exit(1);
	}

	// Store the second argument as the client name string
	clientString = malloc(9 * sizeof(char));
	strcpy(clientString, argv[1]);

	// Determine the currentSeg page size
	pageSize = getpagesize();

	// Determine the max number of clients
	int maxClients = (pageSize / SEG_SIZE);

	// Attempt to open the shared memory page
	int fd_shm = shm_open(SHM_NAME, O_RDWR, 0660);

	// Exit client if the shared memory page was unable to be opened
	if (fd_shm == -1)
	{
		fprintf(stderr, "Unable to open shared memory page\n");
		exit(1);
	}

	// Attmpt to map the shared memory page
	shm = (stats_t *)mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);

	// Verify that the shared memory page was successfully mapped
	if (shm < 0)
	{
		fprintf(stderr, "Unable to map shared memory page\n");
		exit(1);
	}

	// Set the mutex variable to the first segment of the shared memory page
	mutex = (pthread_mutex_t *)shm;

	// Critical section, rerequest lock
	pthread_mutex_lock(mutex);

	// Iterate through the shared memory block to find the first available segment
	int foundValidSeg = 0;
	for (currentSeg = 1; currentSeg < maxClients; currentSeg++) {

		// Update data for the first valid segment, set the foundValidSeg flag and break out of the loop
		if (shm[currentSeg].valid == 0)
		{
			updateSegment(currentSeg);
			foundValidSeg = 1;
			break;
		}
	}

	// Exit if a valid segment was not found
	if (foundValidSeg != 1) {
		pthread_mutex_unlock(mutex);
		fprintf(stderr, "No valid segment found, client cannot be created");
		exit(1);
	}

	// Exit critical section
	pthread_mutex_unlock(mutex);

    // Loop indefinitely and print the list of active clients every second
	while (1)
	{
		// Calculate and save the client runtime in miliseconds and seconds
		gettimeofday(&end_time, NULL);
        time_t current_secs = (end_time.tv_sec - start_time.tv_sec);
        shm[currentSeg].elapsed_msec = (current_secs / 1000.0);
		shm[currentSeg].elapsed_sec = (int) current_secs;
		

		// Pause for a 1 second before printing
		sleep(1);

		// Print the currentSegly active clients
		printf("Active clients :");
		for (int j = 1; j < (maxClients); j++)
		{
			// Cast the raw shared mem address to a stats_t pointer
			if (shm[j].valid == 1)
			{
				printf(" %i", shm[j].pid);
			}
		}
		printf("\n");
	}

	// Terminate the client process with a sucuess code
	return 0;
}

// Update data for the current segment within the shared memory segment
void updateSegment(int currentSeg) {
	// Set the valid bit and update the pid
	shm[currentSeg].valid = 1;
	shm[currentSeg].pid = getpid();

	// Fetch the current local time
	struct tm *tm_local;
	time_t tm_raw;
	time(&tm_raw);
	tm_local = localtime(&tm_raw);

	// Verify that the time has been fetched properly
	if (tm_local == NULL) {
		perror("localtime failed");
		fprintf(stderr, "Failed to fetch localtime\n");
		exit(1);
	}

	// Copy birth and client strings to the current segment
	strcpy(shm[currentSeg].birth, asctime(tm_local));
	strcpy(shm[currentSeg].clientString, clientString);

	// Null terminate the birth and client strings
	shm[currentSeg].birth[24] = '\0';
	shm[currentSeg].clientString[9] = '\0';

	// Fetch the time and update the foundValidSeg flag
	gettimeofday(&start_time, NULL);
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

// Exit routine for the client process
void exit_handler(int sig)
{
	// Entering critical section, request lock
	pthread_mutex_lock(mutex);

	// Reset the valid bit for the shared memory page
	shm[currentSeg].valid = 0;

    // Exit critical section and terminate gracefully with a sucuess code
	pthread_mutex_unlock(mutex);
	exit(0);
}
