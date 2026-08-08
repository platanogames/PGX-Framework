// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Observability/PGXCoreObservability.h"

#include "UObject/Class.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"

namespace
{
	FName GetOwningPluginName(const UObject* Object)
	{
		const UClass* Class = Object ? Object->GetClass() : nullptr;
		const UPackage* Package = Class ? Class->GetOuterUPackage() : nullptr;
		if (!Package)
		{
			return TEXT("PGXCoreRuntime");
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

FPGXJsonValue PGXCoreObservability::MakeJsonEnvelope(const UObject* Object, FName SchemaVersion)
{
	FPGXJsonValue Out;
	if (!Object)
	{
		return Out;
	}

	const UClass* Class = Object->GetClass();
	if (!Class)
	{
		return Out;
	}

	// EN: Mirror PGXCore 8.3.A baseline + PGXEnvironment / PGXAI 8.3.C / PGXUI 8.3.C reference
	//     envelope shape. UPROPERTY-to-JSON value reflection deferred to 8.3.B+ implementation
	//     contract while property-level serialization remains a future extension.
	// ES: Mirror baseline PGXCore 8.3.A + referencias PGXEnvironment / PGXAI / PGXUI 8.3.C.
	Out.JsonString = FString::Printf(
		TEXT("{\"schema\":{\"type\":\"%s\",\"version\":\"%s\",\"plugin\":\"%s\"},\"data\":{}}"),
		*Class->GetName(),
		*SchemaVersion.ToString(),
		*GetOwningPluginName(Object).ToString());
	return Out;
}

FPGXValidationResult PGXCoreObservability::ValidateJsonEnvelope(const FPGXJsonValue& Json)
{
	if (Json.IsEmpty())
	{
		return FPGXValidationResult::MakeFailure(
			TEXT("EmptyPayload"),
			TEXT(""),
			NSLOCTEXT("PGXCoreObservability", "EmptyPayload", "FromJson called with empty FPGXJsonValue payload"));
	}

	return FPGXValidationResult::MakeValid();
}

FPGXSchemaDescriptor PGXCoreObservability::MakeSchemaDescriptor(const UObject* Object, FName SchemaVersion)
{
	FPGXSchemaDescriptor Descriptor;
	const UClass* Class = Object ? Object->GetClass() : nullptr;
	if (!Class)
	{
		return Descriptor;
	}

	Descriptor.TypeName = Class->GetFName();
	Descriptor.SchemaVersion = SchemaVersion;
	Descriptor.OwningPlugin = GetOwningPluginName(Object);

	for (TFieldIterator<FProperty> PropertyIt(Class); PropertyIt; ++PropertyIt)
	{
		FProperty* Property = *PropertyIt;
		if (!Property)
		{
			continue;
		}

		FPGXSchemaField Field;
		Field.FieldName = Property->GetFName();
		Field.FieldType = FName(*Property->GetCPPType());
		Field.bRequired = !Property->HasAnyPropertyFlags(CPF_Transient);

		FString ConstraintBuffer;
		if (Property->HasMetaData(TEXT("ClampMin")))
		{
			ConstraintBuffer += FString::Printf(TEXT("ClampMin=%s "), *Property->GetMetaData(TEXT("ClampMin")));
		}
		if (Property->HasMetaData(TEXT("ClampMax")))
		{
			ConstraintBuffer += FString::Printf(TEXT("ClampMax=%s "), *Property->GetMetaData(TEXT("ClampMax")));
		}
		if (Property->HasMetaData(TEXT("Categories")))
		{
			ConstraintBuffer += FString::Printf(TEXT("Categories=%s "), *Property->GetMetaData(TEXT("Categories")));
		}

		Field.Constraints = FText::FromString(ConstraintBuffer);
		Descriptor.Fields.Add(Field);
	}

	return Descriptor;
}
