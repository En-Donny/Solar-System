#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <gl/GL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shaders.h"
#include "stb_image.h"
#include <iostream>
#include <vector>
#include <math.h>

//параметры камеры по умолчанию
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 100.0f;
const float SENSITIVITY = 0.05f;
const float ZOOM = 45.0f;

class Camera {
public:
    // јтрибуты камеры
     bool firstEditVectors;
    glm::vec3 position;
    glm::vec3 BasePosition;

    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;
    // углы Ёйлера
    float yaw;
    float pitch;
    // Ќастройки камеры

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
	Camera(glm::vec3 cameraPos):position(cameraPos), BasePosition(cameraPos), front(glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f) - position)),worldUp(glm::vec3(0.0f, 1.0f, 0.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
        yaw = YAW;
        pitch = PITCH;
        firstEditVectors = true;
        editCameraVectors();
	}

    glm::mat4 GetLookAtMatrix() {
        return glm::lookAt(position, position + front, up);
    }

    void CameraKeyBoardMoving(int k,float deltaTime,bool firstMouseMoving) {
        float speedMoving = MovementSpeed * deltaTime;
        if (k == 1){
            if (firstMouseMoving) position += 0.5f * speedMoving * front / sqrt(position.z * position.z + position.y * position.y);
            else position += 0.5f * speedMoving * front;
        }
        if (k==2){
            if (firstMouseMoving) position -= 0.5f * speedMoving * front / sqrt(position.z * position.z + position.y * position.y);
            else position -= 0.5f * speedMoving * front;
        }
        if (k==3){
            position -= 0.5f * glm::normalize(glm::cross(front,up)) * speedMoving ;
        }
        if (k==4){
            position += 0.5f * glm::normalize(glm::cross(front,up)) * speedMoving ;
        }
        if (k == 5) {
            position += 0.4f * worldUp ;
        }
        if (k == 6) {
            position -= 0.4f * worldUp;
        }
    }


    void ControlMouseMoving(float xoffset, float yoffset,bool firstCalling, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        yaw -= xoffset;
        if (firstCalling) {//если движение мышки самое первое - направл€ем камеру под другим углом ровно в  точку (0;0;0)
            pitch = -glm::degrees(acos((sqrt(position.x * position.x + position.z * position.z)) / (sqrt(position.x * position.x + position.z * position.z + position.y * position.y))));
        }
        else pitch -= yoffset;

        // ”беждаемс€, что когда тангаж выходит за пределы обзора, экран не переворачиваетс€
        if (constrainPitch)
        {
            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;
        }
        // ќбновл€ем значени€ вектора-пр€мо, вектора-вправо и вектора-вверх, использу€ обновленные значени€ углов Ёйлера
        editCameraVectors();
    }

    void ProcessMouseScroll(float yoffset)
    {
        if (Zoom >= 1.0f && Zoom <= 90.0f)
            Zoom -= yoffset;
        if (Zoom <= 1.0f)
            Zoom = 1.0f;
        if (Zoom >= 90.0f)
            Zoom = 90.0f;
    }

    void lookAtPlanet(glm::vec3 PlanetCoord,float radius) {//изменение параментров камеры дл€ того,чтобы посмотреть на планету
        position = PlanetCoord- glm::vec3(0.0f, -0.6f*radius, -5.0f*radius);
        pitch = -glm::degrees(acos((sqrt((position.x-PlanetCoord.x) * (position.x - PlanetCoord.x) + (position.z - PlanetCoord.z) * (position.z - PlanetCoord.z))) / (sqrt((position.x - PlanetCoord.x) * (position.x - PlanetCoord.x) + (position.z - PlanetCoord.z) * (position.z - PlanetCoord.z) + (position.y - PlanetCoord.y) * (position.y - PlanetCoord.y)))));
        yaw = -glm::degrees(asin((sqrt((position.z - PlanetCoord.z) * (position.z - PlanetCoord.z))) / (sqrt((position.x - PlanetCoord.x) * (position.x - PlanetCoord.x) + (position.z - PlanetCoord.z) * (position.z - PlanetCoord.z) ))));
        editCameraVectors();
    }

    void getBaseLook() {
        position = BasePosition;
        front = glm::vec3(0.0f, 0.0f, 0.0f) - position;
        pitch = -glm::degrees(acos((sqrt(position.x * position.x + position.z * position.z)) / (sqrt(position.x * position.x + position.z * position.z + position.y * position.y))));
        yaw = -90.0f;
        editCameraVectors();

    }

private:
    void editCameraVectors() {
        if (firstEditVectors) {
            front = glm::vec3(0.0f, 0.0f, 0.0f) - position;
            firstEditVectors = false;
        }
        else {  
         glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(direction);
        }
        right = glm::cross(front, worldUp);
        up = glm::cross( right,front);
    }
};