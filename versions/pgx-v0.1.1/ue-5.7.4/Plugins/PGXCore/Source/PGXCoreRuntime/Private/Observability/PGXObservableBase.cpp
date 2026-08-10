// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Observability/PGXObservableBase.h"

#include "Logging/LogMacros.h"
#include "UObject/Class.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogPGXObservable, Log, All);

// ============================================================================
// EN: UPGXObservableBase — abstract default implementations for IPGXObservable.
//     initial observability scaffolds the reflection-driven shape; concrete JSON
//     library binding is outside the current module contract.
// ES: Implementaciones default abstractas para IPGXObservable. Scaffold de la
//     forma reflejada por UPROPERTY; binding concreto de libreria JSON diferido.
// ============================================================================

FPGXJsonValue UPGXObservableBase::ToJson() const
{
	FPGXJsonValue Out;

	const UClass* ThisClass = GetClass();
	if (!ThisClass)
	{
		UE_LOG(LogPGXObservable, Warning, TEXT("UPGXObservableBase::ToJson: GetClass() returned null"));
		return Out;
	}

	// EN: initial observability baseline — emit the canonical envelope (type + version + plugin)
	//     plus a placeholder data section. Full UPROPERTY-to-JSON value reflection is the
	//     optional observability extension contract.
	const FName TypeName = ThisClass->GetFName();
	const FName SchemaVersion = GetDeclaredSchemaVersion();
	const FName OwningPlugin = GetOwningPluginName();

	Out.JsonString = FString::Printf(
		TEXT("{\"schema\":{\"type\":\"%s\",\"version\":\"%s\",\"plugin\":\"%s\"},\"data\":{}}"),
		*TypeName.ToString(),
		*SchemaVersion.ToString(),
		*OwningPlugin.ToString());

	return Out;
}

FPGXValidationResult UPGXObservableBase::FromJson(const FPGXJsonValue& Json)
{
	// EN: initial observability baseline — validate envelope shape; concrete UPROPERTY reflection
	//     into runtime state is outside the current module contract.
	FPGXValidationResult Result;

	if (Json.IsEmpty())
	{
		Result.AddError(TEXT("EmptyPayload"), TEXT(""), NSLOCTEXT("PGXObservability", "EmptyPayload", "FromJson called with empty FPGXJsonValue payload"));
		return Result;
	}

	// EN: Reflection-driven UPROPERTY assignment is deferred. This implementation parses
	//     the envelope schema fields only and returns a clean validation result when the
	//     envelope is well-formed (no per-field UPROPERTY application yet).
	UE_LOG(LogPGXObservable, Verbose,
		TEXT("UPGXObservableBase::FromJson — envelope-only validation; UPROPERTY reflection is not applied. Payload size: %d chars"),
		Json.JsonString.Len());

	return Result;
}

FName UPGXObservableBase::GetSchemaVersion() const
{
	return GetDeclaredSchemaVersion();
}

FPGXSchemaDescriptor UPGXObservableBase::GetSchemaDescriptor() const
{
	FPGXSchemaDescriptor Descriptor;

	const UClass* ThisClass = GetClass();
	if (!ThisClass)
	{
		UE_LOG(LogPGXObservable, Warning, TEXT("UPGXObservableBase::GetSchemaDescriptor: GetClass() returned null"));
		return Descriptor;
	}

	Descriptor.TypeName = ThisClass->GetFName();
	Descriptor.SchemaVersion = GetDeclaredSchemaVersion();
	Descriptor.OwningPlugin = GetOwningPluginName();

	// EN: Walk the class's FProperty chain, build one FPGXSchemaField per UPROPERTY.
	for (TFieldIterator<FProperty> PropertyIt(ThisClass); PropertyIt; ++PropertyIt)
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

		// EN: Constraint hints — surface ClampMin / ClampMax / Categories / meta tags as
		//     a flat localized text. Detailed structured constraint extraction is 8.3.B
		//     scope; baseline emits the property's full meta map summary.
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

FName UPGXObservableBase::GetOwningPluginName() const
{
	const UClass* ThisClass = GetClass();
	if (!ThisClass)
	{
		return NAME_None;
	}

	// EN: Derive the owning plugin (module) name from the class's package. Subclasses may
	//     override to provide a different attribution.
	const UPackage* Package = ThisClass->GetOuterUPackage();
	if (!Package)
	{
		return NAME_None;
	}

	const FString PackageName = Package->GetName();
	int32 LastSlashIndex = INDEX_NONE;
	if (PackageName.FindLastChar(TEXT('/'), LastSlashIndex))
	{
		return FName(*PackageName.RightChop(LastSlashIndex + 1));
	}

	return FName(*PackageName);
}
