#include "../file_manager/manager.h"
#include "funciones.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Función para liberar memoria recursivamente de las colas*/
void free_memory(struct Queue* starting_queue){
    if (starting_queue->next != NULL){
        free_memory(starting_queue->next);
    }
    struct Process* actual_process;
    actual_process = starting_queue->first;
    if (actual_process != NULL) {
        free_memory_process(actual_process);
    }
    free(starting_queue);
}

/* Función para liberar memoria recursivamente de los procesos*/
void free_memory_process(struct Process* actual_process){
    if (actual_process->next != NULL){
        free_memory_process(actual_process->next);
    }
    free(actual_process);
}

/* Función para crear colas recursivamente*/
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

/* Función para crear procesos desde el input*/
void create_process(InputFile* proob, struct Queue* starting_queue, int Q){
    for (int i = 0; i < proob->len; i++){
        char* name = proob->lines[i][0];
        int PID = atoi(proob->lines[i][1]);
        int starting_time = atoi(proob->lines[i][2]);
        int cycles = atoi(proob->lines[i][3]);
        int wait = atoi(proob->lines[i][4]);
        int waiting_delay = atoi(proob->lines[i][5]);
        struct Process* new;
        new = malloc(sizeof(struct Process));
        new->name = name;
        new->PID = PID;
        new->starting_time = starting_time;
        new->cycles = cycles;
        new->wait = wait;
        new->waiting_delay = waiting_delay;
        new->next = NULL;
        new->priority = Q-1;
        new->time_executed_without_wait = 0;
        new->total_time_running = 0;
        new->interrumpions = 0;
        if (new->starting_time == 0){
            new->status = 1;
            new->started = 1;
        }
        else {
            new->status = 2;
            new->started = 0;
        }
        insert_in_order(starting_queue, new);


    }
}

/* Función para insertar los procesos en la primera cola según su tiempo de llegada*/
int insert_in_order(struct Queue* starting_queue, struct Process* new){
    /* Caso 1: No hay procesos en la cola*/
    if (starting_queue->first == NULL){
        starting_queue->first = new;
        return 1;
    }
    else{

        struct Process* actual_process;
        actual_process = starting_queue->first;
        if (actual_process->next == NULL) {
            /* Caso 2: Solo hay 1 proceso en la cola*/
            if (actual_process->starting_time >= new->starting_time) {
                new->next = actual_process;
                starting_queue->first = new;
            }
            else{
                actual_process->next = new;
            }
            return 2;
        }
        else{
            /* Caso 3: Hay más de 1 proceso en la cola*/
            while (actual_process->next != NULL){
                if (actual_process->next->starting_time >= new->starting_time){
                    new->next = actual_process->next;
                    actual_process->next = new;
                    return 3;
                }
                actual_process = actual_process->next;
            }
        }

    }
    printf("HA HABIDO UN ERROR, QUEDÓ EL SAPO\n");
    return 5;
}

/* Función que manda a todos los procesos de vuelta a la primera cola*/
void all_process_back_to_first_queue(struct Queue* starting_queue){
    struct Queue* actual_queue;
    struct Process* actual_process;
    actual_queue = starting_queue->next;
    while (actual_queue != NULL){
        actual_process = actual_queue->first;
        insert_in_queue(starting_queue, actual_process);
        /* La cola ahora queda vacía*/
        actual_queue->first = NULL;
        actual_queue = actual_queue->next;
    }
    actual_process = starting_queue->first;
    while (actual_process != NULL){
        /* Hay que cambiar la prioridad de los procesos por la que corresponda*/
        actual_process->priority = starting_queue->priority;
        actual_process = actual_process->next;
    }
}

/* Función que agrega un proceso al final de una cola que se le da como parámetro*/
void insert_in_queue(struct Queue* starting_queue, struct Process* new_process){
    if (starting_queue->first == NULL){
        starting_queue->first = new_process;
    }

    else{
        struct Process* actual;
        actual = starting_queue->first;
        while (actual->next != NULL){
            actual = actual->next;
        }
        actual->next = new_process;
    }
}

/* Función que agrega un proceso al final de una cola específica, donde solo se indica la prioridad de dicha cola*/
void insert_in_specific_queue(struct Queue* starting_queue, struct Process* actual_process, int priority){
    struct Queue* tmp;
    tmp = starting_queue;
    while (tmp->priority != priority){
        tmp = tmp->next;
    }
    insert_in_queue(tmp, actual_process);
}

/* Función que retorna el proceso de mayor prioridad en estado ready de todos para 1 cola*/
struct Process* extract_first_ready_process(struct Queue* actual_queue, int time){
    if (actual_queue->first == NULL){
        return NULL;
    }
    else{
        struct Process* actual_process;
        struct Process* retorner;
        actual_process = actual_queue->first;
        /*Reviso primero si es que es el primer elemento de la lista el que tiene que salir*/
        /* Si está en waiting, veo si es que ya debería haber empezado */
        if (actual_process->status == 2){
            if (actual_process->starting_time <= time && actual_process->started == 0){
                actual_process->started = 1;
                actual_process->status = 1;
            }
            /* Si está en waiting, veo si es que ya terminó su waiting delay */
            if (time - actual_process->waiting_since >= actual_process->waiting_delay && actual_process->started == 2){
                actual_process->status = 1;
            }
        }

        /* Si está en ready, lo extraigo y actualizo la lista */
        if (actual_process->status == 1){
            actual_queue->first = actual_process->next;
            return actual_process;
        }
        /* Ahora empiezo a buscar en toda la cola */
        while (actual_process->next != NULL){
            /* Si está en waiting, veo si es que ya debería haber empezado */
            if (actual_process->next->status == 2){
                if (actual_process->next->starting_time <= time && actual_process->next->started == 0){
                    actual_process->next->status = 1;
                    actual_process->next->started = 1;
                }
                /* Si está en waiting, veo si es que ya terminó su waiting delay */
                if (time - actual_process->next->waiting_since >= actual_process->next->waiting_delay  && actual_process->started == 2){
                    actual_process->next->status = 1;
                }
            }

            /* Si está en ready, lo extraigo y actualizo la lista */
            if (actual_process->next->status == 1){
                retorner = actual_process->next;
                actual_process->next = actual_process->next->next;
                return retorner;
            }
            actual_process = actual_process->next;
        }
        return NULL;
    }
}

/* Función que retorna el proceso de mayor prioridad en estado ready de todos para todas las colas*/
struct Process* extract_first_ready_process_from_all_queues(struct Queue* starting_queue, int time){
    struct Queue* actual_queue;
    struct Process* actual_process;
    actual_queue = starting_queue;

    while (actual_queue != NULL){
        actual_process = extract_first_ready_process(actual_queue, time);
        if (actual_process != NULL){
            return actual_process;
        }
        actual_queue = actual_queue->next;
    }
    return NULL;
}