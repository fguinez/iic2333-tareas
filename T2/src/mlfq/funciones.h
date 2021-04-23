#pragma once
#include <stdio.h>
#include "../file_manager/manager.h"
struct Process{
    int PID;
    char* name;
    int priority;
    int status;
    int waiting_time;
    int response_time;
    int turnaround_time;
    struct Process* next;
    int starting_time;
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

/*Status information:
 * 0 --> RUNNING
 * 1 --> READY
 * 2 --> WAITING
 * 3 --> FINISHED
 * 4 --> WAITING TO BEGIN* */

/*Started information:
 * 0 --> NO
 * 1 --> YES */

struct Queue{
    struct Queue* next;
    int priority;
    struct Process* first;

};

void free_memory(struct Queue*);
void create_queues(struct Queue*, int);
void create_process(InputFile*, struct Queue*, int, int);
void insert_in_queue(struct Queue*, struct Process*);
void insert_in_specific_queue(struct Queue*, struct Process*, int);
struct Process* extract_first_ready_process(struct Queue*, int);
struct Process* extract_first_ready_process_from_all_queues(struct Queue*, int);
