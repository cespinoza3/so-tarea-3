#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <semaphore.h>

void my_vfprintf(FILE* f, const char* fmt, va_list args) {
    fprintf(f, "so-tarea-3: ");
    vfprintf(f, fmt, args);
}

void my_fprintf(FILE* f, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    my_vfprintf(f, fmt, args);
    va_end(args);
}

void myreport(int exit_code, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    my_vfprintf(stderr, fmt, args);
    va_end(args);
    exit(exit_code);
}

#define MAX_CLIENTS 10
#define MIN_PRODUCTS 5
#define MAX_PRODUCTS 15

typedef struct IIPair {
    int first;
    int second;
} IIPair;

typedef struct Client {
    IIPair delay_range;

    sem_t can_produce;
} Client;

typedef struct CashRegister {
    int capacity;
    IIPair delay_range;

    int client_count;
    Client clients[MAX_CLIENTS];
    int current_client;

    int conveyer_belt_capacity;
    int conveyer_belt_count;
    int conveyer_belt[MAX_PRODUCTS];

    sem_t can_consume;
} CashRegister;

int random_between(int lower, int upper) {
    return lower + random() % (upper - lower);
}

int random_between_iipair(IIPair pair) {
    return random_between(pair.first, pair.second);
}


int main(int argc, char* argv[]) {

    return 0;
}


