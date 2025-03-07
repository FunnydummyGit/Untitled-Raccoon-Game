#pragma once
#include "StaticHitbox.h"

class LinkedList
{
private:
	StaticHitbox* body;
	LinkedList* next;
public:
	LinkedList() {
		body = NULL;
		next = NULL;
	}

	LinkedList(StaticHitbox* b, LinkedList n) {
		body = b;
		next = &n;
	}

	StaticHitbox* getHitbox() {
		return body;
	}

	LinkedList* getNext() {
		return next;
	}

	void setNext(LinkedList* n) {
		next = n;
	}

	void setBody(StaticHitbox* b) {
		body = b;
	}

	LinkedList* push(StaticHitbox* node_data)
	{
		/* 1. create and allocate node */
		LinkedList* newNode = new LinkedList;

		/* 2. assign data to node */
		newNode->body = node_data;

		/* 3. set next of new node as head */
		newNode->next = this;

		return newNode;
	}

};