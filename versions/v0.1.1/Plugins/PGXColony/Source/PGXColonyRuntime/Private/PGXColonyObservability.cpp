// Copyright PGX Framework. All Rights Reserved.

#include "PGXColonyObservability.h"
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
			return TEXT("PGXColonyRuntime");
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

FPGXJsonValue PGXColonyObservability::MakeJsonEnvelope(const UObject* Object, FName SchemaVersion, const FString& DataJson)
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
