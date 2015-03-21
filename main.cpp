#include "useful.h"
#include <iostream>
#include <cstdio>

int main(int argc, char** argv)
{
    printf("%s\n", std::string("derp lol {} hello {derp}"_format % 8 % Keyword<std::string>("derp", "bobs")).c_str());
}