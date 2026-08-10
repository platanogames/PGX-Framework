// Copyright PGX Framework. All Rights Reserved.

#include "PGXScreenDefinition.h"
#include "Observability/PGXCoreObservability.h"

// EN: All Object DA fields are inline UPROPERTY defaults. Translation unit exists so the UCLASS
//     reflection table resolves and Build.cs always finds a matching cpp. IPGXObservable
//     support added in Observability support — delegates to PGXCoreObservability helper namespace.
// ES: Todos los campos del Object DA son inline UPROPERTY defaults. Adopcion IPGXObservable
//     añadida en Observability support — delega al namespace helper PGXCoreObservability.

FPGXJsonValue UPGXScreenDefinition::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXScreenDefinition::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXScreenDefinition::GetSchemaVersion() const
{
	return TEXT("1.0");
}

FPGXSchemaDescriptor UPGXScreenDefinition::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
