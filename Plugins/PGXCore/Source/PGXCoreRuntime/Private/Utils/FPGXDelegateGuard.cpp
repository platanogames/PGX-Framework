// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
//
// EN: FPGXDelegateGuard is header-only — the lambda-based remove callback
//     pattern is fully resolved at compile time via TFunction<>. The .cpp
//     exists only to make the file pair greppable in the build system and
//     to provide a TU where future non-template helpers (e.g. static
//     MakeGuard() factory) can be added without breaking the header.
//
// ES: FPGXDelegateGuard es header-only — el patron de callback de remove
//     basado en lambda se resuelve completamente en compile time via
//     TFunction<>. El .cpp existe solo para hacer el par de archivos
//     grepeable en el build system y para proveer una TU donde futuros
//     helpers non-template puedan ser agregados sin romper el header.

#include "Utils/FPGXDelegateGuard.h"
