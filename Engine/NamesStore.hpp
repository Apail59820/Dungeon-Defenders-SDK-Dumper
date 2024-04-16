#pragma once

#include <iterator>

#include "GenericTypes.hpp"

class NamesIterator;

class NamesStore
{
	friend NamesIterator;

public:

	static bool Initialize();

	static void* GetAddress();

	NamesIterator begin();

	NamesIterator begin() const;

	NamesIterator end();

	NamesIterator end() const;

	size_t GetNamesNum() const;

	bool IsValid(size_t id) const;

	std::string GetById(size_t id) const;
};

struct UENameInfo
{
	size_t Index;
	std::string Name;
};

class NamesIterator : public std::iterator<std::forward_iterator_tag, UENameInfo>
{
	const NamesStore& store;
	size_t index;

public:
	NamesIterator(const NamesStore& store);

	explicit NamesIterator(const NamesStore& store, size_t index);

	void swap(NamesIterator& other) noexcept;

	NamesIterator& operator++();

	NamesIterator operator++ (int);

	bool operator==(const NamesIterator& rhs) const;

	bool operator!=(const NamesIterator& rhs) const;

	UENameInfo operator*() const;

	UENameInfo operator->() const;
};
