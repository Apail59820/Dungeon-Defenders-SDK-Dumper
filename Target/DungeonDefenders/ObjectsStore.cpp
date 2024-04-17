#include <windows.h>

#include "ObjectsStore.hpp"

#include "EngineClasses.hpp"

#include "PatternFinder.hpp"

TArray<UObject*>* GlobalObjects = nullptr;


bool ObjectsStore::Initialize()
{

	auto address = FindPattern(GetModuleHandleW(L"DunDefGame.exe"), reinterpret_cast<const unsigned char*>("\x8B\x00\x00\x00\x00\x00\x8B\x04\x00\x8B\x40\x00\x25\x00\x02\x00\x00"), "x?????xx?xx?xxxxx");
	if (address == -1)
	{
		return false;
	}

	GlobalObjects = reinterpret_cast<decltype(GlobalObjects)>(*reinterpret_cast<uint32_t*>(address + 2));

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
