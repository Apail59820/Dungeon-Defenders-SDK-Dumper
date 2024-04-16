#pragma once

#include <unordered_map>

#include "GenericTypes.hpp"


class ObjectsIterator;

class ObjectsStore
{
public:

	static bool Initialize();

	static void* GetAddress();

	ObjectsIterator begin();

	ObjectsIterator begin() const;

	ObjectsIterator end();

	ObjectsIterator end() const;

	size_t GetObjectsNum() const;

	UEObject GetById(size_t id) const;

	UEClass FindClass(const std::string& name) const;

	template<class T>
	size_t CountObjects(const std::string& name) const
	{
		static std::unordered_map<std::string, size_t> cache;

		auto it = cache.find(name);
		if (it != std::end(cache))
		{
			return it->second;
		}

		size_t count = 0;
		for (auto obj : *this)
		{
			if (obj.IsA<T>() && obj.GetName() == name)
			{
				++count;
			}
		}

		cache[name] = count;

		return count;
	}
};


struct UEObjectInfo
{
	size_t Index;

	UEObject Object;
};

class ObjectsIterator : public std::iterator<std::forward_iterator_tag, UEObject>
{
	const ObjectsStore& store;
	size_t index;
	UEObject current;

public:

	ObjectsIterator(const ObjectsStore& store);

	explicit ObjectsIterator(const ObjectsStore& store, size_t index);

	ObjectsIterator(const ObjectsIterator& other);
	ObjectsIterator(ObjectsIterator&& other) noexcept;

	ObjectsIterator& operator=(const ObjectsIterator& rhs);

	void swap(ObjectsIterator& other) noexcept;

	ObjectsIterator& operator++();

	ObjectsIterator operator++ (int);

	bool operator==(const ObjectsIterator& rhs) const;

	bool operator!=(const ObjectsIterator& rhs) const;

	UEObject operator*() const;

	UEObject operator->() const;
};
