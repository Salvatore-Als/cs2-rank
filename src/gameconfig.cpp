#include "gameconfig.h"
#include "addresses.h"
#include "abstract.h"

CGameConfig::CGameConfig()
{
	CBufferStringGrowable<256> gamePath;
	g_pEngine->GetGameDir(gamePath);

	std::string gameDir = CGameConfig::GetDirectoryName(gamePath.Get());

	m_szGameDir = gameDir;
	m_szPath = GAMEDATA_PATH;
	this->m_pKeyValues = new KeyValues("Games");
}

CGameConfig::~CGameConfig()
{
	delete m_pKeyValues;
}

bool CGameConfig::Init(IFileSystem *filesystem, char *conf_error, int conf_error_size)
{
	if (!m_pKeyValues->LoadFromFile(filesystem, m_szPath.c_str(), nullptr))
	{
		snprintf(conf_error, conf_error_size, "Failed to load gamedata file");
		return false;
	}

	const KeyValues *game = m_pKeyValues->FindKey(m_szGameDir.c_str());
	if (game)
	{
#if defined _LINUX
		const char *platform = "linux";
#else
		const char *platform = "windows";
#endif

		const KeyValues *offsets = game->FindKey("Offsets");
		if (offsets)
		{
			FOR_EACH_SUBKEY(offsets, it)
			{
				m_umOffsets[it->GetName()] = it->GetInt(platform, -1);
			}
		}

		const KeyValues *signatures = game->FindKey("Signatures");
		if (signatures)
		{
			FOR_EACH_SUBKEY(signatures, it)
			{
				m_umLibraries[it->GetName()] = std::string(it->GetString("library"));
				m_umSignatures[it->GetName()] = std::string(it->GetString(platform));
			}
		}

		const KeyValues *patches = game->FindKey("Patches");
		if (patches)
		{
			FOR_EACH_SUBKEY(patches, it)
			{
				m_umPatches[it->GetName()] = std::string(it->GetString(platform));
			}
		}
	}
	else
	{
		snprintf(conf_error, conf_error_size, "Failed to find game: %s", m_szGameDir.c_str());
		return false;
	}
	return true;
}

const std::string CGameConfig::GetPath()
{
	return m_szPath;
}

const char *CGameConfig::GetSignature(const std::string &name)
{
	auto it = m_umSignatures.find(name);
	if (it == m_umSignatures.end())
	{
		return nullptr;
	}
	return it->second.c_str();
}

const char *CGameConfig::GetPatch(const std::string &name)
{
	auto it = m_umPatches.find(name);
	if (it == m_umPatches.end())
	{
		return nullptr;
	}
	return it->second.c_str();
}

int CGameConfig::GetOffset(const std::string &name)
{
	auto it = m_umOffsets.find(name);
	if (it == m_umOffsets.end())
	{
		return -1;
	}
	return it->second;
}

const char *CGameConfig::GetLibrary(const std::string &name)
{
	auto it = m_umLibraries.find(name);
	if (it == m_umLibraries.end())
	{
		return nullptr;
	}
	return it->second.c_str();
}

CModule *CGameConfig::GetModule(const char *name)
{
	const char *library = this->GetLibrary(name);
	if (library == NULL)
	{
		Fatal("Invalid Library %s\n", name);
		return NULL;
	}

	if (strcmp(library, "engine") == 0)
		return g_CAddresses->GetModuleEngine();
	else if (strcmp(library, "server") == 0)
		return g_CAddresses->GetModuleServer();

	return NULL;
}

bool CGameConfig::IsSymbol(const char *name)
{
	const char *sigOrSymbol = this->GetSignature(name);
	if (sigOrSymbol == NULL || strlen(sigOrSymbol) <= 0)
	{
		Fatal("Missing signature or symbol\n", name);
		return false;
	}
	return sigOrSymbol[0] == '@';
}

const char *CGameConfig::GetSymbol(const char *name)
{
	const char *symbol = this->GetSignature(name);

	if (symbol == NULL || strlen(symbol) <= 1)
	{
		Fatal("Missing symbol\n", name);
		return NULL;
	}
	return symbol + 1;
}

void *CGameConfig::ResolveSignature(const char *name)
{
	CModule *module = this->GetModule(name);
	if (!module)
	{
		Fatal("Invalid Module %s %p\n", name, module);
		return NULL;
	}

	void *address = nullptr;
	if (this->IsSymbol(name))
	{
		const char *symbol = this->GetSymbol(name);

		if (symbol == NULL)
		{
			return NULL;
		}
		address = dlsym(module->m_hModule, symbol);
	}
	else
	{
		const char *signature = this->GetSignature(name);

		if (signature == NULL)
		{
			Fatal("Failed to find signature for %s\n", name);
			return NULL;
		}

		size_t iLength = 0;
		byte *pSignature = HexToByte(signature, iLength);
		if (pSignature == NULL)
			return NULL;
		address = module->FindSignature(pSignature, iLength);
	}

	if (address == NULL)
	{
		Fatal("Failed to find address for %s\n", name);
		return NULL;
	}
	return address;
}

// Static functions
std::string CGameConfig::GetDirectoryName(const std::string &directoryPathInput)
{
	std::string directoryPath = std::string(directoryPathInput);

	size_t found = std::string(directoryPath).find_last_of("/\\");
	if (found != std::string::npos)
	{
		return std::string(directoryPath, found + 1);
	}
	return "";
}

int CGameConfig::HexStringToUint8Array(const char *hexString, uint8_t *byteArray, size_t maxBytes)
{
	if (hexString == NULL)
	{
		printf("Invalid hex string.\n");
		return -1;
	}

	size_t hexStringLength = strlen(hexString);
	size_t byteCount = hexStringLength / 4; // Each "\\x" represents one byte.

	if (hexStringLength % 4 != 0 || byteCount == 0 || byteCount > maxBytes)
	{
		printf("Invalid hex string format or byte count.\n");
		return -1; // Return an error code.
	}

	for (size_t i = 0; i < hexStringLength; i += 4)
	{
		if (sscanf(hexString + i, "\\x%2hhX", &byteArray[i / 4]) != 1)
		{
			printf("Failed to parse hex string at position %zu.\n", i);
			return -1; // Return an error code.
		}
	}

	byteArray[byteCount] = '\0'; // Add a null-terminating character.

	return byteCount; // Return the number of bytes successfully converted.
}

byte *CGameConfig::HexToByte(const char *src, size_t &length)
{
	if (src == NULL || strlen(src) <= 0)
	{
		Fatal("Invalid hex string\n");
		return NULL;
	}

	length = strlen(src) / 4;
	uint8_t *dest = new uint8_t[length];
	int byteCount = HexStringToUint8Array(src, dest, length);
	if (byteCount <= 0)
	{
		Fatal("Invalid hex format %s\n", src);
		return NULL;
	}
	return (byte *)dest;
}