#pragma once
#include <stdio.h>


char* buscar_linea(const char*, int);
void crear_hijos_manager(char*, char*);
void actualizar(char* );
void signal_sigint_handler_root(int);
void signal_sigint_handler_nonroot(int);
void signal_sigabrt_handler_worker(int);
void signal_sigabrt_handler(int);

struct lista {
    pid_t hijo;
    struct lista* sig;
};


struct lista *creanodo();
void insert(pid_t*);