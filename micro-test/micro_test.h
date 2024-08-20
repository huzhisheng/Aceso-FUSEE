#ifndef DDCKV_MICRO_TEST_H_
#define DDCKV_MICRO_TEST_H_

#include <stdint.h>
#include <pthread.h>

#include "client.h"

typedef struct TagMicroRunClientArgs {
    int thread_id;
    int main_core_id;
    int poll_core_id;
    char * workload_name;
    char * config_file;
    pthread_barrier_t * insert_start_barrier;
    pthread_barrier_t * insert_finish_barrier;
    pthread_barrier_t * update_start_barrier;
    pthread_barrier_t * update_finish_barrier;
    pthread_barrier_t * search_start_barrier;
    pthread_barrier_t * search_finish_barrier;
    pthread_barrier_t * delete_start_barrier;
    pthread_barrier_t * delete_finish_barrier;
    volatile bool * should_stop;
    // bool * timer_is_ready;
    pthread_barrier_t * timer_barrier;

    uint64_t ret_num_insert_ops;
    uint64_t ret_num_update_ops;
    uint64_t ret_num_search_ops;
    uint64_t ret_num_delete_ops;
    uint64_t ret_fail_insert_num;
    uint64_t ret_fail_update_num;
    uint64_t ret_fail_search_num;
    uint64_t ret_fail_delete_num;
    uint64_t ret_cas_insert_num;
    uint64_t ret_cas_update_num;
    uint64_t ret_cas_search_num;
    uint64_t ret_cas_delete_num;

    uint32_t client_id;
    uint32_t num_threads;
    uint32_t num_CNs;
    char * op_type;
    Client * client;
    // for count consumption
    uint64_t per_coro_ops_cnt;
    uint64_t ret_cur_valid_kv_sz;
    uint64_t ret_max_valid_kv_sz;
    uint64_t ret_raw_valid_kv_sz;
    uint64_t req_search_num[5];
} MicroRunClientArgs;

void * run_client(void * _args);
void * run_client_consumption(void * _args);
void * run_client_cr(void * _args);
void * run_client_lat(void * _args);

#endif