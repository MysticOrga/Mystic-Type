/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** SceneManager
*/

#include "SceneManager.hpp"

void SceneManager::registerScene(const std::string& name, std::shared_ptr<IScene> scene)
{
    _scenes[name] = scene;
}

bool SceneManager::changeScene(const std::string& name)
{
    auto it = _scenes.find(name);
    if (it == _scenes.end()) {
        return false;
    }

    // Call onExit on the current scene if it exists
    if (_currentScene != nullptr) {
        _currentScene->onExit();
    }

    // Switch to the new scene
    _currentScene = it->second.get();
    _currentScene->onEnter();

    return true;
}

void SceneManager::update(float dt)
{
    if (_currentScene != nullptr) {
        _currentScene->update(dt);
    }
}

void SceneManager::render(float dt)
{
    if (_currentScene != nullptr) {
        _currentScene->render(dt);
    }
}
