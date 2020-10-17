#pragma once

#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector2.h>

#pragma warning(push)
#pragma warning(disable : 5054)
#include <QKeyEvent>
#pragma warning(pop)

#include <unordered_set>

class InputManager {
public:
    bool isKeyPressed(int key) const;

    void keyPress(int key);
    void keyRelease(int key);

    Magnum::Vector2 mouseDelta(int viewportWidth, int viewportHeight) const;

    void mouseMoveStart(int x, int y);
    void mouseMove(int x, int y);

private:
    std::unordered_set<int> m_pressedKeys;

    Magnum::Vector2 m_mousePrev;
    Magnum::Vector2 m_mouseCur;
};