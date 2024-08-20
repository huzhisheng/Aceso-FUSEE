#include <stdio.h>
#include <stdlib.h>

#include <stdio.h>
#include <stdlib.h>

#include <atomic>

#include "client.h"
#include "micro_test.h"

static void start_client_threads(char * op_type, int num_clients, int num_CNs, GlobalConfig * config, 
        char * config_fname) {
    MicroRunClientArgs * client_args_list = (MicroRunClientArgs *)malloc(sizeof(MicroRunClientArgs) * num_clients);
    pthread_barrier_t insert_start_barrier;
    pthread_barrier_t insert_finish_barrier;
    pthread_barrier_t update_start_barrier;
    pthread_barrier_t update_finish_barrier;
    pthread_barrier_t search_start_barrier;
    pthread_barrier_t search_finish_barrier;
    pthread_barrier_t delete_start_barrier;
    pthread_barrier_t delete_finish_barrier;
    pthread_barrier_t global_timer_barrier;
    pthread_barrier_init(&insert_start_barrier, NULL, num_clients + 1);
    pthread_barrier_init(&insert_finish_barrier, NULL, num_clients + 1);
    pthread_barrier_init(&update_start_barrier, NULL, num_clients + 1);
    pthread_barrier_init(&update_finish_barrier, NULL, num_clients + 1);
    pthread_barrier_init(&search_start_barrier, NULL, num_clients + 1);
    pthread_barrier_init(&search_finish_barrier, NULL, num_clients + 1);
    pthread_barrier_init(&delete_start_barrier, NULL, num_clients + 1);
    pthread_barrier_init(&delete_finish_barrier, NULL, num_clients + 1);
    pthread_barrier_init(&global_timer_barrier, NULL, num_clients + 1);
    volatile bool should_stop = false;

    pthread_t tid_list[num_clients];
    for (int i = 0; i < num_clients; i ++) {
        client_args_list[i].client_id    = config->server_id - config->memory_num;
        client_args_list[i].thread_id    = i;
        client_args_list[i].num_threads  = num_clients;
        client_args_list[i].num_CNs = num_CNs;
        client_args_list[i].main_core_id = config->main_core_id + i * 2;
        client_args_list[i].poll_core_id = config->poll_core_id + i * 2;
        client_args_list[i].config_file   = config_fname;
        client_args_list[i].insert_start_barrier= &insert_start_barrier;
        client_args_list[i].insert_finish_barrier= &insert_finish_barrier;
        client_args_list[i].update_start_barrier= &update_start_barrier;
        client_args_list[i].update_finish_barrier= &update_finish_barrier;
        client_args_list[i].search_start_barrier= &search_start_barrier;
        client_args_list[i].search_finish_barrier= &search_finish_barrier;
        client_args_list[i].delete_start_barrier= &delete_start_barrier;
        client_args_list[i].delete_finish_barrier= &delete_finish_barrier;
        client_args_list[i].timer_barrier = &global_timer_barrier;
        client_args_list[i].should_stop   = &should_stop;
        client_args_list[i].ret_num_insert_ops = 0;
        client_args_list[i].ret_num_update_ops = 0;
        client_args_list[i].ret_num_search_ops = 0;
        client_args_list[i].ret_num_delete_ops = 0;
        client_args_list[i].ret_fail_insert_num = 0;
        client_args_list[i].ret_fail_update_num = 0;
        client_args_list[i].ret_fail_search_num = 0;
        client_args_list[i].ret_fail_delete_num = 0;
        client_args_list[i].ret_cas_insert_num = 0;
        client_args_list[i].ret_cas_update_num = 0;
        client_args_list[i].ret_cas_search_num = 0;
        client_args_list[i].ret_cas_delete_num = 0;
        client_args_list[i].op_type = op_type;
        pthread_t tid;
        pthread_create(&tid, NULL, run_client, &client_args_list[i]);
        tid_list[i] = tid;
    }

    // 4s insert
    pthread_barrier_wait(&insert_start_barrier);
    usleep(2 * 1000000);
    should_stop = true;
    pthread_barrier_wait(&insert_finish_barrier);

    // 10s search
    pthread_barrier_wait(&search_start_barrier);
    usleep(10 * 1000000);
    should_stop = true;
    pthread_barrier_wait(&search_finish_barrier);

    // 5s update
    pthread_barrier_wait(&update_start_barrier);
    usleep(5 * 1000000);
    should_stop = true;
    pthread_barrier_wait(&update_finish_barrier);

     // 1s delete
    pthread_barrier_wait(&delete_start_barrier);
    usleep(1 * 500000);
    should_stop = true;
    pthread_barrier_wait(&delete_finish_barrier);


    uint64_t total_insert_tpt = 0;
    uint64_t total_insert_failed = 0;
    uint64_t total_insert_cas_tpt = 0;
    uint64_t total_update_tpt = 0;
    uint64_t total_update_failed = 0;
    uint64_t total_update_cas_tpt = 0;
    uint64_t total_search_tpt = 0;
    uint64_t total_search_failed = 0;
    uint64_t total_search_cas_tpt = 0;
    uint64_t total_delete_tpt = 0;
    uint64_t total_delete_failed = 0;
    uint64_t total_delete_cas_tpt = 0;
    uint64_t total_search_num[5] = {0};
    for (int i = 0; i < num_clients; i ++) {
        pthread_join(tid_list[i], NULL);
        total_insert_tpt += client_args_list[i].ret_num_insert_ops;
        total_update_tpt += client_args_list[i].ret_num_update_ops;
        total_search_tpt += client_args_list[i].ret_num_search_ops;
        total_delete_tpt += client_args_list[i].ret_num_delete_ops;
        total_insert_failed += client_args_list[i].ret_fail_insert_num;
        total_update_failed += client_args_list[i].ret_fail_update_num;
        total_search_failed += client_args_list[i].ret_fail_search_num;
        total_delete_failed += client_args_list[i].ret_fail_delete_num;
        total_insert_cas_tpt += client_args_list[i].ret_cas_insert_num;
        total_update_cas_tpt += client_args_list[i].ret_cas_update_num;
        total_search_cas_tpt += client_args_list[i].ret_cas_search_num;
        total_delete_cas_tpt += client_args_list[i].ret_cas_delete_num;
        for (int j = 0; j < 5; j++) {
            total_search_num[j] += client_args_list[i].req_search_num[j];
        }
    }
    printf("insert total: %lld ops\n", total_insert_tpt);
    printf("insert failed: %lld ops\n", total_insert_failed);
    printf("insert tpt: %lld\n", (total_insert_tpt - total_insert_failed) * 1000 / 2000);   // (ops/s)
    printf("insert cas: %lld\n", (total_insert_cas_tpt) * 1000 / 2000);   // (ops/s)
    
    printf("update total: %lld ops\n", total_update_tpt);
    printf("update failed: %lld ops\n", total_update_failed);
    printf("update tpt: %lld\n", (total_update_tpt - total_update_failed) * 1000 / 5000);   // (ops/s)
    printf("update cas: %lld\n", (total_update_cas_tpt) * 1000 / 5000);   // (ops/s)
    
    printf("search total: %lld ops\n", total_search_tpt);
    printf("search failed: %lld ops\n", total_search_failed);
    printf("search tpt: %lld\n", (total_search_tpt - total_search_failed) * 1000 / 10000);   // (ops/s)
    printf("search cas: %lld\n", (total_search_cas_tpt) * 1000 / 10000);   // (ops/s)
    
    printf("delete total: %lld ops\n", total_delete_tpt);
    printf("delete failed: %lld ops\n", total_delete_failed);
    printf("delete tpt: %lld\n", (total_delete_tpt - total_delete_failed) * 1000 / 500);   // (ops/s)
    printf("delete cas: %lld\n", (total_delete_cas_tpt) * 1000 / 500);   // (ops/s)
    
    for (int j = 0; j < 5; j++)
        printf("search%d: %lu ops\n", j, total_search_num[j]);

    free(client_args_list);
    printf("[END]\n");
}

int main(int argc, char ** argv) {
    if (argc != 4) {
        printf("Usage: %s path-to-config-file num-threads num-CNs\n", argv[0]);
        return 1;
    }

    int num_clients = atoi(argv[2]);
    int num_CNs = atoi(argv[3]);

    GlobalConfig config;
    int ret = load_config(argv[1], &config);
    assert(ret == 0);

    start_client_threads("INSERT", num_clients, num_CNs, &config, argv[1]);   
}