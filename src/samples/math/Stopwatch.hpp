#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <iomanip>

class Stopwatch {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point end_time;
    bool is_running;

public:
    Stopwatch() : is_running(false) {}

    void start() {
        start_time = std::chrono::high_resolution_clock::now();
        is_running = true;
    }

    void stop() {
        if (is_running) {
            end_time = std::chrono::high_resolution_clock::now();
            is_running = false;
        }
    }

    double elapsed_ms() const {
        if (is_running) {
            auto current_time = std::chrono::high_resolution_clock::now();
            return std::chrono::duration<double, std::milli>(current_time - start_time).count();
        } else {
            return std::chrono::duration<double, std::milli>(end_time - start_time).count();
        }
    }

    double elapsed_seconds() const {
        return elapsed_ms() / 1000.0;
    }

    void print_elapsed(const std::string& operation_name = "Operation") const {
        double ms = elapsed_ms();
        if (ms < 1000.0) {
            std::cout << operation_name << " completed in " 
                      << std::fixed << std::setprecision(2) << ms << " ms" << std::endl;
        } else {
            std::cout << operation_name << " completed in " 
                      << std::fixed << std::setprecision(3) << elapsed_seconds() << " seconds" << std::endl;
        }
    }
};

// Simple macro for timing function calls
#define TIMED_CALL(name, call) \
    do { \
        Stopwatch _sw; \
        std::cout << "Starting: " << name << "..." << std::endl; \
        _sw.start(); \
        call; \
        _sw.stop(); \
        _sw.print_elapsed(name); \
        std::cout << std::string(50, '-') << std::endl; \
    } while(0)
