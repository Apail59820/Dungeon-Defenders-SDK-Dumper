#pragma once
#include <string>
#include <vector>
#include <stdint.h>

template<typename T>
static T GetCallAddress(void* address) {
	signed __int32 relative = *(signed __int32*)((unsigned __int64)address + 1);
	void* absolute = (void*)((unsigned __int64)address + relative + 5);
	return reinterpret_cast<T>(absolute);
}

template<typename T>
static T GetMovAddress(void* address) {
	signed __int32 relative = *(signed __int32*)((unsigned __int64)address + 3);
	void* absolute = (void*)((unsigned __int64)address + relative + 7);
	return reinterpret_cast<T>(absolute);
}

template<typename T>
static T GetMovValue(void* address) {
	signed __int32 relative = *(signed __int32*)((unsigned __int64)address + 1);
	return reinterpret_cast<T>(relative);
}

template<typename T>
static T GetAndValue(void* address) {
	signed __int32 relative = *(signed __int32*)((unsigned __int64)address + 2);
	return reinterpret_cast<T>(relative);
}

static std::vector<int> PatternToIntVector(const char* Pattern)
{
	std::vector<int> PatternVector = std::vector<int>{};

	char* Start = const_cast<char*>(Pattern);

	char* End = const_cast<char*>(Pattern) + strlen(Pattern);

	for (char* Current = Start; Current < End; ++Current)
	{
		if (*Current == '?')
		{
			++Current;

			PatternVector.push_back(-69);
		}
		else {
			PatternVector.push_back(strtoul(Current, &Current, 16));
		}
	}

	return PatternVector;
}

static inline void* FindPointer(std::vector<int> Pattern, uint8_t* Address, int Addition = 0) {

	int i = 0;
	while (Pattern[i] != -69) i++;

	int Offset = *reinterpret_cast<int*>(Address + i);

	return reinterpret_cast<void*>(Address + i + 0x4 + Offset + Addition);
}

template<typename T>
static T PatternScan(const char* Pattern, uint64_t Start, size_t Size, bool bFindPointer = false, int Addition = 0)
{
	std::vector<int> PatternVector = PatternToIntVector(Pattern);

	uint8_t* Search = reinterpret_cast<uint8_t*>(Start);

	const size_t SizeOfPattern = PatternVector.size();
	const int* PatternData = PatternVector.data();

	for (int i = 0; i < Size - SizeOfPattern; ++i)
	{
		bool FoundSignature = true;

		for (int j = 0; j < SizeOfPattern; ++j)
		{
			if (Search[i + j] != PatternData[j] && PatternData[j] != -69)
			{
				FoundSignature = false;
				break;
			}
		}

		if (FoundSignature) {

			if (bFindPointer) {
				return reinterpret_cast<T>(FindPointer(PatternVector, &Search[i], Addition));
			}

			return reinterpret_cast<T>(&Search[i]);

		}
	}

	return reinterpret_cast<T>(nullptr);
}


