#pragma once

namespace platform
{
class InputAdapter
{
public:
    virtual void forward(bool triggered) {}

    virtual void backward(bool triggered) {}

    virtual void left(bool triggered) {}

    virtual void right(bool triggered) {}

    virtual void mouseLButton(bool triggered, float x, float y) {}

    virtual void mouseRButton(bool triggered, float x, float y) {}

    virtual void mouseMButton(bool triggered, float x, float y) {}

    virtual void mouseMove(float x, float y) {}
};

class InputHandler
{
public:
    InputHandler()
        : inputAdapter(&defaultInputAdapter)
    {

    }

    void registerInputAdapter(InputAdapter* newInputAdapter)
    {
        inputAdapter = newInputAdapter;
    }

    void unregisterInputAdapter(InputAdapter* newInputAdapter)
    {
        inputAdapter = &defaultInputAdapter;
    }

public:
    void handleKeyForward(bool triggered)
    {
        inputAdapter->forward(triggered);
    }

    void handleKeyBackward(bool triggered)
    {
        inputAdapter->backward(triggered);
    }

    void handleKeyLeft(bool triggered)
    {
        inputAdapter->left(triggered);
    }

    void handleKeyRight(bool triggered)
    {
        inputAdapter->right(triggered);
    }

    void handleMouseLButton(bool triggered, float x, float y)
    {
        inputAdapter->mouseLButton(triggered, x, y);
    }

    void handleMouseRButton(bool triggered, float x, float y)
    {
        inputAdapter->mouseRButton(triggered, x, y);
    }

    void handleMouseMButton(bool triggered, float x, float y)
    {
        inputAdapter->mouseMButton(triggered, x, y);
    }

    void handleMouseMove(float x, float y)
    {
        inputAdapter->mouseMove(x, y);
    }
private:
    InputAdapter* inputAdapter;
    InputAdapter defaultInputAdapter;
};
}
