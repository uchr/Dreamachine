#include "InputManager.h"

bool InputManager::isKeyPressed(int key) {
    auto it = pressedKeys.find(key);
    return it != pressedKeys.end();
}

void InputManager::keyPress(int key) {
    pressedKeys.insert(key);
}

void InputManager::keyRelease(int key) {
    pressedKeys.erase(key);
}