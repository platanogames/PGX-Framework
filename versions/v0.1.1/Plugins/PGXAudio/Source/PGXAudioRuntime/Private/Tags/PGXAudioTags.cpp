// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Tags/PGXAudioTags.h"

// ============================================================================
// EN: Audio System Tags — channels, profiles, mix layers, dialogue, backend, events.
//     Hierarchy: PGX.Audio.{Category}.{Value}
//
// ES: Tags del Sistema Audio — canales, perfiles, capas de mezcla, dialogo, backend, eventos.
//     Jerarquia: PGX.Audio.{Categoria}.{Valor}
// ============================================================================

// -- Channel tags --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Channel_Master,   "PGX.Audio.Channel.Master");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Channel_Music,    "PGX.Audio.Channel.Music");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Channel_SFX,      "PGX.Audio.Channel.SFX");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Channel_Voice,    "PGX.Audio.Channel.Voice");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Channel_Ambient,  "PGX.Audio.Channel.Ambient");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Channel_UI,       "PGX.Audio.Channel.UI");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Channel_Dialogue, "PGX.Audio.Channel.Dialogue");

// -- Profile tags --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Profile_Default,     "PGX.Audio.Profile.Default");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Profile_UI,          "PGX.Audio.Profile.UI");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Profile_SFX3D,       "PGX.Audio.Profile.SFX3D");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Profile_SFXAttached, "PGX.Audio.Profile.SFXAttached");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Profile_Ambient3D,   "PGX.Audio.Profile.Ambient3D");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Profile_Voice3D,     "PGX.Audio.Profile.Voice3D");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Profile_Music,       "PGX.Audio.Profile.Music");

// -- Mix layer tags --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Mix_Layer_DefaultBase,   "PGX.Audio.Mix.Layer.DefaultBase");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Mix_Layer_Level,         "PGX.Audio.Mix.Layer.Level");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Mix_Layer_Ducking,       "PGX.Audio.Mix.Layer.Ducking");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Mix_Layer_LoadingScreen, "PGX.Audio.Mix.Layer.LoadingScreen");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Mix_Layer_User,          "PGX.Audio.Mix.Layer.User");

// -- Dialogue priority tags --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Dialogue_Priority_Low,      "PGX.Audio.Dialogue.Priority.Low");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Dialogue_Priority_Normal,   "PGX.Audio.Dialogue.Priority.Normal");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Dialogue_Priority_High,     "PGX.Audio.Dialogue.Priority.High");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Dialogue_Priority_Critical, "PGX.Audio.Dialogue.Priority.Critical");

// -- Backend tags --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Backend_Legacy,     "PGX.Audio.Backend.Legacy");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Backend_Modulation, "PGX.Audio.Backend.Modulation");

// -- Event tags --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Event_Play,                "PGX.Audio.Event.Play");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Event_Stop,                "PGX.Audio.Event.Stop");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Event_VolumeChanged,       "PGX.Audio.Event.VolumeChanged");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Event_MuteChanged,         "PGX.Audio.Event.MuteChanged");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Event_MusicTransition,     "PGX.Audio.Event.MusicTransition");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Event_BackendSwitched,     "PGX.Audio.Event.BackendSwitched");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Event_PoolExhausted,       "PGX.Audio.Event.PoolExhausted");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Event_DuckingActivated,    "PGX.Audio.Event.DuckingActivated");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Event_DuckingDeactivated,  "PGX.Audio.Event.DuckingDeactivated");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Event_MixLayerChanged,     "PGX.Audio.Event.MixLayerChanged");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Event_DialoguePlayed,      "PGX.Audio.Event.DialoguePlayed");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Audio_Event_DialogueInterrupted, "PGX.Audio.Event.DialogueInterrupted");
