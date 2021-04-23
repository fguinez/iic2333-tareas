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
    char** args;
    time_t init_time;
    time_t total_time;
    int status;
    struct worker_data* sig;
};

struct worker_data *creanodo2();

struct lista {
    pid_t hijo;
    struct lista* sig;
};


struct lista *creanodo();
void insert(pid_t*);
void insert_worker(pid_t*, char***, time_t*, time_t*, int*);