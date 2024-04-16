#include <windows.h>

#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <chrono>
#include <filesystem>
#include <bitset>
namespace fs = std::experimental::filesystem;
#include "cpplinq.hpp"

#include "Logger.hpp"

#include "IGenerator.hpp"
#include "ObjectsStore.hpp"
#include "NamesStore.hpp"
#include "Package.hpp"
#include "NameValidator.hpp"
#include "PrintHelper.hpp"


extern IGenerator* generator;

void Dump(const fs::path& path)
{
	{
		std::ofstream o(path / "ObjectsDump.txt");
		tfm::format(o, "Address: 0x%P\n\n", ObjectsStore::GetAddress());

		for (auto obj : ObjectsStore())
		{
			tfm::format(o, "[%06i] %-100s 0x%p\n", obj.GetIndex(), obj.GetFullName(), obj.GetAddress());
		}
	}

	{
		std::ofstream o(path / "NamesDump.txt");
		tfm::format(o, "Address: 0x%P\n\n", NamesStore::GetAddress());

		for (auto name : NamesStore())
		{
			tfm::format(o, "[%06i] %s\n", name.Index, name.Name);
		}
	}
}

void SaveSDKHeader(const fs::path& path, const std::unordered_map<UEObject, bool>& processedObjects, const std::vector<Package>& packages)
{
	std::ofstream os(path / "SDK.hpp");

	os << "#pragma once\n\n"
		<< tfm::format("// %s (%s) SDK\n\n", generator->GetGameName(), generator->GetGameVersion());

	{
		{
			std::ofstream os2(path / "SDK" / tfm::format("%s_Basic.hpp", generator->GetGameNameShort()));

			std::vector<std::string> includes{ { "<unordered_set>" }, { "<string>" } };

			auto&& generatorIncludes = generator->GetIncludes();
			includes.insert(includes.end(), std::begin(generatorIncludes), std::end(generatorIncludes));

			PrintFileHeader(os2, includes, true);
			
			os2 << generator->GetBasicDeclarations() << "\n";

			PrintFileFooter(os2);

			os << "\n#include \"SDK/" << tfm::format("%s_Basic.hpp", generator->GetGameNameShort()) << "\"\n";
		}
		{
			std::ofstream os2(path / "SDK" / tfm::format("%s_Basic.cpp", generator->GetGameNameShort()));

			PrintFileHeader(os2, { "../SDK.hpp" }, false);

			os2 << generator->GetBasicDefinitions() << "\n";

			PrintFileFooter(os2);
		}
	}

	using namespace cpplinq;

	const auto missing = from(processedObjects) >> where([](auto&& kv) { return kv.second == false; });
	if (missing >> any())
	{
		std::ofstream os2(path / "SDK" / tfm::format("%s_MISSING.hpp", generator->GetGameNameShort()));

		PrintFileHeader(os2, true);

		for (auto&& s : missing >> select([](auto&& kv) { return kv.first.Cast<UEStruct>(); }) >> experimental::container())
		{
			os2 << "// " << s.GetFullName() << "\n// ";
			os2 << tfm::format("0x%04X\n", s.GetPropertySize());

			os2 << "struct " << MakeValidName(s.GetNameCPP()) << "\n{\n";
			os2 << "\tunsigned char UnknownData[0x" << tfm::format("%X", s.GetPropertySize()) << "];\n};\n\n";
		}

		PrintFileFooter(os2);

		os << "\n#include \"SDK/" << tfm::format("%s_MISSING.hpp", generator->GetGameNameShort()) << "\"\n";
	}

	os << "\n";

	for (auto&& package : packages)
	{
		os << R"(#include "SDK/)" << GenerateFileName(FileContentType::Classes, package) << "\"\n";
	}
}

void ProcessPackages(const fs::path& path)
{
	using namespace cpplinq;

	const auto sdkPath = path / "SDK";
	fs::create_directories(sdkPath);
	
	std::vector<Package> packages;

	std::unordered_map<UEObject, bool> processedObjects;

	auto packageObjects = from(ObjectsStore())
		>> select([](auto&& o) { return o.GetPackageObject(); })
		>> where([](auto&& o) { return o.IsValid(); })
		>> distinct()
		>> to_vector();

	for (auto obj : packageObjects)
	{
		Package package(obj);

		package.Process(processedObjects);
		if (package.Save(sdkPath))
		{
			packages.emplace_back(std::move(package));
		}
	}

	SaveSDKHeader(path, processedObjects, packages);
}

DWORD WINAPI OnAttach(LPVOID lpParameter)
{
	AllocConsole();
	freopen( "conin$",  "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);

	printf("Initializing object store\n");

	if (!ObjectsStore::Initialize())
	{
		MessageBoxA(nullptr, "ObjectsStore::Initialize failed", "Error", 0);
		return -1;
	}

	printf("Getting name store\n");


	if (!NamesStore::Initialize())
	{
		MessageBoxA(nullptr, "NamesStore::Initialize failed", "Error", 0);
		return -1;
	}

	return 0; // TODO : Remove that after reversing

	printf("Initializing...\n");

	if (!generator->Initialize(lpParameter))
	{
		MessageBoxA(nullptr, "Initialize failed", "Error", 0);
		return -1;
	}

	printf("Get output directory...\n");
	fs::path outputDirectory(generator->GetOutputDirectory());
	if (!outputDirectory.is_absolute())
	{
		char buffer[2048];
		if (GetModuleFileNameA(static_cast<HMODULE>(lpParameter), buffer, sizeof(buffer)) == 0)
		{
			MessageBoxA(nullptr, "GetModuleFileName failed", "Error", 0);
			return -1;
		}

		outputDirectory = fs::path(buffer).remove_filename() / outputDirectory;
	}

	outputDirectory /= generator->GetGameNameShort();
	fs::create_directories(outputDirectory);
	
	std::ofstream log(outputDirectory / "Generator.log");
	Logger::SetStream(&log);

	printf("Dumping...\n");

	if (generator->ShouldDumpArrays())
	{
		Dump(outputDirectory);
	}

	fs::create_directories(outputDirectory);

	const auto begin = std::chrono::system_clock::now();


	printf("Processing packages...\n");
	ProcessPackages(outputDirectory);

	Logger::Log("Finished, took %d seconds.", std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - begin).count());

	Logger::SetStream(nullptr);

	MessageBoxA(nullptr, "Finished!", "Info", 0);

	return 0;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);

		CreateThread(nullptr, 0, OnAttach, hModule, 0, nullptr);

		return TRUE;
	}

	return FALSE;
}
