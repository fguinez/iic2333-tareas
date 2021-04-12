#include <stdio.h>
#include <stdlib.h>
#include "../file_manager/manager.h"
#include "funciones.h"
#include <string.h>

int main(int argc, char **argv)
{
  printf("Hello T1! \n");
    if(argc != 3)
    {
        printf("Modo de uso: %s input output\nDonde:\n", argv[0]);
        printf("\t\"input\" es la ruta al archivo de input\n");
        printf("\t\"output\" es la ruta al archivo de output\n");
        return 1;
    }
    /* Abre el input file */
    FILE* input_stream = fopen(argv[1], "r");

    /* Sacamos el índice y el total de procesos */
    int indice = atoi(argv[2]);
    printf("EL INDICE ES: %d\n", indice);
    int total_procesos;
    fscanf(input_stream, "%d", &total_procesos);
    printf("EL TOTAL DE PROCESOS ES: %d\n", total_procesos);

    /* Como ya abrimos el archivo y ya tenemos el índice, buscamos el proceso asociado al índice */
    char* pedro = buscar_linea(argv[1], indice);
    printf("PROCESO INICIAL: %s", pedro);

    /* Obtenemos el identificador del proceso usando la función strsep()*/
    char* ident = strsep(&pedro, ",");
    printf("IDENTIFICADOR: %s\n", ident);

    if (!strcmp(ident, "R")){
        printf("ES UN PROCESO ROOT\n");
    }

    else if (!strcmp(ident, "M")){
        printf("ES UN PROCESO MANAGE\n");
    }

    else{
        printf("ES UN PROCESO WORKER\n");
    }

    fclose(input_stream);
  return 0;
}
