#include <condition_variable>
#include <chrono>
#include <iostream>
#include <mutex>
#include <stack>
#include <thread>

// Global Variables
std::mutex mtx;
bool updated_fire_time = false;
bool fired_propulsion = false;
int fire_time_sec = -1;
std::stack<int> Commands;
std::stack<int> emptyStack;

// Manager that is in charge of updating firing time as well as firing the propulsion
void fire_propulsion_manager() {
    auto start_time = std::chrono::steady_clock::now();
    while (true) {
        // If fire time has been updated or propulsion has been fired
        if (updated_fire_time || fired_propulsion)
        {
            // Verify Commands stack is not empty
            if(!Commands.empty())
            {
                // Set fire time to the top of the stack
                std::unique_lock<std::mutex> lock(mtx);
                fire_time_sec = Commands.top();
                start_time = std::chrono::steady_clock::now();
                // If fire time is -1, remove from stack
                if (Commands.top() == -1)
                {
                    Commands.pop();
                }
                //std::cout << "Time changed to: " << fire_time_sec << std::endl; // Used for debuggin purposes
                updated_fire_time = false;
                fired_propulsion = false;
                lock.unlock();
            }
        }
        
        // Sleep for 0.1 seconds until its time to fire propulsion
        if (std::chrono::steady_clock::now() - start_time < std::chrono::seconds(fire_time_sec))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        // Continue and do not fire if there are no outstanding fire commands
        if (fire_time_sec == -1)
        {
            continue;
        }

        //std::cout << "Fire Time: " << fire_time_sec << std::endl; // Used for debugging purposes
        std::cout << "firing now!"<< std::endl;
        fired_propulsion = true;
        fire_time_sec = -1;
        
        // Lock mutex to remove fire time from the top of stack
        std::unique_lock<std::mutex> lock(mtx);
        Commands.pop();
        lock.unlock();
    }
}

int main() {
    // Add Fire Propulsion manager to a different thread
    std::thread manager_thread(fire_propulsion_manager);

    int input;
    while (std::cin >> input) {
        // Lock mutex to update Commands stack and update_fire_time to prevent race condition
        std::lock_guard<std::mutex> lock(mtx);
        if (input == -1)
        {
            Commands.swap(emptyStack);
        }
        
        Commands.push(input);
        updated_fire_time = true;
    }

    manager_thread.join();
    return 0;
}
