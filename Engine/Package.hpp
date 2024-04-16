#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
namespace fs = std::experimental::filesystem;

#include "GenericTypes.hpp"

class Package
{
	friend struct std::hash<Package>;
	friend struct PackageDependencyComparer;
	friend bool operator==(const Package& lhs, const Package& rhs);

public:
	Package(const UEObject& packageObj);

	std::string GetName() const { return packageObj.GetName(); }

	void Process(std::unordered_map<UEObject, bool>& processedObjects);

	bool Save(const fs::path& path) const;

private:
	bool AddDependency(const UEObject& package) const;

	void GeneratePrerequisites(const UEObject& obj, std::unordered_map<UEObject, bool>& processedObjects);

	void GenerateMemberPrerequisites(const UEProperty& first, std::unordered_map<UEObject, bool>& processedObjects);

	void GenerateScriptStruct(const UEScriptStruct& scriptStructObj);

	void GenerateConst(const UEConst& constObj);

	void GenerateEnum(const UEEnum& enumObj);

	void GenerateClass(const UEClass& classObj);

	void SaveStructs(const fs::path& path) const;

	void SaveClasses(const fs::path& path) const;

	void SaveFunctions(const fs::path& path) const;

	void SaveFunctionParameters(const fs::path& path) const;

	UEObject packageObj;
	mutable std::unordered_set<UEObject> dependencies;

	void PrintConstant(std::ostream& os, const std::pair<std::string, std::string>& c) const;

	std::unordered_map<std::string, std::string> constants;

	struct Enum
	{
		std::string Name;
		std::string FullName;
		std::vector<std::string> Values;
	};

	void PrintEnum(std::ostream& os, const Enum& e) const;

	std::vector<Enum> enums;

	struct Member
	{
		std::string Name;
		std::string Type;

		size_t Offset;
		size_t Size;

		size_t Flags;
		std::string FlagsString;

		std::string Comment;
	};

	static Member CreatePadding(size_t id, size_t offset, size_t size, std::string reason);

	static Member CreateBitfieldPadding(size_t id, size_t offset, std::string type, size_t bits);

	void GenerateMembers(const UEStruct& structObj, size_t offset, const std::vector<UEProperty>& properties, std::vector<Member>& members) const;

	struct ScriptStruct
	{
		std::string Name;
		std::string FullName;
		std::string NameCpp;
		std::string NameCppFull;

		size_t Size;
		size_t InheritedSize;

		std::vector<Member> Members;

		std::vector<IGenerator::PredefinedMethod> PredefinedMethods;
	};


	void PrintStruct(std::ostream& os, const ScriptStruct& ss) const;

	std::vector<ScriptStruct> scriptStructs;

	struct Method
	{
		struct Parameter
		{
			enum class Type
			{
				Default,
				Out,
				Return
			};

			Type ParamType;
			bool PassByReference;
			std::string CppType;
			std::string Name;
			std::string FlagsString;

			static bool MakeType(UEPropertyFlags flags, Type& type);
		};

		size_t Index;
		std::string Name;
		std::string FullName;
		std::vector<Parameter> Parameters;
		std::string FlagsString;
		bool IsNative;
		bool IsStatic;
	};

	void GenerateMethods(const UEClass& classObj, std::vector<Method>& methods) const;

	struct Class : ScriptStruct
	{
		std::vector<std::string> VirtualFunctions;
		std::vector<Method> Methods;
	};


	std::string BuildMethodSignature(const Method& m, const Class& c, bool inHeader) const;

	std::string BuildMethodBody(const Class& c, const Method& m) const;

	void PrintClass(std::ostream& os, const Class& c) const;

	std::vector<Class> classes;
};

namespace std
{
	template<>
	struct hash<Package>
	{
		size_t operator()(const Package& package) const
		{
			return std::hash<void*>()(package.packageObj.GetAddress());
		}
	};
}

inline bool operator==(const Package& lhs, const Package& rhs) { return rhs.packageObj.GetAddress() == lhs.packageObj.GetAddress(); }
inline bool operator!=(const Package& lhs, const Package& rhs) { return !(lhs == rhs); }
