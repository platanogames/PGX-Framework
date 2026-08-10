// Copyright PGX Framework. All Rights Reserved.

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
