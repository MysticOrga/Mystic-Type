/*
** EPITECH PROJECT, 2025
** B-OOP-400-RUN-4-1-arcade-quentin.hivanhoe
** File description:
** ProtocolLoader
*/

#pragma once

#include <dlfcn.h>
#include <stdexcept>
#include <string>

namespace Network::ProtocolLayer
{
/**
 * @class ProtocolLoader
 * @brief Dynamic protcol loader from shared libraries
 *
 * This classs provides functionality to land and unload protocol implementations
 *
 */
class ProtocolLoader
{
  public:
    /**
     * @brief Default constructor.
     */
    ProtocolLoader() : _handle(nullptr) {};

    /**
     * @brief Constructor that directly opens the given protocol.
     * @param path The path to the protcol file to laod
     */
    ProtocolLoader(const std::string &path)
    {
        if (!_handle)
            _handle = dlopen(path.c_str(), RTLD_LAZY);
    }

    /**
     * @brief Unmap the protocol hanlde loaded
     */
    ~ProtocolLoader()
    {
        if (_handle)
            dlclose(_handle);
    }

    /**
     * @brief Unmap the protocol handle laoded
     *
     * @param key the key of the handle to delete
     * @throw if `dlclose` failed throw an exception
     */
    void close(std::string key)
    {
        if (_handle)
            dlclose(_handle);
    }

    /**
     * @brief open a shared library
     *
     * @param libName the full path of shared library to open
     * @throw if  `dlopen()`  failed, throw an exception
     */
    void open(const std::string libName)
    {
        if (!_handle)
        {
            if (dlclose(_handle) != 0)
                return;
            _handle = dlopen(libName.c_str(), RTLD_LAZY);
        }
        else
            _handle = dlopen(libName.c_str(), RTLD_LAZY);
    }

  private:
    void *_handle;
};
} // namespace Network::ProtocolLayer