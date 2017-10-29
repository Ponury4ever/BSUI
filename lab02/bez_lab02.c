#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>

pthread_mutex_t mutex;


typedef struct
{
    unsigned int id;
    int money;
    bool isFree;
    pthread_cond_t cond;
} bankAccount;

typedef struct
{
    bankAccount* src;
    bankAccount* target;
    int numberOfMoney;
} moneyTransfer;

typedef struct
{
    bankAccount* target;
    int numberOfMoney;
} addMoney;

bankAccount* createAccount(int id, int money)
{
    bankAccount* account = (bankAccount*)malloc(sizeof(bankAccount));
    account->id = id;
    account->money = money;
    account->isFree = true;
    pthread_cond_init(&(account->cond), NULL);

    return account;
}


moneyTransfer* createTransfer(bankAccount* src, bankAccount* target, int numberOfMoney)
{
    moneyTransfer* transfer = (moneyTransfer*)malloc(sizeof(moneyTransfer));
    transfer->src = src;
    transfer->target = target;
    transfer->numberOfMoney = numberOfMoney;

    return transfer;
}

addMoney* createAdder(bankAccount* target, int numberOfMoney)
{
    addMoney* add = (addMoney*)malloc(sizeof(addMoney));
    add->target = target;
    add->numberOfMoney = numberOfMoney;

    return add;
}

void* addToAccount(void* arg)
{

    addMoney* money = (addMoney*)arg;
    bankAccount* target = money->target;

    for (int i = 0; i < 6; i++)
    {

        if (pthread_mutex_init(&mutex, NULL) != 0)
        {
            printf("\n mutex init failed\n");
            return 0;
        }
        while (true)
        {
            if (target->isFree)
            {
                target->isFree = false;
                pthread_mutex_unlock(&mutex);
                break;
            }
            else
            {
                pthread_cond_wait(&(target->cond), &mutex);
            }
        }

        int cache = target->money;
        usleep(rand() % 1000);
        target->money = cache + money->numberOfMoney;
        printf("Dodaje %d pln do konta nr:  %d \nStan konta: %d \n\n", money->numberOfMoney,
            target->id, target->money);


        pthread_mutex_lock(&mutex);
        target->isFree = true;
        pthread_cond_signal(&(target->cond));
        pthread_mutex_unlock(&mutex);
    }
}

void* transferMoney(void* arg)
{
    moneyTransfer* transfer = (moneyTransfer*)arg;
    bankAccount* src = transfer->src;
    bankAccount* target = transfer->target;

    for (int i = 0; i < 6; i++)
    {

        if (pthread_mutex_init(&mutex, NULL) != 0)
        {
            printf("\n mutex init failed\n");
            return 0;
        }

        if (src->id < target->id)
        {

            while (true)
            {
                if (src->isFree)
                {
                    src->isFree = false;
                    pthread_mutex_unlock(&mutex);
                    break;
                }
                else
                {
                    pthread_cond_wait(&(src->cond), &mutex);
                }
            }


            while (true)
            {
                if (target->isFree)
                {
                    target->isFree = false;
                    pthread_mutex_unlock(&mutex);
                    break;
                }
                else
                {
                    pthread_cond_wait(&(target->cond), &mutex);
                }
            }
        }
        else
        {
            while (true)
            {
                if (src->isFree)
                {
                    src->isFree = false;
                    pthread_mutex_unlock(&mutex);
                    break;
                }
                else
                {
                    pthread_cond_wait(&(src->cond), &mutex);
                }
            }

            while (true)
            {
                if (target->isFree)
                {
                    target->isFree = false;
                    pthread_mutex_unlock(&mutex);
                    break;
                }
                else
                {
                    pthread_cond_wait(&(target->cond), &mutex);
                }
            }
        }
        int srcCache = src->money;
        int targetCache = target->money;
        usleep(rand() % 1000);
        src->money = srcCache - transfer->numberOfMoney;
        target->money = targetCache + transfer->numberOfMoney;
        printf("\n przelew: %d \n", transfer->numberOfMoney);
        printf("Z konta nr:  %d  --> stan konta: %d \n", src->id, src->money);
        printf("Do konta nr:  %d --> stan konta: %d \n", target->id, target->money);
        sem_post(&(target->sem));
        sem_post(&(src->sem));
    }

    pthread_mutex_unlock(&lock);
}

int main()
{
    int i;
    time_t t;
    i = time(&t);
    srand(i);

    int threadNumber = 5;

    pthread_t addFirstAccountThread[threadNumber];
    pthread_t addSecondAccountThread[threadNumber];
    // pthread_t transferFirstThread[threadNumber];
    // pthread_t transferSecondThread[threadNumber];

    int returnThread;

    bankAccount* firstAccount = createAccount(1, 0);
    bankAccount* secondAccount = createAccount(2, 0);

    addMoney* addToFirst = createAdder(firstAccount, 100);
    addMoney* addToSecond = createAdder(secondAccount, 100);

    // moneyTransfer* fromFirstTransfer = createTransfer(firstAccount, secondAccount, 50);
    // moneyTransfer* fromSecondTransfer = createTransfer(secondAccount, firstAccount, 150);

    for (int i = 0; i < threadNumber; i++)
    {
        pthread_create(&(addFirstAccountThread[i]), NULL, addToAccount, (void*)addToFirst);


        pthread_create(&(addSecondAccountThread[i]), NULL, addToAccount, (void*)addToSecond);


        // pthread_create(&(transferFirstThread[i]), NULL, transferMoney, (void*)fromFirstTransfer);


        // pthread_create(&(transferSecondThread[i]), NULL, transferMoney,
        // (void*)fromSecondTransfer);
    }

    for (int i = 0; i < threadNumber; i++)
    {
        pthread_join(addFirstAccountThread[i], NULL);

        pthread_join(addSecondAccountThread[i], NULL);

        // pthread_join(transferFirstThread[i], NULL);

        // pthread_join(transferSecondThread[i], NULL);
    }


    printf("\n====================KONIEC====================\n");
    printf("Konto nr:  %d  --> stan konta: %d \n", firstAccount->id, firstAccount->money);
    printf("Konto nr:  %d  --> stan konta: %d \n", secondAccount->id, secondAccount->money);


    pthread_mutex_destroy(&mutex);

    exit(EXIT_SUCCESS);
}
