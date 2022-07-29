#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <semaphore.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <limits.h>

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

void my_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    my_fprintf(stdout, fmt, args);
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
int max_products_per_register;

double min_delay_register;
double max_delay_register;
double min_delay_client;
double max_delay_client;



typedef struct Client {
    int index;
    DPair delay_range;
    int current_product;
    int product_amount;

    pthread_mutex_t mutex;
    sem_t on_register;
    sem_t can_produce;
    sem_t produced_product;
    int done;
} Client;

typedef struct CashRegister {
    int number_of_clients_attended;
    int index;
    int capacity;
    DPair delay_range;

    Client* current_client;

    int conveyer_belt_capacity;
    int conveyer_belt_count;
    int conveyer_belt[MAX_PRODUCTS];

    int queued_clients;

    pthread_mutex_t mutex;
    sem_t new_client;
    sem_t can_attend;
    sem_t can_queue;
} CashRegister;

void CashRegister_change_queued_clients_by(CashRegister*, int);

int Client_get_total_done_clients();
int Client_products_left_count(Client*);

CashRegister* cash_registers;
Client* clients;
pthread_t* cash_register_threads; 
pthread_t* client_threads; 

// == CashRegister

CashRegister* CashRegister_init(CashRegister* self, int index) {
    sem_init(&self->can_queue, 0, number_clients);
    sem_init(&self->can_attend, 0, 0);
    sem_init(&self->new_client, 0, 0);
    self->conveyer_belt_capacity = max_products_per_client;
    pthread_mutex_init(&self->mutex, NULL);
    self->index = index;

    return self;
}

void* cash_register_thread_function(void* data) {

}



void CashRegister_attend(CashRegister* self) {

}
void CashRegister_change_queued_clients_by(CashRegister* self, int diff) {
    pthread_mutex_lock(&self->mutex);
    self->queued_clients += diff;
    pthread_mutex_unlock(&self->mutex);
}
// end

// == Client

Client* Client_init(Client* self, int index) {
    sem_init(&self->can_produce, 0, 0);
    sem_init(&self->produced_product, 0, 0);
    sem_init(&self->on_register, 0, 0);
    self->product_amount = random_between(1, max_products_per_client);
    pthread_mutex_init(&self->mutex, NULL);
    self->index = index;

    return self;
}

void* client_thread_function(void* data) {

}

int Client_products_left_count(Client* self) {
    return self->product_amount - self->current_product;
}

void Client_produce_products(Client* self, CashRegister* reg) {
}

void Client_get_on_queue(Client* self, CashRegister* reg) {

}

void Client_find_cash_register(Client* self) {


}

int Client_get_total_done_clients() {
    int s = 0;
    for (int i = 0; i < number_clients; ++i) {
        s += clients[i].done;
    }
    return s;
}

// end


typedef struct CliArgs CliArgs;
struct CliArgs {
    int number_registers;
    int number_clients;
    int max_clients_per_register;
    int max_products_per_client;
    int max_products_per_register;
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
    max_products_per_register = args.max_products_per_register;
    min_delay_register = args.min_delay_register;
    max_delay_register = args.max_delay_register;
    min_delay_client = args.min_delay_client;
    max_delay_client = args.max_delay_client;

    srandom(time(NULL));

    cash_registers = calloc(sizeof(CashRegister), number_registers);
    clients = calloc(sizeof(Client), number_clients);

    for (int i = 0; i < cash_registers; ++i) {
        CashRegister_init(&cash_registers[i], i);
    }

    for (int i = 0; i < clients; ++i) {
        Client_init(&clients[i], i);
    }

    cash_register_threads = calloc(sizeof(pthread_t), number_registers);
    client_threads = calloc(sizeof(pthread_t), number_clients);

    for (size_t i = 0; i < number_registers; i++)
    {
        if (pthread_create(&cash_register_threads[i], NULL, cash_register_thread_function, &cash_registers[i])) {
            myreport(-1, "no se pudo crear hilo\n");
        }
    }

    for (size_t j = 0; j < number_clients; j++)
    {
        if (pthread_create(&client_threads[i], NULL, client_thread_function, &clients[i])) {
            myreport(-1, "no se pudo crear hilo\n");
        }
    }
    


    return 0;
}


static const char* usage_string = ""
"so-tarea-3 [ <numero cajas> ] [ <numero clientes> ] [ <max clientes por caja> ] [ <max productos por cliente> ] [ <max productos por caja> ] [ <min demora caja> ] [ <max demora caja> ] [ <min demora cliente> ] [ <max demora cliente> ]"
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
"  <max productos por caja>  -  N° máximo de productos que tendrá una caja [por defecto: 10]"
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
        10,
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
        case 10:
            args.max_delay_client = atof(argv[10]);
            EXIT_AND_HELP(10);
        case 9:
            args.min_delay_client = atof(argv[9]);
            EXIT_AND_HELP(9);
        case 8:
            args.max_delay_register = atof(argv[8]);
            EXIT_AND_HELP(8);
        case 7:
            args.min_delay_register = atof(argv[7]);
            EXIT_AND_HELP(7);
        case 6:
            args.max_products_per_register = atoi(argv[5]);
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

    if (args.number_clients == 0 || args.number_registers == 0 || args.max_clients_per_register == 0 || args.max_products_per_client == 0 || args.max_products_per_register == 0
         || args.min_delay_register == 0 || args.max_delay_register == 0 || args.min_delay_client == 0 || args.max_delay_client == 0) {
        myreport(-1, ""
        "error en valores dados:\n"
        "numero cajas: %d\n"
        "numero clientes: %d\n"
        "max clientes por caja: %d\n"
        "max productos por cliente: %d\n"
        "max productos por caja: %d\n"
        "min demora caja: %f\n"
        "max demora caja: %f\n"
        "min demora cliente: %f\n"
        "max demora cliente: %f\n\n"
        "%s"
        "", args.number_registers, args.number_clients, args.max_clients_per_register, args.max_products_per_client, args.max_products_per_register, 
        args.min_delay_register, args.max_delay_register, args.min_delay_client, args.max_delay_client, usage_string);
    }

    return args;
}


