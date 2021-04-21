#pragma once
#include <stdio.h>

struct Process{
    int PID;
    char name;
    int priority;
    int state;
    double wating_time;
    double response_time;
    double turnaround_time;
    struct Process* next;
};

struct Queue{
    struct Queue* next;
    int priority;
    struct Process* first;

};

void free_memory(struct Queue*);
void create_queues(struct Queue*, int);
