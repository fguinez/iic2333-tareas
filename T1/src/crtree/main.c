#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "../file_manager/manager.h"
#include "funciones.h"
#include <string.h>
#include <signal.h>

char* proceso_global;
struct lista lista_hijos;


int main(int argc, char **argv)
{

  printf("Hello T1! \n");
  printf("+++++++++++++++++\n");
    if(argc != 3)
    {
        printf("ARGUMENTOS RECIBIDOS: %d\n", argc);
        printf("Modo de uso: %s input output\nDonde:\n", argv[0]);
        printf("\t\"input\" es la ruta al archivo de input\n");
        printf("\t\"output\" es la ruta al archivo de output\n");
        return 1;
    }
    /* Abre el input file */
    FILE* input_stream = fopen(argv[1], "r");
    int status;

    /* Sacamos el índice y el total de procesos */
    int indice = atoi(argv[2]);
    printf("EL INDICE ES: %d\n", indice);
    int total_procesos;
    fscanf(input_stream, "%d", &total_procesos);

    /* Como ya abrimos el archivo y ya tenemos el índice, buscamos el proceso asociado al índice */
    char* proceso = buscar_linea(argv[1], indice);
    printf("PROCESO EN DESAROLLO: %s", proceso);
    char* proceso_copia;
    proceso_copia = strdup(proceso);

    /* Obtenemos el identificador del proceso usando la función strsep()*/
    char* ident = strsep(&proceso, ",");
    printf("IDENTIFICADOR: %s\n", ident);


    if (!strcmp(ident, "R")){
        printf("ES UN PROCESO ROOT\n");

        /* Esto extrae los valores de los otros parḿetros del proceso*/
        char* timeout = strsep(&proceso, ",");
        char* hijos = strsep(&proceso, ",");
        printf("TIMEOUT: %s\n", timeout);
        printf("TOTAL DE HIJOS: %s\n", hijos);
        actualizar(proceso_copia);
        printf("aaaaa el proceso global es: %s\n", proceso_global);

        /* Para cada hijo, hacemos fork y execve*/
        for (int i = 0; i<atoi(hijos); i++){
            /* Leo el número del proceso hijo*/
            char* num = strsep(&proceso, ",");

            /* Creo una lista con los parámetros para el execve*/
            char* args[4];
            args[0] = "./crtree";
            args[1] = "input.txt";
            args[2] = num;
            args[3] = NULL;

            /* Hacemos fork*/
            printf("**************\n");
            printf("VAMOS A HACER UN FORK\n");
            pid_t childpid;
            childpid = fork();

            /* Si el proceso es hijo, tambíen hacemos execve*/
            if (childpid==0){
                execve("./crtree", args, NULL);
            }
            else{
                printf("---------------------->CHILDPID: %d\n", childpid);
                insert(&childpid);

            }
        }


        /* El proceso padre se queda esperando a que todos los hijos terminen*/
        signal(SIGINT, &signal_sigint_handler_root);
        for (int i = 0; i<atoi(hijos); i++){
            wait(&status);
        }

        printf("PROCESO ROOT TERMINADO\n");
        while (1){
            printf("Jajaja\n");
            sleep(1);
        }

        printf("++++++++++++++++++++++++\n");

    }

    else if (!strcmp(ident, "M")){
        printf("ES UN PROCESO MANAGE\n");
        /* Esto extrae los valores de los otros parḿetros del proceso*/
        char* timeout = strsep(&proceso, ",");
        char* hijos = strsep(&proceso, ",");
        printf("TIMEOUT (manage): %s\n", timeout);
        printf("TOTAL DE HIJOS (manage): %s\n", hijos);

        for (int i = 0; i<atoi(hijos);i++){
            /* Leo el número del proceso hijo*/
            char* num = strsep(&proceso, ",");

            /* Creo una lista con los parámetros para el execve*/
            char* args[4];
            args[0] = "./crtree";
            args[1] = "input.txt";
            args[2] = num;
            args[3] = NULL;

            /* Hacemos fork*/
            printf("**************\n");
            printf("VAMOS A HACER UN FORK\n");
            pid_t childpid;
            childpid = fork();

            /* Si el proceso es hijo, tambíen hacemos execve*/
            if (childpid==0){
                execve("./crtree", args, NULL);
            }
        }

        /* Hacemos Wait de los hijos */
        for (int i = 0; i <atoi(hijos);i++){
            wait(&status);
        }

        printf("PROCESO MANAGE TERMINADO\n");
        printf("++++++++++++++++++++++++\n");
    }

    else{
        printf("ES UN PROCESO WORKER\n");
        printf("PROCESO WORKER TERMINADO\n");
        signal(SIGINT, &signal_sigint_handler_nonroot);

        printf("++++++++++++++++++++++++\n");
        exit(status);
    }


    fclose(input_stream);
  return 0;
}
