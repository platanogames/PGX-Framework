// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXPSOObservability.h"

#include "UObject/Class.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"

namespace
{
	/**
	 * EN: Resolve the owning plugin name from the UClass package outer.
	 * ES: Resuelve el nombre del plugin propietario desde el paquete outer del UClass.
	 */
	FName ResolveOwningPluginName(const UClass* Class)
	{
		if (Class == nullptr)
		{
			return NAME_None;
		}

		const UPackage* Package = Class->GetOuterUPackage();
		if (Package == nullptr)
		{
			return NAME_None;
		}

		const FString PackagePath = Package->GetName();
		if (PackagePath.StartsWith(TEXT("/Script/")))
		{
			return FName(*PackagePath.RightChop(8));
		}
		return FName(*PackagePath);
	}

	/**
	 * EN: Append simple metadata to a schema field descriptor (ClampMin/Max + Categories).
	 * ES: Agrega metadata simple al descriptor de campo del schema (ClampMin/Max + Categories).
	 */
	void AppendFieldMetadata(const FProperty* Property, FPGXSchemaField& OutField)
	{
		if (Property == nullptr)
		{
			return;
		}
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
		OutField.Constraints = FText::FromString(ConstraintBuffer);
	}
}

namespace PGXPSOObservability
{
	FPGXJsonValue MakeJsonEnvelope(const UObject* Owner, FName SchemaVersion)
	{
		FPGXJsonValue Envelope;
		if (Owner == nullptr)
		{
			return Envelope;
		}
		const UClass* Class = Owner->GetClass();
		const FName PluginName = ResolveOwningPluginName(Class);

		Envelope.JsonString = FString::Printf(
			TEXT("{\"type\":\"%s\",\"version\":\"%s\",\"plugin\":\"%s\"}"),
			*Class->GetName(),
			*SchemaVersion.ToString(),
			*PluginName.ToString());
		return Envelope;
	}

	FPGXValidationResult ValidateJsonEnvelope(const FPGXJsonValue& Json)
	{
		FPGXValidationResult Result;
		if (Json.IsEmpty())
		{
			return FPGXValidationResult::MakeFailure(
				TEXT("EmptyPayload"),
				TEXT(""),
				NSLOCTEXT("PGXPSOObservability", "EmptyPayload", "PGXPSO observable envelope is empty."));
		}
		if (!Json.JsonString.Contains(TEXT("\"type\"")))
		{
			return FPGXValidationResult::MakeFailure(
				TEXT("MissingType"),
				TEXT("schema.type"),
				NSLOCTEXT("PGXPSOObservability", "MissingType", "PGXPSO observable envelope missing 'type' field."));
		}
		if (!Json.JsonString.Contains(TEXT("\"version\"")))
		{
			return FPGXValidationResult::MakeFailure(
				TEXT("MissingVersion"),
				TEXT("schema.version"),
				NSLOCTEXT("PGXPSOObservability", "MissingVersion", "PGXPSO observable envelope missing 'version' field."));
		}
		Result.bValid = true;
		return Result;
	}

	FPGXSchemaDescriptor MakeSchemaDescriptor(const UObject* Owner, FName SchemaVersion)
	{
		FPGXSchemaDescriptor Descriptor;
		if (Owner == nullptr)
		{
			return Descriptor;
		}
		const UClass* Class = Owner->GetClass();
		Descriptor.TypeName = Class->GetFName();
		Descriptor.SchemaVersion = SchemaVersion;
		Descriptor.OwningPlugin = ResolveOwningPluginName(Class);

		for (TFieldIterator<FProperty> It(Class); It; ++It)
		{
			const FProperty* Property = *It;
			if (Property == nullptr)
			{
				continue;
			}
			FPGXSchemaField Field;
			Field.FieldName = Property->GetFName();
			Field.FieldType = FName(*Property->GetCPPType());
			Field.bRequired = !Property->HasAnyPropertyFlags(CPF_Transient);
			AppendFieldMetadata(Property, Field);
			Descriptor.Fields.Add(MoveTemp(Field));
		}
		return Descriptor;
	}
}
