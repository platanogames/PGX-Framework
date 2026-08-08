// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Tutorial state machine — manages step navigation, action execution, and lifecycle.
// ES: Maquina de estados del tutorial — gestiona navegacion de pasos, ejecucion de acciones y ciclo de vida.
#pragma once

#include "CoreMinimal.h"
#include "PGXTutorialTypes.h"

DECLARE_DELEGATE(FOnTutorialStepChanged);
DECLARE_DELEGATE(FOnTutorialClosed);

class SPGXTutorialOverlay;

/**
 * EN: Drives a tutorial sequence: tracks current step, handles navigation,
 *     executes actions (create folders/assets), manages overlay on the editor root window.
 * ES: Conduce una secuencia de tutorial: rastrea paso actual, maneja navegacion,
 *     ejecuta acciones (crear carpetas/assets), gestiona overlay en la ventana raiz del editor.
 */
class FPGXTutorialRunner
{
public:
	FPGXTutorialRunner();
	~FPGXTutorialRunner();

	/** EN: Start a tutorial with the given steps / ES: Iniciar tutorial con los pasos dados */
	void StartTutorial(const TArray<FPGXTutorialStep>& InSteps);

	/** EN: Navigate forward / ES: Avanzar al siguiente paso */
	void NextStep();

	/** EN: Navigate backward / ES: Retroceder al paso anterior */
	void PrevStep();

	/** EN: Close the tutorial and clean up overlay / ES: Cerrar tutorial y limpiar overlay */
	void Close();

	/** EN: Is a tutorial currently active? / ES: Hay un tutorial activo? */
	bool IsActive() const { return bIsActive; }

	/** EN: Get current step (nullptr if inactive) / ES: Obtener paso actual */
	const FPGXTutorialStep* GetCurrentStep() const;

	/** EN: Current step index / ES: Indice del paso actual */
	int32 GetCurrentStepIndex() const { return CurrentStepIndex; }

	/** EN: Total step count / ES: Total de pasos */
	int32 GetStepCount() const { return Steps.Num(); }

	/** EN: Progress 0-1 / ES: Progreso 0-1 */
	float GetProgress() const;

	/** EN: Is the current step the last one? / ES: Es el ultimo paso? */
	bool IsLastStep() const { return CurrentStepIndex >= Steps.Num() - 1; }

	/** EN: Is the current step the first one? / ES: Es el primer paso? */
	bool IsFirstStep() const { return CurrentStepIndex <= 0; }

	/** EN: Get last action result (for overlay feedback) / ES: Obtener resultado de la ultima accion */
	const FPGXTutorialActionResult& GetLastActionResult() const { return LastActionResult; }

	// -- Delegates --
	FOnTutorialStepChanged OnStepChanged;
	FOnTutorialClosed OnTutorialClosed;

	// -- Language support --
	static void SetLanguage(EPGXTutorialLanguage Lang);
	static EPGXTutorialLanguage GetLanguage();
	static void LoadLanguagePreference();
	static void SaveLanguagePreference();

	// -- Hub behavior --
	/** EN: Set whether the hub tab stays open when launching a tutorial / ES: Configurar si el hub permanece abierto */
	static void SetKeepHubOpen(bool bKeepOpen);
	static bool GetKeepHubOpen();
	static void LoadKeepHubOpenPreference();
	static void SaveKeepHubOpenPreference();

	// -- Base path support (Constructor tutorials) --
	/** EN: Set the base path for asset creation / ES: Establecer ruta base para creacion de assets */
	static void SetBasePath(const FString& Path);

	/** EN: Get current base path / ES: Obtener ruta base actual */
	static const FString& GetBasePath();

	/** EN: Load base path preference from config / ES: Cargar preferencia de ruta base desde config */
	static void LoadBasePathPreference();

	/** EN: Save base path preference to config / ES: Guardar preferencia de ruta base en config */
	static void SaveBasePathPreference();

private:
	void ApplyCurrentStep();
	void CreateOverlay();
	void DestroyOverlay();

	TArray<FPGXTutorialStep> Steps;
	int32 CurrentStepIndex = 0;
	bool bIsActive = false;

	TSharedPtr<SPGXTutorialOverlay> Overlay;
	TWeakPtr<SWindow> RootWindow;

	/** EN: Result of the last action executed / ES: Resultado de la ultima accion ejecutada */
	FPGXTutorialActionResult LastActionResult;

	/** EN: Track the last tab opened by the tutorial to close it when moving to a different one */
	/** ES: Rastrear el ultimo tab abierto por el tutorial para cerrarlo al avanzar a otro */
	FName LastTutorialOpenedTabId = NAME_None;
	bool bLastTabWasOpenedByUs = false;

	/** EN: Current language for all tutorials / ES: Idioma actual para todos los tutoriales */
	static EPGXTutorialLanguage CurrentLanguage;

	/** EN: Whether hub tab stays open when launching / ES: Si el hub permanece abierto al lanzar */
	static bool bKeepHubOpen;

	/** EN: Base path for Constructor tutorial asset creation / ES: Ruta base para creacion de assets */
	static FString TutorialBasePath;
};
