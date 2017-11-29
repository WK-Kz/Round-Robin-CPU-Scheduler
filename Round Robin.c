#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

/* To do:
 * - Dequeueing from queue 
 * 	 based on queue, process the ids for running the time quantum
 * 	 calculate waiting time, turnaround time (context switching)
 * - CPU Utilization
 */
 
typedef struct processes Process;
typedef struct queue Queue;
typedef struct cpu CPU;
unsigned time_quantum;

void delay(int ms);
void read_csv();
void enqueue(Queue *q, Process *p);
void dequeue(Queue *q, CPU *processor, time_t *exec_time);
void getCurrentState(Queue *q);
void start_scheduler(CPU *cpu, Queue *q, unsigned *tq);
void calculate_process_info(Process *p, double *exec_time);

double cpu_maximum_utilization(CPU *cpu);
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
   unsigned completion_time;
   unsigned turnaround_time;
   unsigned context_switch_count;
} Process;

typedef struct queue {
    Process *arr[4];	// Pointer to the array as we don't know what array we're specifying
    int front;
    int rear;
    size_t original_size, size;
} Queue;

typedef struct cpu{
	unsigned total_waiting_time;
	unsigned total_completion_time;
	unsigned total_context_switches;
	time_t total_time_running, idle_time;
} CPU;

//--------------------------------------------------------------------------------------------------
// Process Methods
//--------------------------------------------------------------------------------------------------

// Prints out the completion time, turnaround time, and waiting time for the process.
void calculate_process_info(Process *p, double *exec_time) {
	
	// Calculate Turnaround time
	p->turnaround_time = *exec_time - p->arrival_time;
	printf("Execution time: %f and Arrival Time: %d\n", *exec_time, p->arrival_time);
	
	// Calculate Waiting time
	p->waiting_time = p->turnaround_time - p->completion_time;
	
	printf("Process Completion Time: %ds\n", p->completion_time);
	printf("Process Waiting Time: %ds\n", p->waiting_time);
	printf("Process Turnaround time: %.2f\n", p->turnaround_time);
	printf("Removing from queue\n\n");
}

//--------------------------------------------------------------------------------------------------
// CPU Methods
//--------------------------------------------------------------------------------------------------

double cpu_maximum_utilization(CPU *unit) {
	printf("Unit idle time:%f\n", (double) unit->idle_time);
	return ((double) (unit->total_time_running - unit->idle_time) / (double) (unit->total_time_running)) * 100;
}

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
	   //printf("Inserting...\n");
	   queue->arr[currentSize] = process;
	   queue->size++;
	}
	queue->original_size = size;
}

// This function either: 
// Shift processes in array by 1 and move front process to the end if process is not null OR
// Dequeues process if process' burst time is zero
// Problems: Does not make it null
void dequeue(Queue *queue, CPU *processor, time_t *start_exec) {
	queue->front = 0;
	queue->rear = queue->size-1;
	Process *tempProcess = queue->arr[queue->front]; 
	time_t start, end;
	
	start = clock();

	// Check if the process in front of the queue is not considered NULL
	// Move front process to the end
	if (tempProcess != NULL) {
		for (size_t i = 0; i < queue->size-1; i++) {
			queue->arr[i] = queue->arr[i+1];
			//printf("--\n");
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
	
	// Get end clock of when context switch ends...
	end = clock();
	
	printf("Start: %d, End %d\n", *start_exec, end);
	
	float val = (float) (end - *start_exec) / CLOCKS_PER_SEC;
	// Assign the difference divided by the clocks per second to the idle time.
	processor->idle_time += val;
	printf("Start exec is: %d\n", processor->idle_time);

}

// After the program runs for specified time quantum, check the current state of all processes
void getCurrentState(Queue *q) {
	for (size_t i = 0; i < q->size; i++) {
		printf("Process id#: %d | Arrival Time: %d | Burst Time %d\n", q->arr[i]->pid, 
		q->arr[i]->arrival_time, q->arr[i]->burst_time);
	}
}

// Sorts the queue based on the arrival time of the read processes from the csv file
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
    //p->turnaround_time = p->completion_time = p->waiting_time = 0;
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

// Start Round Robin Scheduling Algorithm
void start_scheduler(CPU *processor, Queue *ready_queue, unsigned *time_quantum) {
	Process *newProcess = ready_queue->arr[ready_queue->front];
	time_t context_switch_start;
	while (newProcess != NULL) {
		if (newProcess->burst_time > 0) {
			printf("Processing...\n");
			
			
			//time_t begin_execution = clock();
			newProcess->burst_time = compute_burst_time(newProcess, time_quantum);
			time_t end_execution = clock() / CLOCKS_PER_SEC;
			printf("\n");
			
			getCurrentState(ready_queue);
			printf("\n");
			if (newProcess->burst_time == 0) {
				double execution_time = (double) (end_execution - processor->total_time_running);
				//printf("Execution time variable: %f\n", execution_time);
				calculate_process_info(newProcess, &execution_time);
				
				if (newProcess->context_switch_count != 0)	
					processor->total_context_switches += 1;
					
				processor->total_waiting_time += newProcess->waiting_time;
				//printf("Current total waiting time: %d\n", processor->total_waiting_time);
				ready_queue->arr[ready_queue->front] = NULL;
			} 
			else {
				printf("Context switch...\n");
				newProcess->context_switch_count += 1;
			}
			context_switch_start = clock();
			dequeue(ready_queue, processor, &context_switch_start);
			printf("Process ran for time quantum of: %d\nOriginal size of the queue was: %d\n"
			"Current size of the queue is: %d\nAfter dequeue, state of processes looks as follows.\n", 
			*time_quantum, ready_queue->original_size,check_current_size(ready_queue));
			getCurrentState(ready_queue);
			
			// Set the process equal to whatever is in front now
			newProcess = ready_queue->arr[ready_queue->front];
			printf("Current time in seconds: %d", (double)(clock() - context_switch_start) / CLOCKS_PER_SEC);
			printf("\n");
			delay(1000);
		}
		//processor->total_time_running += clock() / CLOCKS_PER_SEC;
	}
	
}

// This function returns burst time after computing difference between process' burst time and the
// time quantum itself
int compute_burst_time(Process *p, unsigned *time_quantum) {
	for (size_t i = *time_quantum; i > 0; i--) {
		if (p->burst_time != 0) {
			p->burst_time -= 1;
			p->completion_time += 1;
			printf("Running on CPU... Burst Time Decremented...\n");
			delay(1000);
		}
	}
	
	return p->burst_time;
}

void analysis(CPU *unit, Queue *ready_queue) {
	// Get end duration of the clock and subtract the end from the beginning to get the total run time
	time_t end = clock() / CLOCKS_PER_SEC;
	unit->total_time_running = end - unit->total_time_running;
	double average_waiting_time = unit->total_waiting_time / ready_queue->original_size;	
	
	printf("This took %.2lfs\n", (double)unit->total_time_running);
	printf("CPU Utilization: %.2fs\n", cpu_maximum_utilization(unit));
	printf("Total Waiting Time: %.2f\n", average_waiting_time);
	printf("Total throughput: %.2f\n", ready_queue->original_size / (double) (unit->total_time_running));	
	printf("Total amount of context switches: %d\n", unit->total_context_switches);
}

//--------------------------------------------------------------------------------------------------
// Simulations Tools
// * Delay - Allows a small delay depending on input
//--------------------------------------------------------------------------------------------------

// @input - Amount of milliseconds that it should be delayed for.
void delay(int ms) {
    long halt;
    clock_t current, previous;
    
    halt = ms*(CLOCKS_PER_SEC/1000);
    current = previous = clock();
    
    while((current-previous) < halt )
        current = clock();
}

int main() {
	// Originally initialize Array to contain null values
    Queue ready_queue = {NULL};
    CPU processing_unit = {0, 0, 0, 0 ,0};
    read_csv(&ready_queue);
	//printf("Size is: %d\n", check_current_size(&ready_queue));
    sortQueue(&ready_queue);
    
    printf("Enter the time quantum of your choice: ");
    scanf("%d", &time_quantum);
    printf("Your time quantum is: %d\n", time_quantum);
    printf("Size is: %d\nProcesses loaded into ready queue:\n", check_current_size(&ready_queue));
    
    processing_unit.total_time_running = clock() / CLOCKS_PER_SEC; 
    getCurrentState(&ready_queue);
    
    printf("\n");
	start_scheduler(&processing_unit, &ready_queue, &time_quantum);
	printf("Done processing\n\n");
	
	analysis(&processing_unit, &ready_queue);
	return 0;
	
}
