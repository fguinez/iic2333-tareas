#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "funciones.h"

#include <string.h>

char* buscar_linea(const char* input, int nro_proceso){
    FILE* input_stream = fopen(input, "r");
    int count = 0;

    char line[256]; /* or other suitable maximum line size */
    while (fgets(line, sizeof line, input_stream) != NULL) /* read a line */
    {
        if (count == nro_proceso+1)
        {
            char* pedro;
            pedro = malloc(sizeof(char) * (strlen(line) + 1));
            strcpy(pedro, line);
            return pedro;
        }
        else
        {
            count++;
        }
    }

    return 0;

}

void crear_hijos(char* proceso){
    printf("PROCESO: %s", proceso);


}