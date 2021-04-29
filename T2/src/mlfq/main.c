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


    // DEBUG BLOCK: Permite analizar si se construyó correctamente la cola
    //struct Process* aux;
    //aux = starting_queue->first;
    //while (aux) {
    //    printf("%s\n", aux->name);
    //    aux = aux->next;
    //}
    //return 5;


    /* Variables para guardar los procesos terminados*/
    /* Los procesos terminados van a un arreglo*/
    struct Process* finished[total_process];

    /* Variables para la simulación*/
    // NOTA: Por qué el tiempo empezaba en 1?
    int time = 0;
    struct Process* processing;
    processing = malloc(sizeof(struct Process));
    processing = NULL;
    int next_event_time = 10000;               // Se inicializa a un número ridiculamente alto
    enum event_t next_event_type = 4;          // Se inicializa como un tipo inválido
    int next_event_match_wait    = 0;          // Toma valor 1 cuando coincide un evento con WAIT
    int next_event_match_cycle   = 0;          // Toma valor 1 cuando coincide un evento con CYCLE
    int times_used_S = 0;
    int ready = 0;

    /*NEXT EVENT INFORMATION
     * 0 --> WAIT
     * 1 --> QUANTUM
     * 2 --> CYCLE*/

    /* Scheduler*/
    while (ready < total_process){
        /* Reviso si termina algún evento */
        if (time == next_event_time){
            if (processing == NULL) {
                printf("Esto no debería ocurrir!!!! processing == NULL && time == next_event_time");
            };
            /* Al proceso del schudeler le toca hacer wait*/
            if (next_event_type == WAIT && processing != NULL){
                printf("PROCESO ENTRA A WAIT\n");
                if (processing->priority != Q-1){
                    /* Aumentamos su prioridad */
                    processing->priority += 1;
                }
                
                /* Actualizamos sus parámetros*/
                apply_wait(processing, time);

                // Actualizamos su tiempo de ejecución
                processing->total_time_running += time - processing->time_started_running;

                if (next_event_match_cycle != 1) {
                    /* Lo metemos de vuelta a la cola que corresponda*/
                    insert_in_specific_queue(starting_queue, processing, processing->priority);
                    processing = NULL;
                } else {
                    printf("Además, entró a CYCLE\n");
                    // Excepción cuando coincid wait y cycle
                    next_event_type = CYCLE;
                    next_event_match_cycle = 0;
                };
            }

            if (next_event_type == QUANTUM && processing != NULL){
                /* Al proceso se le acabó el quantum*/
                printf("AL PROCESO %d SE LE HA ACABADO EL QUANTUM EN T = %d\n", processing->PID, time);
                
                if (processing->priority != 0){
                    /* Disminuimos su prioridad */
                    processing->priority -= 1;
                }
                /* Actualizamos sus parámetros */
                apply_quantum(processing, time);

                // Actualizamos su tiempo de ejecución
                processing->total_time_running += time - processing->time_started_running;

                // Excepción cuando coincide quantum y wait
                if (next_event_match_wait == 1) {
                    printf("Además, entró a WAIT\n");
                    /* Actualizamos sus parámetros */
                    apply_wait(processing, time);
                    next_event_match_wait = 0;
                }
                
                if (next_event_match_cycle != 1) {
                    /* Lo metemos de vuelta a la cola que corresponda*/
                    insert_in_specific_queue(starting_queue, processing, processing->priority);
                    processing = NULL;
                } else {
                    printf("Además, entró a CYCLE\n");
                    // Excepción cuando coincide quantum y cycle
                    next_event_type = CYCLE;
                    next_event_match_cycle = 0;
                };
            }

            if (next_event_type == CYCLE){
                /* EL proceso termina*/
                printf("PROCESO HA TERMINADO\n");
                /* Actualizamos sus parámetros*/
                processing->finished_time = time;
                processing->turnaround_time = time - processing->starting_time;
                processing->waiting_time = processing->turnaround_time - processing->cycles;
                processing->next = NULL;
                /* Lo metemos al arreglo*/
                finished[ready] = processing;
                processing = NULL;
                ready += 1;
            }
        }

        if ( time == S * (times_used_S+1) ){
            /* Revisamos si ocurre S*/
            printf("------------------------------------------>PASANDO EL S en t = %d\n", time);
            times_used_S += 1;
            all_process_back_to_first_queue(starting_queue);
        }

        if (processing == NULL){
            /* Si no hay procesos en el scheduler, buscamos uno*/
            processing = extract_first_ready_process_from_all_queues(starting_queue, time);

            if (processing != NULL){
                /*Si encuentra uno, seteamos sus nuevos parámetros*/
                processing->status = RUNNING;
                processing->next = NULL;
                processing->time_started_running = time;
                processing->chosen += 1;
                if (processing->started == 0){
                    /* Vemos si es que entra por primera vez al scheduler*/
                    processing->response_time = time - processing->starting_time;
                    // NOTA: No entiendo este if
                    //if (processing->starting_time == 0){
                    //    processing->response_time -= 1;
                    //}
                    processing->started = 1;
                }

                /* Calculamos el tiempo de todos los posibles eventos*/
                int quantum      = (Q - processing->priority)*q;
                int next_waiting = (processing->wait - processing->time_executed_without_wait);
                int cycle_ending = (processing->cycles - processing->total_time_running);

                // Forzamos el valor de next_waiting si el proceso nunca concede control por su cuenta
                if (processing->wait == 0) {
                    next_waiting = quantum + cycle_ending + 1;
                };

                // Imprimimos los tiempos para cada evento
                printf("___________________________________\n");
                printf("PROCESO: %d\n", processing->PID);
                printf("---------->quantum = %d\n", quantum);
                printf("---------->next waiting = %d\n", next_waiting);
                printf("---------->cycle ending:  = %d\n", cycle_ending);

                if (quantum <= next_waiting && quantum <= cycle_ending){
                    /* Si el quantum es el que viene, lo seteamos*/
                    printf("PROXIMO EVENTO ES QUE SE LE ACABA EL QUANTUM\n");
                    next_event_type = QUANTUM;
                    next_event_time = time + quantum;
                    if (quantum == next_waiting){
                        next_event_match_wait = 1;
                    }
                    if (quantum == cycle_ending) {
                        next_event_match_cycle = 1;
                    }
                }
                else if (next_waiting < quantum && next_waiting <= cycle_ending){
                    /* Si el wait es el que viene, lo seteamos*/
                    printf("PROXIMO EVENTO ES QUE VA A HACER WAIT\n");
                    next_event_type = WAIT;
                    next_event_time = time + next_waiting;
                    if (next_waiting == cycle_ending) {
                        next_event_match_cycle = 1;
                    }
                }

                else{
                    /* Si el término es lo que viene, lo seteamos*/
                    printf("PROXIMO EVENTO ES QUE TERMINA EL CYCLE\n");
                    next_event_type = CYCLE;
                    next_event_time = time + cycle_ending;
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
