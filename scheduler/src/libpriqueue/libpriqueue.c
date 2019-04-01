/** @file libpriqueue.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "libpriqueue.h"


/**
  Initializes the priqueue_t data structure.

  Assumtions
    - You may assume this function will only be called once per instance of priqueue_t
    - You may assume this function will be the first function called using an instance of priqueue_t.
  @param q a pointer to an instance of the priqueue_t data structure
  @param comparer a function pointer that compares two elements.
  See also @ref comparer-page
 */
void priqueue_init(priqueue_t *q, int(*comparer)(const void *, const void *))
{
	q->m_front = NULL;
	q->m_size = 0;
	q->comp = comparer;
}

/**
  Inserts the specified element into this priority queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr a pointer to the data to be inserted into the priority queue
  @return The zero-based index where ptr is stored in the priority queue, where 0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer(priqueue_t *q, void *ptr)
{
	node_t* newNode = malloc(sizeof(*newNode));
	int index = 0;
	newNode->m_entry = ptr;
	newNode->m_next = NULL;
	if(q->m_size==0)
	{
		q->m_front = newNode;
		q->m_size++;
		return index;
	}
	node_t* temp = q->m_front;
	node_t* prev = NULL;
	while(temp!=NULL&& q->comp(temp->m_entry , ptr) <= 0)
	{
		prev = temp;
		temp = temp->m_next;
		index++;
	}
	if(index==0)
	{
		q->m_front = newNode;
		newNode->m_next = temp;
		q->m_size++;
		return index;
	}

	prev->m_next = newNode;
	newNode->m_next = temp;

	q->m_size++;

	return index;
}


/**
  Retrieves, but does not remove, the head of this queue, returning NULL if
  this queue is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return pointer to element at the head of the queue
  @return NULL if the queue is empty
 */
void *priqueue_peek(priqueue_t *q)
{
	if(q->m_size!=0)
	{
		return q->m_front->m_entry;
	}
	else
	{
		return NULL;
	}
}


/**
  Retrieves and removes the head of this queue, or NULL if this queue
  is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the head of this queue
  @return NULL if this queue is empty
 */
void *priqueue_poll(priqueue_t *q)
{
	if(q->m_size!=0)
	{
		node_t* tempNode = q->m_front;
		q->m_front = q->m_front->m_next;
		q->m_size--;
		void* temp = tempNode->m_entry;
		free(tempNode);
		return temp;
	}
	else
	{
		return NULL;
	}
}


/**
  Returns the element at the specified position in this list, or NULL if
  the queue does not contain an index'th element.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of retrieved element
  @return the index'th element in the queue
  @return NULL if the queue does not contain the index'th element
 */
void *priqueue_at(priqueue_t *q, int index)
{
	if(q->m_size==0)
	{
		return NULL;
	}
	node_t* temp = q->m_front;
	for(int i = 0; i < q->m_size; i++)
	{
		if(i==index)
		{
			return temp->m_entry;
		}
		temp = temp->m_next;
	}
	return NULL;
}


/**
  Removes all instances of ptr from the queue.

  This function should not use the comparer function, but check if the data contained in each element of the queue is equal (==) to ptr.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr address of element to be removed
  @return the number of entries removed
 */
int priqueue_remove(priqueue_t *q, void *ptr)
{
	int index = 0;
	if(q->m_size==0)
	{
		return 0;
	}

	node_t* temp = q->m_front;
	node_t* prev = NULL;

	for(int i = 0; i<q->m_size; i++)
	{
		if(temp->m_entry == ptr)
		{
			if(i==0)
			{
				q->m_front=temp->m_next;
				free(temp);
				temp=q->m_front;
				q->m_size--;
				index++;
				i--;
			}
			else
			{
				prev->m_next = temp->m_next;
				free(temp);
				temp=prev->m_next;
				q->m_size--;
				index++;
				i--;
			}
		}
		else
		{
			prev=temp;
			temp=temp->m_next;
		}
	}
	return index;
}


/**
  Removes the specified index from the queue, moving later elements up
  a spot in the queue to fill the gap.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of element to be removed
  @return the element removed from the queue
  @return NULL if the specified index does not exist
 */
void *priqueue_remove_at(priqueue_t *q, int index)
{
	if(q->m_size==0)
	{
		return NULL;
	}
	node_t* temp = q->m_front;
	node_t* prev = NULL;
	void* entry;
	if (index == 0)
	{
		q->m_front = temp->m_next;
		entry = temp->m_entry;
		free(temp);
		q->m_size--;
		return entry;
	}else
	{
		for(int i = 0; i < q->m_size; i++)
		{
			if(i==index-1)
			{
				prev = temp;
				temp = temp->m_next;
				prev->m_next = temp->m_next;
				entry = temp->m_entry;
				free(temp);
				q->m_size--;
				return entry;
			}
			temp = temp->m_next;
		}
	}
	return NULL;
}

/**
  Returns the number of elements in the queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of elements in the queue
 */
int priqueue_size(priqueue_t *q)
{
	return q->m_size;
}


/**
  Destroys and frees all the memory associated with q.

  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q)
{
	node_t* temp = q->m_front;
	while(temp!=NULL)
	{
		q->m_front=temp->m_next;
		free(temp);
		temp=q->m_front;
	}
	q->m_size=0;
}
