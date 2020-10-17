#pragma once

#pragma warning(push)
#pragma warning(disable : 5054)
#include <QElapsedTimer>
#pragma warning(pop)

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