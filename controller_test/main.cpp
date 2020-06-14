#include <Input.h>
#include <thread>
#include <iostream>
int main()
{
    Input input;
    using namespace std::chrono_literals;
    std::cout << "setup" << std::endl;
    float angle = -0.8;
    while (1)
    {
        input.SetWheelAngle(angle);
        angle += 0.00001;
        std::cout << angle << '\r';
    }
	return 0;
}