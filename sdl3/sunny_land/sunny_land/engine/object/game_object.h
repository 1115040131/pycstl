#pragma once

#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

#include <spdlog/spdlog.h>

#include "common/noncopyable.h"
#include "sunny_land/engine/component/component.h"

namespace pyc::sunny_land {

/**
 * @brief 游戏对象类，负责管理游戏对象的组件。
 *
 * 该类管理游戏对象的组件，并提供添加、获取、检查和移除组件的功能。
 * 它还提供更新和渲染游戏对象的方法。
 */
class GameObject final : Noncopyable {
public:
    ///< @brief 构造函数。默认名称为空，标签为空
    GameObject(std::string_view name = "", std::string_view tag = "");

    // setters and getters
    void setName(std::string_view name) { name_ = name; }                 ///< @brief 设置名称
    std::string_view getName() const { return name_; }                    ///< @brief 获取名称
    void setTag(std::string_view tag) { tag_ = tag; }                     ///< @brief 设置标签
    std::string_view getTag() const { return tag_; }                      ///< @brief 获取标签
    void setNeedRemove(bool need_remove) { need_remove_ = need_remove; }  ///< @brief 设置是否需要删除
    bool isNeedRemove() const { return need_remove_; }                    ///< @brief 获取是否需要删除

    /**
     * @brief 添加组件 (里面会完成组件的init())
     *
     * @tparam T 组件类型
     * @tparam Args 组件构造函数参数类型
     * @param args 组件构造函数参数
     * @return 组件指针
     */
    template <typename T, typename... Args>
    T* addComponent(Args&&... args) {
        // 检测组件是否合法。
        static_assert(std::is_base_of<Component, T>::value, "T 必须继承自 Component");

        // 如果组件已经存在，则直接返回组件指针
        if (hasComponent<T>()) {
            return getComponent<T>();
        }

        // 如果不存在则创建组件
        auto type_index = std::type_index(typeid(T));
        auto new_component = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = new_component.get();                        // 先获取裸指针以便返回
        new_component->setOwner(this);                       // 设置组件的拥有者
        components_[type_index] = std::move(new_component);  // 移动组件
        ptr->init();                                         // 初始化组件 （因此必须用ptr而不能用new_component）
        spdlog::debug("GameObject::addComponent: {} added component {}", name_, typeid(T).name());
        return ptr;  // 返回非拥有指针
    }

    /**
     * @brief 获取组件
     *
     * @tparam T 组件类型
     * @return 组件指针
     */
    template <typename T>
    T* getComponent() const {
        static_assert(std::is_base_of<Component, T>::value, "T 必须继承自 Component");
        auto type_index = std::type_index(typeid(T));
        auto it = components_.find(type_index);
        if (it != components_.end()) {
            // 返回unique_ptr的裸指针。(肯定是T类型, static_cast其实并无必要，但保留可以使我们意图更清晰)
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }

    /**
     * @brief 检查是否存在组件
     *
     * @tparam T 组件类型
     * @return 是否存在组件
     */
    template <typename T>
    bool hasComponent() const {
        static_assert(std::is_base_of<Component, T>::value, "T 必须继承自 Component");
        return components_.contains(std::type_index(typeid(T)));
    }

    /**
     * @brief 移除组件
     *
     * @tparam T 组件类型
     */
    template <typename T>
    void removeComponent() {
        static_assert(std::is_base_of<Component, T>::value, "T 必须继承自 Component");
        auto type_index = std::type_index(typeid(T));
        auto it = components_.find(type_index);
        if (it != components_.end()) {
            it->second->clean();
            components_.erase(it);
        }
    }

    // 关键循环函数
    void handleInput(Context& context);                                      ///< @brief 处理输入
    void update(std::chrono::duration<float> delta_time, Context& context);  ///< @brief 更新所有组件
    void render(Context& context);                                           ///< @brief 渲染所有组件
    void clean();                                                            ///< @brief 清理所有组件

private:
    std::string name_;                                                            ///< @brief 名称
    std::string tag_;                                                             ///< @brief 标签
    std::unordered_map<std::type_index, std::unique_ptr<Component>> components_;  ///< @brief 组件列表
    bool need_remove_ = false;  ///< @brief 延迟删除的标识，将来由场景类负责删除
};

}  // namespace pyc::sunny_land