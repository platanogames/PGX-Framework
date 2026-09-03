// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPGXTrade, Log, All);

/**
 * EN: PGXTrade runtime module. Owns generic barter/offer/reputation baseline state.
 * ES: Modulo runtime de PGXTrade. Posee estado generico base de trueque/ofertas/reputacion.
 */
class FPGXTradeRuntimeModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
