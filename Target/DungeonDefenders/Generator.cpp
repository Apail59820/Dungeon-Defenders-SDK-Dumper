#include "IGenerator.hpp"
#include "ObjectsStore.hpp"

class Generator : public IGenerator
{
public:
	bool Initialize(void* module) override
	{
		predefinedStaticMembers["Class Core.Object"] = {
			{ "TArray<UObject*>*", "GObjects" }
		};
		predefinedMembers["Class Core.Field"] = {
			{ "class UField*", "Next" }
		};
		predefinedMembers["Class Core.Struct"] = {
			{ "unsigned char", "UnknownData00[0x08]" },
			{ "class UField*", "SuperField" },
			{ "class UField*", "Children" },
			{ "unsigned long", "PropertySize" },
			{ "unsigned char", "UnknownData01[0x48]" }
		};
		predefinedMembers["Class Core.Function"] = {
			{ "uint32_t", "FunctionFlags" },
			{ "uint16_t", "iNative" },
			{ "uint16_t", "RepOffset" },
			{ "uint8_t", "OperPrecedence" },
			{ "FName", "FriendlyName" },
			{ "uint8_t", "NumParms" },
			{ "uint16_t", "ParmsSize" },
			{ "uint16_t", "ReturnValueOffset" },
			{ "void*", "Func" }
		};
		predefinedMembers["Class Core.Class"] = {
			{ "unsigned char", "UnknownData00[0x88]" },
			{ "class UObject*", "ClassDefaultObject" },
			{ "unsigned char", "UnknownData01[0x70]" }
		};

		predefinedMethods["ScriptStruct Core.Object.Color"] = {
			PredefinedMethod::Inline(R"(	inline FColor()
		: R(0), G(0), B(0), A(0)
	{ })"),
			PredefinedMethod::Inline(R"(	inline FColor(float r, float g, float b, float a)
		: R(r),
		  G(g),
		  B(b),
		  A(a)
	{ })")
		};

		predefinedMethods["Class Core.Object"] = {
			PredefinedMethod::Inline(R"(	static inline TArray<UObject*>& GetGlobalObjects()
	{
		return *GObjects;
	})"),
			PredefinedMethod::Inline(R"(	inline std::string UObject::GetName() const
	{
		auto name = Name.GetName();
		if (Name.Number > 0)
		{
			name += '_' + std::to_string(Name.Number);
		}
		return name;
	})"),
			PredefinedMethod::Default("std::string GetFullName() const", R"(std::string UObject::GetFullName() const
{
	std::string name;

	if (Class != nullptr)
	{
		std::string temp;
		for (auto p = Outer; p; p = p->Outer)
		{
			temp = p->GetName() + "." + temp;
		}

		name = Class->GetName();
		name += " ";
		name += temp;
		name += GetName();
	}

	return name;
})"),
			PredefinedMethod::Inline(R"(	template<typename T>
	static T* FindObject(const std::string& name)
	{
		for (auto i = 0u; i < GetGlobalObjects().Num(); ++i)
		{
			auto object = GetGlobalObjects().GetByIndex(i);
	
			if (object == nullptr)
			{
				continue;
			}
	
			if (object->GetFullName() == name)
			{
				return static_cast<T*>(object);
			}
		}
		return nullptr;
	})"),
			PredefinedMethod::Inline(R"(	static UClass* FindClass(const std::string& name)
	{
		return FindObject<UClass>(name);
	})"),
			PredefinedMethod::Inline(R"(	template<typename T>
	static T* GetObjectCasted(std::size_t index)
	{
		return static_cast<T*>(GetGlobalObjects().GetByIndex(index));
	})"),
			PredefinedMethod::Default("bool IsA(UClass* cmp) const", R"(bool UObject::IsA(UClass* cmp) const
{
	for (auto super = Class; super; super = static_cast<UClass*>(super->SuperField))
	{
		if (super == cmp)
		{
			return true;
		}
	}

	return false;
})"),
			PredefinedMethod::Inline(R"(	inline void ProcessEvent(class UFunction* function, void* params)
	{
		using Fn = void(__thiscall *)(UObject*, UFunction*, void*, void*);

		static Fn fn = nullptr;

		return fn(this, function, params, nullptr);
	})")
		};
		predefinedMethods["Class Core.Class"] = {
			PredefinedMethod::Inline(R"(	template<typename T>
	inline T* CreateDefaultObject()
	{
		return static_cast<T*>(CreateDefaultObject());
	})"),
			PredefinedMethod::Inline(R"(	inline UObject* CreateDefaultObject()
	{
		using Fn = UObject*(__thiscall *)(UClass*, unsigned int);

		static Fn fn = nullptr;
		
		return fn(this, 0);

		//or use the default object (WARNING: may be null)
		//return ClassDefaultObject;
	})")
		};

		return true;
	}

	std::string GetGameName() const override
	{
		return "Dungeon Defenders";
	}

	std::string GetGameNameShort() const override
	{
		return "DD";
	}

	std::string GetGameVersion() const override
	{
		return "9.3.0";
	}

	std::string GetNamespaceName() const override
	{
		return "Classes";
	}

	std::vector<std::string> GetIncludes() const override
	{
		return { };
	}

	std::string GetBasicDeclarations() const override
	{
		return R"(template<typename Fn>
inline Fn GetVFunction(const void *instance, std::size_t index)
{
	auto vtable = *reinterpret_cast<const void***>(const_cast<void*>(instance));
	return reinterpret_cast<Fn>(vtable[index]);
}

template<class T>
struct TArray
{
	friend struct FString;

public:
	inline TArray()
	{
		Data = nullptr;
		Count = Max = 0;
	};

	inline size_t Num() const
	{
		return Count;
	};

	inline T& operator[](size_t i)
	{
		return Data[i];
	};

	inline const T& operator[](size_t i) const
	{
		return Data[i];
	};

	inline bool IsValidIndex(size_t i) const
	{
		return i < Num();
	}

	inline T& GetByIndex(size_t i)
	{
		return Data[i];
	}

	inline const T& GetByIndex(size_t i) const
	{
		return Data[i];
	}

private:
	T* Data;
	int32_t Count;
	int32_t Max;
};

struct FString : private TArray<wchar_t>
{
	inline FString()
	{
	}

	FString(const wchar_t* other)
	{
		Max = Count = *other ? std::wcslen(other) + 1 : 0;

		if (Count)
		{
			Data = const_cast<wchar_t*>(other);
		}
	};

	inline bool IsValid() const
	{
		return Data != nullptr;
	}

	inline const wchar_t* c_str() const
	{
		return Data;
	}

	std::string ToString() const
	{
		const auto length = std::wcslen(Data);

		std::string str(length, '\0');

		std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data, Data + length, '?', &str[0]);

		return str;
	}
};

template<class TEnum>
class TEnumAsByte
{
public:
	inline TEnumAsByte()
	{
	}

	inline TEnumAsByte(TEnum _value)
		: value(static_cast<uint8_t>(_value))
	{
	}

	explicit inline TEnumAsByte(int32_t _value)
		: value(static_cast<uint8_t>(_value))
	{
	}

	explicit inline TEnumAsByte(uint8_t _value)
		: value(_value)
	{
	}

	inline operator TEnum() const
	{
		return (TEnum)value;
	}

	inline TEnum GetValue() const
	{
		return (TEnum)value;
	}

private:
	uint8_t value;
};

class FNameEntry
{
public:
	uint32_t Flags;
	char UnknownData00[0xC];
	union
	{
		char Name[1024];
		char* NamePtr;
	};

	inline const char* GetName() const
	{
		return Flags & 0x4000 ? NamePtr : Name;
	}
};

struct FName
{
	int32_t Index;
	int32_t Number;

	inline FName()
		: Index(0),
		  Number(0)
	{
	};

	inline FName(int32_t i)
		: Index(i),
		  Number(0)
	{
	};

	FName(const char* nameToFind)
		: Index(0),
		  Number(0)
	{
		static std::unordered_set<size_t> cache;

		for (auto i : cache)
		{
			if (!std::strcmp(GetGlobalNames()[i]->GetName(), nameToFind))
			{
				Index = i;
				
				return;
			}
		}

		for (auto i = 0u; i < GetGlobalNames().Num(); ++i)
		{
			if (GetGlobalNames()[i] != nullptr)
			{
				if (!std::strcmp(GetGlobalNames()[i]->GetName(), nameToFind))
				{
					cache.insert(i);

					Index = i;

					return;
				}
			}
		}
	};

	static TArray<FNameEntry*>* GNames;
	static inline TArray<FNameEntry*>& GetGlobalNames()
	{
		return *GNames;
	};

	inline const char* GetName() const
	{
		return GetGlobalNames()[Index]->GetName();
	};

	inline bool operator==(const FName& other) const
	{
		return Index == other.Index;
	};
};

class UObject;

class FScriptInterface
{
private:
	UObject* ObjectPointer;
	void* InterfacePointer;

public:
	inline UObject* GetObject() const
	{
		return ObjectPointer;
	}

	inline UObject*& GetObjectRef()
	{
		return ObjectPointer;
	}

	inline void* GetInterface() const
	{
		return ObjectPointer != nullptr ? InterfacePointer : nullptr;
	}
};

template<class InterfaceType>
class TScriptInterface : public FScriptInterface
{
public:
	inline InterfaceType* operator->() const
	{
		return (InterfaceType*)GetInterface();
	}

	inline InterfaceType& operator*() const
	{
		return *((InterfaceType*)GetInterface());
	}

	inline operator bool() const
	{
		return GetInterface() != nullptr;
	}
};

struct FScriptDelegate
{
	unsigned char UnknownData[0x0C];
};)";
	}

	std::string GetBasicDefinitions() const override
	{
		return R"(TArray<FNameEntry*>* FName::GNames = nullptr;
TArray<UObject*>* UObject::GObjects = nullptr;)";
	}
};

Generator _generator;
IGenerator* generator = &_generator;
