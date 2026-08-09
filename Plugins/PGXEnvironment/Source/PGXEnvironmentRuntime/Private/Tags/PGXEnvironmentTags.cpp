// Copyright PGX Framework. All Rights Reserved.

#include "Tags/PGXEnvironmentTags.h"

// EN: Native gameplay tag definitions for PGXEnvironment. Pre-registered
//     before any consumer reads — see header comment for the rationale and
//     the runtime-safety precedent.
// ES: Definiciones nativas de gameplay tag para PGXEnvironment.
//     Pre-registradas antes de que cualquier consumer las lea — ver comentario
//     del header para el rationale y el precedente runtime-safety.

UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Environment_Variable, "PGX.Environment.Variable");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Environment_Zone,     "PGX.Environment.Zone");

UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Environment_Severity,          "PGX.Environment.Severity");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Environment_Severity_None,     "PGX.Environment.Severity.None");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Environment_Severity_Minor,    "PGX.Environment.Severity.Minor");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Environment_Severity_Moderate, "PGX.Environment.Severity.Moderate");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Environment_Severity_Severe,   "PGX.Environment.Severity.Severe");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Environment_Severity_Critical, "PGX.Environment.Severity.Critical");

UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Environment_Result, "PGX.Environment.Result");
