#pragma once

#include <QKeyEvent>

#include <unordered_set>

class InputManager {
public:
    bool isKeyPressed(int key);

    void keyPress(int key);
    void keyRelease(int key);
private:
    std::unordered_set<int> pressedKeys;
};