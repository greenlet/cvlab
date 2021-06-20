#pragma once

#import <sstream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>


using u_lock = std::unique_lock<std::mutex>;
using g_lock = std::lock_guard<std::mutex>;

#define U_LOCK(mu) u_lock lock(mu);
#define G_LOCK(mu) g_lock lock(mu);
