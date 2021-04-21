#include "../file_manager/manager.h"
#include "funciones.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void free_memory(struct Queue* starting_queue){
    if (starting_queue->next != NULL){
        free_memory(starting_queue->next);
    }
    free(starting_queue);
}

void create_queues(struct Queue* actual, int remaining){
    if (remaining > 0){
        struct Queue* new;
        new = malloc(sizeof(struct Queue));
        new->first = NULL;
        new->priority = remaining-1;
        new->next = NULL;
        actual->next = new;
        create_queues(new, remaining-1);
    }
}