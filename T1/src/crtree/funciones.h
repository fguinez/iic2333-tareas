#pragma once
#include <stdio.h>


char* buscar_linea(const char*, int);
void crear_hijos_manager(char*, char*, int);
void crear_hijo_worker(char*, int);
void actualizar(char* );
void signal_sigint_handler_root(int);
void signal_sigint_handler_nonroot(int);
void signal_sigabrt_handler_worker(int);
void signal_sigabrt_handler(int);


struct worker_data {
    pid_t pid;
    pid_t childpid;
    char** args;
    time_t init_time;
    time_t total_time;
    int status;
    int nro_proceso;
    int n;
};

struct manager_data {
    time_t init_time;
    time_t timeout;
    int nro_proceso;
};

struct lista {
    pid_t hijo;
    int nro_proceso;
    struct lista* sig;
};


struct lista *creanodo();
void insert(pid_t*, int* nro_proceso);

void esperar_hijos_manager();

void esperar_hijo_worker();

void guardar_archivo_manager(char);

void guardar_archivo_worker(int);

void strip(char*);

void free_manager();

void free_worker();