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
#include "List.h"
#include <vector>
using namespace std;


const int Y_SEGMENTS = 30;
const int X_SEGMENTS = 30;
const int Y_SEGMENTS_METEOR = 4;
const int X_SEGMENTS_METEOR = 4;
const GLfloat PI = 3.14159265358979323846f;


vector<float> initsphereVertices(int Y_SEGMENTS, int X_SEGMENTS) {
	vector<float> sphereVertices;
	for (int y = 0; y <= Y_SEGMENTS; y++)
	{
		for (int x = 0; x <= X_SEGMENTS; x++)
		{
			float xSegment = (float)x / (float)X_SEGMENTS;
			float ySegment = (float)y / (float)Y_SEGMENTS;
			float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
			float yPos = std::cos(ySegment * PI) ;
			float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
			sphereVertices.push_back(xPos);//пихаем в вектор соответсвующие координаты сферы
			sphereVertices.push_back(yPos);//
			sphereVertices.push_back(zPos);//
			sphereVertices.push_back(xSegment);//здесь начинаем кидать в вектор координаты текстуры
			sphereVertices.push_back(ySegment);//
		}
	}
	return sphereVertices;
}


vector<int> initsphereIndices(int Y_SEGMENTS, int X_SEGMENTS) {
	vector<int> sphereIndices;
	for (int i = 0; i < Y_SEGMENTS; i++)
	{
		for (int j = 0; j < X_SEGMENTS; j++)
		{
			sphereIndices.push_back(i * (X_SEGMENTS +1) + j);
			sphereIndices.push_back((i + 1) * (X_SEGMENTS +1) + j);
			sphereIndices.push_back((i + 1) * (X_SEGMENTS +1) + j + 1);
			sphereIndices.push_back(i * (X_SEGMENTS+1 ) + j);
			sphereIndices.push_back((i + 1) * (X_SEGMENTS +1) + j + 1);
			sphereIndices.push_back(i * (X_SEGMENTS+1 ) + j + 1);
		}
	}
	return sphereIndices;
}

//unsigned int MeteorTextureLoad();





//unsigned int MeteorTexture = MeteorTextureLoad();

//const vector<float> sphereVertices = initsphereVertices(Y_SEGMENTS,  X_SEGMENTS);
//const vector<int> sphereIndices = initsphereIndices(Y_SEGMENTS, X_SEGMENTS);
//
//const vector<float> meteorVertices = initsphereVertices(Y_SEGMENTS_METEOR, X_SEGMENTS_METEOR);
//const vector<int> meteorIndices = initsphereIndices(Y_SEGMENTS_METEOR, X_SEGMENTS_METEOR);

class Asteroid {
	

public:

	Asteroid() :radius(0), rasst(0), name("Default"), baseCoordinates(glm::vec3(0.0f, 0.0f, 0.0f)), position(glm::vec3(0.0f, 0.0f, 0.0f )), speedRotate(0),ugol(0),texture(0), OsRotateSpeed(0),angleNakl(0) {}
	Asteroid(float r, float ras, string str, glm::vec3 coord,float scorost,bool firstLoadTexture,float OsSpeed,float Naklon ) :radius(r), rasst(ras), name(str), baseCoordinates(coord),position(coord),speedRotate(scorost),OsRotateSpeed(OsSpeed), angleNakl(Naklon){
		
		if (firstLoadTexture) {
			MeteorTextureLoad();
			
		}
		

	

		if (!baseCoordinates.x && !baseCoordinates.z)ugol = 0;
		else ugol = glm::degrees(atan(baseCoordinates.z / baseCoordinates.x));


	}
	virtual  ostream& print(ostream& os) {
		return os << "јстероид " + this->name + " радиуса " << this->radius;

	}

	friend  ostream& operator<<(ostream& os, Asteroid& n) {

		return n.print(os);
	}



	void MeteorTextureLoad() {

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		// установка параметров наложени€ текстуры
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // установка метода наложени€ текстуры GL_REPEAT (стандартный метод наложени€)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// ”становка параметров фильтрации текстуры
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// «агрузка изображени€, создание текстуры и генерирование мипмап-уровней
		int width, height, nrChannels;

		unsigned char* data = stbi_load("textures/Planets/meteor.jpg", &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}

		stbi_image_free(data);


	}


	void draw(float time, Shader ourShader,double SpeedMultiplier) {
		// ѕрив€зка текстур к соответствующим текстурным юнитам
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->texture);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, position);
		model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
		ourShader.setMat4("model", model);

		

		this->position.z = this->rasst * cos(-(glm::radians(time* SpeedMultiplier /4) + ugol));
		this->position.x = this->rasst * sin(-(glm::radians(time* SpeedMultiplier /4) + ugol));


		/* glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);*/

		glDrawElements(GL_TRIANGLES, X_SEGMENTS * Y_SEGMENTS * 6, GL_UNSIGNED_INT, 0);

	}

	std::string getName() {
		return this->name;
	}

	float getRadius() {
		return this->radius;
	}

	float getRasstoyanie() {
		return this->rasst;
	}

	unsigned int texture;//текстура планеты

	glm::vec3 baseCoordinates;//координаты начального отображени€ планеты
	glm::vec3 position;//координаты начального отображени€ планеты
	float speedRotate;//скорость вращени€ планеты вокруг —олнца
	float ugol;//начальна€ позици€ планеты 
	float OsRotateSpeed;//скорость вращени€ планеты вокруг своей оси
	float angleNakl;//угол наклона планеты
protected:
	float radius;//радиус планеты
	float rasst;//радиус орбиты вокруг —олнца
	string name;//им€ планеты
};




class Sputnik :public Asteroid {


public:

	Sputnik() :Asteroid() {}


	Sputnik(float  r, float rast, string str, string texturePath, glm::vec3 coord, int speed) :Asteroid(r, rast, str, coord, speed, false,0, 0) {


		glGenTextures(1, &(this->texture));
		glBindTexture(GL_TEXTURE_2D, this->texture);
		// установка параметров наложени€ текстуры
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // установка метода наложени€ текстуры GL_REPEAT (стандартный метод наложени€)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// ”становка параметров фильтрации текстуры
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// «агрузка изображени€, создание текстуры и генерирование мипмап-уровней
		int width, height, nrChannels;

		unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);



	}



	void draw(float time, Shader ourShader,float mainX,float mainZ,  double SpeedMultiplier) {
		// ѕрив€зка текстур к соответствующим текстурным юнитам
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->texture);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, position);

		//model = glm::rotate(model, time / 5, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(radius, radius, radius));
		ourShader.setMat4("model", model);


		this->position.z = mainZ+ this->rasst * cos(glm::radians(time* SpeedMultiplier * speedRotate) + ugol);
		this->position.x = mainX+ this->rasst * sin(glm::radians(time* SpeedMultiplier * speedRotate) + ugol);


		/* glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);*/

		glDrawElements(GL_TRIANGLES, X_SEGMENTS * Y_SEGMENTS * 6, GL_UNSIGNED_INT, 0);

		

	}



};




class Planet :public Asteroid {
	

public:

	Planet():Asteroid() {}


	Planet(float  r, float rast, string str,string texturePath, glm::vec3 coord,float speed,float OsSpeed, float ugol) :Asteroid(r, rast, str,coord, speed,false, OsSpeed, ugol) {
	
	
		glGenTextures(1, &(this->texture));
		glBindTexture(GL_TEXTURE_2D, this->texture);
		// установка параметров наложени€ текстуры
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // установка метода наложени€ текстуры GL_REPEAT (стандартный метод наложени€)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// ”становка параметров фильтрации текстуры
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// «агрузка изображени€, создание текстуры и генерирование мипмап-уровней
		int width, height, nrChannels;

		unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);


	
	}



	void draw(float time,Shader ourShader,int PlanetsMoving,double SpeedMultiplier) {
		// ѕрив€зка текстур к соответствующим текстурным юнитам
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->texture);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, position);
		model = glm::rotate(model, glm::radians(this->angleNakl), glm::vec3(1.0f, 0.0f, 1.0f));

		model = glm::rotate(model, time *OsRotateSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(radius, radius, radius));
		ourShader.setMat4("model", model);

	
		this->position.z = this->rasst * cos(glm::radians( time*speedRotate*PlanetsMoving* SpeedMultiplier)+ugol);
		this->position.x = this->rasst * sin(glm::radians(time * speedRotate * PlanetsMoving* SpeedMultiplier) + ugol);

		   
		 //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glDrawElements(GL_TRIANGLES, X_SEGMENTS * Y_SEGMENTS * 6, GL_UNSIGNED_INT, 0);

		for (int i = 0; i < this->Sputniks.size(); ++i) {
			Sputniks[i].draw(time, ourShader, this->position.x, this->position.z, SpeedMultiplier);
		}

	}

	/*virtual  ostream& print(ostream& os) {
		return os << "ѕланета " + this->name + " радиуса " << this->radius << " " <<this->obit;

	}
	friend  ostream& operator<<(ostream& os, Planet& n) {

		return n.print(os);
	}*/

	
	vector<Sputnik> Sputniks;
};
