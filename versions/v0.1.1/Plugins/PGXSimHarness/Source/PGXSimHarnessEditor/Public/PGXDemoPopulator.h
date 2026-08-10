// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UPGXProjectProfileConfig;
class UPGXPlatformConfig;
class UPGXGameFlowConfig;
class UPGXFlowRulesConfig;
class UPGXSaveConfig;
class UPGXPSOWarmUpConfig;
class UPGXLevelFlowConfig;
class UPGXLevelProfile;
class UPGXLoadingConfig;
class UPGXLoadingProfile;
class UPGXAudioConfig;
class UPGXAudioChannelConfig;
class UPGXSoundDefinition;
class UPGXMusicPlaylist;
class UPGXAudioDuckingConfig;
class UPGXAudioProfile;
class UPGXLevelAudioConfig;
class UPGXGCObserverConfig;
class UPGXDocsConfig;
class UPGXMessageConfig;
class UPGXEventHandlerConfig;
class UPGXGameModeConstruction;
class UPGXObjectDataAsset;

/**
 * EN: Populates newly created DataAssets with educational demo values.
 *     Each DA type has a dedicated populate function that sets meaningful example properties.
 *
 * ES: Llena DataAssets recien creados con valores demo educativos.
 *     Cada tipo de DA tiene una funcion dedicada que setea propiedades ejemplo significativas.
 */
class PGXSIMHARNESSEDITOR_API FPGXDemoPopulator
{
public:
	/**
	 * EN: Populate a newly created DA with demo values based on its class.
	 *     Returns true if the DA was recognized and populated.
	 * ES: Llenar un DA recien creado con valores demo basados en su clase.
	 *     Retorna true si el DA fue reconocido y poblado.
	 */
	static bool PopulateDemo(UObject* DataAsset);

private:

	// EN: Each populate function targets a specific DA type, setting example values for its properties.
	// ES: Cada funcion de poblar se dirige a un tipo de DA especifico,
	static void PopulateProjectProfile(UPGXProjectProfileConfig* DA);

	// EN: PlatformConfig has complex budget structs — left at defaults for now to show category layout.
	// ES: PlatformConfig tiene structs de presupuesto complejos — dejados en defaults por ahora para mostrar layout de categorias.
	static void PopulatePlatformConfig(UPGXPlatformConfig* DA);

	// EN: GameFlowConfig has simple properties that can be demoed with example values.
	// ES: GameFlowConfig tiene propiedades simples que pueden ser demoed con valores de ejemplo
	static void PopulateGameFlowConfig(UPGXGameFlowConfig* DA);

	// EN: FlowRulesConfig requires specific channel/state definitions to be meaningful — left at defaults to show rules structure.
	// ES: FlowRulesConfig requiere definiciones especificas de canal/estado para ser significativo
	static void PopulateFlowRulesConfig(UPGXFlowRulesConfig* DA);

	// EN: SaveConfig has straightforward properties that can be demoed with example values.
	// ES: SaveConfig tiene propiedades directas que pueden ser demoed con valores de ejemplo
	static void PopulateSaveConfig(UPGXSaveConfig* DA);

	// EN: PSOWarmUpConfig has properties that can be demoed with example values to show how to configure PSO warm-up.
	// ES: PSOWarmUpConfig tiene propiedades que pueden ser demoed con valores de
	static void PopulatePSOConfig(UPGXPSOWarmUpConfig* DA);

	// EN: LevelFlowConfig contains level catalog — user defines their levels, so left at defaults to show layout.
	// ES: LevelFlowConfig contiene catalogo de niveles — el usuario define sus niveles,
	static void PopulateLevelFlowConfig(UPGXLevelFlowConfig* DA);

	// EN: LevelProfile needs specific level references — user fills these, so left at defaults to show layout.
	// ES: LevelProfile necesita referencias de nivel especificas — el usuario las llena, por
	static void PopulateLevelProfile(UPGXLevelProfile* DA);

	// EN: LoadingConfig has properties that can be demoed with example values to show how to configure loading behavior.
	// ES: LoadingConfig tiene propiedades que pueden ser demoed con valores de ejemplo para mostrar
	static void PopulateLoadingConfig(UPGXLoadingConfig* DA);

	// EN: LoadingProfile needs texture/widget references — user fills these, so left at defaults to show layout.
	// ES: LoadingProfile necesita referencias de texturas/widgets — el usuario las llena, por
	static void PopulateLoadingProfile(UPGXLoadingProfile* DA);

	// EN: AudioConfig has properties that can be demoed with example values to show how to configure audio settings.
	// ES: AudioConfig tiene propiedades que pueden ser demoed con valores de ejemplo para mostrar
	static void PopulateAudioConfig(UPGXAudioConfig* DA);

	// EN: AudioChannelConfig has properties that can be demoed with example values to show how to configure audio channels.
	// ES: AudioChannelConfig tiene propiedades que pueden ser demoed con valores de ejemplo para
	static void PopulateAudioChannelConfig(UPGXAudioChannelConfig* DA);

	// EN: SoundDefinition needs USoundBase references — user fills these, so left at defaults to show layout.
	// ES: SoundDefinition necesita referencias USoundBase — el usuario las llena, por lo
	static void PopulateSoundDefinition(UPGXSoundDefinition* DA);

	// EN: MusicPlaylist needs music track references — user fills these, so left at defaults to show layout.
	// ES: MusicPlaylist necesita referencias de pistas musicales — el usuario las llena, por lo
	static void PopulatePlaylist(UPGXMusicPlaylist* DA);

	// EN: DuckingConfig needs ducking rules with channel relationships — user fills these, so left at defaults to show layout.
	// ES: DuckingConfig necesita reglas de ducking con relaciones entre canales — el usuario
	static void PopulateDuckingConfig(UPGXAudioDuckingConfig* DA);

	// EN: AudioProfile / LevelAudioConfig — container-only fallbacks (deferred population option B); PGXAudio fills demo values.
	// ES: AudioProfile / LevelAudioConfig — fallbacks solo-contenedor; PGXAudio llena los valores.
	static void PopulateAudioProfile(UPGXAudioProfile* DA);
	static void PopulateLevelAudioConfig(UPGXLevelAudioConfig* DA);

	// EN: GCObserverConfig has properties that can be demoed with example values to show how to configure GC monitoring.
	// ES: GCObserverConfig tiene propiedades que pueden ser demoed con valores de ejemplo para
	static void PopulateGCObserverConfig(UPGXGCObserverConfig* DA);

	// EN: DocsConfig has properties that can be demoed with example values to show how to configure the documentation tool.
	// ES: DocsConfig tiene propiedades que pueden ser demoed con valores de ejemplo para mostrar
	static void PopulateDocsConfig(UPGXDocsConfig* DA);

	// EN: MessageConfig has properties that can be demoed with example values to show how to configure the message system.
	// ES: MessageConfig tiene propiedades que pueden ser demoed con valores de ejemplo para mostrar
	static void PopulateMessageConfig(UPGXMessageConfig* DA);

	// EN: EventHandlerConfig has properties that can be demoed with example values to show how to configure event handling.
	// ES: EventHandlerConfig tiene propiedades que pueden ser demoed con valores de ejemplo para
	static void PopulateEventHandlerConfig(UPGXEventHandlerConfig* DA);

	// EN: GameModeConstruction has TSoftClassPtr fields — left empty for user to pick their classes, showing the pattern.
	// ES: GameModeConstruction tiene campos TSoftClassPtr — dejados vacios para
	static void PopulateGameModeConstruction(UPGXGameModeConstruction* DA);

	// ─── 11. Demo Gameplay Assets ───

	// EN: UPGXObjectDataAsset — populate with demo display name and category tag based on asset name.
	// ES: UPGXObjectDataAsset — poblar con nombre demo y category tag basado en nombre del asset.
	static void PopulateObjectDataAsset(UPGXObjectDataAsset* DA);
};
