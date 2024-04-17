#include <windows.h>

#include "NamesStore.hpp"

#include "EngineClasses.hpp"

#include "PatternFinder.hpp"

class FNameEntry
{
public:
	uint32_t Flags;
	char pad_0x0004[0xC];
	union
	{
		char Name[1024];
		char* NamePtr;
	};

	const char* GetName() const
	{
		return Flags & 0x4000 ? NamePtr : Name;
	}
};

TArray<FNameEntry*>* GlobalNames = nullptr;

bool NamesStore::Initialize()
{

	auto address = FindPattern(GetModuleHandleW(L"DunDefGame.exe"), reinterpret_cast<const unsigned char*>("\x8B\x0D\x00\x00\x00\x00\x83\x3C\x81\x00\x74"), "xx????xxxxx");

	if (address == -1)
	{
		return false;
	}

	GlobalNames = reinterpret_cast<decltype(GlobalNames)>(*reinterpret_cast<uint32_t*>(address + 2));

	printf("GObjects : 0x%p\n", (void*)GlobalNames);
	printf("GObjects count : %d\n\n", GlobalNames->Num());

	return true;
}

void* NamesStore::GetAddress()
{
	return GlobalNames;
}

size_t NamesStore::GetNamesNum() const
{
	return GlobalNames->Num();
}

bool NamesStore::IsValid(size_t id) const
{
	return GlobalNames->IsValidIndex(id) && (*GlobalNames)[id] != nullptr;
}

std::string NamesStore::GetById(size_t id) const
{
	return (*GlobalNames)[id]->GetName();
}
