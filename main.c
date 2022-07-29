#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <semaphore.h>
#include <string.h>
#include <math.h>

inline int max(int a, int b) {
    return (a > b) ? a : b;
}

inline int min(int a, int b) {
    return (a < b) ? a : b;
}

inline int clamp(int value, int lo, int hi) {
    return max(lo, min(hi, value));
}

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

typedef struct DPair {
    double first;
    double second;
} DPair;

typedef struct IIPair {
    int first;
    int second;
} IIPair;

double randomf() {
    return (double) (random() % 1000) / 1000.0;
}

double randomf_between(double lo, double hi) {
    return lo + fmod(randomf(), (hi - lo));
}

double randomf_between_dpair(DPair pair) {
    return randomf_between(pair.first, pair.second);
}

int random_between(int lower, int upper) {
    return lower + random() % (upper - lower);
}

int random_between_iipair(IIPair pair) {
    return random_between(pair.first, pair.second);
}

void delay(double secs) {
    unsigned int msecs = secs * 1000000;
    do {
        usleep(msecs % 1000001);
        msecs -= 1000000;
        if (msecs < 0) msecs = 0;
    } while(msecs);
}

void delay_between(int lo, int hi) {
    delay(randomf_between(lo, hi));
}

void delay_between_dpair(DPair pair) {
    delay(randomf_between_dpair(pair));
}

#define MIN_PRODUCTS 5
#define MAX_PRODUCTS 15

int number_registers;
int number_clients;
int max_clients_per_register;
int max_products_per_client;

double min_delay_register;
double max_delay_register;
double min_delay_client;
double max_delay_client;



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




typedef struct CliArgs CliArgs;
struct CliArgs {
    int number_registers;
    int number_clients;
    int max_clients_per_register;
    int max_products_per_client;
    double min_delay_register;
    double max_delay_register;
    double min_delay_client;
    double max_delay_client;
};
CliArgs parse_argv(int, char*[]);

int main(int argc, char* argv[]) {
    CliArgs args = parse_argv(argc, argv);
    number_registers = args.number_registers;
    number_clients = args.number_clients;
    max_clients_per_register = args.max_clients_per_register;
    max_products_per_client = args.max_products_per_client;
    min_delay_register = args.min_delay_register;
    max_delay_register = args.max_delay_register;
    min_delay_client = args.min_delay_client;
    max_delay_client = args.max_delay_client;

    srandom(time(NULL));


    return 0;
}


static const char* usage_string = ""
"so-tarea-3 [ <numero cajas> ] [ <numero clientes> ] [ <max clientes por caja> ] [ <max productos por cliente> ] [ <min demora caja> ] [ <max demora caja> ] [ <min demora cliente> ] [ <max demora cliente> ]"
"\n" 
"so-tarea-3 ( -h | --help )"
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
"  <min demora caja>  -  Tiempo mínimo de demora de la caja [por defecto: 0.5]"
"\n"
"  <max demora caja>  -  Tiempo máximo de demora de la caja [por defecto: 2.0]"
"\n"
"  <min demora cliente>  -  Tiempo mínimo de demora del cliente [por defecto: 0.5]"
"\n"
"  <max demora cliente>  -  Tiempo máximo de demora del cliente [por defecto: 2.0]"
"\n"
"";

CliArgs parse_argv(int argc, char* argv[]) {
    CliArgs args = {
        2,
        20,
        10,
        20,
        0.5,
        2.0,
        0.5,
        2.0
    };

    #define EXIT_AND_HELP(i) do { \
        if (strcmp(argv[(i)], "--help") == 0 || strcmp(argv[(i)], "-h") == 0) {\
            myreport(0, usage_string);\
        }\
    } while(0)

    switch (argc) {
        case 9:
            args.max_delay_client = atof(argv[8]);
            EXIT_AND_HELP(8);
        case 8:
            args.min_delay_client = atof(argv[7]);
            EXIT_AND_HELP(7);
        case 7:
            args.max_delay_register = atof(argv[6]);
            EXIT_AND_HELP(6);
        case 6:
            args.min_delay_register = atof(argv[5]);
            EXIT_AND_HELP(5);
        case 5:
            args.max_products_per_client = atoi(argv[4]);
            EXIT_AND_HELP(4);
        case 4:
            args.max_clients_per_register = atoi(argv[3]);
            EXIT_AND_HELP(3);
        case 3:
            args.number_clients = atoi(argv[2]);
            EXIT_AND_HELP(2);
        case 2:
            args.number_registers = atoi(argv[1]);
            EXIT_AND_HELP(1);
        case 1:
            break;
        default:
            myreport(-1, usage_string);
    }

    if (args.number_clients == 0 || args.number_registers == 0 || args.max_clients_per_register == 0 || args.max_products_per_client == 0 
        || args.min_delay_register == 0 || args.max_delay_register == 0 || args.min_delay_client == 0 || args.max_delay_client == 0) {
        myreport(-1, ""
        "error en valores dados:\n"
        "numero cajas: %d\n"
        "numero clientes: %d\n"
        "max clientes por caja: %d\n"
        "max productos por cliente: %d\n"
        "min demora caja: %f\n"
        "max demora caja: %f\n"
        "min demora cliente: %f\n"
        "max demora cliente: %f\n\n"
        "%s"
        "", args.number_registers, args.number_clients, args.max_clients_per_register, args.max_products_per_client, 
        args.min_delay_register, args.max_delay_register, args.min_delay_client, args.max_delay_client, usage_string);
    }

    return args;
}


