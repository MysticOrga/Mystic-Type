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

/**
* @brief Maximum number of entities in the ECS system.
*/
const int MAX_ENTITIES = 5000;

/**
* @brief Maximum number of component types in the ECS system.
*/
const int MAX_COMPONENTS = 32;

/**
* @brief Entity identifier type.
*/
using Entity = uint32_t;

/**
* @brief Bitset representing which component types an entity has.
*/
using Signature = std::bitset<MAX_COMPONENTS>;

/**
* @class IComponentArray
* @brief Abstract base class for type-erased component storage.
*
* Provides a common interface for different component array types.
*/
class IComponentArray {
public:
    virtual ~IComponentArray() = default;
};

/**
* @class ComponentArray
* @brief Template for storing components of a specific type.
*
* @tparam T The component type to store.
*/
template <typename T>
class ComponentArray : public IComponentArray {
public:
    std::vector<T> data;        /**< Actual component data */
    std::vector<Entity> entities; /**< Associated entity IDs */

    /**
    * @brief Adds or updates a component for an entity.
    * @param e The entity ID.
    * @param component The component to add.
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
    * @brief Retrieves a component for an entity.
    * @param e The entity ID.
    * @return Reference to the component.
    */
    T& get(Entity e) {
        return data[e];
    }
};

/**
* @class ECS
* @brief Entity Component System implementation.
*
* Manages entities, components, and their associations using a data-oriented architecture.
*/
class ECS {
public:
    /**
    * @brief Constructor initializing entity signatures.
    */
    ECS() {
        signatures.resize(MAX_ENTITIES);
    }

    /**
    * @brief Creates a new entity.
    * @return The ID of the newly created entity.
    */
    Entity createEntity() {
        static Entity next = 0;
        return next++;
    }

    /**
    * @brief Adds a component to an entity.
    * @tparam T The component type.
    * @param e The entity ID.
    * @param c The component instance.
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
    * @brief Retrieves a component from an entity.
    * @tparam T The component type.
    * @param e The entity ID.
    * @return Reference to the component.
    */
    template<typename T>
    T& getComponent(Entity e) {
        return static_cast<ComponentArray<T>*>(storages[typeid(T)].get())->get(e);
    }

    /**
    * @brief Checks if an entity has a specific component type.
    * @tparam T The component type.
    * @param e The entity ID.
    * @return True if entity has component, false otherwise.
    */
    template<typename T>
    bool hasComponent(Entity e) const {
        auto idx = const_cast<ECS*>(this)->getComponentTypeIndex<T>();
        if (e >= signatures.size())
            return false;
        return signatures[e].test(idx);
    }

    /**
    * @brief Retrieves all components of a specific type.
    * @tparam T The component type.
    * @return Reference to the vector of all components of this type.
    */
    template<typename T>
    std::vector<T>& getAllComponents() {
        return static_cast<ComponentArray<T>*>(storages[typeid(T)].get())->data;
    }

    /**
    * @brief Gets the type index for a component type.
    * @tparam T The component type.
    * @return Unique type index for this component.
    */
    template<typename T>
    size_t getComponentTypeIndex() {
        static size_t index = nextComponentIndex++;
        return index;
    }

    /**
    * @brief Gets the signature of an entity.
    * @param e The entity ID.
    * @return The entity's component signature.
    */
    Signature getSignature(Entity e) const {
        return signatures[e];
    }

private:
    std::vector<Signature> signatures; /**< Entity component signatures */
    std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>> storages; /**< Component storages */
    static inline size_t nextComponentIndex = 0; /**< Next available component type index */
};
