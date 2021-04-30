#pragma once
#include <stdio.h>
#include "../file_manager/manager.h"




/*NEXT EVENT INFORMATION
* 0 --> WAIT
* 1 --> QUANTUM
* 2 --> CYCLE*/
enum event_t {
    WAIT,
    QUANTUM,
    CYCLE
};

/*Status information:
 * 0 --> RUNNING
 * 1 --> READY
 * 2 --> WAITING
 * 3 --> FINISHED
 * 4 --> WAITING TO BEGIN* */
enum s {
    RUNNING,
    READY,
    WAITING,
    FINISHED,
    WAITING_TO_BEGIN
};

/*Started information:
 * 0 --> NO
 * 1 --> YES */

struct Process{
    int PID;
    char* name;
    int priority;
    enum s status;
    int waiting_time;
    int response_time;
    int turnaround_time;
    struct Process* next;
    int starting_time;
    int insert_time;
    int cycles;
    int wait;
    int waiting_delay;
    int waiting_since;
    int time_executed_without_wait;
    int started;
    int time_started_running;
    int total_time_running;
    int interrumpions;
    int chosen;
    int finished_time;
};

struct Queue{
    struct Queue* next;
    int priority;
    struct Process* first;

};

void free_memory(struct Queue*);
void free_memory_process(struct Process*);
void create_queues(struct Queue*, int);
void create_process(InputFile*, struct Queue*, int);
void insert_in_queue(struct Queue*, struct Process*);
void insert_in_specific_queue(struct Queue*, struct Process*, int);
struct Process* extract_first_ready_process(struct Queue*, int);
struct Process* extract_first_ready_process_from_all_queues(struct Queue*, int);
int insert_in_order(struct Queue*, struct Process*);
void all_process_back_to_first_queue(struct Queue*);
struct Process* apply_quantum(struct Process*, int);
struct Process* apply_wait(struct Process*, int);