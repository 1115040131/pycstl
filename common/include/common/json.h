#pragma once

#include <nlohmann/json.hpp>

#include "common/reflect.h"

namespace pyc {

// 按照 nlohmann 指定的 api
template <typename T>
    requires(reflect_trait<T>::has_member())
void to_json(nlohmann::json& root, const T& object) {
    foreach_member(const_cast<T&>(object), [&](const char* key, auto& value) { root[key] = value; });
}

template <typename T>
    requires(reflect_trait<T>::has_member())
void from_json(const nlohmann::json& json, T& output) {
    foreach_member(output, [&](const char* key, auto& value) { json.at(key).get_to(value); });
}

// 通用 api
template <typename T>
    requires(!reflect_trait<T>::has_member())
nlohmann::json toJson(const T& object) {
    return object;
}

template <typename T>
    requires(reflect_trait<T>::has_member())
nlohmann::json toJson(const T& object) {
    nlohmann::json root;
    foreach_member(object, [&](const char* key, auto& value) { root[key] = toJson(value); });
    return root;
}

template <typename T>
    requires(!reflect_trait<T>::has_member())
T fromJson(const nlohmann::json& root) {
    return root;
}

template <typename T>
    requires(reflect_trait<T>::has_member())
T fromJson(const nlohmann::json& root) {
    T object;
    foreach_member(
        object, [&](const char* key, auto& value) { value = fromJson<std::decay_t<decltype(value)>>(root[key]); });
    return object;
}

std::string toString(const nlohmann::json& root) { return root.dump(); }
nlohmann::json fromString(const std::string& json) { return nlohmann::json::parse(json); }

template <typename T>
std::string serialize(const T& object) {
    return toString(toJson(object));
}

template <typename T>
T deserialize(const std::string& json) {
    return fromJson<T>(fromString(json));
}

}  // namespace pyc
