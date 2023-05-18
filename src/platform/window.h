#pragma once

namespace platform
{
class Window
{
public:
    Window();
    ~Window();

    void updateWindowSize(long width, long height);
    long getWidth() { return width; }
    long getHeight() { return height; }
private:
    long width;
    long height;
};
}