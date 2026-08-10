// Copyright PGX Framework. All Rights Reserved.

#include "PGXVehiclesObservability.h"

#include "Observability/PGXJsonValue.h"
#include "UObject/Class.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"

FString PGXVehiclesObservability::EscapeJsonString(const FString& Value)
{
	FString Escaped = Value;
	Escaped.ReplaceInline(TEXT("\\"), TEXT("\\\\"));
	Escaped.ReplaceInline(TEXT("\""), TEXT("\\\""));
	Escaped.ReplaceInline(TEXT("\r"), TEXT("\\r"));
	Escaped.ReplaceInline(TEXT("\n"), TEXT("\\n"));
	return Escaped;
}

FName PGXVehiclesObservability::GetOwningPluginName(const UObject* Object, FName FallbackPluginName)
{
	const UClass* Class = Object ? Object->GetClass() : nullptr;
	const UPackage* Package = Class ? Class->GetOuterUPackage() : nullptr;
	if (!Package)
	{
		return FallbackPluginName;
	}

	const FString PackageName = Package->GetName();
	int32 LastSlashIndex = INDEX_NONE;
	if (PackageName.FindLastChar(TEXT('/'), LastSlashIndex))
	{
		return FName(*PackageName.RightChop(LastSlashIndex + 1));
	}

	return FName(*PackageName);
}

FPGXValidationResult PGXVehiclesObservability::ValidateJsonEnvelope(const FPGXJsonValue& Json, const FText& EmptyPayloadMessage)
{
	if (Json.IsEmpty())
	{
		return FPGXValidationResult::MakeFailure(TEXT("EmptyPayload"), TEXT(""), EmptyPayloadMessage);
	}

	// EN: Concrete JSON-to-UPROPERTY mutation is not supported because the PGXCore
	//     observability parser/migration policy is locked. Export/schema is safe now.
	return FPGXValidationResult::MakeValid();
}

FPGXSchemaDescriptor PGXVehiclesObservability::MakeSchemaDescriptor(const UObject* Object, FName SchemaVersion, FName FallbackPluginName)
{
	FPGXSchemaDescriptor Descriptor;
	const UClass* Class = Object ? Object->GetClass() : nullptr;
	if (!Class)
	{
		return Descriptor;
	}

	Descriptor.TypeName = Class->GetFName();
	Descriptor.SchemaVersion = SchemaVersion;
	Descriptor.OwningPlugin = GetOwningPluginName(Object, FallbackPluginName);

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
#if WITH_METADATA
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
#endif

		Field.Constraints = FText::FromString(ConstraintBuffer);
		Descriptor.Fields.Add(Field);
	}

	return Descriptor;
}
