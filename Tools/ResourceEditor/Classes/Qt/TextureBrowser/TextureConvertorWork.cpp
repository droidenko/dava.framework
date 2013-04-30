#include "Qt/TextureBrowser/TextureConvertorWork.h"

JobStack::JobStack()
	: head(NULL)
	, itemsCount(0)
{ }

JobStack::~JobStack()
{
	JobItemWrapper *item;

	while(NULL != head)
	{
		item = head;
		head = head->next;

		delete item;
	}
}

void JobStack::push(const JobItem &item)
{
	JobItemWrapper *i = head;

	// search for the same works in list and remove it
	while(NULL != i)
	{
		if(i->type == item.type && i->descriptor == item.descriptor)
		{
			if(NULL != i->prev)
			{
				i->prev->next = i->next;
			}

			if(NULL != i->next)
			{
				i->next->prev = i->prev;
			}

			if(i == head)
			{
				head = i->next;
			}

			delete i;
			itemsCount--;

			break;
		}

		i = i->next;
	}

	// add new work
	i = new JobItemWrapper(item);
	if(NULL != head)
	{
		head->prev = i;
		i->next = head;
	}

	head = i;
	itemsCount++;
}

JobItem* JobStack::pop()
{
	JobItemWrapper *item = head;

	if(NULL != head)
	{
		head = head->next;
		if(NULL != head)
		{
			head->prev = NULL;
		}

		itemsCount--;
	}

	return item;
}

int JobStack::size()
{
	return itemsCount;
}

JobStack::JobItemWrapper::JobItemWrapper(const JobItem &item)
	: JobItem(item)
	, prev(NULL)
	, next(NULL)
{ }
