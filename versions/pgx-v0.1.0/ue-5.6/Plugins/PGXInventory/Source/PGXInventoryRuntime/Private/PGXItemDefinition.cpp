// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXItemDefinition.h"
#include "PGXInventoryObservability.h"

// EN: Item definition data asset implementation
// ES: Implementacion del data asset de definicion de item


const FName UPGXItemDefinition::SchemaVersion(TEXT("1.0"));

FPGXJsonValue UPGXItemDefinition::ToJson() const
{
	FPGXJsonValue Out;
	Out.JsonString = FString::Printf(
		TEXT("{\"schema\":{\"type\":\"%s\",\"version\":\"%s\",\"plugin\":\"%s\"},\"data\":{\"ItemTag\":\"%s\",\"CategoryTags\":\"%s\",\"ItemName\":\"%s\",\"DisplayName\":\"%s\",\"Description\":\"%s\",\"MaxStackSize\":%d,\"Weight\":%.6f}}"),
		*GetClass()->GetName(),
		*GetSchemaVersion().ToString(),
		*PGXInventoryObservability::GetOwningPluginName(this, TEXT("PGXInventoryRuntime")).ToString(),
		*PGXInventoryObservability::EscapeJsonString(ItemTag.ToString()),
		*PGXInventoryObservability::EscapeJsonString(CategoryTags.ToStringSimple()),
		*PGXInventoryObservability::EscapeJsonString(ItemName.ToString()),
		*PGXInventoryObservability::EscapeJsonString(DisplayName.ToString()),
		*PGXInventoryObservability::EscapeJsonString(Description.ToString()),
		MaxStackSize,
		Weight);
	return Out;
}

FPGXValidationResult UPGXItemDefinition::FromJson(const FPGXJsonValue& Json)
{
	return PGXInventoryObservability::ValidateJsonEnvelope(
		Json,
		NSLOCTEXT("PGXInventory", "ObservableEmptyPayload", "UPGXItemDefinition FromJson received an empty payload."));
}

FName UPGXItemDefinition::GetSchemaVersion() const
{
	return SchemaVersion;
}

FPGXSchemaDescriptor UPGXItemDefinition::GetSchemaDescriptor() const
{
	return PGXInventoryObservability::MakeSchemaDescriptor(this, GetSchemaVersion(), TEXT("PGXInventoryRuntime"));
}

bool UPGXItemDefinition::OnUse_Implementation(AActor* Instigator) const
{
	return !bRequiresInstigatorForUse || IsValid(Instigator);
}
