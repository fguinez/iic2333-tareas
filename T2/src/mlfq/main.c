#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funciones.h"

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




    /* Liberar la memoria y  cerrar los archivos*/
    free_memory(starting_queue);
    fclose(input_stream);




}
