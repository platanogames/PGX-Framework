// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXVersionControlSettings.h"

#include "UObject/Class.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"

#define LOCTEXT_NAMESPACE "PGXVersionControlSettings"

UPGXVersionControlSettings::UPGXVersionControlSettings()
{
}

FText UPGXVersionControlSettings::GetSectionText() const
{
	return LOCTEXT("SectionText", "Version Control");
}

FText UPGXVersionControlSettings::GetSectionDescription() const
{
	return LOCTEXT("SectionDesc",
		"PGX Version Control overlay settings: auto-tagging, commit templates, and pre-commit validation rules.");
}

#undef LOCTEXT_NAMESPACE

// ============================================================================
// The plugin has one observable settings class, so its JSON conversion remains
// local instead of introducing a separate helper namespace.
// ============================================================================

namespace
{
	FName GetVersionControlOwningPlugin()
	{
		return TEXT("PGXVersionControlEditor");
	}
}

FPGXJsonValue UPGXVersionControlSettings::ToJson() const
{
	FPGXJsonValue Out;
	const UClass* Class = GetClass();
	if (!Class)
	{
		return Out;
	}

	Out.JsonString = FString::Printf(
		TEXT("{\"schema\":{\"type\":\"%s\",\"version\":\"%s\",\"plugin\":\"%s\"},\"data\":{}}"),
		*Class->GetName(),
		*GetSchemaVersion().ToString(),
		*GetVersionControlOwningPlugin().ToString());
	return Out;
}

FPGXValidationResult UPGXVersionControlSettings::FromJson(const FPGXJsonValue& Json)
{
	if (Json.IsEmpty())
	{
		return FPGXValidationResult::MakeFailure(
			TEXT("EmptyPayload"),
			TEXT(""),
			NSLOCTEXT("PGXVersionControlSettings", "EmptyPayload", "FromJson called with empty FPGXJsonValue payload"));
	}

	return FPGXValidationResult::MakeValid();
}

FName UPGXVersionControlSettings::GetSchemaVersion() const
{
	return TEXT("1.0");
}

FPGXSchemaDescriptor UPGXVersionControlSettings::GetSchemaDescriptor() const
{
	FPGXSchemaDescriptor Descriptor;
	const UClass* Class = GetClass();
	if (!Class)
	{
		return Descriptor;
	}

	Descriptor.TypeName = Class->GetFName();
	Descriptor.SchemaVersion = GetSchemaVersion();
	Descriptor.OwningPlugin = GetVersionControlOwningPlugin();

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
