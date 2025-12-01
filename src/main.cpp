/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** main
*/

#include "GraphicClient/GraphicClient.hpp"
#include <iostream>

int main()
{
    GraphicClient client("127.0.0.1", 4242);
    return client.run();
}
