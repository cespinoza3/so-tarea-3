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


typedef struct CliArgs CliArgs;
struct CliArgs {
    int number_registers;
    int number_clients;
    int max_clients_per_register;
    int max_products_per_client;
};
CliArgs parse_argv(int, char*[]);

int main(int argc, char* argv[]) {
    CliArgs args = parse_argv(argc, argv);
    return 0;
}


static const char* usage_string = ""
"so-tarea-3 [ <numero cajas> ] [ <numero clientes> ] [ <max clientes por caja> ] [ <max productos por cliente> ]"
"\n" "\n"
"Opciones:"
"\n"
"  <numero cajas>               -  N° de cajas en la simulación [por defecto: 2]"
"\n"
"  <numero clientes>            -  N° de clientes en la simulación [por defecto: 20]"
"\n"
"  <max clientes por caja>      -  N° máximo de clientes que se atenderán por caja [por defecto: 10]"
"\n"
"  <max productos por cliente>  -  N° máximo de productos que tendrá un cliente [por defecto: 20]"
"\n"
"";

CliArgs parse_argv(int argc, char* argv[]) {
    CliArgs args = {
        2,
        20,
        10,
        20
    };

    switch (argc) {
        case 5:
            args.max_products_per_client = atoi(argv[4]);
        case 4:
            args.max_clients_per_register = atoi(argv[3]);
        case 3:
            args.number_clients = atoi(argv[2]);
        case 2:
            args.number_registers = atoi(argv[1]);
        case 1:
            break;
        default:
            myreport(-1, usage_string);
    }

    if (args.number_clients == 0 || args.number_registers == 0 || args.max_clients_per_register == 0 || args.max_products_per_client == 0) {
        myreport(-1, ""
        "error en valores dados:\n"
        "numero cajas: %d\n"
        "numero clientes: %d\n"
        "max clientes por caja: %d\n"
        "max productos por cliente: %d\n"
        "", args.number_registers, args.number_clients, args.max_clients_per_register, args.max_products_per_client);
    }

    return args;
}


