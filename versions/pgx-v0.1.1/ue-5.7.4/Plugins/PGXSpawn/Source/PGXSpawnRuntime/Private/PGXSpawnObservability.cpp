// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXSpawnObservability.h"
#include "UObject/Class.h"
#include "UObject/Package.h"

namespace
{
	FName GetOwningPluginName(const UObject* Object)
	{
		const UClass* Class = Object ? Object->GetClass() : nullptr;
		const UPackage* Package = Class ? Class->GetOuterUPackage() : nullptr;
		if (!Package)
		{
			return TEXT("PGXSpawnRuntime");
		}

		const FString PackageName = Package->GetName();
		int32 LastSlashIndex = INDEX_NONE;
		if (PackageName.FindLastChar(TEXT('/'), LastSlashIndex))
		{
			return FName(*PackageName.RightChop(LastSlashIndex + 1));
		}

		return FName(*PackageName);
	}
}

FString PGXSpawnObservability::EscapeJsonString(const FString& Value)
{
	FString Escaped = Value.Replace(TEXT("\\"), TEXT("\\\\"));
	Escaped = Escaped.Replace(TEXT("\""), TEXT("\\\""));
	return Escaped;
}

FPGXJsonValue PGXSpawnObservability::MakeJsonEnvelope(const UObject* Object, FName SchemaVersion, const FString& DataJson)
{
	FPGXJsonValue Out;
	const UClass* Class = Object ? Object->GetClass() : nullptr;
	if (!Class)
	{
		return Out;
	}

	Out.JsonString = FString::Printf(
		TEXT("{\"schema\":{\"type\":\"%s\",\"version\":\"%s\",\"plugin\":\"%s\"},\"data\":%s}"),
		*Class->GetName(),
		*SchemaVersion.ToString(),
		*GetOwningPluginName(Object).ToString(),
		DataJson.IsEmpty() ? TEXT("{}") : *DataJson);
	return Out;
}
