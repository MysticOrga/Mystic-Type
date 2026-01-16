/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** Core
*/

/**
 * @file Core.hpp
 * @brief Entity Component System (ECS) implementation.
 *
 * Provides a lightweight ECS framework for managing entities and components.
 * Supports up to 5000 entities with 32 different component types.
 */

#pragma once
#include <bitset>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>

const int MAX_ENTITIES = 5000;    ///< Maximum number of entities in the system
const int MAX_COMPONENTS = 32;    ///< Maximum number of component types

using Entity = uint32_t;                          ///< Entity identifier type
using Signature = std::bitset<MAX_COMPONENTS>;    ///< Component signature for each entity

/**
 * @class IComponentArray
 * @brief Base interface for component storage arrays.
 */
class IComponentArray {
public:
    virtual ~IComponentArray() = default;
};

/**
 * @template ComponentArray
 * @brief Template class for storing components of a specific type.
 *
 * @tparam T The component type to store
 */
template <typename T>
class ComponentArray : public IComponentArray {
public:
    std::vector<T> data;          ///< Component data storage
    std::vector<Entity> entities; ///< Associated entity identifiers

    /**
     * @brief Add or update a component for an entity.
     * @param e Entity identifier
     * @param component The component to add
     */
    void add(Entity e, const T& component) {
        if (e >= data.size()) {
            data.resize(e + 1);
            entities.resize(e + 1);
        }
        data[e] = component;
        entities[e] = e;
    }

    /**
     * @brief Retrieve a component for an entity.
     * @param e Entity identifier
     * @return Reference to the component
     */
    T& get(Entity e) {
        return data[e];
    }
};

/**
 * @class ECS
 * @brief Entity Component System manager.
 *
 * Manages entity creation and component management using a signature-based approach.
 * Each entity has a signature that tracks which components it possesses.
 */
class ECS {
public:
    /**
     * @brief Constructor initializing the ECS system.
     */
    ECS() {
        signatures.resize(MAX_ENTITIES);
    }

    /**
     * @brief Create a new entity.
     * @return The new entity identifier
     */
    Entity createEntity() {
        static Entity next = 0;
        return next++;
    }

    /**
     * @brief Add a component to an entity.
     * @tparam T The component type
     * @param e Entity identifier
     * @param c The component instance to add
     */
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

    /**
     * @brief Retrieve a component from an entity.
     * @tparam T The component type
     * @param e Entity identifier
     * @return Reference to the component
     */
    template<typename T>
    T& getComponent(Entity e) {
        return static_cast<ComponentArray<T>*>(storages[typeid(T)].get())->get(e);
    }

    /**
     * @brief Check if an entity has a specific component.
     * @tparam T The component type
     * @param e Entity identifier
     * @return True if the entity has the component, false otherwise
     */
    template<typename T>
    bool hasComponent(Entity e) const {
        auto idx = const_cast<ECS*>(this)->getComponentTypeIndex<T>();
        if (e >= signatures.size())
            return false;
        return signatures[e].test(idx);
    }

    /**
     * @brief Retrieve all components of a specific type.
     * @tparam T The component type
     * @return Reference to the vector of all components of type T
     */
    template<typename T>
    std::vector<T>& getAllComponents() {
        return static_cast<ComponentArray<T>*>(storages[typeid(T)].get())->data;
    }

    /**
     * @brief Get the type index for a component type (used internally).
     * @tparam T The component type
     * @return The unique index for this component type
     */
    template<typename T>
    size_t getComponentTypeIndex() {
        static size_t index = nextComponentIndex++;
        return index;
    }

    /**
     * @brief Get the signature of an entity.
     * @param e Entity identifier
     * @return The signature indicating which components the entity has
     */
    Signature getSignature(Entity e) const {
        return signatures[e];
    }

private:
    std::vector<Signature> signatures; ///< Entity component signatures
    std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>> storages; ///< Component storage
    static inline size_t nextComponentIndex = 0; ///< Next available component type index
};
