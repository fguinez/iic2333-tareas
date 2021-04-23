#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funciones.h"
#include "../file_manager/manager.h"

int main(int argc, char **argv)
{
    /* Revisar que la cantidad de argumentos sea correcta*/
    if(argc != 6)
    {
        printf("ARGUMENTOS RECIBIDOS: %d\n", argc);
        printf("Modo de uso: %s input output Q q S\nDonde:\n", argv[0]);
        printf("\t\"input\" es la ruta al archivo de input\n");
        printf("\t\"output\" es la ruta al archivo de output\n");
        printf("\t\"Q\" es la cantidad de colas\n");
        printf("\t\"q\" es el parámetro de quantum\n");
        printf("\t\"S\" es el tiempo hasta que todos vuelvan a la cola de mayor prioridad\n");
        return 1;
    }

    /* Guardar los argumentos en variables*/
    FILE* input_stream = fopen(argv[1], "r");
    /*FILE* output_stream = fopen(argv[2], "r");*/
    int Q = atoi(argv[3]);
    int q = atoi(argv[4]);
    int S = atoi(argv[5]);

    /* Leer la cantidad de colas y crearlas*/
    int total_process;
    fscanf(input_stream, "%d", &total_process);
    printf("TOTAL PROCESOS: %d\n", total_process);
    printf("TOTAL COLAS: %d\n", Q);

    struct Queue* starting_queue;
    starting_queue = malloc(sizeof(struct Queue));
    starting_queue->first = NULL;
    starting_queue->next = NULL;
    starting_queue->priority = Q-1;
    /* Crear colas de forma recursiva*/
    create_queues(starting_queue, Q-1);

    InputFile* proob;
    proob = read_file(argv[1]);
    printf("Linea 1: %s\n", proob->lines[0][0]);
    create_process(proob, starting_queue, 3, Q);

    /* Variables para guardar los procesos terminados*/
    struct Process* finished[total_process];
    int total_finished_process = 0;




    int time = 1;
    struct Process* processing;
    processing = malloc(sizeof(struct Process));
    processing = NULL;
    int next_event_time = 1000;
    int next_event_type = 4;
    int times_used_S = 1;
    int ready = 0;

    /*NEXT EVENT INFORMATION
     * 0 --> WAIT
     * 1 --> QUANTUM
     * 2 --> S TIMEOUT*/

    while (total_finished_process < 400){

        if (time == next_event_time){
            if (next_event_type == 0 && processing != NULL){
                if (processing->priority != Q-1){
                    processing->priority += 1;
                }
                printf("PROCESO HACIENDO WAIT\n");
                processing->waiting_since = time;
                processing->total_time_running += time - processing->time_started_running;
                processing->next = NULL;
                processing->status = 2;
                processing->time_executed_without_wait = 0;
                insert_in_specific_queue(starting_queue, processing, processing->priority);
                processing = NULL;

            }

            if (next_event_type == 1 && processing != NULL){
                printf("AL PROCESO %d SE LE HA ACABADO EL QUANTUM EN T = %d\n", processing->PID, time);
                if (processing->priority != 0){
                    processing->priority -= 1;
                }
                processing->time_executed_without_wait += time - processing->time_started_running;
                processing->total_time_running += time - processing->time_started_running;
                processing->interrumpions += 1;
                processing->next = NULL;
                insert_in_specific_queue(starting_queue, processing, processing->priority);
                processing = NULL;
            }

            if (next_event_type == 2){
                printf("PROCESO HA TERMINADO\n");
                processing->finished_time = time;
                processing->turnaround_time = processing->finished_time - processing->starting_time;
                processing->waiting_time = processing->turnaround_time - processing->cycles;
                finished[ready] = processing;
                processing = NULL;
                ready += 1;
            }

        }

        if (time == S*times_used_S){
            times_used_S += 1;
            printf("PASANDO EL S\n");
        }

        if (processing == NULL){
            processing = extract_first_ready_process_from_all_queues(starting_queue, time);

            if (processing != NULL){
                processing->time_started_running = time;
                processing->chosen += 1;
                if (processing->started == 1){
                    printf("**********VIRGEN*************************************************\n");
                    processing->response_time = time - processing->starting_time;
                    if (processing->starting_time == 0){
                        processing->response_time -= 1;
                    }
                    processing->started = 2;
                }
                int quantum = (Q - processing->priority)*q;
                printf("___________________________________\n");
                printf("PROCESO: %d\n", processing->PID);
                printf("---------->quantum = %d\n", quantum);
                printf("---------->next waiting = %d\n", (processing->wait-processing->time_executed_without_wait));
                printf("---------->cycle ending:  = %d\n", (processing->cycles - processing->total_time_running));

                if (quantum < (processing->wait-processing->time_executed_without_wait) &&
                quantum < (processing->cycles - processing->total_time_running)){
                    printf("PROXIMO EVENTO ES QUE SE LE ACABA EL QUANTUM\n");
                    next_event_type = 1;
                    next_event_time = quantum + time;
                }
                else if ((processing->wait-processing->time_executed_without_wait) < quantum &&
                         (processing->wait-processing->time_executed_without_wait) < (processing->cycles - processing->total_time_running)){
                    printf("PROXIMO EVENTO ES QUE VA A HACER WAIT\n");
                    next_event_time = (processing->wait-processing->time_executed_without_wait)+time;
                    next_event_type = 0;
                }

                else{
                    printf("PROXIMO EVENTO ES QUE TERMINA EL CYCLE\n");
                    next_event_type = 2;
                    next_event_time = time + (processing->cycles - processing->total_time_running);

                }
            }
        }



        time += 1;
        total_finished_process += 1;
    }


    printf("_________________________________\n");
    printf("IMPRIMIENDO RESUMEN DE DATOS:\n");
    printf("PROCESO 1: %d\n", finished[0]->PID);
    printf("ESCOGIDO: %d\n", finished[0]->chosen);
    printf("INTERRUMPIDO: %d\n", finished[0]->interrumpions);
    printf("TURNAROUND TIME: %d\n", finished[0]->turnaround_time);
    printf("RESPONSE TIME: %d\n", finished[0]->response_time);
    printf("WAITING TIME: %d\n", finished[0]->waiting_time);






    /* Liberar la memoria y  cerrar los archivos*/
    free_memory(starting_queue);
    fclose(input_stream);




}
