// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Construction/PGXConstructionStartupValidator.h"
#include "Construction/PGXConstructionSettings.h"
#include "Construction/PGXConstructionResolver.h"
#include "Notifications/PGXEditorNotification.h"
#include "Containers/Ticker.h"

void FPGXConstructionStartupValidator::ScheduleValidation()
{
	const UPGXConstructionSettings* Settings = GetDefault<UPGXConstructionSettings>();
	if (!Settings || !Settings->bValidateOnStartup)
	{
		return;
	}

	// EN: 3s delay so Slate is fully initialized for toast notifications
	// ES: 3s de delay para que Slate este completamente inicializado para notificaciones toast
	FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateLambda([](float) -> bool
		{
			RunValidation();
			return false; // EN: One-shot / ES: Una sola ejecucion
		}),
		3.0f
	);
}

void FPGXConstructionStartupValidator::RunValidation()
{
	const UPGXConstructionSettings* Settings = GetDefault<UPGXConstructionSettings>();
	if (!Settings)
	{
		return;
	}

	TArray<FText> Warnings;
	Settings->ValidateClassSources(Warnings);

	if (Warnings.Num() == 0)
	{
		return;
	}

	// EN: Log each warning individually / ES: Log cada warning individualmente
	for (const FText& Warning : Warnings)
	{
		UE_LOG(LogPGXConstruction, Warning, TEXT("PGX Construction: %s"), *Warning.ToString());
	}

	// EN: Single summary toast / ES: Una sola notificacion resumen
	const FText Summary = FText::Format(
		NSLOCTEXT("PGXConstruction", "ClassSourceSummary", "PGX Construction: {0} warning(s). Check Output Log for details."),
		FText::AsNumber(Warnings.Num())
	);
	UPGXEditorNotification::ShowWarning(Summary);
}
