// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Validation/PGXAssetValidator.h"

bool UPGXAssetValidator::ValidateAsset_Implementation(UObject* /*Asset*/, TArray<FText>& /*OutErrors*/) const
{
	// EN: Default implementation returns true (asset is valid).
	//     Override in subclasses to provide custom validation logic.
	// ES: La implementacion por defecto retorna true (el asset es valido).
	//     Sobreescribir en subclases para proporcionar logica de validacion custom.
	return true;
}
