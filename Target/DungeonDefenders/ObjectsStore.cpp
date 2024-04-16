#include <windows.h>

#include "PatternFinder.hpp"
#include "ObjectsStore.hpp"

#include "EngineClasses.hpp"

TArray<UObject*>* GlobalObjects = nullptr;

template<typename T>
static inline T GetMovValue(void* address) {
	signed __int32 relative = *(signed __int32*)((unsigned __int64)address + 1);
	return reinterpret_cast<T>(relative);
}

bool ObjectsStore::Initialize()
{
	auto address = FindPattern(GetModuleHandleW(nullptr), reinterpret_cast<const unsigned char*>("\x8B\x00\x00\x00\x00\x00\x8B\x04\x00\x8B\x40\x00\x25\x00\x02\x00\x00"), "x?????xx?xx?xxxxx"); 
	if (address == -1)
	{
		return false;
	}

	static void* movValue = GetMovValue<void*>(reinterpret_cast<void*>(address));
	printf("Mov value : 0x%p\n", movValue);
	GlobalObjects = reinterpret_cast<TArray<UObject*>*>(movValue);

	printf("GObjects : 0x%p\n", (void*)GlobalObjects);
	printf("GObjects count : %d\n\n", GlobalObjects->Num());

	return true;
}

void* ObjectsStore::GetAddress()
{
	return GlobalObjects;
}

size_t ObjectsStore::GetObjectsNum() const
{
	return GlobalObjects->Num();
}

UEObject ObjectsStore::GetById(size_t id) const
{
	return (*GlobalObjects)[id];
}
