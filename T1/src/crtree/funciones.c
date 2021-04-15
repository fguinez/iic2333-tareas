#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "funciones.h"
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
extern char* proceso_global;
extern struct lista lista_hijos;

void actualizar(char* proceso){
    proceso_global = proceso;
}


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

void signal_sigint_handler_root(int sig){
    printf("HA LLEGADO UNA SEÑAL DE SIGINT A ROOT\n");
    printf("SE ENVIARÁ SIGABRT A TODOS SUS HIJOS\n");
    if (lista_hijos.hijo == 0){
        printf("NO QUEDAN PROCESOS HIJOS\n");
    }
    else if (lista_hijos.sig == NULL){
        printf("ENVIANDO SEÑAL A 1 HIJO: %d\n", lista_hijos.hijo);
        kill(lista_hijos.hijo, SIGABRT);
    }
    else {
        printf("ENVIANDO SEÑAL A TODOS LOS HIJOS\n");
        struct lista* p;
        p = lista_hijos.sig;
        kill (p->hijo, SIGABRT);
        while (p->sig != NULL){
            p = p->sig;
            kill(p->hijo, SIGABRT);
        }
    }
    pid_t actual = getpid();
    kill(actual, SIGABRT);

}

void signal_sigint_handler_nonroot(int sig){
    printf("HA LLEGADO UNA SEÑAL DE SIGINT A UN M/W\n");
    printf("LA SEÑAL SE OMITE\n");
}

struct lista *creanodo() {
    return (struct lista *) malloc(sizeof(struct lista));
}

void insert(pid_t* hijo){
    if (lista_hijos.hijo == 0){
        lista_hijos.hijo = *hijo;
        lista_hijos.sig = NULL;
    }
    else{
        struct lista* q;
        struct lista* p;
        q = malloc(sizeof(struct lista));
        q->sig = NULL;
        q->hijo = *hijo;
        p = lista_hijos.sig;
        if (p == NULL){
            printf("Es nulo\n");
            lista_hijos.sig = q;
            printf("%d\n", lista_hijos.sig->hijo);
        }
        else{
            while(p->sig != NULL){
                p = p->sig;
            }
            p->sig = q;
            printf("AGREGADO\n");
        }


    }

}