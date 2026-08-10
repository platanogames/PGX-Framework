// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Path/PGXPathResolver.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/FileManager.h"

// ============================================================================
// EN: Static member initialization
// ES: Inicializacion de miembros estaticos
// ============================================================================

TMap<FPGXPathCacheKey, FString> FPGXPathResolver::PathCache;
FCriticalSection FPGXPathResolver::CacheLock;

// ============================================================================
// EN: Core API / ES: API Principal
// ============================================================================

FString FPGXPathResolver::ResolvePath(const FPGXPathContext& Context)
{
	switch (Context.Strategy)
	{
	case EPGXPathStrategy::Unique:
		return FormatUniquePath(Context);

	case EPGXPathStrategy::Session:
		return FormatSessionPath(Context);

	case EPGXPathStrategy::Profile:
		return FormatProfilePath(Context);

	case EPGXPathStrategy::NamedSlot:
		return FormatNamedSlotPath(Context);

	case EPGXPathStrategy::Custom:
		// EN: Custom strategy — system provides its own path logic, we just combine base components
		// ES: Estrategia Custom — el sistema provee su propia logica de path, solo combinamos componentes base
		return FPaths::Combine(GetPlatformBasePath(), Context.BaseDirectory,
			Context.Prefix + Context.FileExtension);

	default:
		return FormatUniquePath(Context);
	}
}

FString FPGXPathResolver::ResolveAndCache(const FPGXPathContext& Context)
{
	FPGXPathCacheKey Key;
	Key.SystemTag = Context.SystemTag;
	Key.DomainTag = Context.DomainTag;

	// EN: For NamedSlot, include the explicit slot name in the key so different slots cache separately
	// ES: Para NamedSlot, incluir el nombre de slot explicito en la key para que diferentes slots se cacheen por separado
	if (Context.Strategy == EPGXPathStrategy::NamedSlot)
	{
		Key.SlotIdentifier = Context.ExplicitSlotName;
	}

	FScopeLock Lock(&CacheLock);

	// EN: Return cached value if available
	// ES: Retornar valor cacheado si esta disponible
	if (const FString* Cached = PathCache.Find(Key))
	{
		return *Cached;
	}

	// EN: Resolve, cache, and return
	// ES: Resolver, cachear, y retornar
	FString Resolved = ResolvePath(Context);
	PathCache.Add(Key, Resolved);
	return Resolved;
}

const FString* FPGXPathResolver::GetCachedPath(FGameplayTag SystemTag, FGameplayTag DomainTag,
	const FString& SlotIdentifier)
{
	FPGXPathCacheKey Key;
	Key.SystemTag = SystemTag;
	Key.DomainTag = DomainTag;
	Key.SlotIdentifier = SlotIdentifier;

	FScopeLock Lock(&CacheLock);
	return PathCache.Find(Key);
}

void FPGXPathResolver::InvalidateCache(FGameplayTag SystemTag)
{
	FScopeLock Lock(&CacheLock);

	// EN: Remove all entries matching this system tag
	// ES: Remover todas las entradas que coincidan con este tag de sistema
	TArray<FPGXPathCacheKey> KeysToRemove;
	for (const auto& Pair : PathCache)
	{
		if (Pair.Key.SystemTag == SystemTag)
		{
			KeysToRemove.Add(Pair.Key);
		}
	}

	for (const FPGXPathCacheKey& Key : KeysToRemove)
	{
		PathCache.Remove(Key);
	}
}

void FPGXPathResolver::ClearCache()
{
	FScopeLock Lock(&CacheLock);
	PathCache.Empty();
}

// ============================================================================
// EN: Utilities / ES: Utilidades
// ============================================================================

bool FPGXPathResolver::ValidatePath(const FString& Path)
{
	const FString Directory = FPaths::GetPath(Path);
	return EnsureDirectoryExists(Directory);
}

bool FPGXPathResolver::EnsureDirectoryExists(const FString& DirectoryPath)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (PlatformFile.DirectoryExists(*DirectoryPath))
	{
		return true;
	}

	return PlatformFile.CreateDirectoryTree(*DirectoryPath);
}

FString FPGXPathResolver::GetPlatformBasePath()
{
	return FPaths::ProjectSavedDir();
}

int32 FPGXPathResolver::GetCacheSize()
{
	FScopeLock Lock(&CacheLock);
	return PathCache.Num();
}

// ============================================================================
// EN: Strategy Implementations / ES: Implementaciones de Estrategia
// ============================================================================

FString FPGXPathResolver::FormatUniquePath(const FPGXPathContext& Context)
{
	// EN: Single static path: BasePath/BaseDirectory/Prefix.ext
	//     Overwritten each time. Simplest strategy.
	// ES: Path estatico unico: BasePath/BaseDirectory/Prefix.ext
	//     Sobreescrito cada vez. Estrategia mas simple.

	const FString FileName = Context.Prefix.IsEmpty()
		? TEXT("data") + Context.FileExtension
		: Context.Prefix + Context.FileExtension;

	return FPaths::Combine(GetPlatformBasePath(), Context.BaseDirectory, FileName);
}

FString FPGXPathResolver::FormatSessionPath(const FPGXPathContext& Context)
{
	// EN: Timestamped subfolder: BasePath/BaseDirectory/Sessions/2026-02-14_143022/Prefix.ext
	//     Each session gets a unique folder. Near-zero collision risk.
	// ES: Subcarpeta con timestamp: BasePath/BaseDirectory/Sessions/2026-02-14_143022/Prefix.ext
	//     Cada sesion obtiene un folder unico. Riesgo de colision casi cero.

	const FDateTime Now = FDateTime::UtcNow();
	const FString Timestamp = Now.ToString(TEXT("%Y-%m-%d_%H%M%S"));

	const FString FileName = Context.Prefix.IsEmpty()
		? TEXT("data") + Context.FileExtension
		: Context.Prefix + Context.FileExtension;

	return FPaths::Combine(
		GetPlatformBasePath(),
		Context.BaseDirectory,
		Context.SessionFolderName,
		Timestamp,
		FileName
	);
}

FString FPGXPathResolver::FormatProfilePath(const FPGXPathContext& Context)
{
	// EN: Auto-incremented: BasePath/BaseDirectory/Profiles/Prefix_N.ext
	//     Counts existing files with same prefix to determine next ID.
	// ES: Auto-incremental: BasePath/BaseDirectory/Profiles/Prefix_N.ext
	//     Cuenta archivos existentes con mismo prefijo para determinar siguiente ID.

	const FString ProfileDir = FPaths::Combine(
		GetPlatformBasePath(),
		Context.BaseDirectory,
		Context.ProfileFolderName
	);

	const FString Prefix = Context.Prefix.IsEmpty() ? TEXT("Save") : Context.Prefix;
	const int32 NextId = CountExistingFiles(ProfileDir, Prefix, Context.FileExtension);
	const FString FileName = FString::Printf(TEXT("%s_%d%s"), *Prefix, NextId, *Context.FileExtension);

	FString FullPath = FPaths::Combine(ProfileDir, FileName);

	// EN: Handle collision (unlikely but defensive)
	// ES: Manejar colision (improbable pero defensivo)
	return HandleCollision(FullPath, Context);
}

FString FPGXPathResolver::FormatNamedSlotPath(const FPGXPathContext& Context)
{
	// EN: Named slot: BasePath/BaseDirectory/ExplicitSlotName.ext
	//     User provides the exact slot name.
	// ES: Slot con nombre: BasePath/BaseDirectory/ExplicitSlotName.ext
	//     El usuario provee el nombre de slot exacto.

	const FString SlotName = Context.ExplicitSlotName.IsEmpty()
		? TEXT("Slot_Default")
		: Context.ExplicitSlotName;

	return FPaths::Combine(
		GetPlatformBasePath(),
		Context.BaseDirectory,
		SlotName + Context.FileExtension
	);
}

// ============================================================================
// EN: Collision Handling / ES: Manejo de Colisiones
// ============================================================================

FString FPGXPathResolver::HandleCollision(const FString& BasePath, const FPGXPathContext& /*Context*/)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.FileExists(*BasePath))
	{
		return BasePath;
	}

	// EN: File exists — auto-increment suffix until we find a free path
	// ES: Archivo existe — auto-incrementar sufijo hasta encontrar un path libre
	const FString Directory = FPaths::GetPath(BasePath);
	const FString BaseFileName = FPaths::GetBaseFilename(BasePath);
	const FString Extension = FPaths::GetExtension(BasePath, true); // includes dot

	for (int32 i = 1; i < 10000; ++i)
	{
		const FString Candidate = FPaths::Combine(
			Directory,
			FString::Printf(TEXT("%s_%d%s"), *BaseFileName, i, *Extension)
		);

		if (!PlatformFile.FileExists(*Candidate))
		{
			return Candidate;
		}
	}

	// EN: Fallback — return base path (overwrite) if somehow 10000 collisions exist
	// ES: Fallback — retornar path base (sobreescribir) si de alguna forma existen 10000 colisiones
	return BasePath;
}

int32 FPGXPathResolver::CountExistingFiles(const FString& Directory, const FString& Prefix,
	const FString& Extension)
{
	IFileManager& FileManager = IFileManager::Get();

	// EN: Find all files matching Prefix_*.Extension in the directory
	// ES: Encontrar todos los archivos que coincidan con Prefix_*.Extension en el directorio
	const FString Pattern = FPaths::Combine(Directory, Prefix + TEXT("_*") + Extension);

	TArray<FString> FoundFiles;
	FileManager.FindFiles(FoundFiles, *Pattern, true, false);

	return FoundFiles.Num();
}
