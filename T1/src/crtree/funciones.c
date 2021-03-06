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
#include <errno.h>


extern char* proceso_global;
extern char* proceso_pointer;
extern struct lista* lista_hijos;
extern struct manager_data manager;
extern struct worker_data worker;
extern FILE* input_stream;

char** hijos;

// Para mostrar procesos en consola, descomentar todas las líneas con '///' en funciones.c y main.c



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
    char type = strsep(&proceso, ",")[0];
    manager.nro_proceso = nro_proceso;
    manager.timeout = atoi(strsep(&proceso, ","));
    int n = atoi(strsep(&proceso, ","));

    /* Crea un array con los hijos*/
    hijos = malloc((n+2) * sizeof(char*));
    for (int i=0; i<n; i++)
    {
        hijos[i] = strsep(&proceso, ",");
        if (i+1 == n)
        {
            strip(hijos[i]);              // Quita \n del último hijo
        };
    };

    manager.init_time = time(NULL);

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
            insert(&childpid, &hijo);
        }
    }

    /* El proceso padre se queda esperando a que todos los hijos terminen*/
    struct lista* actual;
    int continue_wait = 0;
    pid_t cpid;
    do {
        actual = lista_hijos;
        continue_wait = 0;
        while (actual != NULL)
        {
            cpid = waitpid(actual->hijo, NULL, WNOHANG);
            if (cpid == 0)
            {
                continue_wait = 1;
            };
            actual = actual->sig;
        };
        if (time(NULL) - manager.init_time > manager.timeout)
        {
            ///printf("P%i (%c): Se acabó el tiempo para mis hijos (%i segundos)\n", nro_proceso, type, timeout);
            actual = lista_hijos;
            while (actual != NULL)
            {
                kill(actual->hijo, SIGABRT);
                actual = actual->sig;
            };
        };
        
    } while (continue_wait);

    // Guardamos el archivo de salida
    guardar_archivo_manager(type);

    // Liberamos memoria
    free(hijos);
};


void crear_hijo_worker(char* instructions, int nro_proceso){
    // No guardamos primer elemento, siempre será W
    strsep(&instructions, ",");

    // Guardamos el comando a ejecutar
    char* executable = strsep(&instructions, ",");

    // Guardamos la cantidad de argumentos del comando
    int n = atoi(strsep(&instructions, ","));
    
    // Guardamos un array con el ejecutable y los n argumentos
    //char** args;
    worker.args = malloc((n+3) * sizeof(char*));
    int len = strlen(executable);
    worker.args[0] = malloc((len+1) * sizeof(char));
    strcpy(worker.args[0], executable);
    for (int i=1; i<(n+1); i++)
    {
        char* arg = strsep(&instructions, ",");
        len = strlen(arg);
        worker.args[i] = malloc((len+1) * sizeof(char));
        strcpy(worker.args[i], arg);
        if (i == n)
        {
            strip(worker.args[i]);              // Quita \n del último argumento
        }
    };
    worker.args[(n+1)] = NULL;

    // Guardamos datos en struct worker
    worker.pid         = getpid();
    worker.total_time  = -1;
    worker.status      = -1;
    worker.nro_proceso = nro_proceso;
    worker.n           = n;
    
    /* Creamos un hijo de worker para realizar exec */
    pid_t childpid;
    worker.init_time   = time(NULL);
    childpid = fork();

    if (childpid >= 0)  /* El fork se realizó con éxito */
    {
        if (childpid == 0)  /* Proceso hijo */
        {
            int err = execvp(executable, worker.args);
            if (err == -1) {
                int errve = errno;
                printf("%s\n", strerror(errve));

                // Liberamos memoria
                free_worker();

                exit(errve);
            };
            
        } else {    /* Proceso padre worker */
            worker.childpid = childpid;
            
            // Esperamos a que el hijo termine
            esperar_hijo_worker();
            
            ///printf("P%i (W): Child %i exit code is: %d\n",nro_proceso, childpid, WEXITSTATUS(worker.status));

            // Guardamos el archivo de salida
            guardar_archivo_worker(0);
        };
    };
    for (int i=0; i<n+1; i++)
    {
        free(worker.args[i]);
    };
    free(worker.args);
};






// SEÑALES
/* Función que maneja las señales de SIGINT para procesos root*/
void signal_sigint_handler_root(int sig){
    ///printf("   (R): Ha llegado un SIGINT a ROOT\n");
    ///printf("   (R): Se enviará SIGABRT a todos sus hijos\n");
    if (lista_hijos->hijo == 0){
        ///printf("   (R): No hay hijos\n");
    }
    else{
        struct lista* p;
        p = lista_hijos;
        ///printf("P%i (R): Enviando señal a hijo %i\n", manager.nro_proceso, lista_hijos->hijo);
        kill(lista_hijos->hijo, SIGABRT);
        while (p->sig!= NULL){
            p = p->sig;
            ///printf("P%i (R): Enviando señal a hijo %i\n", manager.nro_proceso, p->hijo);
            kill(p->hijo, SIGABRT);
        };
        // Esperamos a los hijos
        esperar_hijos_manager();

        // Se guarda el archivo de salida
        char type = 'R';
        guardar_archivo_manager(type);
    };

    // Liberamos memoria
    free_manager();

    /* Finalmente le hago abort al proceso root */
    pid_t actual = getpid();
    kill(actual, SIGKILL);
};

/* Función que maneja las señales de SIGINT para procesos no root*/
void signal_sigint_handler_nonroot(int sig){
    ///printf("      : Ha llegado una señal SIGINT a un M/W\n");
    ///printf("      : La señal se omite\n");
}

void signal_sigabrt_handler(int sig){
    ///printf("   (M): Ha llegado una señal SIGABRT a un Manager\n");
    ///printf("   (M): Se enviará SIGABRT a todos sus hijos\n");
    if (lista_hijos->hijo == 0){
        ///printf("   (M): No hay hijos\n");
    }
    else{
        struct lista* p;
        p = lista_hijos;
        ///printf("P%i (M): Enviando señal a hijo %i\n", manager.nro_proceso, lista_hijos->hijo);
        kill(lista_hijos->hijo, SIGABRT);
        while (p->sig!= NULL){
            p = p->sig;
            ///printf("P%i (M): Enviando señal a hijo %i\n", manager.nro_proceso, p->hijo);
            kill(p->hijo, SIGABRT);
        };
        // Esperamos a que todos los hijos terminen
        esperar_hijos_manager();

        // Se guarda el archivo de salida
        char type = 'M';
        guardar_archivo_manager(type);
    };

    // Liberamos memoria
    free_manager();

    /* Finalmente le hago abort al proceso manager */
    pid_t actual = getpid();
    kill(actual, SIGKILL);
};


void signal_sigabrt_handler_worker(int sig){
    ///printf("P%i (W): Ha llegado un SIGABRT a un WORKER\n", worker.nro_proceso);
    // Obtenemos el pid correspondiente
    pid_t wpid = getpid();

    // Enviamos SIGABRT al hijo de worker
    ///printf("P%i (W): Abortando proceso %i...\n", worker.nro_proceso, worker.childpid);
    kill(worker.childpid, SIGABRT);

    // Esperamos la respuesta del hijo de worker
    //waitpid(worker.childpid, &worker.status, WUNTRACED);
    esperar_hijo_worker();

    // Guardamos el archivo de salida
    guardar_archivo_worker(1);

    // Liberamos memoria
    free_worker();
    
    kill(wpid, SIGKILL);
};


/* Función que inserta valores en la variable global lista_hijos*/
/* Es una lista enlazada. Está definida en el header de funciones*/
void insert(pid_t* hijo, int* nro_proceso)
{
    struct lista* q;
    struct lista* p;
    q = malloc(sizeof(struct lista));
    q->sig =         NULL;
    q->hijo =        *hijo;
    q->nro_proceso = *nro_proceso;
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





// ESPERA DE HIJOS
// Espera a todos los hijos de un proceso Manager o Root
void esperar_hijos_manager(){
    struct lista* actual;
    int continue_wait = 0;
    pid_t cpid;
    do {
        actual = lista_hijos;
        continue_wait = 0;
        while (actual != NULL)
        {
            cpid = waitpid(actual->hijo, NULL, WNOHANG);
            if (cpid == 0)
            {
                continue_wait = 1;
            };
            actual = actual->sig;
        };
    } while (continue_wait);
}

// espera al hijo de un proceso Worker
void esperar_hijo_worker(){
    int wait_result;
    do {
        wait_result = waitpid(worker.childpid, &worker.status, WNOHANG);
        sleep(1);
        worker.total_time = time(NULL) - worker.init_time;

    } while (wait_result == 0);
};








// GUARDADO DE ARCHIVOS
// Guarda el archivo de salida para procesos Manager y Root
void guardar_archivo_manager(char type)
{
    // Se define el nombre del archivo de salida
    char filename[10];
    sprintf(filename, "%d.txt", manager.nro_proceso);

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
            ///printf("P%i (%c): No se ha encontrado el archivo %s\n", manager.nro_proceso, type, child_filename);
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
    ///printf("P%i (%c): Archivo %s generado\n", manager.nro_proceso, type, filename);
    fclose(file);
};

// Guarda el archivo de salida para procesos Worker
void guardar_archivo_worker(int interrupted)
{
    // Definimos el nombre del archivo de salida
    char filename[10];
    sprintf(filename, "%d.txt", worker.nro_proceso);

    // Abrimos el archivo
    FILE* file = fopen(filename, "w");

    // Escribimos el output en el archivo
    for (int i=0; i<worker.n+1; i++)
    {
        fprintf(file, "%s,", worker.args[i]);
    };
    fprintf(file, "%li,%i,%i\n", worker.total_time, WEXITSTATUS(worker.status), interrupted);

    // Cerramos el archivo
    fclose(file);
    ///printf("P%i (W): Archivo %s generado\n", worker.nro_proceso, filename);
};







// FREE
void free_manager(){
    // Cerramos el archivo de input
    fclose(input_stream);

    // Liberamos memoria
    free(proceso_pointer);
    free(proceso_global);

    struct lista* a;
    struct lista* b;
    a = lista_hijos;
    while (a != NULL)
    {
        b = a;
        a = a->sig;
        free(b);
    };
    free(hijos);
};

void free_worker(){
    // Cerramos el archivo de input
    fclose(input_stream);

    // Liberamos memoria
    free(proceso_pointer);
    free(proceso_global);

    for (int i=0; i<worker.n+1; i++)
    {
        free(worker.args[i]);
    };
    free(worker.args);
};