#include <stdio.h>
#include <sched.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include "latency_test.h"
#include "micro_test.h"

int merge_lat_data(std::string type, std::string op, int thread_num, int coro_num) {
    std::string directoryPath = "results/";
    std::string outputFileName = type + "_" + op + "_lat" + ".txt";
    std::ofstream output(directoryPath + outputFileName);

    if (!output.is_open()) {
        std::cout << "cannot open output file" << std::endl;
        return 1;
    }
    
    std::map<int, int> latencyCountMap;
    for (int i = 0; i < thread_num; i++) {
        for (int j = 0; j < coro_num; j++) {
            std::string fileName = type + "_" + op + "_lat_" + std::to_string(i) + "_" + std::to_string(j) + ".txt";
            std::ifstream input(directoryPath + fileName);
            
            if (!input.is_open()) {
                std::cout << "cannot open input file: " << fileName << std::endl;
                continue;
            }
            
            std::string line;
            while (std::getline(input, line)) {
                int latency = std::stoi(line);
                latencyCountMap[latency]++;
            }
            
            input.close();
        }
    }
    
    for (auto it = latencyCountMap.begin(); it != latencyCountMap.end(); ++it) {
        output << it->first << " " << it->second << std::endl;
    }
    
    output.close();
    
    return 0;
}

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
    pthread_barrier_init(&insert_start_barrier, NULL, num_clients + 1);
    pthread_barrier_init(&insert_finish_barrier, NULL, num_clients + 1);
    pthread_barrier_init(&update_start_barrier, NULL, num_clients + 1);
    pthread_barrier_init(&update_finish_barrier, NULL, num_clients + 1);
    pthread_barrier_init(&search_start_barrier, NULL, num_clients + 1);
    pthread_barrier_init(&search_finish_barrier, NULL, num_clients + 1);
    pthread_barrier_init(&delete_start_barrier, NULL, num_clients + 1);
    pthread_barrier_init(&delete_finish_barrier, NULL, num_clients + 1);
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
        client_args_list[i].timer_barrier = NULL;
        client_args_list[i].should_stop   = &should_stop;
        client_args_list[i].ret_num_insert_ops = 0;
        client_args_list[i].ret_num_update_ops = 0;
        client_args_list[i].ret_num_search_ops = 0;
        client_args_list[i].ret_num_delete_ops = 0;
        client_args_list[i].ret_fail_insert_num = 0;
        client_args_list[i].ret_fail_update_num = 0;
        client_args_list[i].ret_fail_search_num = 0;
        client_args_list[i].ret_fail_delete_num = 0;
        client_args_list[i].op_type = op_type;
        pthread_t tid;
        pthread_create(&tid, NULL, run_client_lat, &client_args_list[i]);
        tid_list[i] = tid;
    }

    // 4s insert
    pthread_barrier_wait(&insert_start_barrier);
    usleep(4 * 1000000);
    should_stop = true;
    pthread_barrier_wait(&insert_finish_barrier);

    // 5s search
    pthread_barrier_wait(&search_start_barrier);
    usleep(5 * 1000000);
    should_stop = true;
    pthread_barrier_wait(&search_finish_barrier);

    // 5s update
    pthread_barrier_wait(&update_start_barrier);
    usleep(5 * 1000000);
    should_stop = true;
    pthread_barrier_wait(&update_finish_barrier);

     // 1s delete
    pthread_barrier_wait(&delete_start_barrier);
    usleep(1 * 1000000);
    should_stop = true;
    pthread_barrier_wait(&delete_finish_barrier);


    for (int i = 0; i < num_clients; i ++) {
        pthread_join(tid_list[i], NULL);
    }
    free(client_args_list);

    // merge output files
    std::vector<std::string> op_list = {"insert", "update", "search", "delete"};
    for(auto & op : op_list) {
        merge_lat_data("micro", op, num_clients, 1);
    }
}

int main(int argc, char ** argv) {
    if (argc != 4) {
        printf("Usage: %s path-to-config-file num-threads num-CNs\n", argv[0]);
        return 1;
    }

    int num_clients = atoi(argv[2]);
    int num_CNs = atoi(argv[3]);

    int ret = 0;
    GlobalConfig config;
    ret = load_config(argv[1], &config);
    assert(ret == 0);

    // cpu_set_t cpuset;
    // CPU_ZERO(&cpuset);
    // CPU_SET(config.main_core_id, &cpuset);
    // ret = sched_setaffinity(0, sizeof(cpuset), &cpuset);
    // assert(ret == 0);
    // ret = sched_getaffinity(0, sizeof(cpuset), &cpuset);
    // assert(ret == 0);
    // for (int i = 0; i < sysconf(_SC_NPROCESSORS_CONF); i ++) {
    //     if (CPU_ISSET(i, &cpuset)) {
    //         printf("main process running on core: %d\n", i);
    //     }
    // }

    // Client client(&config);

    // ret = test_insert_lat(client);
    // assert(ret == 0);

    // ret = test_search_lat(client);
    // assert(ret == 0);

    // ret = test_update_lat(client);
    // assert(ret == 0);

    // ret = test_delete_lat(client);
    // assert(ret == 0);

    start_client_threads("INSERT", num_clients, num_CNs, &config, argv[1]);

    printf("[END]\n");
}