#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

/* To do:
 * - Dequeueing from queue 
 * 	 based on queue, process the ids for running the time quantum
 * 	 calculate waiting time, turnaround time (context switching)
 * 
 */
 
typedef struct processes Process;
typedef struct queue Queue;
unsigned time_quantum;

void delay(int ms);
void read_csv();
void enqueue(Queue *q, Process *p);
void dequeue(Queue *q);
void getCurrentState(Queue *q);
void start_scheduler(Queue *q, unsigned *tq);
int check_current_size(Queue *q);
int compute_burst_time(Process *p, unsigned *tq);
Process *process_pids(unsigned *pid, unsigned *arrival, unsigned *burst);

//--------------------------------------------------------------------------------------------------
/* Structures:
 * 	- Process
 * 	- Queue */
//--------------------------------------------------------------------------------------------------

typedef struct processes {
   unsigned pid;
   unsigned arrival_time;
   unsigned burst_time;
   unsigned waiting_time;
   unsigned turnaround_time;
} Process;

typedef struct queue {
    Process *arr[4];	// Pointer to the array as we don't know what array we're specifying
    int front;
    int rear;
    size_t size;
} Queue;

//--------------------------------------------------------------------------------------------------
/* Queue methods
 * - Check current size of queue
 * - Enqueue
 * - Dequeue
 * - Get current state of queue 
 * - Sort queue in ascending order based off of arrival time*/
//--------------------------------------------------------------------------------------------------

// This function returns amount of items currently in the ready queue
// @returns queue size
int check_current_size(Queue *queue) {
    return queue->size;
}

// This functions adds incoming processes or already processed process into queue
void enqueue(Queue *queue, Process *process) {
	size_t size = sizeof(queue->arr)/sizeof(queue->arr[0]);
	unsigned currentSize = check_current_size(queue);
   
	if (queue->arr[currentSize] == NULL && currentSize < size) {
	   printf("Inserting...\n");
	   queue->arr[currentSize] = process;
	   queue->size++;
	}
}

// This function either: 
// Shift processes in array by 1 and move front process to the end if process is not null OR
// Dequeues process if process' burst time is zero
// Problems: Does not make it null
void dequeue(Queue *queue) {
	queue->front = 0;
	queue->rear = queue->size-1;
	Process *tempProcess = queue->arr[queue->front]; 
	
	// Check if the process in front of the queue is not considered NULL
	// Move front process to the end
	if (tempProcess != NULL) {
		for (size_t i = 0; i < queue->size-1; i++) {
			queue->arr[i] = queue->arr[i+1];
			printf("--\n");
		}
		queue->arr[queue->rear] = tempProcess;
	}
	// Since it's null, dequeue it permanently
	else {
		for (size_t i = 0; i < queue->size-1; i++) {
			queue->arr[i] = queue->arr[i+1]; 
		}
		queue->size--;
		printf("Process has been removed from queue\n");
	}
}

// After the program runs for specified time quantum, check the current state of all processes
void getCurrentState(Queue *q) {
	for (size_t i = 0; i < q->size; i++) {
		printf("Process id#: %d | Arrival Time: %d | Burst Time %d\n", q->arr[i]->pid, 
		q->arr[i]->arrival_time, q->arr[i]->burst_time);
	}
}

void sortQueue(Queue *q) {
	for (size_t i = 0; i < q->size; i++) {
		for (size_t j = 0; j < q->size-1; j++) {
			if (q->arr[j+1]->arrival_time < q->arr[j]->arrival_time) {
				Process *p = q->arr[j];
				q->arr[j] = q->arr[j+1];
				q->arr[j+1] = p;
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Methods needed for starting Round Robin CPU Scheduling
// Process the processes in the ready queue with specified time quantum, etc
// @returns pointer to process with specified id, arrival time, and burst time
//--------------------------------------------------------------------------------------------------
Process *process_pids(unsigned *p_id, unsigned *arrival, unsigned *burst) {
    Process *p;
    p = malloc(sizeof(Process));
    p->pid = *p_id;
    p->arrival_time = *arrival;
    p->burst_time = *burst;
    return p;
}

// Read csv file and check each character for an int. 
// that int is then passed to the process in the queue array.
// @input - file name
void read_csv(Queue *ready_queue) {

    char c;
    FILE *file; // Pointer to file to be read
    unsigned pidFound = 0, arrivalTimeFound = 0, burstTimeFound = 0; // Acts as boolean
    unsigned process_id, arrival_time, burst_time;
    unsigned index = 0;

    file = fopen("pid.csv","r"); // Read the csv file

	// Check if file exists and if it does, continue reading ntil end of file
	if (file) {
		while ((c = getc(file)) != EOF) {
			if (isdigit(c)) {	// Check if character read is a number
				if (c != ',') {
					
					// Read each number and store it after recognizing the ','
					if (pidFound == 0 && (arrivalTimeFound == burstTimeFound)) {
						process_id = c - '0';	// Needed to turn read character in a number
						pidFound = 1;
					}
					else if (arrivalTimeFound == 0 && pidFound == 1) {
						arrival_time = c - '0';
						arrivalTimeFound = 1;
					}
					else if (burstTimeFound == 0 && (arrivalTimeFound == pidFound)) {
						burst_time = c - '0';
						burstTimeFound = 1;
					}
				}
			}
			
			// Begin enqueueing the processes
			else if (c == '\n' && (pidFound == 1 && burstTimeFound == 1 && arrivalTimeFound == 1 )) {
				enqueue(ready_queue, process_pids(&process_id, &arrival_time, &burst_time));
				pidFound = 0, arrivalTimeFound = 0, burstTimeFound = 0;
				index++;
			}
		}
		fclose(file);
	}
}

// Start RR
void start_scheduler(Queue *ready_queue, unsigned *time_quantum) {
	Process *newProcess = ready_queue->arr[ready_queue->front];
	while (newProcess != NULL) {
		if (newProcess->burst_time > 0) {
			printf("Processing...\n");
			newProcess->burst_time = compute_burst_time(newProcess, time_quantum);
			printf("\n");
			
			getCurrentState(ready_queue);
			printf("\n");
			if (newProcess->burst_time == 0) {
				printf("Removing from queue\n\n");
				ready_queue->arr[ready_queue->front] = NULL;
			}
			dequeue(ready_queue);
			printf("Process ran for time quantum of: %d\nCurrent size of the queue is:%d\n"
			"After dequeue, state of processes looks as follows.\n", *time_quantum, check_current_size(ready_queue));
			getCurrentState(ready_queue);
			
			// Set the process equal to whatever is in front now
			newProcess = ready_queue->arr[ready_queue->front];
			printf("\n");
			delay(1000);
		}
	}
}

// This function returns burst time after computing difference between process' burst time and the
// time quantum itself
int compute_burst_time(Process *p, unsigned *time_quantum) {
	
	for (size_t i = *time_quantum; i > 0; i--) {
		if (p->burst_time != 0) {
			p->burst_time -= 1;
			printf("Running on CPU... Burst Time Decremented...\n");
			delay(1000);
		}
	}
	return p->burst_time;
}

//--------------------------------------------------------------------------------------------------
// Debugging tools
//--------------------------------------------------------------------------------------------------
void delay(int milliseconds) {
    long pause;
    clock_t now,then;

    pause = milliseconds*(CLOCKS_PER_SEC/1000);
    now = then = clock();
    while( (now-then) < pause )
        now = clock();
}

int main() {
	
	// Originally initialize Array to contain null values
    Queue ready_queue = {NULL};
    read_csv(&ready_queue);
	printf("Size is: %d\n", check_current_size(&ready_queue));
    sortQueue(&ready_queue);
    
    printf("Enter the time quantum of your choice ");
    scanf("%d", &time_quantum);
    printf("Your time quantum is: %d\n", time_quantum);
    
    printf("Size is: %d\nProcesses loaded into ready queue:\n", check_current_size(&ready_queue));
    
    getCurrentState(&ready_queue);
    
    printf("\n");
	start_scheduler(&ready_queue, &time_quantum);
	printf("Done processing\n");
	exit(0);
}

