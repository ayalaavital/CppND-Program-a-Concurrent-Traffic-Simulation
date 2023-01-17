#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> mLock(_mutex);
    _condition.wait(mLock, [this] { return !_queue.empty(); });
    T msg = std::move(_queue.front());
    _queue.pop_front();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> mLock(_mutex);
    _queue.clear();
    _queue.emplace_back(std::move(msg));
    _condition.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    while (true) {
        TrafficLightPhase phase = std::move(_messageQ.receive());
        if (phase == TrafficLightPhase::green)
            return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

double TrafficLight::getRandCycleDuration()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(4000.0, 6000.0); // duration of a single cycle in ms
    double cycleDuration = dis(gen);

    return cycleDuration;
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    double cycleDuration = getRandCycleDuration();
    auto lastUpdate = std::chrono::system_clock::now();
    while (true)
    {
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now() - lastUpdate).count();
        if (timeSinceLastUpdate >= cycleDuration)
        {
            _currentPhase = _currentPhase == TrafficLightPhase::red ?
                            TrafficLightPhase::green : TrafficLightPhase::red;
            _messageQ.send(std::move(getCurrentPhase()));
            lastUpdate = std::chrono::system_clock::now();
            cycleDuration = getRandCycleDuration();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}