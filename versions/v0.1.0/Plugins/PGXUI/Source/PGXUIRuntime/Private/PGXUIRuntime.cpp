// Copyright PGX Framework. All Rights Reserved.

#include "PGXUIRuntime.h"
#include "Logging/PGXLogMacros.h"

#include "PGXUIConfig.h"
#include "PGXScreenDefinition.h"
#include "PGXNotificationProfile.h"
#include "PGXWidgetPoolProfile.h"
#include "Observability/PGXObservabilityRegistry.h"

DEFINE_LOG_CATEGORY_STATIC(LogPGXUI, Log, All);

#define LOCTEXT_NAMESPACE "FPGXUIRuntimeModule"

void FPGXUIRuntimeModule::StartupModule()
{
	// EN: PGXUIRuntime module started. Presentation services initialize through UPGXUISubsystem.
	// ES: Modulo PGXUIRuntime iniciado. Los servicios de presentacion inicializan via UPGXUISubsystem.
	PGX_LOG_INFO(LogPGXUI, TEXT("PGXUIRuntime: Module started"));

	// EN: Observability support — manual fallback registration of all 4 PGXUI
	//     observable DA classes with FPGXObservabilityRegistry. Mirror
	//     PGXEnvironment 8.3.B / PGXAI 8.3.C reference. Order independent
	//     of BootstrapDiscovery.
	// ES: Observability support — registro manual fallback de las 4 clases DA
	//     observables PGXUI con FPGXObservabilityRegistry. Mirror
	//     referencia PGXEnvironment 8.3.B / PGXAI 8.3.C. Orden
	//     independiente de BootstrapDiscovery.
	FPGXObservabilityRegistry::Register(UPGXUIConfig::StaticClass());
	FPGXObservabilityRegistry::Register(UPGXScreenDefinition::StaticClass());
	FPGXObservabilityRegistry::Register(UPGXNotificationProfile::StaticClass());
	FPGXObservabilityRegistry::Register(UPGXWidgetPoolProfile::StaticClass());
}

void FPGXUIRuntimeModule::ShutdownModule()
{
	// EN: Cleanup UI systems before unloading
	// ES: Limpiar sistemas de UI antes de descargar
	PGX_LOG_INFO(LogPGXUI, TEXT("PGXUIRuntime: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXUIRuntimeModule, PGXUIRuntime)