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

#include <pthread.h>
#include <errno.h>

#define NUM_THREADS     1

extern char* proceso_global;
extern struct lista* lista_hijos;
extern struct worker_data worker_data;






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
            fclose(input_stream);
            return pedro;
        }
        else
        {
            count++;
        }
    }
    fclose(input_stream);
    return 0;
}

// Code from https://stackoverflow.com/questions/1515195/how-to-remove-n-or-t-from-a-given-string-in-c
void strip(char* s)
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
    strsep(&proceso, ",");
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

    // Empieza a correr timeout
    pthread_t tid;
    int* arg = malloc(sizeof(int));
    *arg = timeout;
    pthread_create(&tid, NULL, check_timeout, arg);

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
            int hijo = atoi(hijos[i]);

            // Insertamos al hijo en lista_hijos
            insert(&childpid, &hijo, &nro_proceso);
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
        //waitpid(childpid[i], NULL, WNOHANG);

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
        free(child_filename);
        fclose(child_file);
    };
    // Cerramos el checkeo de timeout
    pthread_cancel(tid);


    printf("P%i    : Archivo %s generado\n", nro_proceso, filename);

    // Liberamos memoria
    free(hijos);
    fclose(file);
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
    char** args;
    args = malloc(n+2 * sizeof(char*));
    int len = strlen(executable);
    args[0] = malloc((len+1) * sizeof(char));
    strcpy(args[0], executable);
    for (int i=1; i<n+1; i++)
    {
        char* arg = strsep(&instructions, ",");
        len = strlen(arg);
        args[i] = malloc((len+1) * sizeof(char));
        strcpy(args[i], arg);
        
        if (i == n)
        {
            strip(args[i]);              // Quita \n del último argumento
        }
    }
    
    /* Guardamos el worker en struct worker */
    //time_t init_time = -1;
    //time_t total_time = -1;
    //int status = -1;
    //////////////////////////////////////////////////////////////////////////////////////////////////
    ///////  ALERTA BUG DETECTADO  ////  INICIO ZONA EN CUARENTENA  ////  ALERTA BUG DETECTADO  //////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //
    //   Ver la función insert_worker para más detalles
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////// 
    //printf("||||||| entrando del worker insert\n");/////////////////////////////////////////////////////////////////
    //printf("||||||| %i, %s, %li, %li, %i, %i, %i\n", worker_pid, args[0], init_time, total_time, status, nro_proceso, n);/////////////////////////////////////////////////////////////////
    worker_data.pid         = getpid();
    worker_data.args        = args;
    worker_data.init_time   = time(NULL);
    worker_data.total_time  = -1;
    worker_data.status      = -1;
    worker_data.nro_proceso = nro_proceso;
    worker_data.n           = n;
    //printf("||||||| saliendo del worker insert\n");/////////////////////////////////////////////////////////////////
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    ///////  ALERTA BUG DETECTADO  ////  FINAL ZONA EN CUARENTENA  ////  ALERTA BUG DETECTADO  ///////
    //////////////////////////////////////////////////////////////////////////////////////////////////

    /* Creamos un hijo de worker para realizar exec */
    pid_t childpid;
    childpid = fork();

    if (childpid >= 0)  /* El fork se realizó con éxito */
    {
        if (childpid == 0)  /* Proceso hijo */
        {
            //printf("P%i (W): Voy a ejecutar %s\n", nro_proceso, executable);
            execvp(executable, args);
        } else {    /* Proceso padre worker */
            while (waitpid(childpid, &worker_data.status, WNOHANG) == 0){
                sleep(0.5);
            };
            //wait(&worker_data.status); /* wait for child to exit, and store its status */
            worker_data.total_time = time(NULL) - worker_data.init_time;
            printf("P%i (W): Child's exit code is: %d\n",nro_proceso, WEXITSTATUS(worker_data.status));
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
            fprintf(file, "%li,%i,0\n", worker_data.total_time, WEXITSTATUS(worker_data.status));
            // Cerramos el archivo
            fclose(file);
            printf("P%i (W): Archivo %s generado\n", nro_proceso, filename);
        };
    };
    for (int i=0; i<n+1; i++)
    {
        free(args[i]);
    };
    free(args);
};






// SEÑALES
/* Función que maneja las señales de SIGINT para procesos root*/
void signal_sigint_handler_root(int sig){
    printf("   (R): Ha llegado un SIGINT a ROOT\n");
    printf("   (R): Se enviará SIGABRT a todos sus hijos\n");
    if (lista_hijos->hijo == 0){
        printf("   (R): No hay hijos\n");
    }
    else{
        struct lista* p;
        int nro_padre = lista_hijos->nro_padre;
        p = lista_hijos;
        printf("P%i (R): Enviando señal a hijo %i\n", nro_padre, lista_hijos->hijo);
        kill(lista_hijos->hijo, SIGABRT);
        while (p->sig!= NULL){
            p = p->sig;
            printf("P%i (R): Enviando señal a hijo %i\n", nro_padre, p->hijo);
            kill(p->hijo, SIGABRT);
        };
        // Se guarda el archivo de salida
        char type = 'R';
        guardar_archivo(type, nro_padre);
    };
    /* Finalmente le hago abort al proceso root */
    pid_t actual = getpid();
    kill(actual, SIGKILL);
};

/* Función que maneja las señales de SIGINT para procesos no root*/
void signal_sigint_handler_nonroot(int sig){
    printf("      : Ha llegado una señal SIGINT a un M/W\n");
    printf("      : La señal se omite\n");
}

void signal_sigabrt_handler(int sig){
    printf("   (M): Ha llegado una señal SIGABRT a un Manager\n");
    printf("   (M): Se enviará SIGABRT a todos sus hijos\n");
    if (lista_hijos->hijo == 0){
        printf("   (M): No hay hijos\n");
    }
    else{
        struct lista* p;
        int nro_padre = lista_hijos->nro_padre;
        p = lista_hijos;
        printf("P%i (M): Enviando señal a hijo %i\n", nro_padre, lista_hijos->hijo);
        kill(lista_hijos->hijo, SIGABRT);
        while (p->sig!= NULL){
            p = p->sig;
            printf("P%i (M): Enviando señal a hijo %i\n", nro_padre, p->hijo);
            kill(p->hijo, SIGABRT);
        };

        // Se guarda el archivo de salida
        char type = 'M';
        guardar_archivo(type, nro_padre);
    };

    /* Finalmente le hago abort al proceso manager */
    pid_t actual = getpid();
    kill(actual, SIGKILL);
};


//////////////////////////////////////////////////////////////////////////////////////////////////
///////  ALERTA BUG DETECTADO  ////  INICIO ZONA EN CUARENTENA  ////  ALERTA BUG DETECTADO  //////
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//   Este bug consiste en que, independiente desde dónde se haga SIGABRT a un worker, este
//   recibirá la señal sin pasar por el handler, muriendo sin escribir su archivo.
// 
//   A mi entender, este handler se está llamando como cualquier otro, por lo que no entiendo
//   porqué no es utilizado cuando los otros handlers funcionan bien.
// 
//   Probé agregando:
//           signal(SIGABRT, &signal_sigabrt_handler_worker);
//   en varias partes del código, pero no resultó.
//
//////////////////////////////////////////////////////////////////////////////////////////////////
void signal_sigabrt_handler_worker(int sig){
    printf("   (W): Ha llegado un SIGABRT a un WORKER\n");
    // Obtenemos el pid correspondiente
    pid_t wpid = getpid();

    // Avisamos del ABRT
    printf("P%i (W): Abortando worker %i...\n", worker_data.nro_proceso, wpid);

    // Definimos el nombre del archivo de salida
    char filename[10];
    sprintf(filename, "%d.txt", worker_data.nro_proceso);

    // Abrimos el archivo
    FILE* file = fopen(filename, "w");

    // Escribimos el output en el archivo
    for (int i=0; i<worker_data.n+1; i++)
    {
        fprintf(file, "%s,", worker_data.args[i]);
    };
    if (worker_data.total_time != -1)
    {
        fprintf(file, "%li,%i,1\n", worker_data.total_time, WEXITSTATUS(worker_data.status));
    } else
    {
        fprintf(file, "%li,%i,1\n", time(NULL) - worker_data.init_time, WEXITSTATUS(worker_data.status));
    }
        
    // Cerramos el archivo
    fclose(file);
    printf("P%i (W): Archivo %s generado\n", worker_data.nro_proceso, filename);
    kill(wpid, SIGKILL);
};
//////////////////////////////////////////////////////////////////////////////////////////////////
///////  ALERTA BUG DETECTADO  ////  FINAL ZONA EN CUARENTENA  ////  ALERTA BUG DETECTADO  ///////
//////////////////////////////////////////////////////////////////////////////////////////////////


/* Función que inserta valores en la variable global lista_hijos*/
/* Es una lista enlazada. Está definida en el header de funciones*/
void insert(pid_t* hijo, int* nro_proceso, int* nro_padre)
{
    struct lista* q;
    struct lista* p;
    q = malloc(sizeof(struct lista));
    q->sig =         NULL;
    q->hijo =        *hijo;
    q->nro_proceso = *nro_proceso;
    q->nro_padre =   *nro_padre;
    p = lista_hijos;
    if (p == NULL){
        lista_hijos = q;
    }
    else
    {
        while(p->sig != NULL)
        {
            p = p->sig;
        }
        p->sig = q;
    };
};










// GUARDADO DE ARCHIVOS
// Guarda el archivo de salida para procesos Manager y Root
void guardar_archivo(char type, int nro_padre)
{
    // Se define el nombre del archivo de salida
    char filename[10];
    sprintf(filename, "%d.txt", nro_padre);

    // Se crea el archivo
    FILE* file = fopen(filename, "w");

    // Se recorren todos los hijos
    struct lista* p;
    p = lista_hijos;
    while (p != NULL){
        // Se define el nombre del archivo hijo
        char child_filename[10];
        sprintf(child_filename, "%d.txt", p->nro_proceso);

        // Se abre el archivo hijo
        FILE* child_file = fopen(child_filename, "r");

        // Comprobamos que child_file existe
        if(!child_file)
        {
            printf("P%i (%c): No se ha encontrado el archivo %s\n", nro_padre, type, child_filename);
            p = p->sig;
            continue;
        };

        // Se escribe hijo en padre
        char line[300];
        while (fgets(line, 300, child_file) != NULL) 
        { 
            fprintf(file, "%s", line);
        };

        // Cerramos child_file
        fclose(child_file);

        // Avanzamos al siguiente hijo
        p = p->sig;
    };
    printf("P%i (%c): Archivo %s generado\n", nro_padre, type, filename);
    fclose(file);
};






// TIMEOUT
/* Corta un proceso si ya cumplió su timeout */
void* check_timeout(void* timeout)
{
    time_t max_time = *((int*) timeout);

    time_t init_time = time(NULL);

    while (time(NULL) - init_time < max_time);

    printf("P%i    : Se acabó el tiempo para mis hijos (%li segundos)\n", lista_hijos->nro_padre, max_time);
    struct lista* actual;
    actual = lista_hijos;
    while (actual != NULL)
    {
        int result = kill(actual->hijo, SIGABRT);
        if (result == 0)
            printf("P%i    : P%i abortado\n", lista_hijos->nro_padre, lista_hijos->nro_proceso);
        actual = actual->sig;
    };
    free(timeout);
    void* a = NULL;
    return a;
};