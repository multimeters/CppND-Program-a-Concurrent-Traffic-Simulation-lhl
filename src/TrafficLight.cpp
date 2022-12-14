#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> lck(_mutex);
    _cond.wait(lck,[this]{return !_messages.empty();});
    T t=std::move(_messages.back());
    _messages.pop_back();
    return t;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> ulock(_mutex);
    _messages.push_back(std::move(msg));
    _cond.notify_one();
    
}


/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true)
    {
        TrafficLightPhase message = _queue->receive();
        if(message==TrafficLightPhase::green){return;}
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(&TrafficLight::cycleThroughPhases,this);
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    auto t_start=std::chrono::high_resolution_clock::now();
    auto t_end=std::chrono::high_resolution_clock::now();
    bool isFirsCycle=true;
    float t_total=0;

    while(true)
    {   
        if(isFirsCycle!=true){
            t_end=std::chrono::high_resolution_clock::now();
            t_total=std::chrono::duration<float,std::milli>(t_end-t_start).count();
            t_start=std::chrono::high_resolution_clock::now();
            std::cout<<"time between cycles: "<<t_total<<"ms"<<std::endl;  
            std::this_thread::sleep_for(std::chrono::milliseconds(rand()%3000+4000));

            std::lock_guard<std::mutex> lck(_mutex);
            _currentPhase=_currentPhase == TrafficLightPhase::green ? TrafficLightPhase::red:TrafficLightPhase::green;
            
            threads.emplace_back(std::thread(&MessageQueue<TrafficLightPhase>::send, _queue, std::move(_currentPhase)));
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        else{
            t_start=std::chrono::high_resolution_clock::now();
            isFirsCycle=false;
        }
    }
}

