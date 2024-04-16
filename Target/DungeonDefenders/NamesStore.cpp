#include <windows.h>

#include "PatternFinder.hpp"
#include "NamesStore.hpp"

#include "EngineClasses.hpp"

template<typename T>
static inline T GetMovValue(void* address) {
	signed __int32 relative = *(signed __int32*)((unsigned __int64)address + 1);
	return reinterpret_cast<T>(relative);
}


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
	auto address = FindPattern(GetModuleHandleW(nullptr), reinterpret_cast<const unsigned char*>("\x8B\x0D\x00\x00\x00\x00\x83\x3C\x81\x00\x74"), "xx????xxxxx");

	if (address == -1)
	{
		return false;
	}

	TArray<FNameEntry*>* ptr = reinterpret_cast<TArray<FNameEntry*>*>(GetMovValue<void*>(reinterpret_cast<void*>(address)));

	GlobalNames = ptr;

	printf("GObjects : 0x%p\n", (void*)GlobalNames);
	//printf("GObjects count : %d\n\n", GlobalNames->Num());

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
