#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <iterator>

class IGenerator
{
public:
	virtual ~IGenerator() = default;

	virtual bool Initialize(void* module) = 0;

	virtual std::string GetOutputDirectory() const
	{
		return "C:/SDK_GEN";
	}

	virtual std::string GetGameName() const = 0;

	virtual std::string GetGameNameShort() const = 0;

	virtual std::string GetGameVersion() const = 0;

	virtual bool ShouldDumpArrays() const
	{
		return true;
	}

	virtual bool ShouldGenerateEmptyFiles() const
	{
		return false;
	}

	virtual bool ShouldUseStrings() const
	{
		return true;
	}

	virtual bool ShouldXorStrings() const
	{
		return false;
	}

	virtual bool ShouldConvertStaticMethods() const
	{
		return true;
	}

	virtual bool ShouldGenerateFunctionParametersFile() const
	{
		return true;
	}

	virtual std::string GetNamespaceName() const
	{
		return std::string();
	}

	virtual std::vector<std::string> GetIncludes() const
	{
		return {};
	}

	virtual size_t GetGlobalMemberAlignment() const
	{
		return sizeof(size_t);
	}

	virtual size_t GetClassAlignas(const std::string& name) const
	{
		auto it = alignasClasses.find(name);
		if (it != std::end(alignasClasses))
		{
			return it->second;
		}
		return 0;
	}

	virtual std::string GetBasicDeclarations() const
	{
		return std::string();
	}

	virtual std::string GetBasicDefinitions() const
	{
		return std::string();
	}

	virtual std::string GetOverrideType(const std::string& type) const
	{
		auto it = overrideTypes.find(type);
		if (it == std::end(overrideTypes))
		{
			return type;
		}
		return it->second;
	}

	struct PredefinedMember
	{
		std::string Type;
		std::string Name;
	};

	virtual bool GetPredefinedClassMembers(const std::string& name, std::vector<PredefinedMember>& members) const
	{
		auto it = predefinedMembers.find(name);
		if (it != std::end(predefinedMembers))
		{
			std::copy(std::begin(it->second), std::end(it->second), std::back_inserter(members));

			return true;
		}

		return false;
	}

	virtual bool GetPredefinedClassStaticMembers(const std::string& name, std::vector<PredefinedMember>& members) const
	{
		auto it = predefinedStaticMembers.find(name);
		if (it != std::end(predefinedStaticMembers))
		{
			std::copy(std::begin(it->second), std::end(it->second), std::back_inserter(members));

			return true;
		}

		return false;
	}

	using VirtualFunctionPatterns = std::vector<std::tuple<const char*, const char*, size_t, const char*>>;

	virtual bool GetVirtualFunctionPatterns(const std::string& name, VirtualFunctionPatterns& patterns) const
	{
		auto it = virtualFunctionPattern.find(name);
		if (it != std::end(virtualFunctionPattern))
		{
			std::copy(std::begin(it->second), std::end(it->second), std::back_inserter(patterns));

			return true;
		}

		return false;
	}

	struct PredefinedMethod
	{
		enum class Type
		{
			Default,
			Inline
		};

		std::string Signature;
		std::string Body;
		Type MethodType;

		static PredefinedMethod Default(std::string&& signature, std::string&& body)
		{
			return { signature, body, Type::Default };
		}

		static PredefinedMethod Inline(std::string&& body)
		{
			return { std::string(), body, Type::Inline };
		}
	};

	virtual bool GetPredefinedClassMethods(const std::string& name, std::vector<PredefinedMethod>& methods) const
	{
		auto it = predefinedMethods.find(name);
		if (it != std::end(predefinedMethods))
		{
			std::copy(std::begin(it->second), std::end(it->second), std::back_inserter(methods));

			return true;
		}

		return false;
	}

protected:
	std::unordered_map<std::string, size_t> alignasClasses;
	std::unordered_map<std::string, std::string> overrideTypes;
	std::unordered_map<std::string, std::vector<PredefinedMember>> predefinedMembers;
	std::unordered_map<std::string, std::vector<PredefinedMember>> predefinedStaticMembers;
	std::unordered_map<std::string, std::vector<PredefinedMethod>> predefinedMethods;
	std::unordered_map<std::string, VirtualFunctionPatterns> virtualFunctionPattern;
};
