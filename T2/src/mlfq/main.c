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
    int Q = atoi(argv[3]);
    int q = atoi(argv[4]);
    int S = atoi(argv[5]);

    /* Leer la cantidad de colas y crearlas*/
    int total_process;
    fscanf(input_stream, "%d", &total_process);
    printf("TOTAL PROCESOS: %d\n", total_process);
    printf("TOTAL COLAS: %d\n", Q);

    /* Crea la cola inicial y después el resto*/
    struct Queue* starting_queue;
    starting_queue = malloc(sizeof(struct Queue));
    starting_queue->first = NULL;
    starting_queue->next = NULL;
    starting_queue->priority = Q-1;
    /* Crear colas de forma recursiva*/
    create_queues(starting_queue, Q-1);

    /* Usamos el file manager que nos dan para leer el input.txt*/
    InputFile* proob;
    proob = read_file(argv[1]);
    printf("Linea 1: %s\n", proob->lines[0][0]);
    create_process(proob, starting_queue, Q);

    /* Variables para guardar los procesos terminados*/
    /* Los procesos terminados van a un arreglo*/
    struct Process* finished[total_process];



    /* Variables para la simulación*/
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
     * 2 --> CYCLE*/

    /* Scheduler*/
    while (ready < total_process){
        /* Reviso si termina algún evento evento*/
        if (time == next_event_time){
            /* Al proceso del schudeler le toca hacer wait*/
            if (next_event_type == 0 && processing != NULL){
                /* Aumentamos su prioridad */
                if (processing->priority != Q-1){
                    processing->priority += 1;
                }
                printf("PROCESO ENTRA A WAIT\n");
                /* Actualizamos sus parámetros*/
                processing->waiting_since = time;
                processing->total_time_running += time - processing->time_started_running;
                processing->next = NULL;
                processing->status = 2;
                processing->time_executed_without_wait = 0;
                /* Lo metemos de vuelta a la cola que corresponda*/
                insert_in_specific_queue(starting_queue, processing, processing->priority);
                processing = NULL;

            }

            if (next_event_type == 1 && processing != NULL){
                /* Al proceso se le acabó el quantum*/
                printf("AL PROCESO %d SE LE HA ACABADO EL QUANTUM EN T = %d\n", processing->PID, time);
                if (processing->priority != 0){
                    /* Dsiminuimos su prioridad */
                    processing->priority -= 1;
                }
                /* Actualizamos sus parámetros*/
                processing->time_executed_without_wait += time - processing->time_started_running;
                processing->total_time_running += time - processing->time_started_running;
                processing->interrumpions += 1;
                processing->next = NULL;
                /* Lo metemos donde le corresponda*/
                insert_in_specific_queue(starting_queue, processing, processing->priority);
                processing = NULL;
            }

            if (next_event_type == 2){
                /* EL proceso termina*/
                printf("PROCESO HA TERMINADO\n");
                /* Actualizamos sus parámetros*/
                processing->finished_time = time;
                processing->turnaround_time = processing->finished_time - processing->starting_time;
                processing->waiting_time = processing->turnaround_time - processing->cycles;
                processing->next = NULL;
                /* Lo metemos al arreglo*/
                finished[ready] = processing;
                processing = NULL;
                ready += 1;
            }

        }

        if (time == S*times_used_S){
            /* Revisamos si ocurre S*/
            times_used_S += 1;
            printf("------------------------------------------>PASANDO EL S en t = %d\n", time);
            all_process_back_to_first_queue(starting_queue);
        }

        if (processing == NULL){
            /* Si no hay procesos en el scheduler, buscamos uno*/
            processing = extract_first_ready_process_from_all_queues(starting_queue, time);

            if (processing != NULL){
                /*Si encuentra uno, seteamos sus nuevos parámetros*/
                processing->next = NULL;
                processing->time_started_running = time;
                processing->chosen += 1;
                if (processing->started == 1){
                    /* Vemos si es que entra por primera vez al scheduler*/
                    processing->response_time = time - processing->starting_time;
                    if (processing->starting_time == 0){
                        processing->response_time -= 1;
                    }
                    processing->started = 2;
                }

                /* Calculamos el tiempo de todos los posibles eventos*/
                int quantum = (Q - processing->priority)*q;
                printf("___________________________________\n");
                printf("PROCESO: %d\n", processing->PID);
                printf("---------->quantum = %d\n", quantum);
                printf("---------->next waiting = %d\n", (processing->wait-processing->time_executed_without_wait));
                printf("---------->cycle ending:  = %d\n", (processing->cycles - processing->total_time_running));

                if (quantum < (processing->wait-processing->time_executed_without_wait) &&
                quantum < (processing->cycles - processing->total_time_running)){
                    /* Si el quantum es el que viene, lo seteamos*/
                    printf("PROXIMO EVENTO ES QUE SE LE ACABA EL QUANTUM\n");
                    next_event_type = 1;
                    next_event_time = quantum + time;
                }
                else if ((processing->wait-processing->time_executed_without_wait) < quantum &&
                         (processing->wait-processing->time_executed_without_wait) < (processing->cycles - processing->total_time_running)){
                    /* Si el wait es el que viene, lo seteamos*/
                    printf("PROXIMO EVENTO ES QUE VA A HACER WAIT\n");
                    next_event_time = (processing->wait-processing->time_executed_without_wait)+time;
                    next_event_type = 0;
                }

                else{
                    /* Si el quantum es el que viene, lo seteamos*/
                    printf("PROXIMO EVENTO ES QUE TERMINA EL CYCLE\n");
                    next_event_type = 2;
                    next_event_time = time + (processing->cycles - processing->total_time_running);

                }
            }
        }


        /* Actualizamos el tiempo*/
        time += 1;
    }


    /* Ejemplo imprimir resumen*/
    printf("_________________________________\n");
    printf("IMPRIMIENDO RESUMEN DE DATOS:\n");
    printf("PROCESO 1: %d\n", finished[0]->PID);
    printf("ESCOGIDO: %d\n", finished[0]->chosen);
    printf("INTERRUMPIDO: %d\n", finished[0]->interrumpions);
    printf("TURNAROUND TIME: %d\n", finished[0]->turnaround_time);
    printf("RESPONSE TIME: %d\n", finished[0]->response_time);
    printf("WAITING TIME: %d\n", finished[0]->waiting_time);

    // Guardamos el resumen en el archivo de salida indicado
    FILE* output_stream = fopen(argv[2], "w");
    for (int i=0; i<total_process;i++){
        // Escribimos una línea por proceso
        fprintf(
            output_stream,
            "%s,%i,%i,%i,%i,%i\n",
            finished[i]->name,            finished[i]->chosen,        finished[i]->interrumpions, 
            finished[i]->turnaround_time, finished[i]->response_time, finished[i]->waiting_time
        );
    };



    /* Liberar la memoria y  cerrar los archivos*/
    for (int i=0; i<total_process;i++){
        printf("PID: %d\n", finished[i]->PID);
        free(finished[i]);
    };
    free(processing);
    free_memory(starting_queue);
    input_file_destroy(proob);
    fclose(input_stream);




}
