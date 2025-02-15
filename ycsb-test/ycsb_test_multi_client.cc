#include <stdio.h>
#include <stdlib.h>

#include <atomic>

#include "client.h"
#include "ycsb_test.h"

int main(int argc, char ** argv) {
    if (argc != 4) {
        printf("Usage: %s path-to-config-file workload-name num-clients\n", argv[0]);
        return 1;
    }

    WorkloadFileName * workload_fnames = get_workload_fname(argv[2]);
    int num_clients = atoi(argv[3]);

    GlobalConfig config;
    int ret = load_config(argv[1], &config);
    assert(ret == 0);

    // bind this process to main core
    // run client args
    RunClientArgs * client_args_list = (RunClientArgs *)malloc(sizeof(RunClientArgs) * num_clients);
    pthread_barrier_t global_load_barrier;
    pthread_barrier_init(&global_load_barrier, NULL, num_clients);
    pthread_barrier_t global_timer_barrier;
    pthread_barrier_init(&global_timer_barrier, NULL, num_clients);
    volatile bool should_stop = false;

    pthread_t tid_list[num_clients];
    
    for (int i = 0; i < num_clients; i ++) {
        printf("create client %d\n", i);
        client_args_list[i].client_id     = config.server_id - config.memory_num;
        client_args_list[i].thread_id     = i;
        client_args_list[i].main_core_id  = i * 2;
        client_args_list[i].poll_core_id  = i * 2 + 1;
        client_args_list[i].workload_name = argv[2];
        client_args_list[i].config_file   = argv[1];
        client_args_list[i].load_barrier  = &global_load_barrier;
        client_args_list[i].should_stop   = &should_stop;
        client_args_list[i].timer_barrier = &global_timer_barrier;
        client_args_list[i].ret_num_ops = 0;
        client_args_list[i].ret_faile_num = 0;
        client_args_list[i].num_threads = num_clients;
        pthread_t tid;
        pthread_create(&tid, NULL, run_client, &client_args_list[i]);
        tid_list[i] = tid;
    }

    uint32_t total_tpt = 0;
    uint32_t total_failed = 0;
    uint64_t total_search_num[5] = {0};
    for (int i = 0; i < num_clients; i ++) {
        pthread_join(tid_list[i], NULL);
        total_tpt += client_args_list[i].ret_num_ops;
        total_failed += client_args_list[i].ret_faile_num;
        for (int j = 0; j < 5; j++) {
            total_search_num[j] += client_args_list[i].req_search_num[j];
        }
    }

    printf("total: %d ops\n", total_tpt);
    printf("failed: %d ops\n", total_failed);
    printf("total tpt: %d\n", (total_tpt - total_failed) / config.workload_run_time);   // (ops/s)
    for (int j = 0; j < 5; j++)
        printf("search%d: %lu ops\n", j, total_search_num[j]);
    printf("[END]\n");
}