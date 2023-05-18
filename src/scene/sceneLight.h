#pragma once
#include "platform/utils.h"

namespace scene
{
enum LightType
{
    LIGHT_TYPE_DIRECTIONAL,
    LIGHT_TYPE_POINT,
    LIGHT_TYPE_SPOT,
    LIGHT_TYPE_COUNT
};

struct Light
{
    glm::vec4 data0;
    glm::vec4 data1;
    glm::vec4 data2;
    glm::vec4 data3;
    glm::mat4 transform;

    inline void set_light_direction(glm::vec3 value)
    {
        data0.x = value.x;
        data0.y = value.y;
        data0.z = value.z;
    }

    inline void set_light_position(glm::vec3 value)
    {
        data1.x = value.x;
        data1.y = value.y;
        data1.z = value.z;
    }

    inline void set_light_color(glm::vec3 value)
    {
        data2.x = value.x;
        data2.y = value.y;
        data2.z = value.z;
    }

    inline void set_light_intensity(float value)
    {
        data0.w = value;
    }

    inline void set_light_radius(float value)
    {
        data1.w = value;
    }

    inline void set_light_type(LightType value)
    {
        data3.x = static_cast<float>(value);
    }

    inline void set_light_cos_theta_outer(float value)
    {
        data3.y = value;
    }

    inline void set_light_cos_theta_inner(float value)
    {
        data3.z = value;
    }
};

class SceneLight
{
public:
    void updateLight(Light& light)
    {
        updateMatrix();
        light.transform = matrices.view;
        glm::vec3 lightDirection = glm::normalize(glm::mat3(light.transform) * glm::vec3(0.0f, -1.0f, 0.0f));
        glm::vec3 lightPosition = glm::vec3(light.transform[3][0], light.transform[3][1], light.transform[3][2]);

        light.set_light_radius(lightRadius);
        light.set_light_color(lightColor);
        light.set_light_intensity(lightIntensity);
        light.set_light_type(type);
        light.set_light_direction(-lightDirection);
        light.set_light_position(lightPosition);
        light.set_light_cos_theta_inner(glm::cos(glm::radians(lightConeAngleInner)));
        light.set_light_cos_theta_outer(glm::cos(glm::radians(lightConeAngleOuter)));
    }

    void updateMatrix()
    {
        glm::mat4 rotM = glm::mat4(1.f);
        glm::mat4 transM;

        rotM = glm::rotate(rotM, glm::radians(rotation.x * (flipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
        rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::vec3 translation = position;

        if (flipY)
        {
            translation.y *= -1.0f;
        }
        transM = glm::translate(glm::mat4(1.0f), translation);

        matrices.view = rotM * transM;
    }

    void updateAspectRatio(float aspect)
    {
        matrices.perspective = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 96.0f);
        if (flipY) {
            matrices.perspective[1][1] *= -1.0f;
        }
    }

    void setPosition(glm::vec3 position)
    {
        this->position = position;
    }

    void setRotation(glm::vec3 rotation)
    {
        this->rotation = rotation;
    }

    void rotate(glm::vec3 delta)
    {
        this->rotation += delta;
    }

    void setTranslation(glm::vec3 translation)
    {
        this->position = translation;
    };

    void translate(glm::vec3 delta)
    {
        this->position += delta;
    }

public:
    struct
    {
        glm::mat4 perspective;
        glm::mat4 view;
    } matrices;

    LightType type = LIGHT_TYPE_DIRECTIONAL;
    bool flipY = false;
    glm::vec3 rotation = glm::vec3();
    glm::vec3 position = glm::vec3();
    glm::vec3 lightColor = glm::vec3(1.0f);
    float lightConeAngleInner = 40.0f;
    float lightConeAngleOuter = 50.0f;
    float lightRadius = 1.0f;
    float lightIntensity = 1.0f;
};
}