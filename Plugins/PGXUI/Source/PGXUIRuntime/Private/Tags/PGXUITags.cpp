// Copyright PGX Framework. All Rights Reserved.

#include "Tags/PGXUITags.h"

// ============================================================
// Compatibility leaves used by the runtime and automation tests.
// ============================================================
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Screen_Default,                      "PGX.UI.Screen.Default");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Notification_Info,                   "PGX.UI.Notification.Info");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_View_Default,                        "PGX.UI.View.Default");

// ============================================================
// Runtime tag tree.
// ============================================================

// --- Root anchor ---
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI,                                     "PGX.UI");

// --- Context branch ---
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Context,                             "PGX.UI.Context");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Context_State,                       "PGX.UI.Context.State");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Context_State_Active,                "PGX.UI.Context.State.Active");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Context_State_Suspended,             "PGX.UI.Context.State.Suspended");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Context_Scope,                       "PGX.UI.Context.Scope");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Context_Scope_Game,                  "PGX.UI.Context.Scope.Game");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Context_Scope_Editor,                "PGX.UI.Context.Scope.Editor");

// --- Screen branch ---
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Screen,                              "PGX.UI.Screen");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Screen_Type,                         "PGX.UI.Screen.Type");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Screen_Type_Default,                 "PGX.UI.Screen.Type.Default");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Screen_Layer,                        "PGX.UI.Screen.Layer");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Screen_Layer_HUD,                    "PGX.UI.Screen.Layer.HUD");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Screen_Layer_Menu,                   "PGX.UI.Screen.Layer.Menu");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Screen_Layer_Modal,                  "PGX.UI.Screen.Layer.Modal");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Screen_State,                        "PGX.UI.Screen.State");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Screen_State_Open,                   "PGX.UI.Screen.State.Open");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Screen_State_Closed,                 "PGX.UI.Screen.State.Closed");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Screen_Result,                       "PGX.UI.Screen.Result");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Screen_Result_Confirmed,             "PGX.UI.Screen.Result.Confirmed");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Screen_Result_Cancelled,             "PGX.UI.Screen.Result.Cancelled");

// --- Notification branch ---
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Notification,                        "PGX.UI.Notification");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Notification_Category,               "PGX.UI.Notification.Category");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Notification_Category_Default,       "PGX.UI.Notification.Category.Default");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Notification_Priority,               "PGX.UI.Notification.Priority");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Notification_Priority_Low,           "PGX.UI.Notification.Priority.Low");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Notification_Priority_Normal,        "PGX.UI.Notification.Priority.Normal");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Notification_Priority_High,          "PGX.UI.Notification.Priority.High");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Notification_State,                  "PGX.UI.Notification.State");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Notification_State_Queued,           "PGX.UI.Notification.State.Queued");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Notification_State_Dismissed,        "PGX.UI.Notification.State.Dismissed");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Notification_Source,                 "PGX.UI.Notification.Source");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Notification_Source_System,          "PGX.UI.Notification.Source.System");

// --- WidgetPool branch ---
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_WidgetPool,                          "PGX.UI.WidgetPool");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_WidgetPool_Type,                     "PGX.UI.WidgetPool.Type");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_WidgetPool_Type_Default,             "PGX.UI.WidgetPool.Type.Default");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_WidgetPool_State,                    "PGX.UI.WidgetPool.State");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_WidgetPool_State_Available,          "PGX.UI.WidgetPool.State.Available");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_WidgetPool_State_Acquired,           "PGX.UI.WidgetPool.State.Acquired");

// --- View branch ---
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_View,                                "PGX.UI.View");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_View_Source,                         "PGX.UI.View.Source");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_View_Source_Default,                 "PGX.UI.View.Source.Default");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_View_Binding,                        "PGX.UI.View.Binding");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_View_Binding_Default,                "PGX.UI.View.Binding.Default");

// --- Loading branch ---
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Loading,                             "PGX.UI.Loading");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Loading_State,                       "PGX.UI.Loading.State");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Loading_State_Idle,                  "PGX.UI.Loading.State.Idle");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Loading_State_InProgress,            "PGX.UI.Loading.State.InProgress");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Loading_State_Complete,              "PGX.UI.Loading.State.Complete");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Loading_Source,                      "PGX.UI.Loading.Source");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Loading_Source_LevelChange,          "PGX.UI.Loading.Source.LevelChange");

// --- Theme branch ---
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Theme,                               "PGX.UI.Theme");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Theme_Token,                         "PGX.UI.Theme.Token");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Theme_Token_Default,                 "PGX.UI.Theme.Token.Default");

// --- Accessibility branch ---
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Accessibility,                       "PGX.UI.Accessibility");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Accessibility_Profile,               "PGX.UI.Accessibility.Profile");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Accessibility_Profile_Default,       "PGX.UI.Accessibility.Profile.Default");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Accessibility_Violation,             "PGX.UI.Accessibility.Violation");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Accessibility_Violation_MissingLabel,"PGX.UI.Accessibility.Violation.MissingLabel");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Accessibility_Violation_LowContrast, "PGX.UI.Accessibility.Violation.LowContrast");

// --- Suspension branch (explicit leaves per design section 11 — no .* sub-anchors) ---
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Suspension,                          "PGX.UI.Suspension");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Suspension_Quiescent,                "PGX.UI.Suspension.Quiescent");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Suspension_Resumed,                  "PGX.UI.Suspension.Resumed");

// --- Provenance branch ---
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Provenance,                          "PGX.UI.Provenance");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Provenance_Source,                   "PGX.UI.Provenance.Source");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Provenance_Source_Default,           "PGX.UI.Provenance.Source.Default");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Provenance_Outcome,                  "PGX.UI.Provenance.Outcome");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Provenance_Outcome_Success,          "PGX.UI.Provenance.Outcome.Success");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_UI_Provenance_Outcome_Failure,          "PGX.UI.Provenance.Outcome.Failure");
