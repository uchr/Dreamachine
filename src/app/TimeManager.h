#pragma once

#include <QElapsedTimer>

class TimeManager {
public:
    float deltaTime() const;

    void init();
    void update();

private:
    QElapsedTimer m_time;
    float m_deltaTime = 0.0f;
    int m_currentTime = 0;
    int m_oldTime = 0;
};