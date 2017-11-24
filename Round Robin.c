#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* To do:
 * - Shift main() to top and use prototyping effectively
 * 
 * - Dequeueing from queue 
 * 		based on queue, process the ids for running the time quantum
 * 		calculate waiting time, turnaround time (context switching)
 * -  
 * 
 */
 
typedef struct processes Process;
typedef struct queue Queue;
unsigned time_quantum;

void read_csv();
void enqueue(Queue *queue, Process *processToBeAdded);
void dequeue(Queue *queue);
int check_current_size(Queue *queue);
Process *process_pids(unsigned *pids, unsigned *arrival, unsigned *burst);

/* Structures:
 * 	- Process
 * 	- Queue
 */
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
// Queue Methods

// Not necessary
Process *peek(Queue *queue) {
    queue->front = 0;
    return queue->arr[queue->front];
}

// @output: Returns amount of items currently in the ready_queue
int check_current_size(Queue *queue) {
    return queue->size;
}

// Adds incoming processes or already processed process into queue
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

// Shift processes in array by 1 and move front process to the end
void dequeue(Queue *queue) {
	queue->front = 0;
	queue->rear = 3;
	Process (*tempProcess) = queue->arr[queue->front]; 
	
	for (size_t i = 1; i < (size_t) queue->size; i++) {
		queue->arr[i-1] = queue->arr[i];
	}
	
	queue->arr[queue->rear] = tempProcess;
}

// After the program runs for specified
// time quantum, check the current state of all processes
void getCurrentState(Queue *queue) {
	for (size_t i = 0; i < (size_t) queue->size; i++) {
		printf("Process id#: %d | Arrival Time: %d | Burst Time %d\n", queue->arr[i]->pid, 
		queue->arr[i]->arrival_time, queue->arr[i]->burst_time);
	}
}

//--------------------------------------------------------------------------------------------------
// Methods needed for starting Round Robin CPU Scheduling

// Process the processes in the ready queue with specified time quantum, etc
Process *process_pids(unsigned *p_id, unsigned *arrival, unsigned *burst) {
    Process *p;
    p = malloc(sizeof(Process));
    p->pid = *p_id;
    p->arrival_time = *arrival;
    p->burst_time = *burst;
    //printf("%d %d %d\n", p->pid, p->arrival_time, p->burst_time);
    return p;
}

// Read csv file and check each character for an int. 
// that int is then passed to the process in the queue array.
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
				// This works to add process to queue
				//ready_queue->arr[index] = process_pids(&process_id, &arrival_time, &burst_time); 
				enqueue(ready_queue, process_pids(&process_id, &arrival_time, &burst_time));
				//ready_queue->size++;
				//printf("Process %d finished processing...\n", index);
				pidFound = 0, arrivalTimeFound = 0, burstTimeFound = 0;
				index++;
			}
		}
		fclose(file);
	}
}

void start_scheduler(Queue *ready_queue, unsigned *time_quantum) { 
	while (ready_queue->arr != NULL) {
		Process *newProcess = ready_queue->arr[ready_queue->front];
		if (newProcess->burst_time != 0) {
			newProcess->burst_time = newProcess->burst_time - *time_quantum;
			dequeue(ready_queue);
			getCurrentState(ready_queue);
		}
		else {
			newProcess = NULL;
		}
	}
}

int main() {
	// Originally Initialize Array to contain null values
    Queue qtest = {NULL};
    //printf("%d %d %d %d\n" , qtest.arr[0], qtest.arr[1], qtest.arr[2], qtest.arr[3]);
    read_csv(&qtest);
    printf("Enter the time quantum of your choice ");
    scanf("%d", &time_quantum);
    printf("Your time quantum is: %d\n", time_quantum);

    printf("Size is: %d\n", check_current_size(&qtest));
    printf("Burst times:\n");
    
    // It works
    for (size_t i = 0; i < check_current_size(&qtest); i++) {
		printf("Process id#: %d | Arrival Time: %d | Burst Time %d\n", qtest.arr[i]-> pid, 
		qtest.arr[i] -> arrival_time, qtest.arr[i]->burst_time);
    }
	
	start_scheduler(&qtest, &time_quantum);
}

