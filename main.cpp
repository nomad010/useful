#include "useful.h"
#include <iostream>

int main(int argc, char** argv)
{
    std::cout << "ls"_sys << std::endl;
    std::cout << "PATH"_env << std::endl;
    std::cout << "derp lol {} hello {0}"_format % 4 % 3 % 2 % 1 << std::endl;
}