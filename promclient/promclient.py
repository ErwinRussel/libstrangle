from prometheus_client import start_http_server, Summary, Gauge
import random
import time
from os.path import exists

file_exists = False
sleep_time = 0
overhead = 0
target_frame_time = 0
FILE = "./strangle.prom"

g_sleep_time = Gauge('sleep_time', 'Sleep of the limiter in nanoseconds')
g_overhead = Gauge('overhead', 'Overhead of the limiter in nanoseconds')
g_target_frame_time = Gauge('target_frame_time', 'Target frame time of the limiter in nanoseconds')
g_fps = Gauge('fps', 'Current frames per second of application')
g_adjusted_sleep_time = Gauge('adjusted_sleep_time', 'Adjusted sleep of the limiter in nanoseconds')
g_achievable_fps = Gauge('achievable_fps', 'Achievable frames per second of application')
g_free_fps = Gauge('free_fps', 'Free frames per second of the limiter')

def process():
    g_sleep_time.set(sleep_time)
    g_overhead.set(overhead)
    g_target_frame_time.set(target_frame_time)
    g_fps.set(int(1000000000 / sleep_time))
    g_adjusted_sleep_time.set(sleep_time)
    if(overhead > 0):
        g_achievable_fps.set(int(1000000000 / overhead))
        g_free_fps.set(int(1000000000 / overhead) - int(1000000000 / sleep_time))

if __name__ == '__main__':
    # Start up the server to expose the metrics.
    start_http_server(8000)
    # Generate some requests.
    file_exists = exists(FILE)
    while True:
        # read from file
        if not file_exists:
            file_exists = exists(FILE)
        else:
            with open(FILE) as file:
                data = file.read().split(",")
                sleep_time = int(data[0])
                overhead = int(data[1])
                target_frame_time = int(data[2])

        process()
        time.sleep(5)

        # // Current fps
        # update_curr_fps((int) (1000000000 / sleepTime));
        # // update buffer in nanoseconds
        # update_buff_ns(sleepTime - overhead);
        # // How much overhead there is in nanoseconds
        # update_overhead_ns(overhead);
        # // How much overhead in fps (achievable fps)
        # update_achievable_fps((int) (1000000000 / overhead));
        # // update buffer in fps
        # update_buff_fps((int) ((1000000000 / overhead) - (1000000000 / sleepTime)));