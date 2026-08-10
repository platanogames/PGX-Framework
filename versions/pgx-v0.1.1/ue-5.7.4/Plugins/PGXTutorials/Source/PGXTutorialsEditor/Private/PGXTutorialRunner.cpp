// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Tutorial runner — state machine + action execution + overlay lifecycle.
// ES: Runner del tutorial — maquina de estados + ejecucion de acciones + ciclo de vida del overlay.

#include "PGXTutorialRunner.h"
#include "Logging/PGXLogMacros.h"
#include "SPGXTutorialOverlay.h"
#include "PGXTutorialActionExecutor.h"
#include "Style/PGXVisualTokens.h"

#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SWindow.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/ConfigCacheIni.h"

DEFINE_LOG_CATEGORY_STATIC(LogPGXTutorials, Log, All);

// EN: Static members
// ES: Miembros estaticos
EPGXTutorialLanguage FPGXTutorialRunner::CurrentLanguage = EPGXTutorialLanguage::English;
bool FPGXTutorialRunner::bKeepHubOpen = true;
FString FPGXTutorialRunner::TutorialBasePath = TEXT("/Game/PGX_Tutorial");

FPGXTutorialRunner::FPGXTutorialRunner()
{
}

FPGXTutorialRunner::~FPGXTutorialRunner()
{
	if (bIsActive)
	{
		Close();
	}
}

void FPGXTutorialRunner::StartTutorial(const TArray<FPGXTutorialStep>& InSteps)
{
	if (InSteps.Num() == 0)
	{
		PGX_LOG_WARNING(LogPGXTutorials, TEXT("StartTutorial called with empty steps array."));
		return;
	}

	if (bIsActive)
	{
		Close();
	}

	// EN: Reset action tracking for new tutorial / ES: Resetear tracking de acciones para nuevo tutorial
	FPGXTutorialActionExecutor::ResetTracking();

	Steps = InSteps;
	CurrentStepIndex = 0;
	bIsActive = true;
	LastActionResult = FPGXTutorialActionResult();
	LastTutorialOpenedTabId = NAME_None;
	bLastTabWasOpenedByUs = false;

	PGX_LOG_INFO(LogPGXTutorials, TEXT("Tutorial started with %d steps. BasePath: %s"), Steps.Num(), *TutorialBasePath);

	CreateOverlay();
	ApplyCurrentStep();
}

void FPGXTutorialRunner::NextStep()
{
	if (!bIsActive) return;

	if (IsLastStep())
	{
		Close();
		return;
	}

	CurrentStepIndex++;
	ApplyCurrentStep();
}

void FPGXTutorialRunner::PrevStep()
{
	if (!bIsActive || IsFirstStep()) return;

	CurrentStepIndex--;
	ApplyCurrentStep();
}

void FPGXTutorialRunner::Close()
{
	if (!bIsActive) return;

	bIsActive = false;
	DestroyOverlay();
	Steps.Empty();
	CurrentStepIndex = 0;
	LastActionResult = FPGXTutorialActionResult();
	LastTutorialOpenedTabId = NAME_None;
	bLastTabWasOpenedByUs = false;

	PGX_LOG_INFO(LogPGXTutorials, TEXT("Tutorial closed."));
	OnTutorialClosed.ExecuteIfBound();
}

const FPGXTutorialStep* FPGXTutorialRunner::GetCurrentStep() const
{
	if (!bIsActive || !Steps.IsValidIndex(CurrentStepIndex))
	{
		return nullptr;
	}
	return &Steps[CurrentStepIndex];
}

float FPGXTutorialRunner::GetProgress() const
{
	if (Steps.Num() <= 1) return 1.0f;
	return static_cast<float>(CurrentStepIndex) / static_cast<float>(Steps.Num() - 1);
}

void FPGXTutorialRunner::ApplyCurrentStep()
{
	const FPGXTutorialStep* Step = GetCurrentStep();
	if (!Step) return;

	// EN: Close the previous tab if we opened it and the new target is different
	// ES: Cerrar el tab anterior si lo abrimos nosotros y el nuevo target es diferente
	if (bLastTabWasOpenedByUs && LastTutorialOpenedTabId != NAME_None
		&& LastTutorialOpenedTabId != Step->TargetTabId)
	{
		TSharedPtr<SDockTab> PrevTab = FGlobalTabmanager::Get()->FindExistingLiveTab(LastTutorialOpenedTabId);
		if (PrevTab.IsValid())
		{
			PrevTab->RequestCloseTab();
		}
	}

	// EN: If the step targets a tab and bOpenTab is true, try to invoke it
	// ES: Si el paso apunta a un tab y bOpenTab es true, intentar abrirlo
	if (Step->TargetTabId != NAME_None && Step->bOpenTab)
	{
		TSharedPtr<SDockTab> ExistingTab = FGlobalTabmanager::Get()->FindExistingLiveTab(Step->TargetTabId);
		const bool bTabExistedBefore = ExistingTab.IsValid();
		if (!bTabExistedBefore)
		{
			FGlobalTabmanager::Get()->TryInvokeTab(Step->TargetTabId);
		}
		// EN: Activate the tab so it's in front
		// ES: Activar el tab para que este al frente
		ExistingTab = FGlobalTabmanager::Get()->FindExistingLiveTab(Step->TargetTabId);
		if (ExistingTab.IsValid())
		{
			ExistingTab->ActivateInParent(ETabActivationCause::SetDirectly);
		}

		LastTutorialOpenedTabId = Step->TargetTabId;
		bLastTabWasOpenedByUs = !bTabExistedBefore;
	}

	// EN: Execute step action if any (idempotent — safe on Back navigation)
	// ES: Ejecutar accion del paso si tiene (idempotente — seguro al navegar atras)
	LastActionResult = FPGXTutorialActionResult();
	if (Step->Action != EPGXTutorialAction::None && Step->Action != EPGXTutorialAction::ConfigBasePath)
	{
		LastActionResult = FPGXTutorialActionExecutor::Execute(*Step, TutorialBasePath);
		if (!LastActionResult.bSuccess)
		{
			PGX_LOG_WARNING(LogPGXTutorials, TEXT("Action failed at step %d: %s"),
				CurrentStepIndex, *LastActionResult.FeedbackText.ToString());
		}
	}

	OnStepChanged.ExecuteIfBound();
}

void FPGXTutorialRunner::CreateOverlay()
{
	// EN: Find the main editor window
	// ES: Encontrar la ventana principal del editor
	TSharedPtr<SWindow> Window = FGlobalTabmanager::Get()->GetRootWindow();
	if (!Window.IsValid())
	{
		// EN: Fallback — use the active top-level window
		// ES: Fallback — usar la ventana activa de nivel superior
		Window = FSlateApplication::Get().GetActiveTopLevelWindow();
	}

	if (!Window.IsValid())
	{
		PGX_LOG_ERROR(LogPGXTutorials, TEXT("Cannot find root window for tutorial overlay."));
		return;
	}

	RootWindow = Window;

	Overlay = SNew(SPGXTutorialOverlay)
		.Runner(this);

	Window->AddOverlaySlot(100)
		[
			Overlay.ToSharedRef()
		];

	PGX_LOG_INFO(LogPGXTutorials, TEXT("Tutorial overlay added to root window."));
}

void FPGXTutorialRunner::DestroyOverlay()
{
	// EN: Unbind delegate before destroying overlay (S1 compliance)
	// ES: Desvincular delegado antes de destruir overlay (cumplimiento S1)
	OnStepChanged.Unbind();

	TSharedPtr<SWindow> Window = RootWindow.Pin();
	if (Window.IsValid() && Overlay.IsValid())
	{
		Window->RemoveOverlaySlot(Overlay.ToSharedRef());
		PGX_LOG_INFO(LogPGXTutorials, TEXT("Tutorial overlay removed from root window."));
	}

	Overlay.Reset();
	RootWindow.Reset();
}

// ============================================================================
// Language support
// ============================================================================

void FPGXTutorialRunner::SetLanguage(EPGXTutorialLanguage Lang)
{
	CurrentLanguage = Lang;
	SaveLanguagePreference();
	PGX_LOG_INFO(LogPGXTutorials, TEXT("Tutorial language set to: %s"),
		Lang == EPGXTutorialLanguage::Spanish ? TEXT("Spanish") : TEXT("English"));
}

EPGXTutorialLanguage FPGXTutorialRunner::GetLanguage()
{
	return CurrentLanguage;
}

void FPGXTutorialRunner::LoadLanguagePreference()
{
	FString LangStr;
	if (GConfig && GConfig->GetString(TEXT("PGXTutorials"), TEXT("Language"), LangStr, GEditorPerProjectIni))
	{
		if (LangStr == TEXT("Spanish"))
		{
			CurrentLanguage = EPGXTutorialLanguage::Spanish;
		}
		else
		{
			CurrentLanguage = EPGXTutorialLanguage::English;
		}
		PGX_LOG_INFO(LogPGXTutorials, TEXT("Tutorial language loaded: %s"), *LangStr);
	}
}

void FPGXTutorialRunner::SaveLanguagePreference()
{
	if (GConfig)
	{
		const FString LangStr = (CurrentLanguage == EPGXTutorialLanguage::Spanish)
			? TEXT("Spanish") : TEXT("English");
		GConfig->SetString(TEXT("PGXTutorials"), TEXT("Language"), *LangStr, GEditorPerProjectIni);
		GConfig->Flush(false, GEditorPerProjectIni);
	}
}

// ============================================================================
// Hub behavior
// ============================================================================

void FPGXTutorialRunner::SetKeepHubOpen(bool bKeepOpen)
{
	bKeepHubOpen = bKeepOpen;
	SaveKeepHubOpenPreference();
}

bool FPGXTutorialRunner::GetKeepHubOpen()
{
	return bKeepHubOpen;
}

void FPGXTutorialRunner::LoadKeepHubOpenPreference()
{
	bool bValue = true;
	if (GConfig && GConfig->GetBool(TEXT("PGXTutorials"), TEXT("KeepHubOpen"), bValue, GEditorPerProjectIni))
	{
		bKeepHubOpen = bValue;
	}
}

void FPGXTutorialRunner::SaveKeepHubOpenPreference()
{
	if (GConfig)
	{
		GConfig->SetBool(TEXT("PGXTutorials"), TEXT("KeepHubOpen"), bKeepHubOpen, GEditorPerProjectIni);
		GConfig->Flush(false, GEditorPerProjectIni);
	}
}

// ============================================================================
// Base path support
// ============================================================================

void FPGXTutorialRunner::SetBasePath(const FString& Path)
{
	TutorialBasePath = Path;
	SaveBasePathPreference();
	PGX_LOG_INFO(LogPGXTutorials, TEXT("Tutorial base path set to: %s"), *TutorialBasePath);
}

const FString& FPGXTutorialRunner::GetBasePath()
{
	return TutorialBasePath;
}

void FPGXTutorialRunner::LoadBasePathPreference()
{
	FString PathStr;
	if (GConfig && GConfig->GetString(TEXT("PGXTutorials"), TEXT("BasePath"), PathStr, GEditorPerProjectIni))
	{
		if (!PathStr.IsEmpty())
		{
			TutorialBasePath = PathStr;
			PGX_LOG_INFO(LogPGXTutorials, TEXT("Tutorial base path loaded: %s"), *TutorialBasePath);
		}
	}
}

void FPGXTutorialRunner::SaveBasePathPreference()
{
	if (GConfig)
	{
		GConfig->SetString(TEXT("PGXTutorials"), TEXT("BasePath"), *TutorialBasePath, GEditorPerProjectIni);
		GConfig->Flush(false, GEditorPerProjectIni);
	}
}
