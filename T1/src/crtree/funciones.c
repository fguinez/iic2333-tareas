#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
#include <math.h>
#include "funciones.h"
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
extern char* proceso_global;
extern struct lista lista_hijos;




/* Actualiza el valor del proceso actual */
void actualizar(char* proceso){
    proceso_global = proceso;
}

/* Busca el proceso a ejecutar en el input.txt*/
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



// CREADORES DE HIJOS
/* Crea los hijos de un proceso manager*/
void crear_hijos_manager(char* proceso, char* filename){
    char* ident = strsep(&proceso, ",");
    int timeout = atoi(strsep(&proceso, ","));
    char* hijos = strsep(&proceso, ",");
    int status;
    /* Para cada hijo, hacemos fork y execve*/
    for (int i = 0; i<atoi(hijos); i++){
        /* Leo el número del proceso hijo*/
        char* num = strsep(&proceso, ",");

        /* Creo una lista con los parámetros para el execve*/
        char* args[4];
        args[0] = "./crtree";
        args[1] = filename;
        args[2] = num;
        args[3] = NULL;

        /* Hacemos fork*/
        pid_t childpid;
        childpid = fork();

        /* Si el proceso es hijo hacemos execve*/
        if (childpid==0){
            execve("./crtree", args, NULL);
        }
        else{
            insert(&childpid);
        }
    }

    /* Se setean las señales dependiendo si el proceso de root o nonroot*/

    /* El proceso padre se queda esperando a que todos los hijos terminen*/
    for (int i = 0; i<atoi(hijos); i++){
        wait(&status);
        //waitpid(childpids[i], NULL; 0);

    }
};


void crear_hijo_worker(char* instructions, int nro_proceso){
    /* Separamos las instrucciones */
    // No guardamos primer elemento, siempre será W
    strsep(&instructions, ",");
    // Guardamos el comando a ejecutar
    char* executable = strsep(&instructions, ",");
    // Guardamos la cantidad de argumentos del comando
    int n = atoi(strsep(&instructions, ","));
    
    // Guardamos un array con el ejecutable y los n argumentos
    char** args = malloc(n+1 * sizeof(char*));
    args[0] = executable;
    //printf("====== arg[0]: %s\n", args[0]);
    for (int i=1; i<n; i++)
    {
        args[i] = strsep(&instructions, ",");
    }

    /* Creamos un hijo de worker para realizar exec */
    pid_t childpid;
    childpid = fork();

    if (childpid >= 0)  /* El fork se realizó con éxito */
    {
        if (childpid == 0)  /* Proceso hijo */
        {
            printf("P%i (W): Voy a ejecutar %s\n", nro_proceso, executable);
            execvp(executable, args);
        } else {    /* Proceso padre worker */
            int status;
            //waitpid(childpid, &status, WUNTRACED);
            wait(&status); /* wait for child to exit, and store its status */
            printf("P%i (W): Child's exit code is: %d\n",nro_proceso, WEXITSTATUS(status));
            printf("P%i (W): Child pid: %i\n", nro_proceso, childpid);
        };
    };
};




// SEÑALES
/* Función que maneja las señales de SIGINT para procesos root*/
void signal_sigint_handler_root(int sig){
    printf("HA LLEGADO UNA SEÑAL A ROOT\n");
    printf("SE ENVIARÁ SIGABRT A TODOS SUS HIJOS\n");
    if (lista_hijos.hijo == 0){
        printf("NO HAY HIJOS\n");
    }
    else if (lista_hijos.sig == NULL){
        printf("ENVIANDO SEÑAL A 1 HIJO\n");
        kill(lista_hijos.hijo, SIGABRT);
    }
    else{
        kill(lista_hijos.hijo, SIGABRT);
        struct lista* p;
        p = lista_hijos.sig;
        kill(p->hijo, SIGABRT);
        while (p->sig!= NULL){
            p = p->sig;
            kill(p->hijo, SIGABRT);
        }
    }

    /* Finalmente le hago abort al proceso root */
    pid_t actual = getpid();
    kill(actual, SIGKILL);


}

/* Función que maneja las señales de SIGINT para procesos no root*/
void signal_sigint_handler_nonroot(int sig){
    printf("HA LLEGADO UNA SEÑAL DE SIGINT A UN M/W\n");
    printf("LA SEÑAL SE OMITE\n");
}

void signal_sigabrt_handler(int sig){
    printf("HA LLEGADO UNA SEÑAL DE SIGABRT A UN MANAGER\n");
    printf("PROCEDIENDO A ABORTAR TODOS LOS HIJOS\n");
    if (lista_hijos.hijo == 0){
        printf("NO HAY HIJOS\n");
    }
    else if (lista_hijos.sig == NULL){
        printf("ENVIANDO SEÑAL A 1 HIJO\n");
        kill(lista_hijos.hijo, SIGABRT);
    }
    else{
        kill(lista_hijos.hijo, SIGABRT);
        struct lista* p;
        p = lista_hijos.sig;
        kill(p->hijo, SIGABRT);
        while (p->sig!= NULL){
            p = p->sig;
            kill(p->hijo, SIGABRT);
        }
    }

    /* Acá se debiesen juntar todos los archivos en caso de que haya un abort*/
    /* Finalmente le hago abort al proceso manager */
    pid_t actual = getpid();
    kill(actual, SIGKILL);

}

void signal_sigabrt_handler_worker(int sig){
    printf("HA LLEGADO UNA SEÑAL DE ABRT A UN WORKER\n");
    /* Acá se debiesen crear el archivo diciendo que el proceso fue interrumpido*/

    printf("WORKER ABORTADO\n");
    pid_t actual = getpid();
    kill(actual, SIGKILL);

}


/* Función que inserta valores en la variable global lista_hijos*/
/* Es una lista enlazada. Está definida en el header de funciones*/
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
            lista_hijos.sig = q;
        }
        else{
            while(p->sig != NULL){
                p = p->sig;
            }
            p->sig = q;
        }
    }

}