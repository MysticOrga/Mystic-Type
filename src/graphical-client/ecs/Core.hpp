/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** Core
*/
#pragma once
#include <bitset>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>

const int MAX_ENTITIES = 5000;
const int MAX_COMPONENTS = 32;

using Entity = uint32_t;
using Signature = std::bitset<MAX_COMPONENTS>;

class IComponentArray {
public:
    virtual ~IComponentArray() = default;
};

template <typename T>
class ComponentArray : public IComponentArray {
public:
    std::vector<T> data;
    std::vector<Entity> entities;

    void add(Entity e, const T& component) {
        if (e >= data.size()) {
            data.resize(e + 1);
            entities.resize(e + 1);
        }
        data[e] = component;
        entities[e] = e;
    }

    T& get(Entity e) {
        return data[e];
    }
};

class ECS {
public:
    ECS() {
        signatures.resize(MAX_ENTITIES);
    }

    Entity createEntity() {
        static Entity next = 0;
        return next++;
    }

    template<typename T>
    void addComponent(Entity e, const T& c) {
        std::type_index type = typeid(T);

        if (!storages.count(type)) {
            storages[type] = std::make_unique<ComponentArray<T>>();
        }
        auto* storage = static_cast<ComponentArray<T>*>(storages[type].get());
        storage->add(e, c);

        signatures[e].set(getComponentTypeIndex<T>());
    }

    template<typename T>
    T& getComponent(Entity e) {
        return static_cast<ComponentArray<T>*>(storages[typeid(T)].get())->get(e);
    }

    template<typename T>
    bool hasComponent(Entity e) const {
        auto idx = const_cast<ECS*>(this)->getComponentTypeIndex<T>();
        if (e >= signatures.size())
            return false;
        return signatures[e].test(idx);
    }

    template<typename T>
    std::vector<T>& getAllComponents() {
        return static_cast<ComponentArray<T>*>(storages[typeid(T)].get())->data;
    }

    template<typename T>
    size_t getComponentTypeIndex() {
        static size_t index = nextComponentIndex++;
        return index;
    }

    Signature getSignature(Entity e) const {
        return signatures[e];
    }

private:
    std::vector<Signature> signatures;
    std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>> storages;
    static inline size_t nextComponentIndex = 0;
};
