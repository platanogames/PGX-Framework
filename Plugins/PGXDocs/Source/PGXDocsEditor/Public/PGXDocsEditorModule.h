// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EN: PGXDocsEditor module. Provides the documentation viewer UI:
 *     native Slate renderer (SDocSlateRenderer), navigation tree,
 *     file watcher, and NomadTab registration.
 *     Only loaded in Editor builds.
 *
 * ES: Modulo PGXDocsEditor. Proporciona la UI del visor de documentacion:
 *     renderer Slate nativo (SDocSlateRenderer), arbol de navegacion,
 *     file watcher y registro NomadTab.
 *     Solo se carga en builds de Editor.
 */
class FPGXDocsEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
