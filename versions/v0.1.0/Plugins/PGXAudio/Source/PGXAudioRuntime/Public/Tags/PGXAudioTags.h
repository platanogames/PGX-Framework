// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "NativeGameplayTags.h"

/**
 * EN: Native gameplay tags for the PGX Audio System.
 *     Using UE_DECLARE_GAMEPLAY_TAG_EXTERN / UE_DEFINE_GAMEPLAY_TAG pattern.
 *     Definitions in Private/Tags/PGXAudioTags.cpp.
 *     For cross-DLL access, use FGameplayTag::RequestGameplayTag().
 *     Ownership: Audio system. Channel/Profile dev-extensible; Event/Mix/Backend fixed.
 * ES: Tags nativos de gameplay para el Sistema PGX Audio.
 *     Usando patron UE_DECLARE_GAMEPLAY_TAG_EXTERN / UE_DEFINE_GAMEPLAY_TAG.
 *     Definiciones en Private/Tags/PGXAudioTags.cpp.
 *     Para acceso cross-DLL, usar FGameplayTag::RequestGameplayTag().
 */

// -- Channel tags --
// EN: Audio channels for volume control and routing / ES: Canales de audio para control de volumen y enrutamiento
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Channel_Master);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Channel_Music);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Channel_SFX);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Channel_Voice);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Channel_Ambient);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Channel_UI);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Channel_Dialogue);

// -- Profile tags --
// EN: Audio playback profiles / ES: Perfiles de reproduccion de audio
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Profile_Default);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Profile_UI);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Profile_SFX3D);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Profile_SFXAttached);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Profile_Ambient3D);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Profile_Voice3D);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Profile_Music);

// -- Mix layer tags --
// EN: 5-layer mix model identifiers / ES: Identificadores del modelo de mezcla de 5 capas
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Mix_Layer_DefaultBase);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Mix_Layer_Level);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Mix_Layer_Ducking);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Mix_Layer_LoadingScreen);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Mix_Layer_User);

// -- Dialogue priority tags --
// EN: Priority levels for dialogue queue / ES: Niveles de prioridad para cola de dialogo
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Dialogue_Priority_Low);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Dialogue_Priority_Normal);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Dialogue_Priority_High);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Dialogue_Priority_Critical);

// -- Backend tags --
// EN: Backend type identifiers / ES: Identificadores de tipo de backend
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Backend_Legacy);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Backend_Modulation);

// -- Event tags --
// EN: Audio event categories for history tracking / ES: Categorias de eventos de audio para seguimiento de historial
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Event_Play);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Event_Stop);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Event_VolumeChanged);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Event_MuteChanged);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Event_MusicTransition);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Event_BackendSwitched);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Event_PoolExhausted);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Event_DuckingActivated);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Event_DuckingDeactivated);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Event_MixLayerChanged);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Event_DialoguePlayed);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Audio_Event_DialogueInterrupted);
