#include "TimeManager.h"

float TimeManager::deltaTime() const {
    return m_deltaTime;
}

void TimeManager::init() {
    m_time.start();
    m_oldTime = m_time.elapsed();
    m_deltaTime = 0.0f;
}

void TimeManager::update() {
    m_currentTime = m_time.elapsed();
    m_deltaTime = (m_currentTime - m_oldTime) / 1000.0f;
    m_oldTime = m_currentTime;
}
