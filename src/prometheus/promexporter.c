#include <stdio.h>
#include "../limiter.h"
#include "promexporter.h"
#include <unistd.h>

#include "microhttpd.h"
#include "prom.h"
#include "promhttp.h"

prom_gauge_t *strangle_gauge;

__attribute__((constructor)) void init (void);
__attribute__((destructor)) void fini (void);

struct MHD_Daemon *glob_d;

const char *labels[] = { "pid", "value" };

const char *cur_labels[] = {NULL, NULL};

char pid_str[6];

// integer gauge
int gauge_i(int v, const char *label) {
    cur_labels[1] = label;
    return prom_gauge_set(strangle_gauge, v, cur_labels);
}

// long long gauge
int gauge_ll(long long v, const char *label) {
    cur_labels[1] = label;
    return prom_gauge_set(strangle_gauge, v, cur_labels);
}

// start gauge
void gauge_init(void) {
    int pid = getpid();
    sprintf(pid_str, "%d", pid);
    cur_labels[0] = pid_str;
    strangle_gauge = prom_collector_registry_must_register_metric(prom_gauge_new("libstrangle", "Stats for libstrangle", 2, labels));
}

// Static public functions

// update buffer in nanoseconds
int update_buff_ns(nanotime_t value) {
    int r = 0;
    r = gauge_ll(value,  "buffer_nanoSeconds");
    if(r) printf("PROMCLIENT: Could not update buffer_NanoSeconds gauge\n");
    return r;
}

// update buffer in fps
int update_buff_fps(int value){
    int r = 0;
    r = gauge_i(value,  "buffer_FPS");
    if(r) printf("PROMCLIENT: Could not update buffer_FPS gauge\n");
    return r;
}

// Frame duration / sleepTime
int update_curr_ns(nanotime_t value){
    int r = 0;
    r = gauge_ll(value,  "sleepTime_nanoSeconds");
    if(r) printf("PROMCLIENT: Could not update SleepTime_NanoSeconds gauge\n");
    return r;
}

// Current fps
int update_curr_fps(int value){
    int r = 0;
    r = gauge_i(value,  "current_FPS");
    if(r) printf("PROMCLIENT: Could not update curr_FPS gauge\n");
    return r;
}

// How much overhead there is in nanoseconds
int update_overhead_ns(nanotime_t value){
    int r = 0;
    r = gauge_ll(value,  "overhead_nanoSeconds");
    if(r) printf("PROMCLIENT: Could not update Overhead_NanoSeconds gauge\n");
    return r;
}

// How much overhead in fps (achievable fps)
int update_achievable_fps(int value){
    int r = 0;
    r = gauge_i(value,  "achievable_FPS");
    if(r) printf("PROMCLIENT: Could not update achievable_FPS gauge\n");
    return r;
}

void init(void) {
    printf("PROMCLIENT: Loaded!\n");
    // Initialize the Default registry
    prom_collector_registry_default_init();

    // Register file-based metrics for each file
    gauge_init();

    // Set the active registry for the HTTP handler
    promhttp_set_active_collector_registry(NULL);

    struct MHD_Daemon *daemon = promhttp_start_daemon(MHD_USE_SELECT_INTERNALLY, 6463, NULL, NULL);

    glob_d = daemon;

    if (daemon == NULL) {
        printf("PROMCLIENT: Daemon not found!\n");
    }

}
//
void fini() {
    printf("PROMCLIENT: shutting down...\n");
    fflush(stdout);
    prom_collector_registry_destroy(PROM_COLLECTOR_REGISTRY_DEFAULT);
    if (glob_d == NULL) {
        printf("PROMCLIENT: Daemon not found!\n");
    }
    MHD_stop_daemon(glob_d);
}
