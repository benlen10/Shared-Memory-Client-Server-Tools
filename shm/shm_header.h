// Header file description

typedef struct {
    int pid;
    char birth[25];
    char clientString[10];
    int elapsed_sec;
    double elapsed_msec;
    int valid;
} stats_t;

// The const filename for the shared memory page
const char * SHM_NAME = "lenington";

// The predefined size for each segment within the shared memory page
#define SEG_SIZE 64

// The current page size
int pageSize;

// The max number of clients at any given time
int maxClients;

// Mutex variable
pthread_mutex_t * mutex;
pthread_mutexattr_t mutexAttribute;

// Shared memory pointer
stats_t * shm;

// Function prototypes
void updateSegment(int currentSeg);
void overrideSigHandlers();
void exit_handler(int sig);