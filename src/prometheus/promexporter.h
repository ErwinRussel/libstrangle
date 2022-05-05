#pragma once

int update_buff_ns(nanotime_t value);
int update_buff_fps(int value);
int update_curr_ns(nanotime_t value);
int update_curr_fps(int value);
int update_overhead_ns(nanotime_t value);
int update_achievable_fps(int value);
int incr_violation(int amount);
