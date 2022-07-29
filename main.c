#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <semaphore.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <limits.h>
#include <unistd.h>

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
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&mutex);
    fprintf(f, "so-tarea-3: ");
    vfprintf(f, fmt, args);
    pthread_mutex_unlock(&mutex);
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
    my_vfprintf(stdout, fmt, args);
    va_end(args);
}

void myreport(int exit_code, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    my_vfprintf(stderr, fmt, args);
    va_end(args);
    exit(exit_code);
}

#ifdef DEBUG
    #define DEBUG_PRINT(fmt, ...) my_fprintf(stderr, "DEBUG::%s:%d: " fmt "\n", __func__, __LINE__, __VA_ARGS__)
    #define DEBUG_PRINT1(fmt) DEBUG_PRINT("%s" fmt, "")
#else
    #define DEBUG_PRINT(fmt, ...) ((void)0)
    #define DEBUG_PRINT1(fmt) ((void)0)
#endif

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
    double diff = hi - lo;
    if (diff < 0.00001 && diff > -0.00001) {
        return lo;
    }
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
    DEBUG_PRINT("%d msecs", msecs);
    if (!msecs) return;
    do {
        usleep(msecs % 1000001);
        if (msecs < 1000000) {
            msecs = 0;
        } else {
            msecs -= 1000000;
        }
        DEBUG_PRINT("new msecs %u", msecs);
    } while(msecs);
    DEBUG_PRINT1("finish delay");
}

void delay_between(int lo, int hi) {
    delay(randomf_between(lo, hi));
}

void delay_between_dpair(DPair pair) {
    double the_delay = randomf_between_dpair(pair);
    DEBUG_PRINT("delay from dpair: %f %f = %f", pair.first, pair.second, the_delay);
    delay(the_delay);
}

#define MIN_PRODUCTS 5
#define MAX_PRODUCTS 15

int number_registers = 2;
int number_clients = 20;
int max_clients_per_register = 10;
int max_products_per_client = 20;
int max_products_per_register = 10;

double min_delay_register = 0.5;
double max_delay_register = 2.0;
double min_delay_client = 0.5;
double max_delay_client = 2.0;



typedef struct Client {
    int index;
    DPair delay_range;
    int current_product;
    int product_amount;

    pthread_mutex_t mutex;
    sem_t leaves_register;
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
void CashRegister_attend(CashRegister*);

int Client_get_total_done_clients();
int Client_products_left_count(Client*);
void Client_get_out_supermarket(Client*);

CashRegister* cash_registers;
Client* clients;
pthread_t* cash_register_threads; 
pthread_t* client_threads; 

// == CashRegister

CashRegister* CashRegister_init(CashRegister* self, int index) {
    sem_init(&self->can_queue, 0, number_clients);
    sem_init(&self->can_attend, 0, 1);
    sem_init(&self->new_client, 0, 0);
    self->conveyer_belt_capacity = max_products_per_register;
    pthread_mutex_init(&self->mutex, NULL);
    self->index = index;
    self->delay_range = (DPair){ min_delay_register, max_delay_register };

    return self;
}

void* cash_register_thread_function(void* data) {
    CashRegister_attend((CashRegister*) data);
}


#define CAJA_PRINT(fmt, ...) my_printf("%s:%d\tCAJA %d " fmt "\n", __func__, __LINE__, self->index, __VA_ARGS__)
#define CAJA_PRINT1(str) CAJA_PRINT(str "%s", "")

#ifdef DEBUG
    #define CAJA_DEBUG(fmt, ...) CAJA_PRINT("!!DEBUG!! " fmt, __VA_ARGS__)
    #define CAJA_DEBUG1(fmt) CAJA_DEBUG("%s" fmt, "")
#else
    #define CAJA_DEBUG(fmt, ...) ((void)0)
    #define CAJA_DEBUG1(fmt) ((void)0)
#endif

void CashRegister_attend(CashRegister* self) {
    bool first_client_arrived = false;
    sem_wait(&self->new_client);
    while (self->queued_clients) {
        if (first_client_arrived) {
            sem_wait(&self->new_client);
        }
        first_client_arrived = true;
        Client* client = self->current_client;
        CAJA_PRINT("EMPIEZA A ATENDER CLIENTE %d", client->index);

        self->conveyer_belt_count = 0;
        CAJA_DEBUG1("prepost can_product");
        sem_post(&client->can_produce);
        CAJA_DEBUG1("post can_produce");

        while (Client_products_left_count(client) > 0 && self->conveyer_belt_count < self->conveyer_belt_capacity) {
            CAJA_DEBUG1("prewait prodcued_product");
            sem_wait(&client->produced_product);
            CAJA_DEBUG1("postwait prodcued_product");

            delay_between_dpair(self->delay_range);
            CAJA_PRINT("CONSUME PRODUCTO %d", self->conveyer_belt[self->conveyer_belt_count]);
            self->conveyer_belt_count++;
            sem_post(&client->can_produce);
        }

        self->number_of_clients_attended++;
        CashRegister_change_queued_clients_by(self, -1);
        CAJA_PRINT("LIBERA A CLIENTE %d", client->index);
        sem_post(&self->can_attend);
        sem_post(&self->can_queue);
    }
    CAJA_PRINT1("CIERRA");
}
#undef CAJA_PRINT
#undef CAJA_PRINT1
#undef CAJA_DEBUG

void CashRegister_change_queued_clients_by(CashRegister* self, int diff) {
    pthread_mutex_lock(&self->mutex);
    self->queued_clients += diff;
    pthread_mutex_unlock(&self->mutex);
}
// end

// == Client

#define CLIENT_PRINT(fmt, ...) my_printf("%s:%d CLIENTE %d " fmt "\n", __func__, __LINE__, self->index, __VA_ARGS__)
#define CLIENT_PRINT1(str) CLIENT_PRINT(str "%s", "")
#ifdef DEBUG
    #define CLIENT_DEBUG(fmt, ...) CLIENT_PRINT("!!DEBUG!! " fmt, __VA_ARGS__)
    #define CLIENT_DEBUG1(fmt) CLIENT_DEBUG("%s" fmt, "")
#else
    #define CLIENT_DEBUG(fmt, ...) ((void)0)
    #define CLIENT_DEBUG1(fmt) ((void)0)
#endif
Client* Client_init(Client* self, int index) {
    sem_init(&self->can_produce, 0, 0);
    sem_init(&self->produced_product, 0, 0);
    sem_init(&self->leaves_register, 0, 0);
    self->product_amount = random_between(1, max_products_per_client);
    pthread_mutex_init(&self->mutex, NULL);
    self->index = index;
    self->delay_range = (DPair){ min_delay_client, max_delay_client };

    return self;
}

void* client_thread_function(void* data) {
    Client_get_out_supermarket((Client*) data);
}

int Client_products_left_count(Client* self) {
    return self->product_amount - self->current_product;
}

void Client_produce_products(Client* self, CashRegister* reg) {
    sem_wait(&reg->can_attend);
    reg->current_client = self;
    CLIENT_PRINT("PASA A CAJA %d", reg->index);
    sem_post(&reg->new_client);
    for (int i = 0; i < reg->conveyer_belt_capacity && Client_products_left_count(self) > 0; ++i) {

        sem_wait(&self->can_produce);
        delay_between_dpair(self->delay_range);
        CLIENT_PRINT("PRODUCE PRODUCTO %d", self->current_product);
        reg->conveyer_belt[i] = self->current_product;
        self->current_product++;
        sem_post(&self->produced_product);
    }
    CLIENT_PRINT1("SALE DE LA CAJA");
    sem_post(&self->leaves_register);

}


int compare_queued_clients(const void* p1, const void* p2) {
    int a = *(int*) p1;
    int b = *(int*) p2;
    return cash_registers[a].queued_clients - cash_registers[b].queued_clients;
}

pthread_mutex_t search_for_register_mutex;
CashRegister* Client_find_cash_register(Client* self) {
    pthread_mutex_lock(&search_for_register_mutex);
    CashRegister* reg = NULL;
    int indices[number_registers];
    for (int i = 0; i < number_registers; ++i) indices[i] = i;
    qsort(indices, number_registers, sizeof(int), &compare_queued_clients);

    for (int i = 0; i < number_registers; ++i) {
        reg = &cash_registers[indices[i]];
        if (!sem_trywait(&reg->can_queue)) {
            CashRegister_change_queued_clients_by(reg, 1);
            pthread_mutex_unlock(&search_for_register_mutex);
            return reg;
        }
    }

    reg = &cash_registers[0];
    sem_wait(&reg->can_queue);
    CashRegister_change_queued_clients_by(reg, 1);
    pthread_mutex_unlock(&search_for_register_mutex);
    return &cash_registers[0];
}

void Client_get_out_supermarket(Client* self) {
    CLIENT_PRINT1("ENTRA A SUPERMERCADO");
    while (Client_products_left_count(self) > 0) {
        CLIENT_PRINT1("BUSCA CAJA");
        CashRegister* reg = Client_find_cash_register(self);
        Client_produce_products(self, reg);
    }
    CLIENT_PRINT1("SALE DE SUPERMERCADO");
}

int Client_get_total_done_clients() {
    int s = 0;
    for (int i = 0; i < number_clients; ++i) {
        s += clients[i].done;
    }
    return s;
}

#undef CLIENT_PRINT
#undef CLIENT_PRINT1
#undef CLIENT_DEBUG
#undef CLIENT_DEBUG1
// end


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

struct CliArgs parse_argv(int, char*[]);
void print_cliargs(const struct CliArgs*);

int main(int argc, char* argv[]) {
    struct CliArgs args = parse_argv(argc, argv);
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

    pthread_mutex_init(&search_for_register_mutex, NULL);

    cash_registers = calloc(sizeof(CashRegister), number_registers);
    clients = calloc(sizeof(Client), number_clients);

    for (int i = 0; i < number_registers; ++i) {
        CashRegister_init(&cash_registers[i], i);
    }

    for (int i = 0; i < number_clients; ++i) {
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

    for (size_t i = 0; i < number_clients; i++)
    {
        if (pthread_create(&client_threads[i], NULL, client_thread_function, &clients[i])) {
            myreport(-1, "no se pudo crear hilo\n");
        }
    }

    for (int i = 0; i < number_clients; i++) {
        pthread_join(client_threads[i], NULL);
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

struct CliArgs parse_argv(int argc, char* argv[]) {
    struct CliArgs args;
    
    args.number_registers = number_registers;
    args.number_clients = number_clients;
    args.max_clients_per_register = max_clients_per_register;
    args.max_products_per_client = max_products_per_client;
    args.max_products_per_register = max_products_per_register;
    args.min_delay_register = min_delay_register;
    args.max_delay_register = max_delay_register;
    args.min_delay_client = min_delay_client;
    args.max_delay_client = max_delay_client;

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

void print_cliargs(const struct CliArgs* self) {
    my_printf("CliArgs {\n"
    "number_registers = %d\n"
    "number_clients = %d\n"
    "max_clients_per_register = %d\n"
    "max_products_per_client = %d\n"
    "min_delay_register = %f\n"
    "max_delay_register = %f\n"
    "min_delay_client = %f\n"
    "max_delay_client = %f\n"
    "}\n",
    self->number_registers, 
    self->number_clients, 
    self->max_clients_per_register, 
    self->max_products_per_client, 
    self->min_delay_register, 
    self->max_delay_register, 
    self->min_delay_client, 
    self->max_delay_client);
}

