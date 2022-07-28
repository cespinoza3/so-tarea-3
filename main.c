#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

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


