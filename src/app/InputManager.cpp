#include "InputManager.h"

using namespace Magnum;

bool InputManager::isKeyPressed(int key) const {
    auto it = m_pressedKeys.find(key);
    return it != m_pressedKeys.end();
}

void InputManager::keyPress(int key) {
    m_pressedKeys.insert(key);
}

void InputManager::keyRelease(int key) {
    m_pressedKeys.erase(key);
}

Magnum::Vector2 InputManager::mouseDelta(int viewportWidth, int viewportHeight) const {
    return (m_mouseCur - m_mousePrev) / Vector2(viewportWidth, viewportHeight);
}

void InputManager::mouseMoveStart(int x, int y) {
    m_mousePrev = Vector2(x, y);
    m_mouseCur = Vector2(x, y);
}

void InputManager::mouseMove(int x, int y) {
    m_mousePrev = m_mouseCur;
    m_mouseCur = Vector2(x, y);
}