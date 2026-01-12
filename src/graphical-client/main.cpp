/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** main
*/

#include "./GraphicClient/GraphicClient.hpp"
#include <iostream>

int main()
{
    GraphicClient client("10.49.84.165", 4243);
    return client.run();
}
