#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
#include <math.h>
#include "funciones.h"
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
extern char* proceso_global;
extern struct lista lista_hijos;
extern struct worker_data lista_workers;



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

// Code from https://stackoverflow.com/questions/1515195/how-to-remove-n-or-t-from-a-given-string-in-c
void strip(char *s)
{
    char *p2 = s;
    while(*s != '\0') {
        if(*s != '\t' && *s != '\n')
        {
            *p2++ = *s++;
        } else
        {
            ++s;
        }
    }
    *p2 = '\0';
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
    char** args = malloc(n+2 * sizeof(char*));
    args[0] = executable;
    for (int i=1; i<n+1; i++)
    {
        args[i] = strsep(&instructions, ",");
        if (i == n)
        {
            strip(args[i]);              // Quita \n del último argumento
        }
    }
    for (int i=0; i<n+1; i++)
    {
        printf("%s,", args[i]);
    };
    printf("\n");

    /* Guardamos el worker en lista_workers */
    pid_t worker_pid = getpid();
    time_t init_time = -1;
    time_t total_time = -1;
    int status = -1;
    insert_worker(&worker_pid, &args, &init_time, &total_time, &status);

    /* Creamos un hijo de worker para realizar exec */
    pid_t childpid;
    childpid = fork();

    if (childpid >= 0)  /* El fork se realizó con éxito */
    {
        init_time = time(NULL);
        if (childpid == 0)  /* Proceso hijo */
        {
            //printf("P%i (W): Voy a ejecutar %s\n", nro_proceso, executable);
            execvp(executable, args);
        } else {    /* Proceso padre worker */
            //waitpid(childpid, &status, WUNTRACED);
            wait(&status); /* wait for child to exit, and store its status */
            total_time = time(NULL) - init_time;
            printf("P%i (W): Child's exit code is: %d\n",nro_proceso, WEXITSTATUS(status));
            printf("P%i (W): Child pid: %i\n", nro_proceso, childpid);

            // Definimos el nombre del archivo de salida
            char filename[10];
            sprintf(filename, "%d.txt", nro_proceso);
            // Abrimos el archivo
            FILE* file = fopen(filename, "w");
            // Escribimos el output en el archivo
            for (int i=0; i<n+1; i++)
            {
                fprintf(file, "%s,", args[i]);
            };
            fprintf(file, "%li,%i,0\n", total_time, WEXITSTATUS(status));
            // Cerramos el archivo
            fclose(file);
            printf("P%i (W): Archivo %s generado\n", nro_proceso, filename);
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
    printf("actual: %i\n", actual);
    kill(actual, SIGKILL);

}


/* Función que inserta valores en la variable global lista_hijos*/
/* Es una lista enlazada. Está definida en el header de funciones*/
void insert(pid_t* hijo)
{
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



/* Función que inserta valores en la variable global lista_workers */
/* Es una lista enlazada. Está definida en el header de funciones */
void insert_worker(pid_t* pid, char*** args, time_t* init_time, time_t* total_time, int* status)
{
    if (lista_workers.pid == 0)
    {
        lista_workers.pid = *pid;
        lista_workers.args = *args;
        lista_workers.init_time = *init_time;
        lista_workers.total_time = *total_time;
        lista_workers.status = *status;
        lista_workers.sig = NULL;
    } else
    {
        struct worker_data* q;
        struct worker_data* p;
        q = malloc(sizeof(struct worker_data));
        q->sig = NULL;
        q->pid = *pid;
        q->args = *args;
        q->init_time = *init_time;
        q->total_time = *total_time;
        q->status = *status;
        p = lista_workers.sig;
        if (p == NULL)
        {
            lista_workers.sig = q;
        } else
        {
            while(p->sig != NULL)
            {
                p = p->sig;
            };
            p->sig = q;
        };
    };
};


/* Retorna el worker en lista_workers que coincide con wpid*/
struct worker_data buscar_worker(pid_t* wpid)
{
    struct worker_data* actual;
    if (lista_workers.pid == *wpid)
    {
        return lista_workers;
    };
    actual = lista_workers.sig;
    while (actual->pid != *wpid)
    {
        actual = actual->sig;
    };
    return *actual;
};