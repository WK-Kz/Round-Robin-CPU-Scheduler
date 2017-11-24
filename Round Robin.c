#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

/* To do:
 * - Shift main() to top and use prototyping effectively
 * 
 * - Dequeueing from queue 
 * 		based on queue, process the ids for running the time quantum
 * 		calculate waiting time, turnaround time (context switching)
 * 
 * - Sort based on arrival time
 * - After completing burst time, remove from queue
 * 
 */
 
typedef struct processes Process;
typedef struct queue Queue;
unsigned time_quantum;

void delay(int ms);
void read_csv();
void enqueue(Queue *q, Process *p);
void dequeue(Queue *q);
int check_current_size(Queue *q);
void start_scheduler(Queue *q, unsigned *tq);
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
    int size;
} Queue;

//--------------------------------------------------------------------------------------------------
/* Queue methods
 * - Check current size of queue
 * - Enqueue
 * - Dequeue
 * - Get current state of queue */
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

   //printf("Pid: %d, Arr: %d Burs %d\n", process->pid, process->arrival_time, process->burst_time);
}

// This functions either: 
// Shift processes in array by 1 and move front process to the end if process is not null OR
// Dequeues process if process' burst time is zero
void dequeue(Queue *queue) {
	queue->front = 0;
	queue->rear = 3;
	Process *tempProcess = queue->arr[queue->front]; 
	
	// Check if the process in front of the queue is not considered NULL
	// Move front process to the end
	if (tempProcess != NULL) {
		for (size_t i = 1; i < (size_t) queue->size; i++) {
			queue->arr[i-1] = queue->arr[i];
		}
		queue->arr[queue->rear] = tempProcess;
	}
	// Since it's null, dequeue it permanently
	else {
		for (size_t i = 0; i < (size_t) queue->size-1; i++) {
			queue->arr[i] = queue->arr[i+1]; 
		}
		queue->size--;
		printf("Process has been removed from queue");
	}
}

// After the program runs for specified
// time quantum, check the current state of all processes
void getCurrentState(Queue *queue) {
	for (size_t i = 0; i < (size_t) queue->size; i++) {
		printf("Process id#: %d | Arrival Time: %d | Burst Time %d\n", queue->arr[i]->pid, 
		queue->arr[i]->arrival_time, queue->arr[i]->burst_time);
		delay(1000);
	}
}

//--------------------------------------------------------------------------------------------------
// Methods needed for starting Round Robin CPU Scheduling
// Process the processes in the ready queue with specified time quantum, etc
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

	if (file) {
		while ((c = getc(file)) != EOF) {
			if (isdigit(c)) {
				if (c != ',') {
					if (pidFound == 0 && (arrivalTimeFound == burstTimeFound)) {
						process_id = c - '0';
						//printf("Process id: %d", process_id);			// Debugging
						pidFound = 1;
					}
					else if (arrivalTimeFound == 0 && pidFound == 1) {
						arrival_time = c - '0';
						//printf(" Arrival: %d", arrival_time);		// Debugging
						arrivalTimeFound = 1;
					}
					else if (burstTimeFound == 0 && (arrivalTimeFound == pidFound)) {
						burst_time = c - '0';
						//printf(" Burst Time: %d", burst_time);			// Debugging
						burstTimeFound = 1;
						//printf("\n");
					}
				}
			}
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
			newProcess->burst_time = compute_burst_time(newProcess, time_quantum);
			if (newProcess->burst_time == 0) {
				newProcess = NULL;
			}
			printf("Process ran for time quantum of: %d\n", *time_quantum);
			printf("Current size of queue is: %d\n",check_current_size(ready_queue));
			getCurrentState(ready_queue);
			dequeue(ready_queue);
			
			// Set the process equal to whatever is in front now
			newProcess = ready_queue->arr[ready_queue->front];
			printf("\n");
			delay(1000);
		}
		/*
		else {
			newProcess = NULL;
			ready_queue->size--;
		}*/
	}
}

// This function returns burst time after computing difference between process' burst time and the
// time quantum itself
int compute_burst_time(Process *p, unsigned *time_quantum) {
	
	for (size_t i = *time_quantum; i > 0; i--) {
		if (p->burst_time != 0) {
			p->burst_time -= 1;
			printf("Burst time decremented\n");
			delay(1000);
		}
	}
	return p->burst_time;
	
}

// Other crap
void delay(int milliseconds)
{
    long pause;
    clock_t now,then;

    pause = milliseconds*(CLOCKS_PER_SEC/1000);
    now = then = clock();
    while( (now-then) < pause )
        now = clock();
}

// Start
int main() {
	
	// Originally initialize Array to contain null values
    Queue qtest = {NULL};
    
    read_csv(&qtest);
    printf("Enter the time quantum of your choice ");
    scanf("%d", &time_quantum);
    printf("Your time quantum is: %d\n", time_quantum);

    printf("Size is: %d\n", check_current_size(&qtest));
    printf("Processes loaded into ready queue:\n");
    
    // It works
    for (size_t i = 0; i < check_current_size(&qtest); i++) {
		printf("Process id#: %d | Arrival Time: %d | Burst Time %d\n", qtest.arr[i]-> pid, 
		qtest.arr[i] -> arrival_time, qtest.arr[i]->burst_time);
    }
    
    printf("\n");
    
	start_scheduler(&qtest, &time_quantum);
	exit(0);
}

