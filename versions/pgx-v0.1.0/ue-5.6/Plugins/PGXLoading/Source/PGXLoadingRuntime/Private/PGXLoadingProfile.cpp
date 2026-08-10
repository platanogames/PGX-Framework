// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXLoadingProfile.h"

UPGXLoadingProfile::UPGXLoadingProfile()
{
	// EN: Populate widget contract — tells the developer which named widgets
	//     their Loading Widget Blueprint should contain.
	// ES: Poblar contrato de widget — le dice al desarrollador que widgets con nombre
	//     debe contener su Widget Blueprint de Loading.
	WidgetContract.ExpectedSlots = {
		FPGXWidgetSlotInfo(TEXT("IMG_Background"), NSLOCTEXT("PGXLoading", "BG", "Background image or material"), TEXT("UImage"), false),
		FPGXWidgetSlotInfo(TEXT("PB_Progress"),    NSLOCTEXT("PGXLoading", "PB", "Progress bar (level load + PSO)"), TEXT("UProgressBar"), true),
		FPGXWidgetSlotInfo(TEXT("TXT_Status"),     NSLOCTEXT("PGXLoading", "St", "Status text"), TEXT("UTextBlock"), false),
		FPGXWidgetSlotInfo(TEXT("TXT_Tip"),        NSLOCTEXT("PGXLoading", "Tp", "Rotating tips"), TEXT("UTextBlock"), false),
		FPGXWidgetSlotInfo(TEXT("BTN_Skip"),       NSLOCTEXT("PGXLoading", "Sk", "Skip button"), TEXT("UButton"), false),
		FPGXWidgetSlotInfo(TEXT("BTN_Continue"),    NSLOCTEXT("PGXLoading", "Co", "Continue button (manual close)"), TEXT("UButton"), false),
	};
}
