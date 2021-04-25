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
extern struct worker_data* lista_workers;



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
void crear_hijos_manager(char* proceso, char* input_filename, int nro_proceso){
    char* ident = strsep(&proceso, ",");
    int timeout = atoi(strsep(&proceso, ","));
    int n = atoi(strsep(&proceso, ","));
    int status;

    /* Crea un array con los hijos*/
    char** hijos = malloc(n+2 * sizeof(char*));
    for (int i=0; i<n; i++)
    {
        hijos[i] = strsep(&proceso, ",");
        if (i+1 == n)
        {
            strip(hijos[i]);              // Quita \n del último hijo
        };
    };

    /* Para cada hijo, hacemos fork y execve*/
    for (int i = 0; i<n; i++){
        /* Creo una lista con los parámetros para el execve*/
        char* args[4];
        args[0] = "./crtree";
        args[1] = input_filename;
        args[2] = hijos[i];
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

    // Definimos el nombre del archivo de salida
    char filename[10];
    sprintf(filename, "%d.txt", nro_proceso);
    // Se crea el archivo del manager
    FILE* file = fopen(filename, "w");

    /* El proceso padre se queda esperando a que todos los hijos terminen*/
    for (int i = 0; i<n; i++){
        wait(&status);
        //waitpid(childpids[i], NULL; 0);

        /* Se escribe el archivo del hijo en el archivo del padre */
        // Se define el nombre del archivo del hijo
        char* child_filename;
        child_filename = malloc(sizeof(char) * (strlen(hijos[i]) + 6));
        sprintf(child_filename, "%s.txt", hijos[i]);
        // Se abre en el archivo hijo
        FILE* child_file = fopen(child_filename, "r");
        // Comprueba que el archivo exista
        if(!child_file)
        {
            printf("P%i    : No se ha encontrado el archivo %s\n", nro_proceso, child_filename);
            continue;
        };
        // Se escribe hijo en padre
        char line[300];
        while (fgets(line, 200, child_file) != NULL) 
        { 
            fprintf(file, "%s", line);
        };
    };
    printf("P%i    : Archivo %s generado\n", nro_proceso, filename);
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
    
    /* Guardamos el worker en struct worker */
    pid_t worker_pid = getpid();
    time_t init_time = -1;
    time_t total_time = -1;
    int status = -1;
    printf("||||||| entrando del worker insert\n");/////////////////////////////////////////////////////////////////
    printf("||||||| %i, %s, %i, %i, %i, %i, %i\n", worker_pid, args[0], init_time, total_time, status, nro_proceso, n);/////////////////////////////////////////////////////////////////
    insert_worker(&worker_pid, &args, &init_time, &total_time, &status, &nro_proceso, &n);
    printf("||||||| saliendo del worker insert\n");/////////////////////////////////////////////////////////////////

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
    // Obtenemos el pid correspondiente
    pid_t wpid = getpid();

    // Buscamos los datos del worker correspondiente a wpid
    struct worker_data* worker;
    worker = buscar_worker(&wpid);

    // Avisamos del ABRT
    printf("P%i (W): Abortando worker %i...\n", worker->nro_proceso, wpid);

    // Definimos el nombre del archivo de salida
    char filename[10];
    sprintf(filename, "%d.txt", worker->nro_proceso);
    // Abrimos el archivo
    FILE* file = fopen(filename, "w");
    // Escribimos el output en el archivo
    for (int i=0; i<worker->n+1; i++)
    {
        fprintf(file, "%s,", worker->args[i]);
    };
    if (worker->total_time != -1)
    {
        fprintf(file, "%li,%i,1\n", worker->total_time, WEXITSTATUS(worker->status));
    } else
    {
        fprintf(file, "%li,%i,1\n", time(NULL) - worker->init_time, WEXITSTATUS(worker->status));
    }
        
    // Cerramos el archivo
    fclose(file);
    printf("P%i (W): Archivo %s generado\n", worker->nro_proceso, filename);
    kill(wpid, SIGKILL);
    printf("P%i (W): Worker %i abortado\n", worker->nro_proceso, wpid);
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




// Función que inserta valores en la variable global lista_workers
// Es una lista enlazada. Está definida en el header de funciones
void insert_worker(pid_t* pid, char*** args, time_t* init_time, time_t* total_time, int* status, int* nro_proceso, int* n)
{
    struct worker_data* nuevo_worker;
    struct worker_data* actual_worker;
    printf("||||||| entrando al worker malloc\n");/////////////////////////////////////////////////////////////////
    nuevo_worker = malloc(sizeof(struct worker_data));
    printf("||||||| saliendo del worker malloc\n");/////////////////////////////////////////////////////////////////
    nuevo_worker->pid =         *pid;
    nuevo_worker->args =        *args;
    nuevo_worker->init_time =   *init_time;
    nuevo_worker->total_time =  *total_time;
    nuevo_worker->status =      *status;
    nuevo_worker->nro_proceso = *nro_proceso;
    nuevo_worker->n =           *n;
    nuevo_worker->sig =         NULL;
    actual_worker = lista_workers;
    if (actual_worker == NULL)
    {
        actual_worker = nuevo_worker;
    } else
    {
        while(actual_worker->sig != NULL)
        {
            actual_worker = actual_worker->sig;
        };
        actual_worker->sig = nuevo_worker;
    };
};


// Retorna el worker en lista_workers que coincide con wpid
struct worker_data* buscar_worker(pid_t* wpid)
{
    struct worker_data* actual;
    actual = lista_workers;
    while (actual->pid != *wpid)
    {
        actual = actual->sig;
    };
    return actual;
};
