// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Tables/PGXDataTableAsset.h"
#include "Tables/PGXTableTypes.h"

UPGXDataTableAsset::UPGXDataTableAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// EN: Pre-configure row struct to FPGXRegistryCategoryRow so designers
	//     get the correct columns when opening the DataTable editor.
	// ES: Pre-configurar row struct a FPGXRegistryCategoryRow para que los
	//     disenadores obtengan las columnas correctas al abrir el editor de DataTable.
	RowStruct = FPGXRegistryCategoryRow::StaticStruct();
}
