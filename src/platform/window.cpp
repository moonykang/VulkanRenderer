#include "platform/window.h"

namespace platform
{
Window::Window()
    : width(0)
    , height(0)
{
    
}

Window::~Window()
{

}

void Window::updateWindowSize(long inWidth, long inHeight)
{
    width = inWidth;
    height = inHeight;
}
}