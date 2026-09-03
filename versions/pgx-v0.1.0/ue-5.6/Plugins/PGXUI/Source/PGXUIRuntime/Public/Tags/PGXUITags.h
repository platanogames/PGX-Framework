// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "NativeGameplayTags.h"

// EN: Native gameplay tags owned by PGXUI. Registered as the canonical PGXUI GameplayTag structure.
//     Tree (canonical anchors and curated leaves; project
//     extensions live in Object DAs / project tags, not in this header):
//
//       PGX.UI
//       ├── Context
//       │   ├── State.{Active, Suspended}
//       │   └── Scope.{Game, Editor}
//       ├── Screen
//       │   ├── Default                      (compatibility leaf)
//       │   ├── Type.{Default}
//       │   ├── Layer.{HUD, Menu, Modal}
//       │   ├── State.{Open, Closed}
//       │   └── Result.{Confirmed, Cancelled}
//       ├── Notification
//       │   ├── Info                         (compatibility leaf)
//       │   ├── Category.{Default}
//       │   ├── Priority.{Low, Normal, High}
//       │   ├── State.{Queued, Dismissed}
//       │   └── Source.{System}
//       ├── WidgetPool
//       │   ├── Type.{Default}
//       │   └── State.{Available, Acquired}
//       ├── View
//       │   ├── Default                      (compatibility leaf)
//       │   ├── Source.{Default}
//       │   └── Binding.{Default}
//       ├── Loading
//       │   ├── State.{Idle, InProgress, Complete}
//       │   └── Source.{LevelChange}
//       ├── Theme
//       │   └── Token.{Default}
//       ├── Accessibility
//       │   ├── Profile.{Default}
//       │   └── Violation.{MissingLabel, LowContrast}
//       ├── Suspension
//       │   ├── Quiescent
//       │   └── Resumed
//       └── Provenance
//           ├── Source.{Default}
//           └── Outcome.{Success, Failure}
//
// ES: Tags nativos de gameplay propiedad de PGXUI. Registrados como estructura canonica de PGXUI.
//
// Compatibility leaves provide stable native anchors for inspector use and authored Object DA validation.
//
// All tags use UE_DECLARE_GAMEPLAY_TAG_EXTERN + UE_DEFINE_GAMEPLAY_TAG, so they register when the module loads and do not depend on fallible runtime name lookup.

// ============================================================
// Compatibility leaves
// ============================================================
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Screen_Default);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Notification_Info);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_View_Default);

// ============================================================
// Canonical tag tree
// ============================================================

// --- Root anchor ---
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI);

// --- Context branch ---
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Context);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Context_State);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Context_State_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Context_State_Suspended);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Context_Scope);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Context_Scope_Game);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Context_Scope_Editor);

// --- Screen branch ---
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Screen);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Screen_Type);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Screen_Type_Default);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Screen_Layer);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Screen_Layer_HUD);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Screen_Layer_Menu);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Screen_Layer_Modal);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Screen_State);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Screen_State_Open);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Screen_State_Closed);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Screen_Result);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Screen_Result_Confirmed);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Screen_Result_Cancelled);

// --- Notification branch ---
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Notification);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Notification_Category);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Notification_Category_Default);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Notification_Priority);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Notification_Priority_Low);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Notification_Priority_Normal);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Notification_Priority_High);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Notification_State);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Notification_State_Queued);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Notification_State_Dismissed);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Notification_Source);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Notification_Source_System);

// --- WidgetPool branch ---
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_WidgetPool);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_WidgetPool_Type);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_WidgetPool_Type_Default);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_WidgetPool_State);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_WidgetPool_State_Available);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_WidgetPool_State_Acquired);

// --- View branch ---
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_View);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_View_Source);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_View_Source_Default);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_View_Binding);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_View_Binding_Default);

// --- Loading branch ---
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Loading);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Loading_State);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Loading_State_Idle);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Loading_State_InProgress);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Loading_State_Complete);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Loading_Source);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Loading_Source_LevelChange);

// --- Theme branch ---
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Theme);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Theme_Token);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Theme_Token_Default);

// --- Accessibility branch ---
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Accessibility);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Accessibility_Profile);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Accessibility_Profile_Default);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Accessibility_Violation);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Accessibility_Violation_MissingLabel);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Accessibility_Violation_LowContrast);

// --- Suspension branch (explicit leaves; no wildcard sub-anchors) ---
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Suspension);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Suspension_Quiescent);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Suspension_Resumed);

// --- Provenance branch ---
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Provenance);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Provenance_Source);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Provenance_Source_Default);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Provenance_Outcome);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Provenance_Outcome_Success);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_UI_Provenance_Outcome_Failure);
