#include "semaphore.hpp"

Semaphore::Semaphore(int _val)
{
    pthread_mutex_init(&m, NULL);
    pthread_cond_init(&c, NULL);
    value = _val;
}

Semaphore::~Semaphore()
{
    pthread_mutex_destroy(&m);
    pthread_cond_destroy(&c);;
    //std::cout << "Sephamore destroyed" << std::endl;
}

int Semaphore::P()
{
    int err;
    if (err = pthread_mutex_lock(&m)!=0)
        return err;
    while (value <= 0)
        if (err = pthread_cond_wait(&c, &m) != 0)
            return err;
    value--;
    if (err = pthread_mutex_unlock(&m) !=0)
        return err;
    return 0;
}

int Semaphore::V()
{
    int err;
    if (err = pthread_mutex_lock(&m) != 0)
        return err;
    value++;
    if(err = pthread_cond_broadcast(&c) != 0)
        return err;
    if (err = pthread_mutex_unlock(&m) != 0)
        return err;
    return 0;
}