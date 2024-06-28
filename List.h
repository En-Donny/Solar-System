#pragma once
#include <iostream>
#include <string>
#include "Planets.h"


using namespace std;

template <typename T>
class nodeList {//����� �������� ������
public:
	T inf;//��������
	nodeList<T>* next;//������ �����
	nodeList<T>* prev;//����� �����

	nodeList(T x) {//����������� ��� ������������ �������� ������
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
class List {//����� ��� ������������ ������ ������
public:


	List();//����������� ��� ������� ����������





	T& operator[](int i); /*������������� ��������� � ��������� ������ ����� []*/



	int length(); //������� ������������ ����� ������ ������



	void pushElement(T x); //������� ���������� �������� � ����� ������



	void printList();//������� ����� ������ �� �����


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

	string find(T m);//������� ������ ��������� � ������




	void delNodeAfter(nodeList<T> a);

	void Clear();


	void insertNodeAfter(nodeList<T> a, T m);




	~List();//���������� ������




private:

	nodeList<T>* head;//��������� �� ������ ������
	nodeList<T>* tail;//��������� �� ����� ������


};








template <typename T>
List<T>::List() {//����������� ��� ������� ����������
	head = NULL;
	tail = NULL;

}






template <typename T>

T& List<T>:: operator[](int i) {/*������������� ��������� � ��������� ������ ����� []*/
	nodeList<T>* t = head;
	int j = 0;
	while (i != j && t) {
		t = t->next;
		++j;
	}
	return (t->inf);
}

template <typename T>

int List<T>::length() {//������� ������������ ����� ������ ������
	nodeList<T>* r = head;
	int size = 0;
	while (r) {
		r = r->next;
		++size;
	}
	return size;
}


template <typename T>


void List<T>::pushElement(T x) {//������� ���������� �������� � ����� ������
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

void List<T>::printList() {//������� ����� ������ �� �����
	if (!head && !tail) {
		cout << "������ ����!" << endl;
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
	while (h) { //���� ������ �� ����
		nodeList<T>* p = h; //��������� �� ������
		h = h->next; //��������� ������
		h->prev = NULL; //��������
		delete p; //������� p

	}
	}
}


//string List<complexNumber>::find(complexNumber m) {//������� ������ ��������� � ������
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
//	return flag ? "������� " + m.getNumber() + " �� ������!" : "������� " + m.getNumber() + " ������ ��� ������� " + to_string(i);
//}




template <typename T>

string List<T>::find(T m) {//������� ������ ��������� � ������
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
	return flag ? "������� " + m + " �� ������!" : "������� " + m + " ������ ��� ������� " + to_string(i);
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


	if (p == tail) { //���� ��������� ����� ������
		s->next = NULL; //����������� ��-� - ���������
		s->prev = p; //��������� ����� p
		(p->next) = s;
		tail = s; //������ ����� - s

	}
	else { //��������� � �������� ������
		p->next->prev = s; //��� ���������� �� p ��-�� ���������� - s
		s->next = p->next; //��������� �� p - ��������� �� r
		s->prev = p; //s ��������� ����� p
		p->next = s;

	}
}


template <typename T>



List<T>::~List() {//���������� ������
	nodeList<T>* r = head;
	nodeList<T>* s = head;
	while (s) {
		s = s->next;
		delete r;
		r = s;
	}
}