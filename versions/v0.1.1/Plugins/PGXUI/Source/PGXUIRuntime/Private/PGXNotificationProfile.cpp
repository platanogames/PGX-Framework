// Copyright PGX Framework. All Rights Reserved.

#include "PGXNotificationProfile.h"
#include "Observability/PGXCoreObservability.h"

// EN: All Object DA fields are inline UPROPERTY defaults. Translation unit exists so the UCLASS
//     reflection table resolves. IPGXObservable support added in Observability support.
// ES: Todos los campos del Object DA son inline UPROPERTY defaults. Adopcion IPGXObservable
//     añadida en Observability support.

FPGXJsonValue UPGXNotificationProfile::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXNotificationProfile::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXNotificationProfile::GetSchemaVersion() const
{
	return TEXT("1.0");
}

FPGXSchemaDescriptor UPGXNotificationProfile::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
