/*
  ==============================================================================

    BaseManager.h
    Created: 28 Oct 2016 8:03:13pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef BASEMANAGER_H_INCLUDED
#define BASEMANAGER_H_INCLUDED

#include "BaseItem.h"

template <class T>
class BaseManager :
	public ControllableContainer,
	public BaseItem::Listener
{
public : 
	BaseManager<T>(const String &name);
	virtual ~BaseManager<T>();

	OwnedArray<T> items;
	
	T * addItem();
	void removeItem(T *);

	void clear();

	void askForRemoveBaseItem(BaseItem * item) override;


	class  Listener
	{
	public:
		/** Destructor. */
		virtual ~Listener() {}
		virtual void itemAdded(BaseItem *) {}
		virtual void itemRemoved(BaseItem *) {}
	};

	ListenerList<Listener> baseManagerListeners;
	void addBaseManagerListener(Listener* newListener) { baseManagerListeners.add(newListener); }
	void removeBaseManagerListener(Listener* listener) { baseManagerListeners.remove(listener); }


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaseManager<T>)
};


template<class T>
BaseManager<T>::BaseManager(const String & name) :
	ControllableContainer(name)
{
}

template<class T>
inline BaseManager<T>::~BaseManager()
{
	clear();
}

template<class T>
T * BaseManager<T>::addItem()
{
	T * item = new T();
	items.add(item);
	addChildControllableContainer(static_cast<ControllableContainer *>(item));
	static_cast<BaseItem *>(item)->addBaseItemListener(this);
	baseManagerListeners.call(&BaseManager::Listener::itemAdded, item);
	return item;
}

template<class T>
void BaseManager<T>::removeItem(T * item)
{
	items.removeObject(item, false);
	removeChildControllableContainer(static_cast<ControllableContainer *>(item));
	static_cast<BaseItem *>(item)->removeBaseItemListener(this);
	baseManagerListeners.call(&BaseManager::Listener::itemRemoved, item);
	delete item;
}

template<class T>
void BaseManager<T>::clear()
{
	while (items.size() > 0) removeItem(items[0]);
}

template<class T>
void BaseManager<T>::askForRemoveBaseItem(BaseItem * item)
{
	removeItem(static_cast<T*>(item));
}




#endif  // BASEMANAGER_H_INCLUDED

