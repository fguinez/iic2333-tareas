#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "../file_manager/manager.h"
#include "funciones.h"
#include <string.h>
#include <signal.h>

/* Hay dos variables globales */
/* proceso_global es un puntero a un string que tiene información del proceso actual que se está ejecutando */
/* lista_hijos es una lista enlazada que cree yo que tiene los pid_t de todos los hijos del proceso que se está
 * ejecutando actualmente*/
/* Puedes usar estas variables en cualquier archivo. Las programé para que se actualizaran según corresponda */
char* proceso_global;
struct lista lista_hijos;


int main(int argc, char **argv)
{

  printf("&&&&&&&&&&&&&&&&&&&&&&\n");
    if(argc != 3)
    {
      printf("ARGUMENTOS RECIBIDOS: %d\n", argc);
      printf("Modo de uso: %s <input> <process>\nDonde:\n", argv[0]);
      printf("\t\"<input>\" es la ruta al archivo de input\n");
      printf("\t\"<process>\" es índice del proceso desde donde debe empezar la ejecución\n");
      return 1;
    }
    /* Abre el input file */
    FILE* input_stream = fopen(argv[1], "r");

    // Comprueba que el archivo exista
    if(!input_stream)
    {
      fprintf(stderr, "No se ha encontrado el archivo %s\n", argv[1]);
      return 2;
    };

    /* Sacamos el índice y el total de procesos */
    int indice = atoi(argv[2]);
    printf("EL INDICE ES: %d\n", indice);
    int total_procesos;
    fscanf(input_stream, "%d", &total_procesos);

    /* Como ya abrimos el archivo y ya tenemos el índice, buscamos el proceso asociado al índice */
    char* proceso = buscar_linea(argv[1], indice);
    printf("PROCESO EN DESAROLLO: %s\n", proceso);
    char* proceso_copia;
    proceso_copia = strdup(proceso);
    actualizar(proceso_copia);

    /* Obtenemos el identificador del proceso usando la función strsep()*/
    char* ident = strsep(&proceso, ",");


    if (!strcmp(ident, "R")){
        printf("P%i (R) Iniciando\n", indice);

        /* Esto extrae los valores de los otros parḿetros del proceso*/
        char* timeout = strsep(&proceso, ",");
        char* hijos = strsep(&proceso, ",");
        printf("P%i (R) TIMEOUT: %s\n", indice, timeout);
        printf("P%i (R) TOTAL DE HIJOS: %s\n", indice, hijos);

        /* Acá falta el TIMEOUT que vaya en paralelo con la función crear hijos*/
        signal(SIGINT, &signal_sigint_handler_root);
        signal(SIGABRT, &signal_sigint_handler_root);

        crear_hijos_manager(proceso_copia, argv[1]);
        printf("P%i (R) Terminado\n", indice);
        printf("++++++++++++++++++++++++\n");
    }

    else if (!strcmp(ident, "M")){
        printf("P%i (M): Iniciando\n", indice);
        /* Esto extrae los valores de los otros parḿetros del proceso*/
        char* timeout = strsep(&proceso, ",");
        char* hijos = strsep(&proceso, ",");
        printf("P%i (M): TIMEOUT: %s\n", indice, timeout);
        printf("P%i (M): TOTAL DE HIJOS: %s\n", indice, hijos);

        /* Acá falta el TIMEOUT que vaya en paralelo con la función crear hijos*/
        signal(SIGINT, &signal_sigint_handler_nonroot);
        signal(SIGABRT, &signal_sigabrt_handler);

        crear_hijos_manager(proceso_copia, argv[1]);
        printf("P%i (M): Terminado\n", indice);

        /* Acá el proceso manager debiese manejar los archivos de sus procesos workers y juntarlos todos
         * en un mismo archivo, tal como lo pide el enunciado.*/

        printf("++++++++++++++++++++++++\n");
        exit(0);
    }

    else{
        printf("P%i (W): Iniciando...\n", indice);
        signal(SIGINT, &signal_sigint_handler_nonroot);
        signal(SIGABRT, &signal_sigabrt_handler_worker);
        
        // Debiese hacer fork y que el proceso hijo haga execve al ejecutable y que ese proceso
        crear_hijo_worker(proceso_copia, indice);
        /* * hijo se encargue de eso. Después, el hijo le devuelve el resultado al padre (worker) y este
         * debiese guardar el resultado en un archivo como se pide en el enunciado*/
        printf("P%i (W): Terminado\n", indice);
        printf("++++++++++++++++++++++++\n");
        exit(0);
    }



    fclose(input_stream);
  return 0;
}
