import time
from os.path import exists
import os
import requests
import socket
import atexit
import signal

file_exists = False
# sleep_time = 0
# overhead = 0
# target_frame_time = 0
# frame_time = 0
FILE = "./strangle.prom"

# desired fps
# int(1000000000 / target_frame_time)

def process(sleep_time, overhead, target_frame_time, cur_frame_time):
    push_dict = {}
    push_dict['host_name'] = os.getenv('NODENAME')# todo: get_from_env
    push_dict['container_name'] = socket.gethostname()

    adjusted_sleep_time = sleep_time - overhead
    push_dict['sleep_time'] = sleep_time
    push_dict['overhead'] = overhead
    push_dict['adjusted_sleep_time'] = adjusted_sleep_time
    push_dict['target_frame_time'] = target_frame_time

    if(target_frame_time > 0):
        target_fps = 1000000000 / target_frame_time
        push_dict['target_fps'] = target_fps

    if(cur_frame_time > 0):
        fps = 1000000000 / cur_frame_time
        push_dict['fps'] = fps

    if(cur_frame_time > 0 and sleep_time > 0):
        achievable_fps = 1000000000 / (cur_frame_time - sleep_time)
        push_dict['achievable_fps'] = achievable_fps

        if(overhead > 0):
            free_fps = achievable_fps - (1000000000 / sleep_time)
            push_dict['free_fps'] = free_fps

    # todo: put this in a JSON and send to
    try:
        # 172.17.0.1
        r = requests.post('http://172.17.0.1:8001/push_metric', json=push_dict)
        print(f"Status Code: {r.status_code}") # , Response: {r.json()}
    except:
        print("Could not connect to port 8001")

def metric_remove():
    push_dict = {}
    push_dict['host_name'] = os.getenv('NODENAME')# todo: get_from_env
    push_dict['container_name'] = socket.gethostname()
    try:
        r = requests.post('http://172.17.0.1:8001/delete_metric', json=push_dict)
        print(f"Status Code: {r.status_code}") # , Response: {r.json()}
    except:
        print("Could not connect to port 8001")

if __name__ == '__main__':
    atexit.register(metric_remove)
    signal.signal(signal.SIGTERM, metric_remove)
    signal.signal(signal.SIGINT, metric_remove)
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
                cur_frame_time = int(data[3])

                process(sleep_time, overhead, target_frame_time, cur_frame_time)
        time.sleep(5)