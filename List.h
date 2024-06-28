#pragma once
#include <iostream>
#include <string>
#include "Planets.h"


using namespace std;

template <typename T>
class nodeList {//класс элемента списка
public:
	T inf;//значение
	nodeList<T>* next;//правый сосед
	nodeList<T>* prev;//левый сосед

	nodeList(T x) {//конструктор дл€ формировани€ элемента списка
		this->inf = x;
		this->prev = NULL;
		this->next = NULL;
	}



	ostream& printNode(ostream& os) {
		return os << this->inf;

	}

	friend	ostream& operator<<(ostream& os, nodeList& n) {

		return n.printNode(os);
	}
};




template <typename T>
class List {//класс дл€ формировани€ самого списка
public:


	List();//конструктор без входных параметров





	T& operator[](int i); /*переопределим обращение к элементам списка через []*/



	int length(); //фнукци€ возварщающа€ длину нашего списка



	void pushElement(T x); //функци€ добавлени€ элемента в конец списка



	void printList();//быстрый вывод списка на экран


	List<T>& operator= (const List<T>& droped) {
		nodeList<T>* t = droped.head;

			while(t){
				nodeList<T>* X = new nodeList<T>(t->inf);
				X->inf = t->inf;
				
				X->next = NULL;
				if (!(this->head) && !(this->tail)) {
					this->head = X;
					X->prev = NULL;

				}
				else {
					(this->tail)->next = X;
					X->prev = this->tail;
				}
				this->tail = X;
				t = t->next;
			}
			return *this;
	}

	string find(T m);//функци€ поиска элементов в списке




	void delNodeAfter(nodeList<T> a);

	void Clear();


	void insertNodeAfter(nodeList<T> a, T m);




	~List();//деструктор списка




private:

	nodeList<T>* head;//указатель на голову списка
	nodeList<T>* tail;//указатель на хвост списка


};








template <typename T>
List<T>::List() {//конструктор без входных параметров
	head = NULL;
	tail = NULL;

}






template <typename T>

T& List<T>:: operator[](int i) {/*переопределим обращение к элементам списка через []*/
	nodeList<T>* t = head;
	int j = 0;
	while (i != j && t) {
		t = t->next;
		++j;
	}
	return (t->inf);
}

template <typename T>

int List<T>::length() {//фнукци€ возварщающа€ длину нашего списка
	nodeList<T>* r = head;
	int size = 0;
	while (r) {
		r = r->next;
		++size;
	}
	return size;
}


template <typename T>


void List<T>::pushElement(T x) {//функци€ добавлени€ элемента в конец списка
	nodeList<T>* X = new nodeList<T>(x);
	X->inf = x;
	X->next = NULL;
	if (!(this->head) && !(this->tail)) {
		this->head = X;
		X->prev = NULL;

	}
	else {
		(this->tail)->next = X;
		X->prev = this->tail;
	}
	this->tail = X;
}



template <typename T>

void List<T>::printList() {//быстрый вывод списка на экран
	if (!head && !tail) {
		cout << "—писок пуст!" << endl;
	}
	else {
		nodeList<T>* p = head;
		while (p) {

			cout << p->inf << " ";
			p = p->next;
		}
	}
}



template <typename T>

void List<T>::Clear() {
	nodeList<T>* h = this->head;
	if (head == tail) {
		head = NULL;
		tail = NULL;
		delete h;
	}

	else{
	while (h) { //пока список не пуст
		nodeList<T>* p = h; //указатель на голову
		h = h->next; //переносим голову
		h->prev = NULL; //обнул€ем
		delete p; //удал€ем p

	}
	}
}


//string List<complexNumber>::find(complexNumber m) {//функци€ поиска элементов в списке
//	nodeList<complexNumber>* p = head;
//	int i = 0;
//	bool flag = true;
//	while (p) {
//		if (p->inf == m) {
//			flag = false;
//			break;
//		}
//		p = p->next;
//		++i;
//	}
//	return flag ? "Ёлемент " + m.getNumber() + " не найден!" : "Ёлемент " + m.getNumber() + " найден под номером " + to_string(i);
//}




template <typename T>

string List<T>::find(T m) {//функци€ поиска элементов в списке
	nodeList<T>* p = head;
	int i = 0;
	bool flag = true;
	while (p) {
		if (p->inf == m) {
			flag = false;
			break;
		}
		p = p->next;
		++i;
	}
	return flag ? "Ёлемент " + m + " не найден!" : "Ёлемент " + m + " найден под номером " + to_string(i);
}





template <typename T>

void List<T>::delNodeAfter(nodeList<T> a) {
	nodeList<T>* p = head;
	while (p && (p->inf != a.inf)) {
		p = p->next;
	}
	if (p == head && p == tail) {
		head = tail = NULL;
	}
	else if (p == head) {
		head = head->next;
		head->prev = NULL;

	}
	else if (p == tail) {
		tail = tail->prev;
		tail->next = NULL;

	}
	else {
		p->next->prev = p->prev;
		p->prev->next = p->next;
	}
	delete p;
}

template <typename T>

void List<T>::insertNodeAfter(nodeList<T> a, T m) {
	nodeList<T>* p = head;
	while (p && (p->inf != a.inf)) {
		p = p->next;
	}
	nodeList<T>* s = new nodeList<T>(m);


	if (p == tail) { //если вставл€ем после хвоста
		s->next = NULL; //вставл€емый эл-т - последний
		s->prev = p; //вставл€ем после p
		(p->next) = s;
		tail = s; //теперь хвост - s

	}
	else { //вставл€ем в середину списка
		p->next->prev = s; //дл€ следующего за p эл-та предыдущий - s
		s->next = p->next; //следующий за p - следующий за r
		s->prev = p; //s вставл€ем после p
		p->next = s;

	}
}


template <typename T>



List<T>::~List() {//деструктор списка
	nodeList<T>* r = head;
	nodeList<T>* s = head;
	while (s) {
		s = s->next;
		delete r;
		r = s;
	}
}