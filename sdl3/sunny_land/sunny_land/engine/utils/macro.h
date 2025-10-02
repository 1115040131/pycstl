#pragma once

#define ASSET_PATH "sdl3/sunny_land/assets/"
#define ASSET(filename) (ASSET_PATH filename)

#define DECLARE_COMPONENT(ComponentType, component_name)                 \
public:                                                                  \
    ComponentType* get##ComponentType() const { return component_name; } \
                                                                         \
private:                                                                 \
    ComponentType* component_name { nullptr }
