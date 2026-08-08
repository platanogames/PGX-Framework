// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Per-system tutorial content — S1-S13 step definitions (bilingual EN/ES).
// ES: Contenido de tutoriales por sistema — definiciones de pasos S1-S13 (bilingue EN/ES).

#include "Tutorials/PGXSystemTutorials.h"
#include "Style/PGXVisualTokens.h"

// ============================================================================
// S1: Profile System (10 steps)
// ============================================================================
TArray<FPGXTutorialStep> PGXSystemTutorials::GetS1_Profile(EPGXTutorialLanguage Lang)
{
	const bool bES = (Lang == EPGXTutorialLanguage::Spanish);
	TArray<FPGXTutorialStep> Steps;

	// Step 0: ConfigBasePath
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Configurar Ruta Base del Proyecto")
			: TEXT("Configure Project Base Path"));
		S.Description = FText::FromString(bES
			? TEXT("Antes de crear assets, define la carpeta raiz donde PGX guardara la configuracion de tu proyecto. "
				"Esta ruta sera el punto de partida para todos los DataAssets de este tutorial.\n\n"
				"Ejemplo: /Game/Config/PGX/Profile. Puedes cambiarla despues desde las PGX Editor Settings.")
			: TEXT("Before creating assets, define the root folder where PGX will store your project configuration. "
				"This path will be the starting point for all DataAssets in this tutorial.\n\n"
				"Example: /Game/Config/PGX/Profile. You can change it later from PGX Editor Settings."));
		S.AccentColor = PGX::System::Profile;
		S.Action = EPGXTutorialAction::ConfigBasePath;
		Steps.Add(S);
	}

	// Step 1: Create PGXProjectProfileConfig
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Crear Project Profile Config")
			: TEXT("Create Project Profile Config"));
		S.Description = FText::FromString(bES
			? TEXT("El UPGXProjectProfileConfig es el DataAsset central del sistema Profile. "
				"Define la identidad de tu proyecto: nombre, version, estudio, y los budgets de plataforma.\n\n"
				"Sin este asset, PGX no puede aplicar limites de calidad ni configuraciones por plataforma. "
				"BENEFICIO: Centraliza toda la metadata del proyecto en un solo lugar consultable por cualquier sistema. "
				"EJEMPLO: Un juego multiplataforma define aqui que en Switch el budget de audio es 32 canales, mientras que en PC son 128.")
			: TEXT("UPGXProjectProfileConfig is the central DataAsset of the Profile system. "
				"It defines your project identity: name, version, studio, and platform budgets.\n\n"
				"Without this asset, PGX cannot enforce quality limits or per-platform configurations. "
				"BENEFIT: Centralizes all project metadata in a single location queryable by any system. "
				"EXAMPLE: A cross-platform game defines here that on Switch the audio budget is 32 channels, while on PC it's 128."));
		S.AccentColor = PGX::System::Profile;
		S.Action = EPGXTutorialAction::CreateAsset;
		S.ActionPath = TEXT("Profile");
		S.AssetClass = TEXT("/Script/PGXCoreRuntime.PGXProjectProfileConfig");
		S.AssetName = TEXT("DA_ProjectProfile");
		Steps.Add(S);
	}

	// Step 2: Guide — Profile Inspector
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXProfileInspector");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Explorar el Profile Inspector")
			: TEXT("Explore the Profile Inspector"));
		S.Description = FText::FromString(bES
			? TEXT("El Profile Inspector muestra en tiempo real la identidad de tu proyecto y los budgets activos por plataforma. "
				"Puedes ver que plataforma esta seleccionada, cuantos canales de audio, memoria de texturas, y otros limites se aplican.\n\n"
				"CUANDO USARLO: Siempre que necesites verificar que configuracion de plataforma esta activa o debuggear por que un sistema no respeta un limite.")
			: TEXT("The Profile Inspector shows your project identity and active platform budgets in real-time. "
				"You can see which platform is selected, how many audio channels, texture memory, and other limits are applied.\n\n"
				"WHEN TO USE IT: Whenever you need to verify which platform configuration is active or debug why a system isn't respecting a budget."));
		S.AccentColor = PGX::System::Profile;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 3: Guide — Platform Health Dashboard
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXPlatformHealthDashboard");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Panel de Salud de Plataforma")
			: TEXT("Platform Health Dashboard"));
		S.Description = FText::FromString(bES
			? TEXT("El Platform Health Dashboard agrega datos de TODOS los subsistemas PGX y los compara contra los budgets del Profile. "
				"Muestra barras de progreso con codigos de color: verde (dentro del budget), amarillo (cerca del limite), rojo (excedido).\n\n"
				"CUANDO USARLO: Durante la optimizacion, para identificar de un vistazo que sistemas estan consumiendo mas recursos de los permitidos.")
			: TEXT("The Platform Health Dashboard aggregates data from ALL PGX subsystems and compares it against Profile budgets. "
				"It shows progress bars with color codes: green (within budget), yellow (near limit), red (exceeded).\n\n"
				"WHEN TO USE IT: During optimization, to identify at a glance which systems are consuming more resources than allowed."));
		S.AccentColor = PGX::System::Profile;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 4: Guide — Understanding Platform Budgets
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXProfileInspector");
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Entender los Budgets de Plataforma")
			: TEXT("Understanding Platform Budgets"));
		S.Description = FText::FromString(bES
			? TEXT("Cada UPGXPlatformConfig define 10 structs de budget: Audio, Textura, Mesh, Particulas, UI, Save, PSO, Loading, Animacion y Fisicas. "
				"Los 11 subsistemas de PGX consultan estos budgets en Initialize() para auto-limitarse.\n\n"
				"No necesitas configurar nada en cada subsistema individual — solo definir los limites en el Profile y PGX los respeta automaticamente.")
			: TEXT("Each UPGXPlatformConfig defines 10 budget structs: Audio, Texture, Mesh, Particles, UI, Save, PSO, Loading, Animation and Physics. "
				"All 11 PGX subsystems query these budgets in Initialize() to self-limit.\n\n"
				"You don't need to configure anything in each individual subsystem — just define the limits in the Profile and PGX respects them automatically."));
		S.AccentColor = PGX::System::Profile;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 5: Guide — Open the created config
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Abrir y Configurar el Profile")
			: TEXT("Open and Configure the Profile"));
		S.Description = FText::FromString(bES
			? TEXT("Abre el DA_ProjectProfile que acabas de crear. En el panel Details veras las propiedades principales: "
				"ProjectName, ProjectVersion, StudioName, y la referencia a UPGXPlatformConfig.\n\n"
				"Rellena la identidad de tu proyecto. El PlatformConfig se puede crear por separado o usar el default. "
				"EJEMPLO: ProjectName='MiJuegoRPG', Version='0.1.0', Studio='MiEstudio'.")
			: TEXT("Open the DA_ProjectProfile you just created. In the Details panel you'll see the main properties: "
				"ProjectName, ProjectVersion, StudioName, and the UPGXPlatformConfig reference.\n\n"
				"Fill in your project identity. The PlatformConfig can be created separately or use the default. "
				"EXAMPLE: ProjectName='MyRPGGame', Version='0.1.0', Studio='MyStudio'."));
		S.AccentColor = PGX::System::Profile;
		S.Action = EPGXTutorialAction::OpenAsset;
		S.ActionPath = TEXT("Profile");
		S.AssetName = TEXT("DA_ProjectProfile");
		Steps.Add(S);
	}

	// Step 6: Guide — Console commands
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Comandos de Consola del Profile")
			: TEXT("Profile Console Commands"));
		S.Description = FText::FromString(bES
			? TEXT("El sistema Profile registra comandos de consola para consultas rapidas en runtime: "
				"'pgx.profile.status' muestra la identidad y plataforma activa.\n\n"
				"'pgx.profile.budgets' lista todos los budgets de la plataforma actual. "
				"Utiles para verificar configuracion sin abrir el editor.")
			: TEXT("The Profile system registers console commands for quick runtime queries: "
				"'pgx.profile.status' shows the identity and active platform.\n\n"
				"'pgx.profile.budgets' lists all budgets for the current platform. "
				"Useful for verifying configuration without opening the editor."));
		S.AccentColor = PGX::System::Profile;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 7: Guide — Blueprint integration
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Acceso desde Blueprint")
			: TEXT("Blueprint Access"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXProfileBlueprintLibrary expone nodos como GetProjectName, GetCurrentPlatform, GetAudioBudget, y GetTextureBudget. "
				"Desde cualquier Blueprint puedes consultar los budgets sin referencias directas al subsistema.\n\n"
				"EJEMPLO: En tu UI de opciones, usa GetAudioBudget para mostrar el maximo de canales al jugador.")
			: TEXT("UPGXProfileBlueprintLibrary exposes nodes like GetProjectName, GetCurrentPlatform, GetAudioBudget, and GetTextureBudget. "
				"From any Blueprint you can query budgets without direct subsystem references.\n\n"
				"EXAMPLE: In your options UI, use GetAudioBudget to show the maximum channels to the player."));
		S.AccentColor = PGX::System::Profile;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 8: Guide — System Observer
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXSystemObserver");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Profile en el System Observer")
			: TEXT("Profile in the System Observer"));
		S.Description = FText::FromString(bES
			? TEXT("El System Observer muestra el estado de TODOS los subsistemas PGX, incluyendo Profile. "
				"Verifica que Profile aparezca como 'Initialized' y que el config DA fue descubierto correctamente.\n\n"
				"Si el estado es 'No Config', revisa que DA_ProjectProfile exista y tenga la ruta correcta.")
			: TEXT("The System Observer shows the state of ALL PGX subsystems, including Profile. "
				"Verify that Profile appears as 'Initialized' and that the config DA was discovered correctly.\n\n"
				"If the state is 'No Config', check that DA_ProjectProfile exists and has the correct path."));
		S.AccentColor = PGX::System::Profile;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 9: Summary
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXHub");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Resumen: Profile Configurado")
			: TEXT("Summary: Profile Configured"));
		S.Description = FText::FromString(bES
			? TEXT("Has completado la configuracion del sistema Profile. Resumen:\n\n"
				"- CREADO: DA_ProjectProfile (identidad del proyecto + referencia a plataforma)\n"
				"- EXPLORADO: Profile Inspector (identidad y budgets en vivo)\n"
				"- EXPLORADO: Platform Health Dashboard (salud global vs budgets)\n"
				"- APRENDIDO: Los 11 subsistemas consultan budgets automaticamente\n"
				"- APRENDIDO: Comandos pgx.profile.* y nodos Blueprint disponibles\n\n"
				"Siguiente paso recomendado: Tutorial S2 (GameFlow) para definir los estados de tu juego.")
			: TEXT("You have completed the Profile system configuration. Summary:\n\n"
				"- CREATED: DA_ProjectProfile (project identity + platform reference)\n"
				"- EXPLORED: Profile Inspector (live identity and budgets)\n"
				"- EXPLORED: Platform Health Dashboard (global health vs budgets)\n"
				"- LEARNED: All 11 subsystems query budgets automatically\n"
				"- LEARNED: pgx.profile.* commands and Blueprint nodes available\n\n"
				"Recommended next step: Tutorial S2 (GameFlow) to define your game states."));
		S.AccentColor = PGX::Semantic::Good;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	return Steps;
}

// ============================================================================
// S2: GameFlow System (10 steps)
// ============================================================================
TArray<FPGXTutorialStep> PGXSystemTutorials::GetS2_GameFlow(EPGXTutorialLanguage Lang)
{
	const bool bES = (Lang == EPGXTutorialLanguage::Spanish);
	TArray<FPGXTutorialStep> Steps;

	// Step 0: ConfigBasePath
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Configurar Ruta Base del Proyecto")
			: TEXT("Configure Project Base Path"));
		S.Description = FText::FromString(bES
			? TEXT("Define la carpeta donde se guardaran los assets de GameFlow. "
				"Ejemplo: /Game/Config/PGX/GameFlow. Esta ruta debe ser consistente con la estructura de tu proyecto.")
			: TEXT("Define the folder where GameFlow assets will be stored. "
				"Example: /Game/Config/PGX/GameFlow. This path should be consistent with your project structure."));
		S.AccentColor = PGX::System::GameFlow;
		S.Action = EPGXTutorialAction::ConfigBasePath;
		Steps.Add(S);
	}

	// Step 1: Create PGXGameFlowConfig
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Crear GameFlow Config")
			: TEXT("Create GameFlow Config"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXGameFlowConfig es el DataAsset principal del sistema GameFlow. "
				"Define los GameplayTags que representan los estados de tu juego: MainMenu, Loading, InGame, Paused, etc.\n\n"
				"POR QUE EXISTE: Sin estados definidos, los sistemas PGX no saben en que fase esta el juego y no pueden activarse o desactivarse correctamente. "
				"BENEFICIO: Un unico asset controla la maquina de estados global del juego. "
				"EJEMPLO: Define 'PGX.GameFlow.State.MainMenu', 'PGX.GameFlow.State.Gameplay', 'PGX.GameFlow.State.Paused'.")
			: TEXT("UPGXGameFlowConfig is the main DataAsset of the GameFlow system. "
				"It defines the GameplayTags representing your game states: MainMenu, Loading, InGame, Paused, etc.\n\n"
				"WHY IT EXISTS: Without defined states, PGX systems don't know what phase the game is in and can't activate/deactivate correctly. "
				"BENEFIT: A single asset controls the game's global state machine. "
				"EXAMPLE: Define 'PGX.GameFlow.State.MainMenu', 'PGX.GameFlow.State.Gameplay', 'PGX.GameFlow.State.Paused'."));
		S.AccentColor = PGX::System::GameFlow;
		S.Action = EPGXTutorialAction::CreateAsset;
		S.ActionPath = TEXT("GameFlow");
		S.AssetClass = TEXT("/Script/PGXGameFlowRuntime.PGXGameFlowConfig");
		S.AssetName = TEXT("DA_GameFlowConfig");
		Steps.Add(S);
	}

	// Step 2: Create PGXFlowRulesConfig
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Crear Flow Rules Config")
			: TEXT("Create Flow Rules Config"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXFlowRulesConfig define las REGLAS de transicion entre estados. "
				"Especifica que transiciones son validas: desde MainMenu puedes ir a Loading, pero NO directamente a Paused.\n\n"
				"POR QUE EXISTE: Previene transiciones invalidas que causan bugs dificiles de rastrear (ej: pausar durante una pantalla de carga). "
				"BENEFICIO: El subsistema rechaza transiciones ilegales con logs claros en vez de crashes silenciosos. "
				"EJEMPLO: AllowedTransitions: MainMenu->Loading, Loading->Gameplay, Gameplay->Paused, Paused->Gameplay.")
			: TEXT("UPGXFlowRulesConfig defines the RULES for transitions between states. "
				"It specifies which transitions are valid: from MainMenu you can go to Loading, but NOT directly to Paused.\n\n"
				"WHY IT EXISTS: Prevents invalid transitions that cause hard-to-track bugs (e.g., pausing during a loading screen). "
				"BENEFIT: The subsystem rejects illegal transitions with clear logs instead of silent crashes. "
				"EXAMPLE: AllowedTransitions: MainMenu->Loading, Loading->Gameplay, Gameplay->Paused, Paused->Gameplay."));
		S.AccentColor = PGX::System::GameFlow;
		S.Action = EPGXTutorialAction::CreateAsset;
		S.ActionPath = TEXT("GameFlow");
		S.AssetClass = TEXT("/Script/PGXGameFlowRuntime.PGXFlowRulesConfig");
		S.AssetName = TEXT("DA_FlowRules");
		Steps.Add(S);
	}

	// Step 3: Guide — GameFlow Inspector
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXGameFlowInspector");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Explorar el GameFlow Inspector")
			: TEXT("Explore the GameFlow Inspector"));
		S.Description = FText::FromString(bES
			? TEXT("El GameFlow Inspector muestra el estado actual del juego, el historial de transiciones, y las reglas activas. "
				"Puedes ver en tiempo real que estado esta activo y cuales son las transiciones permitidas desde ese estado.\n\n"
				"CUANDO USARLO: Para debuggear por que una transicion fallo o verificar que la maquina de estados esta en el estado esperado.")
			: TEXT("The GameFlow Inspector shows the current game state, transition history, and active rules. "
				"You can see in real-time which state is active and which transitions are allowed from that state.\n\n"
				"WHEN TO USE IT: To debug why a transition failed or verify the state machine is in the expected state."));
		S.AccentColor = PGX::System::GameFlow;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 4: Guide — Open GameFlowConfig
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Configurar Estados del Juego")
			: TEXT("Configure Game States"));
		S.Description = FText::FromString(bES
			? TEXT("Abre DA_GameFlowConfig y define tus estados. Cada estado es un GameplayTag bajo PGX.GameFlow.State.*. "
				"Propiedades clave: InitialState (estado al arrancar), States (lista de estados validos).\n\n"
				"El sistema usa tags en vez de enums para que puedas extender estados sin modificar codigo PGX. "
				"EJEMPLO: Para un RPG, agrega estados como Exploration, Combat, Dialogue, CutScene.")
			: TEXT("Open DA_GameFlowConfig and define your states. Each state is a GameplayTag under PGX.GameFlow.State.*. "
				"Key properties: InitialState (state at startup), States (list of valid states).\n\n"
				"The system uses tags instead of enums so you can extend states without modifying PGX code. "
				"EXAMPLE: For an RPG, add states like Exploration, Combat, Dialogue, CutScene."));
		S.AccentColor = PGX::System::GameFlow;
		S.Action = EPGXTutorialAction::OpenAsset;
		S.ActionPath = TEXT("GameFlow");
		S.AssetName = TEXT("DA_GameFlowConfig");
		Steps.Add(S);
	}

	// Step 5: Guide — Delegates
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Delegates de GameFlow")
			: TEXT("GameFlow Delegates"));
		S.Description = FText::FromString(bES
			? TEXT("GameFlow expone delegates que otros sistemas escuchan para reaccionar a cambios de estado: "
				"OnGameFlowStateChanged(OldState, NewState) — disparado en cada transicion exitosa.\n\n"
				"OnGameFlowTransitionRejected(From, To, Reason) — disparado cuando una transicion es bloqueada. "
				"Loading, PSO, Audio y Save se suscriben a estos delegates para activarse/desactivarse automaticamente.")
			: TEXT("GameFlow exposes delegates that other systems listen to for reacting to state changes: "
				"OnGameFlowStateChanged(OldState, NewState) — fired on each successful transition.\n\n"
				"OnGameFlowTransitionRejected(From, To, Reason) — fired when a transition is blocked. "
				"Loading, PSO, Audio and Save subscribe to these delegates to auto-activate/deactivate."));
		S.AccentColor = PGX::System::GameFlow;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 6: Guide — Console commands
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Comandos de Consola de GameFlow")
			: TEXT("GameFlow Console Commands"));
		S.Description = FText::FromString(bES
			? TEXT("Comandos disponibles: 'pgx.gameflow.status' muestra el estado actual. "
				"'pgx.gameflow.transition <Tag>' fuerza una transicion (debug only).\n\n"
				"'pgx.gameflow.history' muestra las ultimas N transiciones con timestamps. "
				"Utiles durante desarrollo para simular flujos sin tener que navegar por menus del juego.")
			: TEXT("Available commands: 'pgx.gameflow.status' shows current state. "
				"'pgx.gameflow.transition <Tag>' forces a transition (debug only).\n\n"
				"'pgx.gameflow.history' shows the last N transitions with timestamps. "
				"Useful during development to simulate flows without navigating game menus."));
		S.AccentColor = PGX::System::GameFlow;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 7: Guide — Blueprint integration
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("GameFlow en Blueprint")
			: TEXT("GameFlow in Blueprint"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXGameFlowBlueprintLibrary provee nodos como RequestTransition, GetCurrentState, IsInState, y GetTransitionHistory. "
				"Puedes bindear el delegate OnGameFlowStateChanged desde Blueprint para reaccionar visualmente a cambios de estado.\n\n"
				"EJEMPLO: Tu HUD usa IsInState('PGX.GameFlow.State.Combat') para mostrar la barra de vida.")
			: TEXT("UPGXGameFlowBlueprintLibrary provides nodes like RequestTransition, GetCurrentState, IsInState, and GetTransitionHistory. "
				"You can bind the OnGameFlowStateChanged delegate from Blueprint to visually react to state changes.\n\n"
				"EXAMPLE: Your HUD uses IsInState('PGX.GameFlow.State.Combat') to show the health bar."));
		S.AccentColor = PGX::System::GameFlow;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 8: Guide — System Observer
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXSystemObserver");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("GameFlow en el System Observer")
			: TEXT("GameFlow in the System Observer"));
		S.Description = FText::FromString(bES
			? TEXT("En el System Observer, GameFlow muestra su estado actual, numero de transiciones realizadas, y si el config DA fue encontrado. "
				"Verifica que aparezca como 'Initialized' con estado valido. Si dice 'No Config', asegurate de que DA_GameFlowConfig existe.")
			: TEXT("In the System Observer, GameFlow shows its current state, number of transitions made, and whether the config DA was found. "
				"Verify it appears as 'Initialized' with a valid state. If it says 'No Config', ensure DA_GameFlowConfig exists."));
		S.AccentColor = PGX::System::GameFlow;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 9: Summary
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXHub");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Resumen: GameFlow Configurado")
			: TEXT("Summary: GameFlow Configured"));
		S.Description = FText::FromString(bES
			? TEXT("Has completado la configuracion del sistema GameFlow. Resumen:\n\n"
				"- CREADO: DA_GameFlowConfig (estados del juego via GameplayTags)\n"
				"- CREADO: DA_FlowRules (reglas de transicion validas)\n"
				"- EXPLORADO: GameFlow Inspector (estado actual + historial)\n"
				"- APRENDIDO: Delegates OnStateChanged/OnTransitionRejected\n"
				"- APRENDIDO: Comandos pgx.gameflow.* y nodos Blueprint\n\n"
				"Siguiente paso recomendado: Tutorial S3 (Save) para configurar el guardado de partidas.")
			: TEXT("You have completed the GameFlow system configuration. Summary:\n\n"
				"- CREATED: DA_GameFlowConfig (game states via GameplayTags)\n"
				"- CREATED: DA_FlowRules (valid transition rules)\n"
				"- EXPLORED: GameFlow Inspector (current state + history)\n"
				"- LEARNED: Delegates OnStateChanged/OnTransitionRejected\n"
				"- LEARNED: pgx.gameflow.* commands and Blueprint nodes\n\n"
				"Recommended next step: Tutorial S3 (Save) to configure game saving."));
		S.AccentColor = PGX::Semantic::Good;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	return Steps;
}

// ============================================================================
// S3: Save System (10 steps)
// ============================================================================
TArray<FPGXTutorialStep> PGXSystemTutorials::GetS3_Save(EPGXTutorialLanguage Lang)
{
	const bool bES = (Lang == EPGXTutorialLanguage::Spanish);
	TArray<FPGXTutorialStep> Steps;

	// Step 0: ConfigBasePath
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Configurar Ruta Base del Proyecto")
			: TEXT("Configure Project Base Path"));
		S.Description = FText::FromString(bES
			? TEXT("Define la carpeta donde se guardaran los assets de configuracion del sistema Save. "
				"Ejemplo: /Game/Config/PGX/Save. Los DataAssets creados aqui controlan como y donde se guardan las partidas.")
			: TEXT("Define the folder where Save system configuration assets will be stored. "
				"Example: /Game/Config/PGX/Save. DataAssets created here control how and where game saves are stored."));
		S.AccentColor = PGX::System::Save;
		S.Action = EPGXTutorialAction::ConfigBasePath;
		Steps.Add(S);
	}

	// Step 1: Create PGXSaveConfig
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Crear Save Config")
			: TEXT("Create Save Config"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXSaveConfig es el DataAsset que controla el sistema de guardado completo. "
				"Define el numero maximo de slots, el formato de serializacion, compresion, y politicas de auto-save.\n\n"
				"POR QUE EXISTE: Centraliza toda la configuracion de persistencia en un solo lugar editable sin tocar codigo. "
				"BENEFICIO: Cambia de 3 a 10 slots, activa compresion, o modifica el intervalo de auto-save sin recompilar. "
				"EJEMPLO: MaxSlots=5, bAutoSave=true, AutoSaveIntervalSeconds=300, bCompressData=true.")
			: TEXT("UPGXSaveConfig is the DataAsset controlling the complete save system. "
				"It defines maximum slot count, serialization format, compression, and auto-save policies.\n\n"
				"WHY IT EXISTS: Centralizes all persistence configuration in a single editable location without touching code. "
				"BENEFIT: Change from 3 to 10 slots, enable compression, or modify auto-save interval without recompiling. "
				"EXAMPLE: MaxSlots=5, bAutoSave=true, AutoSaveIntervalSeconds=300, bCompressData=true."));
		S.AccentColor = PGX::System::Save;
		S.Action = EPGXTutorialAction::CreateAsset;
		S.ActionPath = TEXT("Save");
		S.AssetClass = TEXT("/Script/PGXSaveRuntime.PGXSaveConfig");
		S.AssetName = TEXT("DA_SaveConfig");
		Steps.Add(S);
	}

	// Step 2: Guide - Save Inspector
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXSaveInspector");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Explorar el Save Inspector")
			: TEXT("Explore the Save Inspector"));
		S.Description = FText::FromString(bES
			? TEXT("El Save Inspector muestra los slots activos, sus metadatos (timestamp, tamano, dominio), y el estado de cada operacion. "
				"Puedes ver en vivo si hay un save/load en progreso y su porcentaje de completitud.\n\n"
				"CUANDO USARLO: Para verificar que los datos se estan guardando correctamente o debuggear slots corruptos.")
			: TEXT("The Save Inspector shows active slots, their metadata (timestamp, size, domain), and the state of each operation. "
				"You can see live if a save/load is in progress and its completion percentage.\n\n"
				"WHEN TO USE IT: To verify data is saving correctly or debug corrupt slots."));
		S.AccentColor = PGX::System::Save;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 3: Guide - IPGXSaveable
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Interfaz IPGXSaveable")
			: TEXT("IPGXSaveable Interface"));
		S.Description = FText::FromString(bES
			? TEXT("Para que un objeto participe en el guardado, implementa IPGXSaveable con GetSaveData() y RestoreSaveData(). "
				"El subsistema descubre automaticamente todos los objetos con esta interfaz.\n\n"
				"No necesitas registrar nada manualmente — la deteccion es por reflexion de UE. "
				"EJEMPLO: Tu PlayerController implementa IPGXSaveable para guardar posicion e inventario.")
			: TEXT("For an object to participate in saving, implement IPGXSaveable with GetSaveData() and RestoreSaveData(). "
				"The subsystem automatically discovers all objects with this interface.\n\n"
				"You don't need to register anything manually — detection is by UE reflection. "
				"EXAMPLE: Your PlayerController implements IPGXSaveable to save position and inventory."));
		S.AccentColor = PGX::System::Save;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 4: Guide - Open config
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Configurar Propiedades de Save")
			: TEXT("Configure Save Properties"));
		S.Description = FText::FromString(bES
			? TEXT("Abre DA_SaveConfig. MaxSlots controla cuantas partidas puede tener el jugador. "
				"bAutoSave y AutoSaveInterval controlan el guardado automatico.\n\n"
				"SaveDomains permite segmentar datos (ej: 'Settings' separado de 'Progress'). "
				"Cada dominio puede tener su propia politica de compresion.")
			: TEXT("Open DA_SaveConfig. MaxSlots controls how many saves the player can have. "
				"bAutoSave and AutoSaveInterval control automatic saving.\n\n"
				"SaveDomains allow segmenting data (e.g., 'Settings' separate from 'Progress'). "
				"Each domain can have its own compression policy."));
		S.AccentColor = PGX::System::Save;
		S.Action = EPGXTutorialAction::OpenAsset;
		S.ActionPath = TEXT("Save");
		S.AssetName = TEXT("DA_SaveConfig");
		Steps.Add(S);
	}

	// Step 5: Guide - Domains
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Dominios de Guardado")
			: TEXT("Save Domains"));
		S.Description = FText::FromString(bES
			? TEXT("Los dominios segmentan datos de guardado. 'Settings' guarda opciones por separado del progreso. "
				"Esto permite guardar settings inmediatamente sin tocar el save de progreso.\n\n"
				"EJEMPLO: Dominio 'Progress' con auto-save cada 5 min, Dominio 'Settings' con save inmediato.")
			: TEXT("Domains segment save data. 'Settings' stores options separately from progress. "
				"This allows saving settings immediately without touching the progress save.\n\n"
				"EXAMPLE: Domain 'Progress' with auto-save every 5 min, Domain 'Settings' with immediate save."));
		S.AccentColor = PGX::System::Save;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 6: Guide - Async
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Operaciones Asincronas")
			: TEXT("Async Operations"));
		S.Description = FText::FromString(bES
			? TEXT("Save/Load son asincronos para no bloquear el game thread. "
				"OnSaveCompleted y OnLoadCompleted notifican cuando termina.\n\n"
				"Blueprint: SaveGameToSlot, LoadGameFromSlot, DeleteSlot. "
				"Nunca asumas que un save es inmediato — siempre escucha el delegate.")
			: TEXT("Save/Load are asynchronous to avoid blocking the game thread. "
				"OnSaveCompleted and OnLoadCompleted notify when finished.\n\n"
				"Blueprint: SaveGameToSlot, LoadGameFromSlot, DeleteSlot. "
				"Never assume a save is immediate — always listen for the delegate."));
		S.AccentColor = PGX::System::Save;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 7: Guide - Console
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Comandos de Consola de Save")
			: TEXT("Save Console Commands"));
		S.Description = FText::FromString(bES
			? TEXT("'pgx.save.status' muestra slots y operaciones activas. "
				"'pgx.save.list' muestra todos los slots con metadatos.\n\n"
				"'pgx.save.save <slot>' fuerza un guardado inmediato. "
				"Utiles para testing rapido sin UI de guardado.")
			: TEXT("'pgx.save.status' shows slots and active operations. "
				"'pgx.save.list' shows all slots with metadata.\n\n"
				"'pgx.save.save <slot>' forces an immediate save. "
				"Useful for quick testing without a save UI."));
		S.AccentColor = PGX::System::Save;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 8: Guide - System Observer
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXSystemObserver");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Save en el System Observer")
			: TEXT("Save in the System Observer"));
		S.Description = FText::FromString(bES
			? TEXT("Save muestra estado de inicializacion, slots usados vs maximo, y si auto-save esta activo. "
				"Verifica que aparezca como 'Initialized'. Si dice 'No Config', revisa que DA_SaveConfig exista.")
			: TEXT("Save shows initialization state, slots used vs maximum, and whether auto-save is active. "
				"Verify it appears as 'Initialized'. If it says 'No Config', check DA_SaveConfig exists."));
		S.AccentColor = PGX::System::Save;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 9: Summary
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXHub");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Resumen: Save Configurado")
			: TEXT("Summary: Save Configured"));
		S.Description = FText::FromString(bES
			? TEXT("Has completado la configuracion del sistema Save. Resumen:\n\n"
				"- CREADO: DA_SaveConfig (slots, auto-save, compresion, dominios)\n"
				"- EXPLORADO: Save Inspector (slots activos y operaciones en vivo)\n"
				"- APRENDIDO: IPGXSaveable para participar en el guardado\n"
				"- APRENDIDO: Dominios para segmentar datos\n"
				"- APRENDIDO: Operaciones async con delegates\n\n"
				"Siguiente paso recomendado: Tutorial S4 (PSO) para optimizar shaders.")
			: TEXT("You have completed the Save system configuration. Summary:\n\n"
				"- CREATED: DA_SaveConfig (slots, auto-save, compression, domains)\n"
				"- EXPLORED: Save Inspector (active slots and live operations)\n"
				"- LEARNED: IPGXSaveable for save participation\n"
				"- LEARNED: Domains for data segmentation\n"
				"- LEARNED: Async operations with delegates\n\n"
				"Recommended next step: Tutorial S4 (PSO) to optimize shaders."));
		S.AccentColor = PGX::Semantic::Good;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	return Steps;
}

// ============================================================================
// S4: PSO System (10 steps)
// ============================================================================
TArray<FPGXTutorialStep> PGXSystemTutorials::GetS4_PSO(EPGXTutorialLanguage Lang)
{
	const bool bES = (Lang == EPGXTutorialLanguage::Spanish);
	TArray<FPGXTutorialStep> Steps;

	// Step 0: ConfigBasePath
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES ? TEXT("Configurar Ruta Base") : TEXT("Configure Base Path"));
		S.Description = FText::FromString(bES
			? TEXT("Define la carpeta para assets PSO. Ejemplo: /Game/Config/PGX/PSO. Controlan el precacheo de shaders.")
			: TEXT("Define the folder for PSO assets. Example: /Game/Config/PGX/PSO. They control shader precaching."));
		S.AccentColor = PGX::System::PSO;
		S.Action = EPGXTutorialAction::ConfigBasePath;
		Steps.Add(S);
	}

	// Step 1: Create PGXPSOWarmUpConfig
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES ? TEXT("Crear PSO WarmUp Config") : TEXT("Create PSO WarmUp Config"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXPSOWarmUpConfig define QUE shaders precompilar y CUANDO. "
				"Los PSOs son shaders compilados para una GPU especifica. Sin precacheo, la primera vez causa un hitch visible.\n\n"
				"BENEFICIO: Elimina hitches configurando que materiales pre-calentar por contexto. "
				"EJEMPLO: Precachea materiales de personajes durante la pantalla de carga.")
			: TEXT("UPGXPSOWarmUpConfig defines WHICH shaders to precompile and WHEN. "
				"PSOs are shaders compiled for a specific GPU. Without precaching, first use causes a visible hitch.\n\n"
				"BENEFIT: Eliminates hitches by configuring which materials to warm up per context. "
				"EXAMPLE: Precache character materials during loading screen."));
		S.AccentColor = PGX::System::PSO;
		S.Action = EPGXTutorialAction::CreateAsset;
		S.ActionPath = TEXT("PSO");
		S.AssetClass = TEXT("/Script/PGXPSORuntime.PGXPSOWarmUpConfig");
		S.AssetName = TEXT("DA_PSOWarmUp");
		Steps.Add(S);
	}

	// Step 2: Guide - PSO Inspector
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXPSOInspector");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES ? TEXT("Explorar el PSO Inspector") : TEXT("Explore the PSO Inspector"));
		S.Description = FText::FromString(bES
			? TEXT("Muestra PSOs compilados, pendientes, y fallidos con barra de progreso. "
				"CUANDO USARLO: Para verificar que el precacheo se completa antes de entrar al gameplay.")
			: TEXT("Shows compiled, pending, and failed PSOs with progress bar. "
				"WHEN TO USE IT: To verify precaching completes before entering gameplay."));
		S.AccentColor = PGX::System::PSO;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 3: Guide - Auto Populator
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXPSOAutoPopulator");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES ? TEXT("PSO Auto Populator") : TEXT("PSO Auto Populator"));
		S.Description = FText::FromString(bES
			? TEXT("Escanea tu proyecto y genera entradas PSO automaticamente analizando materiales referenciados. "
				"CUANDO USARLO: Despues de agregar nuevos materiales o niveles.")
			: TEXT("Scans your project and auto-generates PSO entries by analyzing referenced materials. "
				"WHEN TO USE IT: After adding new materials or levels."));
		S.AccentColor = PGX::System::PSO;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 4: Guide - Open config
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES ? TEXT("Configurar Contextos PSO") : TEXT("Configure PSO Contexts"));
		S.Description = FText::FromString(bES
			? TEXT("Abre DA_PSOWarmUp. Cada contexto vincula un grupo de PSOs a un GameplayTag de GameFlow. "
				"WarmUpContexts: array de contextos con MaterialPaths y tag de activacion.\n\n"
				"EJEMPLO: Contexto 'Combat' con PGX.GameFlow.State.Combat precachea materiales VFX.")
			: TEXT("Open DA_PSOWarmUp. Each context links a PSO group to a GameFlow GameplayTag. "
				"WarmUpContexts: array of contexts with MaterialPaths and activation tag.\n\n"
				"EXAMPLE: Context 'Combat' with PGX.GameFlow.State.Combat precaches VFX materials."));
		S.AccentColor = PGX::System::PSO;
		S.Action = EPGXTutorialAction::OpenAsset;
		S.ActionPath = TEXT("PSO");
		S.AssetName = TEXT("DA_PSOWarmUp");
		Steps.Add(S);
	}

	// Step 5: Guide - GameFlow integration
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES ? TEXT("Integracion PSO + GameFlow") : TEXT("PSO + GameFlow Integration"));
		S.Description = FText::FromString(bES
			? TEXT("PSO escucha delegates de GameFlow para activar contextos. Loading espera que PSO complete. "
				"Esta coordinacion es automatica — solo define contextos en el config DA.")
			: TEXT("PSO listens to GameFlow delegates to activate contexts. Loading waits for PSO to complete. "
				"This coordination is automatic — just define contexts in the config DA."));
		S.AccentColor = PGX::System::PSO;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 6: Guide - Recording
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES ? TEXT("Modo de Grabacion PSO") : TEXT("PSO Recording Mode"));
		S.Description = FText::FromString(bES
			? TEXT("Modo grabacion captura PSOs usados durante gameplay. Juegas normalmente, el sistema registra cada PSO compilado on-the-fly. "
				"Exporta la grabacion al config DA. Consola: 'pgx.pso.record start/stop'.")
			: TEXT("Recording mode captures PSOs used during gameplay. Play normally, the system records every PSO compiled on-the-fly. "
				"Export the recording to config DA. Console: 'pgx.pso.record start/stop'."));
		S.AccentColor = PGX::System::PSO;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 7: Guide - Console
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES ? TEXT("Comandos de Consola PSO") : TEXT("PSO Console Commands"));
		S.Description = FText::FromString(bES
			? TEXT("'pgx.pso.status' muestra compilados/pendientes. 'pgx.pso.warmup <ctx>' fuerza precacheo. "
				"'pgx.pso.record start/stop' grabacion. 'pgx.pso.pause/resume' controla compilacion background.")
			: TEXT("'pgx.pso.status' shows compiled/pending. 'pgx.pso.warmup <ctx>' forces precaching. "
				"'pgx.pso.record start/stop' recording. 'pgx.pso.pause/resume' controls background compilation."));
		S.AccentColor = PGX::System::PSO;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 8: Guide - System Observer
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXSystemObserver");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES ? TEXT("PSO en el System Observer") : TEXT("PSO in the System Observer"));
		S.Description = FText::FromString(bES
			? TEXT("PSO muestra total de PSOs, compilados, y tasa de exito. Fallidos en rojo. Verifica contexto activo vs GameFlow.")
			: TEXT("PSO shows total PSOs, compiled, and success rate. Failed in red. Verify active context vs GameFlow."));
		S.AccentColor = PGX::System::PSO;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 9: Summary
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXHub");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES ? TEXT("Resumen: PSO Configurado") : TEXT("Summary: PSO Configured"));
		S.Description = FText::FromString(bES
			? TEXT("Sistema PSO configurado.\n\n"
				"- CREADO: DA_PSOWarmUp (contextos de precacheo)\n"
				"- EXPLORADO: PSO Inspector + Auto Populator\n"
				"- APRENDIDO: Integracion GameFlow + modo grabacion\n\n"
				"Siguiente: Tutorial S5 (Loading).")
			: TEXT("PSO system configured.\n\n"
				"- CREATED: DA_PSOWarmUp (precaching contexts)\n"
				"- EXPLORED: PSO Inspector + Auto Populator\n"
				"- LEARNED: GameFlow integration + recording mode\n\n"
				"Next: Tutorial S5 (Loading)."));
		S.AccentColor = PGX::Semantic::Good;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	return Steps;
}

// ============================================================================
// S5: Loading System (10 steps)
// ============================================================================
TArray<FPGXTutorialStep> PGXSystemTutorials::GetS5_Loading(EPGXTutorialLanguage Lang)
{
	const bool bES = (Lang == EPGXTutorialLanguage::Spanish);
	TArray<FPGXTutorialStep> Steps;

	// Step 0: ConfigBasePath
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES ? TEXT("Configurar Ruta Base") : TEXT("Configure Base Path"));
		S.Description = FText::FromString(bES
			? TEXT("Define la carpeta para assets Loading. Ejemplo: /Game/Config/PGX/Loading.")
			: TEXT("Define the folder for Loading assets. Example: /Game/Config/PGX/Loading."));
		S.AccentColor = PGX::System::Loading;
		S.Action = EPGXTutorialAction::ConfigBasePath;
		Steps.Add(S);
	}

	// Step 1: Create PGXLoadingConfig
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES ? TEXT("Crear Loading Config") : TEXT("Create Loading Config"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXLoadingConfig es el DataAsset central del sistema de carga. "
				"Define tiempos minimos, fade in/out, y tips/hints.\n\n"
				"POR QUE EXISTE: Centraliza TODAS las pantallas de carga en un config editable. "
				"BENEFICIO: Cambia duracion o activa tips sin modificar widgets. "
				"EJEMPLO: MinDisplayTime=2.0s, FadeOutDuration=0.5s, bShowTips=true.")
			: TEXT("UPGXLoadingConfig is the central DataAsset of the loading system. "
				"It defines minimum times, fade in/out, and tips/hints.\n\n"
				"WHY IT EXISTS: Centralizes ALL loading screens in an editable config. "
				"BENEFIT: Change duration or enable tips without modifying widgets. "
				"EXAMPLE: MinDisplayTime=2.0s, FadeOutDuration=0.5s, bShowTips=true."));
		S.AccentColor = PGX::System::Loading;
		S.Action = EPGXTutorialAction::CreateAsset;
		S.ActionPath = TEXT("Loading");
		S.AssetClass = TEXT("/Script/PGXLoadingRuntime.PGXLoadingConfig");
		S.AssetName = TEXT("DA_LoadingConfig");
		Steps.Add(S);
	}

	// Step 2: Create PGXLoadingProfile
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES ? TEXT("Crear Loading Profile") : TEXT("Create Loading Profile"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXLoadingProfile define un estilo de pantalla de carga: widget, fondo, musica, tips. "
				"POR QUE EXISTE: Diferentes niveles necesitan diferentes loading screens.\n\n"
				"BENEFICIO: Multiples profiles sin duplicar logica. "
				"EJEMPLO: 'CombatLoading' agresivo, 'ExploreLoading' sereno.")
			: TEXT("UPGXLoadingProfile defines a loading screen style: widget, background, music, tips. "
				"WHY IT EXISTS: Different levels need different loading screens.\n\n"
				"BENEFIT: Multiple profiles without duplicating logic. "
				"EXAMPLE: 'CombatLoading' aggressive, 'ExploreLoading' serene."));
		S.AccentColor = PGX::System::Loading;
		S.Action = EPGXTutorialAction::CreateAsset;
		S.ActionPath = TEXT("Loading");
		S.AssetClass = TEXT("/Script/PGXLoadingRuntime.PGXLoadingProfile");
		S.AssetName = TEXT("DA_LoadingProfile_Default");
		Steps.Add(S);
	}

	// Step 3: Guide - Loading Inspector
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXLoadingInspector");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES ? TEXT("Explorar el Loading Inspector") : TEXT("Explore the Loading Inspector"));
		S.Description = FText::FromString(bES
			? TEXT("Muestra pantallas activas, profile usado, progreso, e historial de cargas recientes con tiempos. "
				"CUANDO USARLO: Para verificar profiles o medir tiempos de carga reales.")
			: TEXT("Shows active screens, profile used, progress, and recent load history with times. "
				"WHEN TO USE IT: To verify profiles or measure actual loading times."));
		S.AccentColor = PGX::System::Loading;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 4: Guide - Open config
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES ? TEXT("Configurar Loading") : TEXT("Configure Loading"));
		S.Description = FText::FromString(bES
			? TEXT("Abre DA_LoadingConfig. DefaultProfile referencia el profile por defecto. "
				"bWaitForPSO espera compilacion de PSOs. MinDisplayTime evita parpadeo en cargas rapidas.")
			: TEXT("Open DA_LoadingConfig. DefaultProfile references the default profile. "
				"bWaitForPSO waits for PSO compilation. MinDisplayTime prevents flashing on fast loads."));
		S.AccentColor = PGX::System::Loading;
		S.Action = EPGXTutorialAction::OpenAsset;
		S.ActionPath = TEXT("Loading");
		S.AssetName = TEXT("DA_LoadingConfig");
		Steps.Add(S);
	}

	// Step 5: Guide - PSO coordination
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES ? TEXT("Coordinacion Loading + PSO") : TEXT("Loading + PSO Coordination"));
		S.Description = FText::FromString(bES
			? TEXT("Loading depende de PSO (excepcion L2->L2 documentada). Con bWaitForPSO, mantiene la pantalla hasta que PSO termine. "
				"Garantiza 0 hitches al entrar al nivel.")
			: TEXT("Loading depends on PSO (documented L2->L2 exception). With bWaitForPSO, keeps the screen until PSO finishes. "
				"Guarantees 0 hitches when entering the level."));
		S.AccentColor = PGX::System::Loading;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 6: Guide - Blueprint
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES ? TEXT("Loading en Blueprint") : TEXT("Loading in Blueprint"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXLoadingBlueprintLibrary: RequestLoadingScreen, ForceClose, IsActive, GetProgress. "
				"EJEMPLO: Al abrir puerta de boss, RequestLoadingScreen con profile 'BossLoading'.")
			: TEXT("UPGXLoadingBlueprintLibrary: RequestLoadingScreen, ForceClose, IsActive, GetProgress. "
				"EXAMPLE: When opening boss door, RequestLoadingScreen with 'BossLoading' profile."));
		S.AccentColor = PGX::System::Loading;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 7: Guide - Console
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES ? TEXT("Comandos de Consola Loading") : TEXT("Loading Console Commands"));
		S.Description = FText::FromString(bES
			? TEXT("'pgx.loading.status', 'pgx.loading.show', 'pgx.loading.hide', 'pgx.loading.skip' (salta min display time).")
			: TEXT("'pgx.loading.status', 'pgx.loading.show', 'pgx.loading.hide', 'pgx.loading.skip' (skips min display time)."));
		S.AccentColor = PGX::System::Loading;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 8: Guide - System Observer
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXSystemObserver");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES ? TEXT("Loading en System Observer") : TEXT("Loading in System Observer"));
		S.Description = FText::FromString(bES
			? TEXT("Loading muestra estado, profile activo, y estadisticas de cargas. Verifica config DA descubierto.")
			: TEXT("Loading shows state, active profile, and load statistics. Verify config DA discovered."));
		S.AccentColor = PGX::System::Loading;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 9: Summary
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXHub");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES ? TEXT("Resumen: Loading Configurado") : TEXT("Summary: Loading Configured"));
		S.Description = FText::FromString(bES
			? TEXT("Sistema Loading configurado.\n\n"
				"- CREADO: DA_LoadingConfig + DA_LoadingProfile_Default\n"
				"- EXPLORADO: Loading Inspector\n"
				"- APRENDIDO: Coordinacion Loading-PSO + profiles multiples\n\n"
				"Siguiente: Tutorial S6 (LevelFlow).")
			: TEXT("Loading system configured.\n\n"
				"- CREATED: DA_LoadingConfig + DA_LoadingProfile_Default\n"
				"- EXPLORED: Loading Inspector\n"
				"- LEARNED: Loading-PSO coordination + multiple profiles\n\n"
				"Next: Tutorial S6 (LevelFlow)."));
		S.AccentColor = PGX::Semantic::Good;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	return Steps;
}

// ============================================================================
// S6: LevelFlow System (9 steps)
// ============================================================================
TArray<FPGXTutorialStep> PGXSystemTutorials::GetS6_LevelFlow(EPGXTutorialLanguage Lang)
{
	const bool bES = (Lang == EPGXTutorialLanguage::Spanish);
	TArray<FPGXTutorialStep> Steps;

	// Step 0: ConfigBasePath
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES ? TEXT("Configurar Ruta Base") : TEXT("Configure Base Path"));
		S.Description = FText::FromString(bES
			? TEXT("Define la carpeta para assets LevelFlow. Ejemplo: /Game/Config/PGX/LevelFlow.")
			: TEXT("Define the folder for LevelFlow assets. Example: /Game/Config/PGX/LevelFlow."));
		S.AccentColor = PGX::System::LevelFlow;
		S.Action = EPGXTutorialAction::ConfigBasePath;
		Steps.Add(S);
	}

	// Step 1: Create PGXLevelFlowConfig
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES ? TEXT("Crear LevelFlow Config") : TEXT("Create LevelFlow Config"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXLevelFlowConfig gestiona transiciones entre niveles: mapa, streaming, y politicas de sublevels. "
				"POR QUE EXISTE: Transiciones de nivel en UE son propensas a errores sin coordinacion con Loading y PSO.\n\n"
				"BENEFICIO: Un config que orquesta travel, streaming, y sublevels con garantias. "
				"EJEMPLO: 'Overworld' con 4 sublevels por region, streaming por distancia.")
			: TEXT("UPGXLevelFlowConfig manages level transitions: map, streaming, and sublevel policies. "
				"WHY IT EXISTS: Level transitions in UE are error-prone without Loading and PSO coordination.\n\n"
				"BENEFIT: A single config orchestrating travel, streaming, and sublevels with guarantees. "
				"EXAMPLE: 'Overworld' with 4 sublevels per region, distance-based streaming."));
		S.AccentColor = PGX::System::LevelFlow;
		S.Action = EPGXTutorialAction::CreateAsset;
		S.ActionPath = TEXT("LevelFlow");
		S.AssetClass = TEXT("/Script/PGXLoadingRuntime.PGXLevelFlowConfig");
		S.AssetName = TEXT("DA_LevelFlowConfig");
		Steps.Add(S);
	}

	// Step 2: Create PGXLevelProfile
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES ? TEXT("Crear Level Profile") : TEXT("Create Level Profile"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXLevelProfile configura UN nivel: sublevels, loading profile, y PSO contexto. "
				"POR QUE EXISTE: Cada nivel tiene necesidades diferentes.\n\n"
				"BENEFICIO: Configura nivel por nivel sin codigo. "
				"EJEMPLO: 'BossFight_01' usa 'BossLoading', precachea VFX, sin sublevels.")
			: TEXT("UPGXLevelProfile configures ONE level: sublevels, loading profile, and PSO context. "
				"WHY IT EXISTS: Each level has different needs.\n\n"
				"BENEFIT: Configure level-by-level without code. "
				"EXAMPLE: 'BossFight_01' uses 'BossLoading', precaches VFX, no sublevels."));
		S.AccentColor = PGX::System::LevelFlow;
		S.Action = EPGXTutorialAction::CreateAsset;
		S.ActionPath = TEXT("LevelFlow");
		S.AssetClass = TEXT("/Script/PGXLoadingRuntime.PGXLevelProfile");
		S.AssetName = TEXT("DA_LevelProfile_Default");
		Steps.Add(S);
	}

	// Step 3: Guide - LevelFlow Inspector
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXLevelFlowInspector");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES ? TEXT("Explorar LevelFlow Inspector") : TEXT("Explore LevelFlow Inspector"));
		S.Description = FText::FromString(bES
			? TEXT("Muestra nivel actual, sublevels cargados, transiciones en progreso, e historial con resultados. "
				"CUANDO USARLO: Debuggear transiciones fallidas o verificar sublevels.")
			: TEXT("Shows current level, loaded sublevels, transitions in progress, and history with results. "
				"WHEN TO USE IT: Debug failed transitions or verify sublevels."));
		S.AccentColor = PGX::System::LevelFlow;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 4: Guide - Configure
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES ? TEXT("Configurar Level Profile") : TEXT("Configure Level Profile"));
		S.Description = FText::FromString(bES
			? TEXT("Abre DA_LevelProfile_Default. LevelPath, SubLevels, LoadingProfile, PSOContext. "
				"Crea multiples LevelProfiles para diferentes niveles.")
			: TEXT("Open DA_LevelProfile_Default. LevelPath, SubLevels, LoadingProfile, PSOContext. "
				"Create multiple LevelProfiles for different levels."));
		S.AccentColor = PGX::System::LevelFlow;
		S.Action = EPGXTutorialAction::OpenAsset;
		S.ActionPath = TEXT("LevelFlow");
		S.AssetName = TEXT("DA_LevelProfile_Default");
		Steps.Add(S);
	}

	// Step 5: Guide - Transition API
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES ? TEXT("API de Transicion") : TEXT("Transition API"));
		S.Description = FText::FromString(bES
			? TEXT("RequestLevelTransition(LevelProfile) coordina: Loading screen, PSO wait, ServerTravel, hide Loading. "
				"Delegates: OnLevelTransitionStarted, OnLevelTransitionCompleted, OnSubLevelLoaded.\n\n"
				"Blueprint: RequestLevelTransition, CancelTransition, LoadSubLevel, UnloadSubLevel.")
			: TEXT("RequestLevelTransition(LevelProfile) coordinates: Loading screen, PSO wait, ServerTravel, hide Loading. "
				"Delegates: OnLevelTransitionStarted, OnLevelTransitionCompleted, OnSubLevelLoaded.\n\n"
				"Blueprint: RequestLevelTransition, CancelTransition, LoadSubLevel, UnloadSubLevel."));
		S.AccentColor = PGX::System::LevelFlow;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 6: Guide - Console
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES ? TEXT("Comandos de Consola LevelFlow") : TEXT("LevelFlow Console Commands"));
		S.Description = FText::FromString(bES
			? TEXT("'pgx.levelflow.status', 'pgx.levelflow.transition <level>', 'pgx.levelflow.sublevels'.")
			: TEXT("'pgx.levelflow.status', 'pgx.levelflow.transition <level>', 'pgx.levelflow.sublevels'."));
		S.AccentColor = PGX::System::LevelFlow;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 7: Guide - System Observer
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXSystemObserver");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES ? TEXT("LevelFlow en System Observer") : TEXT("LevelFlow in System Observer"));
		S.Description = FText::FromString(bES
			? TEXT("Muestra nivel actual, sublevels activos, transiciones en progreso. Verifica inicializacion y config DA.")
			: TEXT("Shows current level, active sublevels, transitions in progress. Verify initialization and config DA."));
		S.AccentColor = PGX::System::LevelFlow;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 8: Summary
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXHub");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES ? TEXT("Resumen: LevelFlow Configurado") : TEXT("Summary: LevelFlow Configured"));
		S.Description = FText::FromString(bES
			? TEXT("Sistema LevelFlow configurado.\n\n"
				"- CREADO: DA_LevelFlowConfig + DA_LevelProfile_Default\n"
				"- EXPLORADO: LevelFlow Inspector\n"
				"- APRENDIDO: Coordinacion con Loading/PSO + API de transicion\n\n"
				"Siguiente: Tutorial S7 (Audio).")
			: TEXT("LevelFlow system configured.\n\n"
				"- CREATED: DA_LevelFlowConfig + DA_LevelProfile_Default\n"
				"- EXPLORED: LevelFlow Inspector\n"
				"- LEARNED: Loading/PSO coordination + transition API\n\n"
				"Next: Tutorial S7 (Audio)."));
		S.AccentColor = PGX::Semantic::Good;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	return Steps;
}

// ============================================================================
// S7: Audio System (12 steps)
// ============================================================================
TArray<FPGXTutorialStep> PGXSystemTutorials::GetS7_Audio(EPGXTutorialLanguage Lang)
{
	const bool bES = (Lang == EPGXTutorialLanguage::Spanish);
	TArray<FPGXTutorialStep> Steps;

	// Step 0: ConfigBasePath
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Configurar Ruta Base del Proyecto")
			: TEXT("Configure Project Base Path"));
		S.Description = FText::FromString(bES
			? TEXT("Define la carpeta donde se guardaran los assets del sistema Audio. "
				"Ejemplo: /Game/Config/PGX/Audio. Aqui van la configuracion general, canales, y playlists de musica.")
			: TEXT("Define the folder where Audio system assets will be stored. "
				"Example: /Game/Config/PGX/Audio. This holds general config, channels, and music playlists."));
		S.AccentColor = PGX::System::Audio;
		S.Action = EPGXTutorialAction::ConfigBasePath;
		Steps.Add(S);
	}

	// Step 1: Create PGXAudioConfig
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Crear Audio Config")
			: TEXT("Create Audio Config"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXAudioConfig es el DataAsset maestro del sistema de audio dual-backend. "
				"Controla que backend usar (MetaSound o Legacy), el numero maximo de voces simultaneas, y la politica de prioridades.\n\n"
				"POR QUE EXISTE: PGX Audio soporta dos backends (MetaSound y Legacy SoundCue) con la misma API. Este config decide cual usar. "
				"BENEFICIO: Cambia de backend sin modificar una linea de gameplay code. El subsistema abstrae la implementacion. "
				"EJEMPLO: En desarrollo usas Legacy (mas estable), en produccion migras a MetaSound cambiando un solo campo.")
			: TEXT("UPGXAudioConfig is the master DataAsset of the dual-backend audio system. "
				"It controls which backend to use (MetaSound or Legacy), maximum simultaneous voices, and priority policy.\n\n"
				"WHY IT EXISTS: PGX Audio supports two backends (MetaSound and Legacy SoundCue) with the same API. This config decides which one. "
				"BENEFIT: Switch backends without modifying a single line of gameplay code. The subsystem abstracts the implementation. "
				"EXAMPLE: During development use Legacy (more stable), migrate to MetaSound in production by changing a single field."));
		S.AccentColor = PGX::System::Audio;
		S.Action = EPGXTutorialAction::CreateAsset;
		S.ActionPath = TEXT("Audio");
		S.AssetClass = TEXT("/Script/PGXAudioRuntime.PGXAudioConfig");
		S.AssetName = TEXT("DA_AudioConfig");
		Steps.Add(S);
	}

	// Step 2: Create PGXChannelConfig
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Crear Channel Config")
			: TEXT("Create Channel Config"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXChannelConfig define un CANAL de audio con volumen independiente, mute, y bus de efectos. "
				"Los canales tipicos son: Master, Music, SFX, Voice, Ambient, UI.\n\n"
				"POR QUE EXISTE: Permite al jugador controlar volumenes independientemente (ej: bajar musica sin afectar SFX). "
				"BENEFICIO: Cada canal se puede mute/unmute, hacer fade, y vincular a Sound Classes de UE. "
				"EJEMPLO: Canal 'Music' con volumen default 0.7, vinculado al Sound Class 'SC_Music'.")
			: TEXT("UPGXChannelConfig defines an audio CHANNEL with independent volume, mute, and effects bus. "
				"Typical channels are: Master, Music, SFX, Voice, Ambient, UI.\n\n"
				"WHY IT EXISTS: Allows the player to control volumes independently (e.g., lower music without affecting SFX). "
				"BENEFIT: Each channel can be muted/unmuted, faded, and linked to UE Sound Classes. "
				"EXAMPLE: Channel 'Music' with default volume 0.7, linked to Sound Class 'SC_Music'."));
		S.AccentColor = PGX::System::Audio;
		S.Action = EPGXTutorialAction::CreateAsset;
		S.ActionPath = TEXT("Audio");
		S.AssetClass = TEXT("/Script/PGXAudioRuntime.PGXChannelConfig");
		S.AssetName = TEXT("DA_Channel_Master");
		Steps.Add(S);
	}

	// Step 3: Create PGXMusicPlaylist
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Crear Music Playlist")
			: TEXT("Create Music Playlist"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXMusicPlaylist define una lista de pistas musicales con orden, shuffle, loop, y crossfade. "
				"POR QUE EXISTE: La musica de juego necesita transiciones suaves entre pistas y soporte para diferentes modos (combate, exploracion).\n\n"
				"BENEFICIO: Configura playlists completas sin codigo — solo arrastra SoundWaves al array y ajusta crossfade. "
				"EJEMPLO: Playlist 'ExplorationMusic' con 5 pistas ambientales, shuffle=true, crossfade=2.0s, loop=true.")
			: TEXT("UPGXMusicPlaylist defines a list of music tracks with order, shuffle, loop, and crossfade. "
				"WHY IT EXISTS: Game music needs smooth transitions between tracks and support for different modes (combat, exploration).\n\n"
				"BENEFIT: Configure complete playlists without code — just drag SoundWaves to the array and adjust crossfade. "
				"EXAMPLE: Playlist 'ExplorationMusic' with 5 ambient tracks, shuffle=true, crossfade=2.0s, loop=true."));
		S.AccentColor = PGX::System::Audio;
		S.Action = EPGXTutorialAction::CreateAsset;
		S.ActionPath = TEXT("Audio");
		S.AssetClass = TEXT("/Script/PGXAudioRuntime.PGXMusicPlaylist");
		S.AssetName = TEXT("DA_Playlist_Exploration");
		Steps.Add(S);
	}

	// Step 4: Guide - Audio Inspector
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXAudioInspector");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Explorar el Audio Inspector")
			: TEXT("Explore the Audio Inspector"));
		S.Description = FText::FromString(bES
			? TEXT("El Audio Inspector muestra en vivo: backend activo, voces en uso, canales con sus volumenes, y la playlist actual. "
				"Incluye controles para hacer mute/unmute de canales y cambiar playlists en caliente.\n\n"
				"CUANDO USARLO: Para debuggear por que un sonido no se escucha, verificar que canal esta muteado, o monitorear voces activas.")
			: TEXT("The Audio Inspector shows live: active backend, voices in use, channels with their volumes, and current playlist. "
				"It includes controls to mute/unmute channels and switch playlists on-the-fly.\n\n"
				"WHEN TO USE IT: To debug why a sound isn't playing, verify which channel is muted, or monitor active voices."));
		S.AccentColor = PGX::System::Audio;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 5: Guide - Open AudioConfig
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Configurar Audio Config")
			: TEXT("Configure Audio Config"));
		S.Description = FText::FromString(bES
			? TEXT("Abre DA_AudioConfig. Propiedades clave: Backend (MetaSound/Legacy), MaxVoices, ChannelConfigs (referencia los canales creados). "
				"VoicePriorityPolicy define que pasa cuando se excede MaxVoices: StealOldest, StealLowestPriority, o Reject.\n\n"
				"Vincular los ChannelConfigs aqui permite al subsistema descubrirlos automaticamente en Initialize().")
			: TEXT("Open DA_AudioConfig. Key properties: Backend (MetaSound/Legacy), MaxVoices, ChannelConfigs (references created channels). "
				"VoicePriorityPolicy defines what happens when MaxVoices is exceeded: StealOldest, StealLowestPriority, or Reject.\n\n"
				"Linking ChannelConfigs here allows the subsystem to auto-discover them in Initialize()."));
		S.AccentColor = PGX::System::Audio;
		S.Action = EPGXTutorialAction::OpenAsset;
		S.ActionPath = TEXT("Audio");
		S.AssetName = TEXT("DA_AudioConfig");
		Steps.Add(S);
	}

	// Step 6: Guide - Dual backend
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Arquitectura Dual-Backend")
			: TEXT("Dual-Backend Architecture"));
		S.Description = FText::FromString(bES
			? TEXT("PGX Audio tiene dos subsistemas internos: UPGXAudioSubsystem (mundo) y UPGXAudioEngineSubsystem (motor). "
				"El subsistema de mundo maneja playback por nivel. El de motor maneja estado global (musica, volumenes).\n\n"
				"Ambos exponen la misma API — el backend se selecciona en el config DA y es transparente para el gameplay code. "
				"Puedes cambiar de Legacy a MetaSound sin tocar ni un solo nodo Blueprint.")
			: TEXT("PGX Audio has two internal subsystems: UPGXAudioSubsystem (world) and UPGXAudioEngineSubsystem (engine). "
				"The world subsystem handles per-level playback. The engine one handles global state (music, volumes).\n\n"
				"Both expose the same API — the backend is selected in the config DA and is transparent to gameplay code. "
				"You can switch from Legacy to MetaSound without touching a single Blueprint node."));
		S.AccentColor = PGX::System::Audio;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 7: Guide - Playback API
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("API de Reproduccion")
			: TEXT("Playback API"));
		S.Description = FText::FromString(bES
			? TEXT("El API de audio incluye: PlaySound2D, PlaySoundAtLocation, PlayMusic, StopMusic, FadeChannel, SetChannelVolume. "
				"PlayDialogue maneja colas de dialogos con prioridades. PlayAmbience gestiona loops ambientales por zona.\n\n"
				"Todos los metodos aceptan un canal como parametro para routing automatico de volumen. "
				"EJEMPLO: PlaySoundAtLocation(ExplosionSound, Location, 'SFX') rutea el sonido al canal SFX.")
			: TEXT("The audio API includes: PlaySound2D, PlaySoundAtLocation, PlayMusic, StopMusic, FadeChannel, SetChannelVolume. "
				"PlayDialogue handles dialogue queues with priorities. PlayAmbience manages ambient loops per zone.\n\n"
				"All methods accept a channel as parameter for automatic volume routing. "
				"EXAMPLE: PlaySoundAtLocation(ExplosionSound, Location, 'SFX') routes the sound to the SFX channel."));
		S.AccentColor = PGX::System::Audio;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 8: Guide - Blueprint nodes
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Audio en Blueprint")
			: TEXT("Audio in Blueprint"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXAudioBlueprintLibrary expone ~25 nodos: PlaySound, PlayMusic, StopAll, SetVolume, MuteChannel, etc. "
				"Los delegates OnMusicTrackChanged y OnPlaybackCompleted permiten reaccionar a eventos de audio desde Blueprint.\n\n"
				"EJEMPLO: Tu menu de opciones usa SetChannelVolume('Music', SliderValue) para controlar la musica.")
			: TEXT("UPGXAudioBlueprintLibrary exposes ~25 nodes: PlaySound, PlayMusic, StopAll, SetVolume, MuteChannel, etc. "
				"The OnMusicTrackChanged and OnPlaybackCompleted delegates allow reacting to audio events from Blueprint.\n\n"
				"EXAMPLE: Your options menu uses SetChannelVolume('Music', SliderValue) to control music."));
		S.AccentColor = PGX::System::Audio;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 9: Guide - Console commands
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Comandos de Consola de Audio")
			: TEXT("Audio Console Commands"));
		S.Description = FText::FromString(bES
			? TEXT("Comandos: 'pgx.audio.status' muestra backend, voces, canales. "
				"'pgx.audio.play <sound>' reproduce un sonido.\n\n"
				"'pgx.audio.mute <channel>' mutea un canal. "
				"'pgx.audio.backend <type>' cambia de backend en caliente (debug). "
				"'pgx.audio.playlist <name>' cambia la playlist activa.")
			: TEXT("Commands: 'pgx.audio.status' shows backend, voices, channels. "
				"'pgx.audio.play <sound>' plays a sound.\n\n"
				"'pgx.audio.mute <channel>' mutes a channel. "
				"'pgx.audio.backend <type>' switches backend on-the-fly (debug). "
				"'pgx.audio.playlist <name>' changes the active playlist."));
		S.AccentColor = PGX::System::Audio;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 10: Guide - System Observer
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXSystemObserver");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Audio en el System Observer")
			: TEXT("Audio in the System Observer"));
		S.Description = FText::FromString(bES
			? TEXT("En el System Observer, Audio muestra ambos subsistemas (World y Engine), el backend activo, voces en uso vs maximo. "
				"Verifica que ambos subsistemas aparezcan como 'Initialized' y que el backend sea el esperado.")
			: TEXT("In the System Observer, Audio shows both subsystems (World and Engine), active backend, voices in use vs maximum. "
				"Verify both subsystems appear as 'Initialized' and the backend is the expected one."));
		S.AccentColor = PGX::System::Audio;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 11: Summary
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXHub");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Resumen: Audio Configurado")
			: TEXT("Summary: Audio Configured"));
		S.Description = FText::FromString(bES
			? TEXT("Has completado la configuracion del sistema Audio. Resumen:\n\n"
				"- CREADO: DA_AudioConfig (backend, voces, prioridades)\n"
				"- CREADO: DA_Channel_Master (canal de audio con volumen independiente)\n"
				"- CREADO: DA_Playlist_Exploration (playlist de musica con crossfade)\n"
				"- EXPLORADO: Audio Inspector (backend, voces, canales en vivo)\n"
				"- APRENDIDO: Arquitectura dual-backend (MetaSound/Legacy)\n"
				"- APRENDIDO: API de ~50 funciones + ~25 nodos Blueprint\n\n"
				"Siguiente paso recomendado: Tutorial S8 (Log) para configurar el sistema de logging.")
			: TEXT("You have completed the Audio system configuration. Summary:\n\n"
				"- CREATED: DA_AudioConfig (backend, voices, priorities)\n"
				"- CREATED: DA_Channel_Master (audio channel with independent volume)\n"
				"- CREATED: DA_Playlist_Exploration (music playlist with crossfade)\n"
				"- EXPLORED: Audio Inspector (backend, voices, live channels)\n"
				"- LEARNED: Dual-backend architecture (MetaSound/Legacy)\n"
				"- LEARNED: API of ~50 functions + ~25 Blueprint nodes\n\n"
				"Recommended next step: Tutorial S8 (Log) to configure the logging system."));
		S.AccentColor = PGX::Semantic::Good;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	return Steps;
}

// ============================================================================
// S8: Log System (10 steps)
// ============================================================================
TArray<FPGXTutorialStep> PGXSystemTutorials::GetS8_Log(EPGXTutorialLanguage Lang)
{
	const bool bES = (Lang == EPGXTutorialLanguage::Spanish);
	TArray<FPGXTutorialStep> Steps;

	// Step 0: ConfigBasePath
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Configurar Ruta Base del Proyecto")
			: TEXT("Configure Project Base Path"));
		S.Description = FText::FromString(bES
			? TEXT("Define la carpeta donde se guardaran los assets del sistema Log. "
				"Ejemplo: /Game/Config/PGX/Log. Aqui va la configuracion de dominios y filtros de logging.")
			: TEXT("Define the folder where Log system assets will be stored. "
				"Example: /Game/Config/PGX/Log. This holds domain configuration and logging filters."));
		S.AccentColor = PGX::System::Log;
		S.Action = EPGXTutorialAction::ConfigBasePath;
		Steps.Add(S);
	}

	// Step 1: Create PGXLogDomainConfig
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Crear Log Domain Config")
			: TEXT("Create Log Domain Config"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXLogDomainConfig define un DOMINIO de log con verbosidad, formateo, y sink (destino) personalizados. "
				"PGX Log v3.0 es un sistema polimorfico: cada dominio puede tener su propia pipeline de procesamiento.\n\n"
				"POR QUE EXISTE: UE_LOG es plano y sin estructura. PGX Log agrega dominios, filtros, sinks, y formateo configurables. "
				"BENEFICIO: Filtra logs por dominio ('Combat' vs 'UI'), redirige a archivo, o formatea con JSON — todo desde un DA. "
				"EJEMPLO: Dominio 'Networking' con verbosidad Warning, sink a archivo + consola, formato '[NET] {Message}'.")
			: TEXT("UPGXLogDomainConfig defines a log DOMAIN with custom verbosity, formatting, and sink (destination). "
				"PGX Log v3.0 is a polymorphic system: each domain can have its own processing pipeline.\n\n"
				"WHY IT EXISTS: UE_LOG is flat and unstructured. PGX Log adds configurable domains, filters, sinks, and formatting. "
				"BENEFIT: Filter logs by domain ('Combat' vs 'UI'), redirect to file, or format as JSON — all from a DA. "
				"EXAMPLE: Domain 'Networking' with Warning verbosity, sink to file + console, format '[NET] {Message}'."));
		S.AccentColor = PGX::System::Log;
		S.Action = EPGXTutorialAction::CreateAsset;
		S.ActionPath = TEXT("Log");
		S.AssetClass = TEXT("/Script/PGXCoreRuntime.PGXLogDomainConfig");
		S.AssetName = TEXT("DA_LogDomain_Gameplay");
		Steps.Add(S);
	}

	// Step 2: Guide - Log Viewer
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXLogViewer");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Explorar el Log Viewer")
			: TEXT("Explore the Log Viewer"));
		S.Description = FText::FromString(bES
			? TEXT("El PGX Log Viewer es un visor de logs estructurado que supera al Output Log de UE. "
				"Soporta filtrado por dominio, verbosidad, y busqueda de texto. Los logs se muestran con colores por severidad.\n\n"
				"CUANDO USARLO: Siempre que necesites debuggear — reemplaza el habito de usar el Output Log estandar.")
			: TEXT("The PGX Log Viewer is a structured log viewer that surpasses UE's Output Log. "
				"It supports filtering by domain, verbosity, and text search. Logs are color-coded by severity.\n\n"
				"WHEN TO USE IT: Whenever you need to debug — replace the habit of using the standard Output Log."));
		S.AccentColor = PGX::System::Log;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 3: Guide - Open domain config
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Configurar el Dominio de Log")
			: TEXT("Configure the Log Domain"));
		S.Description = FText::FromString(bES
			? TEXT("Abre DA_LogDomain_Gameplay y configura: DomainName (identificador unico), DefaultVerbosity, Sinks (Console, File, Widget). "
				"FormatPattern permite personalizar el formato: '{Timestamp} [{Domain}] {Verbosity}: {Message}'.\n\n"
				"bEnabled permite desactivar un dominio completo sin eliminarlo. "
				"Puedes crear multiples dominios: uno para Combat, otro para UI, otro para Networking.")
			: TEXT("Open DA_LogDomain_Gameplay and configure: DomainName (unique identifier), DefaultVerbosity, Sinks (Console, File, Widget). "
				"FormatPattern allows customizing format: '{Timestamp} [{Domain}] {Verbosity}: {Message}'.\n\n"
				"bEnabled allows disabling an entire domain without deleting it. "
				"You can create multiple domains: one for Combat, another for UI, another for Networking."));
		S.AccentColor = PGX::System::Log;
		S.Action = EPGXTutorialAction::OpenAsset;
		S.ActionPath = TEXT("Log");
		S.AssetName = TEXT("DA_LogDomain_Gameplay");
		Steps.Add(S);
	}

	// Step 4: Guide - Polymorphic architecture
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Arquitectura Polimorfica de Log")
			: TEXT("Polymorphic Log Architecture"));
		S.Description = FText::FromString(bES
			? TEXT("PGX Log v3.0 usa una pipeline polimorfica: Ingress -> Filter -> Format -> Sink. "
				"Cada etapa es extensible via herencia. Puedes crear filtros custom (ej: solo logs de un actor especifico)\n\n"
				"o sinks custom (ej: enviar logs a un servidor remoto). "
				"Los dominios se componen de estas piezas como bloques LEGO — sin tocar el core del sistema.")
			: TEXT("PGX Log v3.0 uses a polymorphic pipeline: Ingress -> Filter -> Format -> Sink. "
				"Each stage is extensible via inheritance. You can create custom filters (e.g., only logs from a specific actor)\n\n"
				"or custom sinks (e.g., send logs to a remote server). "
				"Domains compose these pieces like LEGO blocks — without touching the system core."));
		S.AccentColor = PGX::System::Log;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 5: Guide - Blueprint logging
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Logging desde Blueprint")
			: TEXT("Logging from Blueprint"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXLogBlueprintLibrary expone nodos: PGXLog, PGXLogWarning, PGXLogError, con dominio como parametro. "
				"Los logs de Blueprint pasan por la misma pipeline que los de C++ — filtros, formatos, y sinks se aplican igual.\n\n"
				"EJEMPLO: PGXLog('Combat', 'Player took {0} damage from {1}', DamageAmount, SourceName).")
			: TEXT("UPGXLogBlueprintLibrary exposes nodes: PGXLog, PGXLogWarning, PGXLogError, with domain as parameter. "
				"Blueprint logs pass through the same pipeline as C++ ones — filters, formats, and sinks apply equally.\n\n"
				"EXAMPLE: PGXLog('Combat', 'Player took {0} damage from {1}', DamageAmount, SourceName)."));
		S.AccentColor = PGX::System::Log;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 6: Guide - Console commands
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Comandos de Consola de Log")
			: TEXT("Log Console Commands"));
		S.Description = FText::FromString(bES
			? TEXT("Comandos: 'pgx.log.status' muestra dominios activos y sus verbosidades. "
				"'pgx.log.verbosity <domain> <level>' cambia verbosidad en caliente.\n\n"
				"'pgx.log.enable <domain>'/'pgx.log.disable <domain>' activa/desactiva dominios. "
				"'pgx.log.flush' fuerza escritura de todos los sinks a disco.")
			: TEXT("Commands: 'pgx.log.status' shows active domains and their verbosities. "
				"'pgx.log.verbosity <domain> <level>' changes verbosity on-the-fly.\n\n"
				"'pgx.log.enable <domain>'/'pgx.log.disable <domain>' enables/disables domains. "
				"'pgx.log.flush' forces all sinks to write to disk."));
		S.AccentColor = PGX::System::Log;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 7: Guide - Log in PGX Docs
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXDocs");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Documentacion de Log en PGX Docs")
			: TEXT("Log Documentation in PGX Docs"));
		S.Description = FText::FromString(bES
			? TEXT("PGX Docs incluye la guia completa del sistema Log v3.0 con diagramas de la pipeline, ejemplos de custom filters y sinks. "
				"Busca '05_PGX_Log' en la navegacion para encontrar la arquitectura detallada.\n\n"
				"CUANDO USARLO: Cuando necesites extender el sistema con filtros o sinks custom.")
			: TEXT("PGX Docs includes the complete Log v3.0 guide with pipeline diagrams, examples of custom filters and sinks. "
				"Search '05_PGX_Log' in navigation to find detailed architecture.\n\n"
				"WHEN TO USE IT: When you need to extend the system with custom filters or sinks."));
		S.AccentColor = PGX::System::Log;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 8: Guide - System Observer
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXSystemObserver");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Log en el System Observer")
			: TEXT("Log in the System Observer"));
		S.Description = FText::FromString(bES
			? TEXT("En el System Observer, Log muestra el numero de dominios activos, mensajes totales por verbosidad, y el estado de los sinks. "
				"Verifica que el subsistema este inicializado y que los dominios configurados aparezcan.")
			: TEXT("In the System Observer, Log shows the number of active domains, total messages per verbosity, and sink states. "
				"Verify the subsystem is initialized and configured domains appear."));
		S.AccentColor = PGX::System::Log;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 9: Summary
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXHub");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Resumen: Log Configurado")
			: TEXT("Summary: Log Configured"));
		S.Description = FText::FromString(bES
			? TEXT("Has completado la configuracion del sistema Log. Resumen:\n\n"
				"- CREADO: DA_LogDomain_Gameplay (dominio con verbosidad, formato, y sinks)\n"
				"- EXPLORADO: PGX Log Viewer (visor estructurado con filtros)\n"
				"- APRENDIDO: Pipeline polimorfica Ingress->Filter->Format->Sink\n"
				"- APRENDIDO: Logging desde Blueprint con dominios\n"
				"- APRENDIDO: Comandos pgx.log.* para control en caliente\n\n"
				"Siguiente paso recomendado: Tutorial S9 (Data Registry) para gestionar tablas de datos.")
			: TEXT("You have completed the Log system configuration. Summary:\n\n"
				"- CREATED: DA_LogDomain_Gameplay (domain with verbosity, format, and sinks)\n"
				"- EXPLORED: PGX Log Viewer (structured viewer with filters)\n"
				"- LEARNED: Polymorphic pipeline Ingress->Filter->Format->Sink\n"
				"- LEARNED: Blueprint logging with domains\n"
				"- LEARNED: pgx.log.* commands for live control\n\n"
				"Recommended next step: Tutorial S9 (Data Registry) to manage data tables."));
		S.AccentColor = PGX::Semantic::Good;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	return Steps;
}

// ============================================================================
// S9: Data Registry System (10 steps)
// ============================================================================
TArray<FPGXTutorialStep> PGXSystemTutorials::GetS9_DataRegistry(EPGXTutorialLanguage Lang)
{
	const bool bES = (Lang == EPGXTutorialLanguage::Spanish);
	TArray<FPGXTutorialStep> Steps;

	// Step 0: ConfigBasePath
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Configurar Ruta Base del Proyecto")
			: TEXT("Configure Project Base Path"));
		S.Description = FText::FromString(bES
			? TEXT("Define la carpeta donde se guardaran los assets del Data Registry. "
				"Ejemplo: /Game/Config/PGX/DataRegistry. Aqui van las definiciones de tablas de datos del juego.")
			: TEXT("Define the folder where Data Registry assets will be stored. "
				"Example: /Game/Config/PGX/DataRegistry. This holds game data table definitions."));
		S.AccentColor = PGX::System::DataRegistry;
		S.Action = EPGXTutorialAction::ConfigBasePath;
		Steps.Add(S);
	}

	// Step 1: Create PGXRegistryDefinition
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Crear Registry Definition")
			: TEXT("Create Registry Definition"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXRegistryDefinition define UNA tabla de datos compilada en el Data Registry. "
				"Funciona como un wrapper tipado sobre UDataTable con cache, validacion, y queries optimizadas.\n\n"
				"POR QUE EXISTE: Los DataTables de UE son potentes pero carecen de validacion, versionado, y queries tipadas. "
				"BENEFICIO: Accede a datos de gameplay con type safety, cache automatica, y estadisticas de acceso. "
				"EJEMPLO: RegistryDefinition 'Items' referencia DT_Items (DataTable) y expone queries como GetItemByTag().")
			: TEXT("UPGXRegistryDefinition defines ONE compiled data table in the Data Registry. "
				"It works as a typed wrapper over UDataTable with caching, validation, and optimized queries.\n\n"
				"WHY IT EXISTS: UE DataTables are powerful but lack validation, versioning, and typed queries. "
				"BENEFIT: Access gameplay data with type safety, automatic caching, and access statistics. "
				"EXAMPLE: RegistryDefinition 'Items' references DT_Items (DataTable) and exposes queries like GetItemByTag()."));
		S.AccentColor = PGX::System::DataRegistry;
		S.Action = EPGXTutorialAction::CreateAsset;
		S.ActionPath = TEXT("DataRegistry");
		S.AssetClass = TEXT("/Script/PGXCoreRuntime.PGXRegistryDefinition");
		S.AssetName = TEXT("DA_Registry_Items");
		Steps.Add(S);
	}

	// Step 2: Guide - Data Registry Browser
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXDataRegistryBrowser");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Explorar el Data Registry Browser")
			: TEXT("Explore the Data Registry Browser"));
		S.Description = FText::FromString(bES
			? TEXT("El Data Registry Browser muestra todas las definiciones registradas, sus DataTables vinculadas, y estadisticas de cache. "
				"Puedes navegar entradas, ver hit/miss ratios, y validar que las tablas estan correctamente compiladas.\n\n"
				"CUANDO USARLO: Para verificar que tus DataTables estan registradas y las queries funcionan correctamente.")
			: TEXT("The Data Registry Browser shows all registered definitions, their linked DataTables, and cache statistics. "
				"You can browse entries, see hit/miss ratios, and validate tables are correctly compiled.\n\n"
				"WHEN TO USE IT: To verify your DataTables are registered and queries work correctly."));
		S.AccentColor = PGX::System::DataRegistry;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 3: Guide - Config Dashboard
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXConfigDashboard");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Config Dashboard")
			: TEXT("Config Dashboard"));
		S.Description = FText::FromString(bES
			? TEXT("El Config Dashboard muestra una vista global de TODOS los config DAs de todos los sistemas PGX. "
				"Puedes ver de un vistazo que sistemas tienen config, cuales faltan, y navegar directamente al DA.\n\n"
				"CUANDO USARLO: Para tener una vista panoramica de toda la configuracion de tu proyecto PGX.")
			: TEXT("The Config Dashboard shows a global view of ALL config DAs from all PGX systems. "
				"You can see at a glance which systems have config, which are missing, and navigate directly to the DA.\n\n"
				"WHEN TO USE IT: For a panoramic view of all your PGX project configuration."));
		S.AccentColor = PGX::System::DataRegistry;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 4: Guide - Open Registry Definition
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Configurar Registry Definition")
			: TEXT("Configure Registry Definition"));
		S.Description = FText::FromString(bES
			? TEXT("Abre DA_Registry_Items y configura: RegistryName (identificador unico), DataTable (referencia al UDataTable), RowStruct (tipo de fila). "
				"bAutoCompile hace que el registro se compile al inicializar. CachingPolicy controla la estrategia de cache.\n\n"
				"El DataTable debe existir previamente — crealo desde el Content Browser de UE como normalmente harias.")
			: TEXT("Open DA_Registry_Items and configure: RegistryName (unique identifier), DataTable (reference to UDataTable), RowStruct (row type). "
				"bAutoCompile makes the registry compile on initialization. CachingPolicy controls the caching strategy.\n\n"
				"The DataTable must already exist — create it from UE's Content Browser as you normally would."));
		S.AccentColor = PGX::System::DataRegistry;
		S.Action = EPGXTutorialAction::OpenAsset;
		S.ActionPath = TEXT("DataRegistry");
		S.AssetName = TEXT("DA_Registry_Items");
		Steps.Add(S);
	}

	// Step 5: Guide - Query API
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("API de Consultas")
			: TEXT("Query API"));
		S.Description = FText::FromString(bES
			? TEXT("El subsistema expone queries tipadas: FindRow<T>(RegistryName, RowName), GetAllRows<T>(RegistryName), FindByTag(Tag). "
				"Las queries usan cache automatica — la primera consulta lee del DataTable, las siguientes del cache.\n\n"
				"GetRegistryStats(RegistryName) retorna hit ratio, miss count, y tiempo promedio de query. "
				"Desde Blueprint: GetRegistryRow, GetAllRegistryRows, IsRegistryCompiled.")
			: TEXT("The subsystem exposes typed queries: FindRow<T>(RegistryName, RowName), GetAllRows<T>(RegistryName), FindByTag(Tag). "
				"Queries use automatic caching — the first query reads from DataTable, subsequent ones from cache.\n\n"
				"GetRegistryStats(RegistryName) returns hit ratio, miss count, and average query time. "
				"From Blueprint: GetRegistryRow, GetAllRegistryRows, IsRegistryCompiled."));
		S.AccentColor = PGX::System::DataRegistry;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 6: Guide - Validation
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Validacion de Datos")
			: TEXT("Data Validation"));
		S.Description = FText::FromString(bES
			? TEXT("Data Registry valida los DataTables durante la compilacion: filas duplicadas, campos obligatorios vacios, tipos incompatibles. "
				"Los errores de validacion aparecen en el Log Viewer bajo el dominio 'DataRegistry'.\n\n"
				"Puedes agregar validadores custom via la interfaz IPGXRegistryValidator. "
				"EJEMPLO: Un validador custom verifica que todos los items tengan un precio mayor que 0.")
			: TEXT("Data Registry validates DataTables during compilation: duplicate rows, empty required fields, incompatible types. "
				"Validation errors appear in the Log Viewer under the 'DataRegistry' domain.\n\n"
				"You can add custom validators via the IPGXRegistryValidator interface. "
				"EXAMPLE: A custom validator verifies all items have a price greater than 0."));
		S.AccentColor = PGX::System::DataRegistry;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 7: Guide - Console commands
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Comandos de Consola de Data Registry")
			: TEXT("Data Registry Console Commands"));
		S.Description = FText::FromString(bES
			? TEXT("Comandos: 'pgx.registry.status' muestra registros compilados y sus estadisticas. "
				"'pgx.registry.compile <name>' fuerza recompilacion de un registro.\n\n"
				"'pgx.registry.cache clear' limpia la cache. "
				"'pgx.registry.stats' muestra hit/miss ratios globales.")
			: TEXT("Commands: 'pgx.registry.status' shows compiled registries and their statistics. "
				"'pgx.registry.compile <name>' forces recompilation of a registry.\n\n"
				"'pgx.registry.cache clear' clears the cache. "
				"'pgx.registry.stats' shows global hit/miss ratios."));
		S.AccentColor = PGX::System::DataRegistry;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 8: Guide - System Observer
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXSystemObserver");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Data Registry en el System Observer")
			: TEXT("Data Registry in the System Observer"));
		S.Description = FText::FromString(bES
			? TEXT("En el System Observer, Data Registry muestra registros compilados, entradas totales, y estado de cache. "
				"Verifica que todos tus registros aparezcan como compilados. Los no compilados no responderan a queries.")
			: TEXT("In the System Observer, Data Registry shows compiled registries, total entries, and cache state. "
				"Verify all your registries appear as compiled. Non-compiled ones won't respond to queries."));
		S.AccentColor = PGX::System::DataRegistry;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 9: Summary
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXHub");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Resumen: Data Registry Configurado")
			: TEXT("Summary: Data Registry Configured"));
		S.Description = FText::FromString(bES
			? TEXT("Has completado la configuracion del sistema Data Registry. Resumen:\n\n"
				"- CREADO: DA_Registry_Items (definicion de registro con DataTable vinculado)\n"
				"- EXPLORADO: Data Registry Browser (registros, cache, validacion)\n"
				"- EXPLORADO: Config Dashboard (vista panoramica de todos los configs)\n"
				"- APRENDIDO: Queries tipadas con cache automatica\n"
				"- APRENDIDO: Validacion de datos y validadores custom\n\n"
				"Siguiente paso recomendado: Tutorial S10 (Construction) para configurar clases custom.")
			: TEXT("You have completed the Data Registry system configuration. Summary:\n\n"
				"- CREATED: DA_Registry_Items (registry definition with linked DataTable)\n"
				"- EXPLORED: Data Registry Browser (registries, cache, validation)\n"
				"- EXPLORED: Config Dashboard (panoramic view of all configs)\n"
				"- LEARNED: Typed queries with automatic caching\n"
				"- LEARNED: Data validation and custom validators\n\n"
				"Recommended next step: Tutorial S10 (Construction) to configure custom classes."));
		S.AccentColor = PGX::Semantic::Good;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	return Steps;
}

// ============================================================================
// S10: Construction System (9 steps)
// ============================================================================
TArray<FPGXTutorialStep> PGXSystemTutorials::GetS10_Construction(EPGXTutorialLanguage Lang)
{
	const bool bES = (Lang == EPGXTutorialLanguage::Spanish);
	TArray<FPGXTutorialStep> Steps;

	// Step 0: ConfigBasePath
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Configurar Ruta Base del Proyecto")
			: TEXT("Configure Project Base Path"));
		S.Description = FText::FromString(bES
			? TEXT("El sistema Construction no usa un Config DA — su configuracion vive en los Project Settings de UE. "
				"Primero define la carpeta raiz donde el tutorial creara assets de demostracion (Construction DAs, clases custom).\n\n"
				"Ejemplo: /Game/Config/PGX/Construction. PGX dispone de 58 factories listas para usar desde el Content Browser.")
			: TEXT("The Construction system does not use a Config DA — its configuration lives in UE Project Settings. "
				"First define the root folder where this tutorial will create demonstration assets (Construction DAs, custom classes).\n\n"
				"Example: /Game/Config/PGX/Construction. PGX provides 58 factories ready to use from the Content Browser."));
		S.AccentColor = PGX::System::Construction;
		S.Action = EPGXTutorialAction::ConfigBasePath;
		Steps.Add(S);
	}

	// Step 1: CreateFolder
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Crear Carpeta de Construction")
			: TEXT("Create Construction Folder"));
		S.Description = FText::FromString(bES
			? TEXT("Crea la subcarpeta 'Construction' dentro de tu ruta base. "
				"Aqui organizaras los DataAssets que definen tus tipos de objetos construibles.\n\n"
				"POR QUE IMPORTA: Una estructura de carpetas limpia hace que el Content Browser sea navegable y que AssetRegistry encuentre tus DAs automaticamente.")
			: TEXT("Create the 'Construction' subfolder inside your base path. "
				"Here you will organize the DataAssets that define your constructible object types.\n\n"
				"WHY IT MATTERS: A clean folder structure makes the Content Browser navigable and allows AssetRegistry to auto-discover your DAs."));
		S.AccentColor = PGX::System::Construction;
		S.Action = EPGXTutorialAction::CreateFolder;
		S.ActionPath = TEXT("Construction");
		Steps.Add(S);
	}

	// Step 2: NavigateCB
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Explorar el Content Browser")
			: TEXT("Explore the Content Browser"));
		S.Description = FText::FromString(bES
			? TEXT("Navega al Content Browser para ver la carpeta Construction recien creada. "
				"Desde aqui puedes usar click derecho -> PGX -> Construction para acceder a las 58 factories de Construction.\n\n"
				"Las factories estan organizadas por categoria: Spawners, Props, Volumes, Triggers, etc.")
			: TEXT("Navigate to the Content Browser to see the newly created Construction folder. "
				"From here you can right-click -> PGX -> Construction to access the 58 Construction factories.\n\n"
				"Factories are organized by category: Spawners, Props, Volumes, Triggers, etc."));
		S.AccentColor = PGX::System::Construction;
		S.Action = EPGXTutorialAction::NavigateCB;
		S.ActionPath = TEXT("Construction");
		Steps.Add(S);
	}

	// Step 3: Guide — Construction DA Pattern
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("El Patron Construction DA")
			: TEXT("The Construction DA Pattern"));
		S.Description = FText::FromString(bES
			? TEXT("Cada Construction DA es una 'caja' con 4 campos: la clase C++ o Blueprint a instanciar, parametros de spawn, condiciones de activacion, y referencias a otros DAs. "
				"El dev NO necesita saber como funciona el sistema internamente — solo rellena los campos en el Details panel y el framework hace el resto.\n\n"
				"BENEFICIO: Un disenador puede crear objetos construibles sin escribir una sola linea de codigo. "
				"EJEMPLO: Un DA de tipo 'TorreDefensiva' define TorreCPP como clase, rango=500, cooldown=3s, precio=100 monedas.")
			: TEXT("Each Construction DA is a 'box' with 4 fields: the C++ or Blueprint class to instantiate, spawn parameters, activation conditions, and references to other DAs. "
				"The dev does NOT need to know how the system works internally — just fill in the fields in the Details panel and the framework does the rest.\n\n"
				"BENEFIT: A designer can create constructible objects without writing a single line of code. "
				"EXAMPLE: A 'DefenseTower' DA defines TowerCPP as the class, range=500, cooldown=3s, price=100 coins."));
		S.AccentColor = PGX::System::Construction;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 4: Guide — EPGXClassSourceMode
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("EPGXClassSourceMode: 3 Modos de Clase")
			: TEXT("EPGXClassSourceMode: 3 Class Modes"));
		S.Description = FText::FromString(bES
			? TEXT("Cada slot de clase en un Construction DA tiene 3 modos seleccionables via dropdown: "
				"Default (usa la clase base de PGX, cero configuracion), CppClass (apunta a una clase C++ custom heredada de la base PGX),\n\n"
				"Blueprint (activa un checkbox y un picker de BP class para devs que trabajan solo en Blueprint). "
				"FILOSOFIA: El mismo DA funciona para un dev C++ y para un disenador BP — el modo cambia la interfaz, no el comportamiento. "
				"GARANTIA: El sistema usa ShouldCreateSubsystem() internamente para asegurar que nunca haya instancias duplicadas.")
			: TEXT("Each class slot in a Construction DA has 3 selectable modes via dropdown: "
				"Default (uses the PGX base class, zero configuration), CppClass (points to a custom C++ class inherited from the PGX base),\n\n"
				"Blueprint (enables a checkbox and a BP class picker for devs working only in Blueprint). "
				"PHILOSOPHY: The same DA works for a C++ dev and a BP designer — the mode changes the interface, not the behavior. "
				"GUARANTEE: The system uses ShouldCreateSubsystem() internally to ensure there are never duplicate instances."));
		S.AccentColor = PGX::System::Construction;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 5: Guide — Config Dashboard
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXConfigDashboard");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Construction en el Config Dashboard")
			: TEXT("Construction in the Config Dashboard"));
		S.Description = FText::FromString(bES
			? TEXT("El Config Dashboard muestra todos los Construction DAs descubiertos por AssetRegistry en tu proyecto. "
				"Puedes ver cuantos DAs estan configurados, cuales tienen modo Default/CppClass/Blueprint, y navegar directamente a cada uno.\n\n"
				"CUANDO USARLO: Para tener una vista centralizada de todos los objetos construibles de tu juego sin tener que navegar el Content Browser manualmente.")
			: TEXT("The Config Dashboard shows all Construction DAs discovered by AssetRegistry in your project. "
				"You can see how many DAs are configured, which ones are in Default/CppClass/Blueprint mode, and navigate directly to each one.\n\n"
				"WHEN TO USE IT: For a centralized view of all constructible objects in your game without manually browsing the Content Browser."));
		S.AccentColor = PGX::System::Construction;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 6: Guide — "La caja y la forma" philosophy
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Filosofia: La Caja y la Forma")
			: TEXT("Philosophy: The Box and the Shape"));
		S.Description = FText::FromString(bES
			? TEXT("'La caja y las formas del colegio' — el dev no piensa en abstraccion. Solo mete la pieza en el hueco. "
				"En Construction: el DA es la caja (siempre el mismo contenedor) y tu clase es la forma (la pieza que encaja en el slot).\n\n"
				"No hay decision de arquitectura — hay UN slot para la clase y UN dropdown para el modo. "
				"RESULTADO: Un dev que nunca ha usado PGX puede crear su primer objeto construible en menos de 2 minutos, sin leer documentacion.")
			: TEXT("'The box and the school shapes' — the dev doesn't think about abstraction. They just put the piece in the slot. "
				"In Construction: the DA is the box (always the same container) and your class is the shape (the piece that fits in the slot).\n\n"
				"There is no architecture decision — there is ONE slot for the class and ONE dropdown for the mode. "
				"RESULT: A dev who has never used PGX can create their first constructible object in under 2 minutes, without reading documentation."));
		S.AccentColor = PGX::System::Construction;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 7: Guide — System Observer
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXSystemObserver");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Construction en el System Observer")
			: TEXT("Construction in the System Observer"));
		S.Description = FText::FromString(bES
			? TEXT("El System Observer muestra los Construction DAs descubiertos, el numero de clases registradas, y el modo de cada slot. "
				"Construction es un sistema editor-only — no tiene subsistema en runtime, sino que opera a traves del pipeline de factories y AssetRegistry.\n\n"
				"VERIFICA: Que tus DAs aparezcan con el modo correcto y que las clases C++ o BP referenciadas sean validas.")
			: TEXT("The System Observer shows discovered Construction DAs, the number of registered classes, and the mode of each slot. "
				"Construction is an editor-only system — it has no runtime subsystem, but operates through the factories and AssetRegistry pipeline.\n\n"
				"VERIFY: That your DAs appear with the correct mode and that the referenced C++ or BP classes are valid."));
		S.AccentColor = PGX::System::Construction;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 8: Summary
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXHub");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Resumen: Construction Configurado")
			: TEXT("Summary: Construction Configured"));
		S.Description = FText::FromString(bES
			? TEXT("Has completado el tutorial del sistema Construction. Resumen:\n\n"
				"- EXPLORADO: 58 factories via Content Browser -> PGX -> Construction\n"
				"- APRENDIDO: Patron Construction DA (4 campos: clase, spawn, condiciones, refs)\n"
				"- APRENDIDO: EPGXClassSourceMode (Default / CppClass / Blueprint)\n"
				"- EXPLORADO: Config Dashboard (vista global de todos los Construction DAs)\n"
				"- COMPRENDIDO: Filosofia 'la caja y la forma' — zero decision de arquitectura\n"
				"- NOTA: Construction no tiene Config DA propio — usa Project Settings\n\n"
				"Siguiente paso recomendado: Tutorial S11 (Message) para comunicacion entre sistemas sin dependencias.")
			: TEXT("You have completed the Construction system tutorial. Summary:\n\n"
				"- EXPLORED: 58 factories via Content Browser -> PGX -> Construction\n"
				"- LEARNED: Construction DA pattern (4 fields: class, spawn, conditions, refs)\n"
				"- LEARNED: EPGXClassSourceMode (Default / CppClass / Blueprint)\n"
				"- EXPLORED: Config Dashboard (global view of all Construction DAs)\n"
				"- UNDERSTOOD: 'Box and shape' philosophy — zero architecture decisions\n"
				"- NOTE: Construction has no Config DA — it uses Project Settings\n\n"
				"Recommended next step: Tutorial S11 (Message) for cross-system communication without dependencies."));
		S.AccentColor = PGX::Semantic::Good;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	return Steps;
}

// ============================================================================
// S11: Message System (10 steps)
// ============================================================================
TArray<FPGXTutorialStep> PGXSystemTutorials::GetS11_Message(EPGXTutorialLanguage Lang)
{
	const bool bES = (Lang == EPGXTutorialLanguage::Spanish);
	TArray<FPGXTutorialStep> Steps;

	// Step 0: ConfigBasePath
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Configurar Ruta Base del Proyecto")
			: TEXT("Configure Project Base Path"));
		S.Description = FText::FromString(bES
			? TEXT("Define la carpeta raiz donde PGX guardara la configuracion del sistema Message. "
				"Ejemplo: /Game/Config/PGX/Message. El sistema Message v1.0 es un bus pub/sub tipado que permite comunicacion entre sistemas sin dependencias directas.")
			: TEXT("Define the root folder where PGX will store the Message system configuration. "
				"Example: /Game/Config/PGX/Message. The Message v1.0 system is a typed pub/sub bus that enables cross-system communication without direct dependencies."));
		S.AccentColor = PGX::System::Message;
		S.Action = EPGXTutorialAction::ConfigBasePath;
		Steps.Add(S);
	}

	// Step 1: CreateFolder
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Crear Carpeta de Message")
			: TEXT("Create Message Folder"));
		S.Description = FText::FromString(bES
			? TEXT("Crea la subcarpeta 'Message' dentro de tu ruta base. "
				"Aqui organizaras el config DA del sistema Message y cualquier definicion de canal personalizado.\n\n"
				"Una estructura ordenada permite que AssetRegistry descubra el DA automaticamente al inicializar el subsistema.")
			: TEXT("Create the 'Message' subfolder inside your base path. "
				"Here you will organize the Message system config DA and any custom channel definitions.\n\n"
				"An ordered structure allows AssetRegistry to auto-discover the DA when the subsystem initializes."));
		S.AccentColor = PGX::System::Message;
		S.Action = EPGXTutorialAction::CreateFolder;
		S.ActionPath = TEXT("Message");
		Steps.Add(S);
	}

	// Step 2: CreateAsset
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Crear Message Config")
			: TEXT("Create Message Config"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXMessageConfig es el DataAsset de configuracion del bus de mensajes. "
				"Define politicas del bus: capacidad maxima de la cola, modo de entrega (fire-and-forget o confirmado), y canales pre-registrados.\n\n"
				"POR QUE EXISTE: El Message system necesita saber los canales activos y sus politicas antes de recibir broadcasts. "
				"BENEFICIO: Configura el bus una vez y todos los sistemas del proyecto lo usan sin acoplamiento directo entre ellos. "
				"EJEMPLO: Canal 'PGX.Combat.Events' con capacidad 256, delivery inmediato; canal 'PGX.UI.Updates' con batching a 60Hz.")
			: TEXT("UPGXMessageConfig is the configuration DataAsset for the message bus. "
				"It defines bus policies: maximum queue capacity, delivery mode (fire-and-forget or confirmed), and pre-registered channels.\n\n"
				"WHY IT EXISTS: The Message system needs to know active channels and their policies before receiving broadcasts. "
				"BENEFIT: Configure the bus once and all project systems use it without direct coupling between them. "
				"EXAMPLE: Channel 'PGX.Combat.Events' with capacity 256, immediate delivery; channel 'PGX.UI.Updates' with 60Hz batching."));
		S.AccentColor = PGX::System::Message;
		S.Action = EPGXTutorialAction::CreateAsset;
		S.ActionPath = TEXT("Message");
		S.AssetClass = TEXT("/Script/PGXCoreRuntime.PGXMessageConfig");
		S.AssetName = TEXT("DA_MessageConfig");
		Steps.Add(S);
	}

	// Step 3: Guide — Message Inspector
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXMessageInspector");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Explorar el Message Inspector")
			: TEXT("Explore the Message Inspector"));
		S.Description = FText::FromString(bES
			? TEXT("El Message Inspector muestra en vivo: canales activos con sus suscriptores, estadisticas de entrega (broadcasts/segundo, mensajes encolados), y el historial de mensajes recientes. "
				"Puedes filtrar por canal o por tipo de mensaje para rastrear flujos especificos.\n\n"
				"CUANDO USARLO: Para debuggear por que un suscriptor no recibe mensajes, o para medir el throughput del bus bajo carga.")
			: TEXT("The Message Inspector shows live: active channels with their subscribers, delivery statistics (broadcasts/second, queued messages), and recent message history. "
				"You can filter by channel or message type to track specific flows.\n\n"
				"WHEN TO USE IT: To debug why a subscriber isn't receiving messages, or to measure bus throughput under load."));
		S.AccentColor = PGX::System::Message;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 4: Guide — Pub/Sub Architecture
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Arquitectura Pub/Sub")
			: TEXT("Pub/Sub Architecture"));
		S.Description = FText::FromString(bES
			? TEXT("El Message system implementa el patron publisher/subscriber: un emisor hace Broadcast(Tag, Payload) sin saber quienes escuchan. "
				"Los suscriptores llaman a Subscribe(Tag, Callback) y reciben todos los mensajes de ese canal automaticamente.\n\n"
				"CLAVE: El emisor y el receptor NUNCA se referencian mutuamente — el bus actua como intermediario. "
				"RESULTADO: Puedes agregar o eliminar suscriptores sin modificar el emisor, y viceversa — acoplamiento cero en runtime. "
				"Delegates expuestos: OnMessageReceived(Channel, Payload), OnChannelCreated(Channel).")
			: TEXT("The Message system implements the publisher/subscriber pattern: a sender calls Broadcast(Tag, Payload) without knowing who listens. "
				"Subscribers call Subscribe(Tag, Callback) and receive all messages on that channel automatically.\n\n"
				"KEY: The sender and receiver NEVER reference each other — the bus acts as intermediary. "
				"RESULT: You can add or remove subscribers without modifying the sender, and vice versa — zero runtime coupling. "
				"Exposed delegates: OnMessageReceived(Channel, Payload), OnChannelCreated(Channel)."));
		S.AccentColor = PGX::System::Message;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 5: Guide — Tag-Based Channels
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Canales Basados en Tags")
			: TEXT("Tag-Based Channels"));
		S.Description = FText::FromString(bES
			? TEXT("Los canales del Message system se identifican con GameplayTags bajo la rama PGX.Message.*. "
				"Esto permite jerarquia de canales: suscribirse a 'PGX.Message.Combat' recibe todo lo de 'PGX.Message.Combat.Damage' y 'PGX.Message.Combat.Heal' automaticamente.\n\n"
				"BENEFICIO: No hay strings magicos ni IDs numericos — los canales son refactorizables, buscables, y validados por el engine. "
				"EJEMPLO: Subscribe('PGX.Message.Player') recibe eventos de dano, curacion, XP, muerte — todos sub-canales de Player.")
			: TEXT("Message system channels are identified with GameplayTags under the PGX.Message.* branch. "
				"This allows channel hierarchy: subscribing to 'PGX.Message.Combat' receives everything from 'PGX.Message.Combat.Damage' and 'PGX.Message.Combat.Heal' automatically.\n\n"
				"BENEFIT: No magic strings or numeric IDs — channels are refactorable, searchable, and validated by the engine. "
				"EXAMPLE: Subscribe('PGX.Message.Player') receives damage, healing, XP, death events — all sub-channels of Player."));
		S.AccentColor = PGX::System::Message;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 6: Open DA
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Abrir y Configurar el Message Config")
			: TEXT("Open and Configure the Message Config"));
		S.Description = FText::FromString(bES
			? TEXT("Abre DA_MessageConfig. Propiedades clave: MaxQueueCapacity (mensajes maximos en cola antes de descartar), DefaultDeliveryMode (Immediate/Batched), PreregisteredChannels (canales que existen desde el arranque). "
				"Para cada canal pre-registrado puedes definir su capacidad individual y modo de entrega.\n\n"
				"RECOMENDACION: Pre-registra solo los canales de alto trafico (Combat, UI) — el resto se crean dinamicamente al primer broadcast.")
			: TEXT("Open DA_MessageConfig. Key properties: MaxQueueCapacity (max messages in queue before discarding), DefaultDeliveryMode (Immediate/Batched), PreregisteredChannels (channels that exist from startup). "
				"For each pre-registered channel you can define its individual capacity and delivery mode.\n\n"
				"RECOMMENDATION: Pre-register only high-traffic channels (Combat, UI) — the rest are created dynamically on first broadcast."));
		S.AccentColor = PGX::System::Message;
		S.Action = EPGXTutorialAction::OpenAsset;
		S.ActionPath = TEXT("Message");
		S.AssetName = TEXT("DA_MessageConfig");
		Steps.Add(S);
	}

	// Step 7: Guide — Console Commands
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Comandos de Consola de Message")
			: TEXT("Message Console Commands"));
		S.Description = FText::FromString(bES
			? TEXT("Comandos disponibles: 'pgx.message.status' muestra canales activos, suscriptores totales, y mensajes procesados. "
				"'pgx.message.broadcast <Tag> <Payload>' envia un mensaje de prueba a un canal desde la consola.\n\n"
				"'pgx.message.subscribe <Tag>' activa logging de todos los mensajes de un canal (debug). "
				"Utiles para verificar que los broadcasts llegan y que los suscriptores responden correctamente.")
			: TEXT("Available commands: 'pgx.message.status' shows active channels, total subscribers, and processed messages. "
				"'pgx.message.broadcast <Tag> <Payload>' sends a test message to a channel from the console.\n\n"
				"'pgx.message.subscribe <Tag>' activates logging of all messages on a channel (debug). "
				"Useful for verifying broadcasts arrive and subscribers respond correctly."));
		S.AccentColor = PGX::System::Message;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 8: Guide — System Observer
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXSystemObserver");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Message en el System Observer")
			: TEXT("Message in the System Observer"));
		S.Description = FText::FromString(bES
			? TEXT("En el System Observer, Message muestra: estado del subsistema (Initialized/No Config), canales activos, mensajes totales broadcast, y estadisticas de entrega. "
				"Verifica que aparezca como 'Initialized' y que los canales pre-registrados en el DA esten listados.\n\n"
				"Si el contador de mensajes permanece en 0 durante el juego, verifica que los emisores esten usando el Tag correcto.")
			: TEXT("In the System Observer, Message shows: subsystem state (Initialized/No Config), active channels, total broadcast messages, and delivery statistics. "
				"Verify it appears as 'Initialized' and pre-registered channels from the DA are listed.\n\n"
				"If the message counter stays at 0 during play, verify senders are using the correct Tag."));
		S.AccentColor = PGX::System::Message;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 9: Summary
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXHub");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Resumen: Message Configurado")
			: TEXT("Summary: Message Configured"));
		S.Description = FText::FromString(bES
			? TEXT("Has completado la configuracion del sistema Message. Resumen:\n\n"
				"- CREADO: DA_MessageConfig (politicas del bus, capacidad, canales pre-registrados)\n"
				"- EXPLORADO: Message Inspector (canales, suscriptores, estadisticas en vivo)\n"
				"- APRENDIDO: Arquitectura pub/sub — emisor y receptor sin referencia mutua\n"
				"- APRENDIDO: Canales jerarquicos basados en GameplayTags (PGX.Message.*)\n"
				"- APRENDIDO: Comandos pgx.message.* para testing y debug\n"
				"- DELEGATES: OnMessageReceived, OnChannelCreated\n\n"
				"Siguiente paso recomendado: Tutorial S12 (EventHandler) para resolucion de comportamiento con prioridades.")
			: TEXT("You have completed the Message system configuration. Summary:\n\n"
				"- CREATED: DA_MessageConfig (bus policies, capacity, pre-registered channels)\n"
				"- EXPLORED: Message Inspector (channels, subscribers, live statistics)\n"
				"- LEARNED: Pub/sub architecture — sender and receiver without mutual reference\n"
				"- LEARNED: Hierarchical channels based on GameplayTags (PGX.Message.*)\n"
				"- LEARNED: pgx.message.* commands for testing and debug\n"
				"- DELEGATES: OnMessageReceived, OnChannelCreated\n\n"
				"Recommended next step: Tutorial S12 (EventHandler) for priority-based behavior resolution."));
		S.AccentColor = PGX::Semantic::Good;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	return Steps;
}

// ============================================================================
// S12: EventHandler System (10 steps)
// ============================================================================
TArray<FPGXTutorialStep> PGXSystemTutorials::GetS12_EventHandler(EPGXTutorialLanguage Lang)
{
	const bool bES = (Lang == EPGXTutorialLanguage::Spanish);
	TArray<FPGXTutorialStep> Steps;

	// Step 0: ConfigBasePath
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Configurar Ruta Base del Proyecto")
			: TEXT("Configure Project Base Path"));
		S.Description = FText::FromString(bES
			? TEXT("Define la carpeta raiz donde PGX guardara la configuracion del sistema EventHandler. "
				"Ejemplo: /Game/Config/PGX/EventHandler. EventHandler v1.0 es un bus de resolucion de comportamiento: a diferencia de Message (fire-and-forget), EventHandler resuelve QUIEN maneja un evento usando prioridades y cadenas de handlers.")
			: TEXT("Define the root folder where PGX will store the EventHandler system configuration. "
				"Example: /Game/Config/PGX/EventHandler. EventHandler v1.0 is a behavior resolution bus: unlike Message (fire-and-forget), EventHandler resolves WHO handles an event using priorities and handler chains."));
		S.AccentColor = PGX::System::EventHandler;
		S.Action = EPGXTutorialAction::ConfigBasePath;
		Steps.Add(S);
	}

	// Step 1: CreateFolder
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Crear Carpeta de EventHandler")
			: TEXT("Create EventHandler Folder"));
		S.Description = FText::FromString(bES
			? TEXT("Crea la subcarpeta 'EventHandler' dentro de tu ruta base. "
				"Aqui viviran el Config DA del sistema y los DataAssets de definicion de handlers.\n\n"
				"AssetRegistry descubrira automaticamente estos assets al inicializar UPGXEventHandlerSubsystem.")
			: TEXT("Create the 'EventHandler' subfolder inside your base path. "
				"Here will live the system Config DA and handler definition DataAssets.\n\n"
				"AssetRegistry will automatically discover these assets when UPGXEventHandlerSubsystem initializes."));
		S.AccentColor = PGX::System::EventHandler;
		S.Action = EPGXTutorialAction::CreateFolder;
		S.ActionPath = TEXT("EventHandler");
		Steps.Add(S);
	}

	// Step 2: CreateAsset
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Crear EventHandler Config")
			: TEXT("Create EventHandler Config"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXEventHandlerConfig configura el motor de resolucion de comportamiento. "
				"Define: MaxHandlersPerEvent (limite de handlers en cadena), DefaultResolutionMode (First/All/Priority), RecordingMode (blackbox activo/inactivo).\n\n"
				"POR QUE EXISTE: EventHandler necesita saber como resolver conflictos cuando multiples handlers compiten por el mismo evento. "
				"BENEFICIO: Sistema data-driven de IA de comportamiento — los disenadores pueden cambiar quien maneja un evento sin recompilar. "
				"EJEMPLO: Evento 'InteractRequest' manejado por CombatHandler (prioridad 10) antes que ExplorationHandler (prioridad 5) cuando el jugador esta en combate.")
			: TEXT("UPGXEventHandlerConfig configures the behavior resolution engine. "
				"Defines: MaxHandlersPerEvent (handler chain limit), DefaultResolutionMode (First/All/Priority), RecordingMode (blackbox active/inactive).\n\n"
				"WHY IT EXISTS: EventHandler needs to know how to resolve conflicts when multiple handlers compete for the same event. "
				"BENEFIT: Data-driven behavior AI system — designers can change who handles an event without recompiling. "
				"EXAMPLE: Event 'InteractRequest' handled by CombatHandler (priority 10) before ExplorationHandler (priority 5) when the player is in combat."));
		S.AccentColor = PGX::System::EventHandler;
		S.Action = EPGXTutorialAction::CreateAsset;
		S.ActionPath = TEXT("EventHandler");
		S.AssetClass = TEXT("/Script/PGXCoreRuntime.PGXEventHandlerConfig");
		S.AssetName = TEXT("DA_EventHandlerConfig");
		Steps.Add(S);
	}

	// Step 3: Guide — Event Debugger
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXEventDebugger");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Explorar el Event Debugger")
			: TEXT("Explore the Event Debugger"));
		S.Description = FText::FromString(bES
			? TEXT("El Event Debugger muestra en vivo: eventos despachados, la cadena de handlers evaluados por cada evento, el resultado de resolucion, y las metricas de telemetria. "
				"Incluye una vista de 'execution waterfall' que muestra cada handler en la cadena con su tiempo de evaluacion y si fue aceptado o rechazado.\n\n"
				"CUANDO USARLO: Para debuggear por que un evento no fue manejado por el handler esperado, o para identificar handlers lentos.")
			: TEXT("The Event Debugger shows live: dispatched events, the handler chain evaluated for each event, resolution result, and telemetry metrics. "
				"It includes an 'execution waterfall' view showing each handler in the chain with its evaluation time and whether it was accepted or rejected.\n\n"
				"WHEN TO USE IT: To debug why an event wasn't handled by the expected handler, or to identify slow handlers."));
		S.AccentColor = PGX::System::EventHandler;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 4: Guide — Behavior Resolution
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Resolucion de Comportamiento")
			: TEXT("Behavior Resolution"));
		S.Description = FText::FromString(bES
			? TEXT("Cuando se despacha un evento, EventHandler evalua los handlers registrados para ese tag en orden de prioridad. "
				"Cada handler retorna: Handled (resolucion completa, cadena se detiene), Unhandled (pasa al siguiente), o Deferred (acepta pero no bloquea la cadena).\n\n"
				"El blackbox registra cada ejecucion para analisis post-mortem — puedes reproducir secuencias de eventos en el Debugger. "
				"DIFERENCIA CON MESSAGE: Message es broadcast (todos reciben). EventHandler es resolution (uno maneja, o ninguno). "
				"Delegates: OnEventDispatched(Tag), OnHandlerRegistered(Tag, Handler), OnResolutionCompleted(Tag, Result).")
			: TEXT("When an event is dispatched, EventHandler evaluates handlers registered for that tag in priority order. "
				"Each handler returns: Handled (complete resolution, chain stops), Unhandled (passes to next), or Deferred (accepts but doesn't block chain).\n\n"
				"The blackbox records each execution for post-mortem analysis — you can replay event sequences in the Debugger. "
				"DIFFERENCE FROM MESSAGE: Message is broadcast (all receive). EventHandler is resolution (one handles, or none). "
				"Delegates: OnEventDispatched(Tag), OnHandlerRegistered(Tag, Handler), OnResolutionCompleted(Tag, Result)."));
		S.AccentColor = PGX::System::EventHandler;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 5: Guide — Priority System
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Sistema de Prioridades")
			: TEXT("Priority System"));
		S.Description = FText::FromString(bES
			? TEXT("Cada handler se registra con una prioridad numerica (mayor numero = mayor prioridad, evaluado primero). "
				"Prioridades recomendadas: Sistema critico=1000, Gameplay principal=500, Modifiers=200, Fallback=0.\n\n"
				"En modo Priority, se evalua TODA la cadena y el resultado del handler de mayor prioridad que retorna Handled gana. "
				"CASO DE USO: Enemigo bloqueado retorna Handled para 'AttackRequest' antes que el handler normal — sin ifs en codigo, solo configuracion.")
			: TEXT("Each handler registers with a numeric priority (higher number = higher priority, evaluated first). "
				"Recommended priorities: Critical system=1000, Main gameplay=500, Modifiers=200, Fallback=0.\n\n"
				"In Priority mode, the ENTIRE chain is evaluated and the result of the highest-priority handler returning Handled wins. "
				"USE CASE: A blocked enemy returns Handled for 'AttackRequest' before the normal handler — no ifs in code, just configuration."));
		S.AccentColor = PGX::System::EventHandler;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 6: Open DA
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Abrir y Configurar el EventHandler Config")
			: TEXT("Open and Configure the EventHandler Config"));
		S.Description = FText::FromString(bES
			? TEXT("Abre DA_EventHandlerConfig. Propiedades clave: DefaultResolutionMode (First=para con primer Handled, All=evalua todos, Priority=gana el mas prioritario). "
				"bEnableBlackbox activa la grabacion de ejecuciones para replay en el Debugger.\n\n"
				"BlackboxMaxEntries controla cuantas ejecuciones se retienen en memoria. "
				"TelemetryFlushInterval controla cada cuanto se vuelcan las estadisticas al Log.")
			: TEXT("Open DA_EventHandlerConfig. Key properties: DefaultResolutionMode (First=stop on first Handled, All=evaluate all, Priority=highest priority wins). "
				"bEnableBlackbox activates execution recording for replay in the Debugger.\n\n"
				"BlackboxMaxEntries controls how many executions are retained in memory. "
				"TelemetryFlushInterval controls how often statistics are flushed to the Log."));
		S.AccentColor = PGX::System::EventHandler;
		S.Action = EPGXTutorialAction::OpenAsset;
		S.ActionPath = TEXT("EventHandler");
		S.AssetName = TEXT("DA_EventHandlerConfig");
		Steps.Add(S);
	}

	// Step 7: Guide — Console Commands
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Comandos de Consola de EventHandler")
			: TEXT("EventHandler Console Commands"));
		S.Description = FText::FromString(bES
			? TEXT("Comandos disponibles: 'pgx.events.status' muestra eventos registrados, handlers activos, y metricas globales. "
				"'pgx.events.dispatch <Tag>' despacha un evento de prueba desde la consola (util para testear handlers sin gameplay code).\n\n"
				"'pgx.events.handlers <Tag>' lista todos los handlers registrados para un evento con sus prioridades y estados. "
				"Esenciales para verificar que el sistema de resolucion esta configurado correctamente antes de entrar en PIE.")
			: TEXT("Available commands: 'pgx.events.status' shows registered events, active handlers, and global metrics. "
				"'pgx.events.dispatch <Tag>' dispatches a test event from the console (useful for testing handlers without gameplay code).\n\n"
				"'pgx.events.handlers <Tag>' lists all handlers registered for an event with their priorities and states. "
				"Essential for verifying the resolution system is correctly configured before entering PIE."));
		S.AccentColor = PGX::System::EventHandler;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 8: Guide — System Observer
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXSystemObserver");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("EventHandler en el System Observer")
			: TEXT("EventHandler in the System Observer"));
		S.Description = FText::FromString(bES
			? TEXT("En el System Observer, EventHandler muestra: estado del subsistema, eventos registrados, handlers activos, ejecuciones totales, y estado del blackbox. "
				"Verifica que aparezca como 'Initialized'. Si el contador de ejecuciones es 0 durante PIE, verifica que los handlers se esten registrando en BeginPlay, no en el Constructor.")
			: TEXT("In the System Observer, EventHandler shows: subsystem state, registered events, active handlers, total executions, and blackbox state. "
				"Verify it appears as 'Initialized'. If the execution counter is 0 during PIE, verify handlers are registering in BeginPlay, not in the Constructor."));
		S.AccentColor = PGX::System::EventHandler;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 9: Summary
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXHub");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Resumen: EventHandler Configurado")
			: TEXT("Summary: EventHandler Configured"));
		S.Description = FText::FromString(bES
			? TEXT("Has completado la configuracion del sistema EventHandler. Resumen:\n\n"
				"- CREADO: DA_EventHandlerConfig (modo de resolucion, blackbox, telemetria)\n"
				"- EXPLORADO: Event Debugger (waterfall de ejecucion, replay de eventos)\n"
				"- APRENDIDO: Resolucion de comportamiento (Handled/Unhandled/Deferred)\n"
				"- APRENDIDO: Sistema de prioridades numericas para cadenas de handlers\n"
				"- APRENDIDO: Comandos pgx.events.* para testing sin gameplay code\n"
				"- DELEGATES: OnEventDispatched, OnHandlerRegistered, OnResolutionCompleted\n"
				"- DIFERENCIA: Message=broadcast (todos reciben). EventHandler=resolution (uno maneja).\n\n"
				"Siguiente paso recomendado: Tutorial S13 (MGOS) para observabilidad de GC y memoria.")
			: TEXT("You have completed the EventHandler system configuration. Summary:\n\n"
				"- CREATED: DA_EventHandlerConfig (resolution mode, blackbox, telemetry)\n"
				"- EXPLORED: Event Debugger (execution waterfall, event replay)\n"
				"- LEARNED: Behavior resolution (Handled/Unhandled/Deferred)\n"
				"- LEARNED: Numeric priority system for handler chains\n"
				"- LEARNED: pgx.events.* commands for testing without gameplay code\n"
				"- DELEGATES: OnEventDispatched, OnHandlerRegistered, OnResolutionCompleted\n"
				"- DIFFERENCE: Message=broadcast (all receive). EventHandler=resolution (one handles).\n\n"
				"Recommended next step: Tutorial S13 (MGOS) for GC and memory observability."));
		S.AccentColor = PGX::Semantic::Good;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	return Steps;
}

// ============================================================================
// S13: MGOS System (10 steps)
// ============================================================================
TArray<FPGXTutorialStep> PGXSystemTutorials::GetS13_MGOS(EPGXTutorialLanguage Lang)
{
	const bool bES = (Lang == EPGXTutorialLanguage::Spanish);
	TArray<FPGXTutorialStep> Steps;

	// Step 0: ConfigBasePath
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Configurar Ruta Base del Proyecto")
			: TEXT("Configure Project Base Path"));
		S.Description = FText::FromString(bES
			? TEXT("Define la carpeta raiz donde PGX guardara la configuracion del sistema MGOS. "
				"Ejemplo: /Game/Config/PGX/MGOS. MGOS v1.0 (Memory and GC Observation System) es UNICO en PGX: es un UEngineSubsystem — vive durante toda la vida del engine, no solo durante una sesion de GameInstance.")
			: TEXT("Define the root folder where PGX will store the MGOS system configuration. "
				"Example: /Game/Config/PGX/MGOS. MGOS v1.0 (Memory and GC Observation System) is UNIQUE in PGX: it is a UEngineSubsystem — it lives for the entire engine lifetime, not just during a GameInstance session."));
		S.AccentColor = PGX::System::MGOS;
		S.Action = EPGXTutorialAction::ConfigBasePath;
		Steps.Add(S);
	}

	// Step 1: CreateFolder
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Crear Carpeta de MGOS")
			: TEXT("Create MGOS Folder"));
		S.Description = FText::FromString(bES
			? TEXT("Crea la subcarpeta 'MGOS' dentro de tu ruta base. "
				"Aqui vivira el DA_MGOSConfig que configura los umbrales de memoria, frecuencias de GC, y modos de observacion.\n\n"
				"MGOS se inicializa muy temprano (UEngineSubsystem), por lo que su config debe estar disponible antes que cualquier otro sistema PGX.")
			: TEXT("Create the 'MGOS' subfolder inside your base path. "
				"Here will live the DA_MGOSConfig that configures memory thresholds, GC frequencies, and observation modes.\n\n"
				"MGOS initializes very early (UEngineSubsystem), so its config must be available before any other PGX system."));
		S.AccentColor = PGX::System::MGOS;
		S.Action = EPGXTutorialAction::CreateFolder;
		S.ActionPath = TEXT("MGOS");
		Steps.Add(S);
	}

	// Step 2: CreateAsset
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Crear MGOS Config")
			: TEXT("Create MGOS Config"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXGCObserverConfig configura el sistema de observacion de GC y memoria. "
				"Define: ObservationMode inicial (Passive/Active/Profiling/Emergency), umbrales de memoria para cada estado del profile engine (Warning/Alert/Critical), y frecuencia de sampling.\n\n"
				"POR QUE EXISTE: UE tiene GC pero no da visibilidad sobre cuando ocurre, cuanto cuesta, ni que objetos estan generando presion de memoria. "
				"BENEFICIO: MGOS detecta fugas de memoria, picos de GC, y patrones anomalos antes de que causen crashes en produccion. "
				"EJEMPLO: Umbral Warning=512MB, Alert=768MB, Critical=1GB — el profile engine escala automaticamente la agresividad del GC.")
			: TEXT("UPGXGCObserverConfig configures the GC and memory observation system. "
				"Defines: initial ObservationMode (Passive/Active/Profiling/Emergency), memory thresholds for each profile engine state (Warning/Alert/Critical), and sampling frequency.\n\n"
				"WHY IT EXISTS: UE has GC but gives no visibility into when it occurs, how much it costs, or which objects are generating memory pressure. "
				"BENEFIT: MGOS detects memory leaks, GC spikes, and anomalous patterns before they cause production crashes. "
				"EXAMPLE: Warning=512MB threshold, Alert=768MB, Critical=1GB — the profile engine automatically scales GC aggressiveness."));
		S.AccentColor = PGX::System::MGOS;
		S.Action = EPGXTutorialAction::CreateAsset;
		S.ActionPath = TEXT("MGOS");
		S.AssetClass = TEXT("/Script/PGXMGOSRuntime.PGXGCObserverConfig");
		S.AssetName = TEXT("DA_MGOSConfig");
		Steps.Add(S);
	}

	// Step 3: Guide — MGOS Inspector
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXMGOSInspector");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Explorar el MGOS Inspector")
			: TEXT("Explore the MGOS Inspector"));
		S.Description = FText::FromString(bES
			? TEXT("El MGOS Inspector muestra en vivo: modo de observacion actual, estado del profile engine (Idle/Monitoring/Warning/Alert/Critical/Recovery), uso de memoria con grafica temporal, y frecuencia de GC con histograma. "
				"Incluye controles para cambiar el modo de observacion manualmente y forzar un GC de prueba.\n\n"
				"CUANDO USARLO: Durante sesiones de profiling de memoria o cuando sospeches de una fuga de memoria — el inspector te dira exactamente que esta creciendo.")
			: TEXT("The MGOS Inspector shows live: current observation mode, profile engine state (Idle/Monitoring/Warning/Alert/Critical/Recovery), memory usage with temporal graph, and GC frequency with histogram. "
				"Includes controls to manually change the observation mode and force a test GC.\n\n"
				"WHEN TO USE IT: During memory profiling sessions or when you suspect a memory leak — the inspector will tell you exactly what is growing."));
		S.AccentColor = PGX::System::MGOS;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 4: Guide — 4 Observation Modes
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("4 Modos de Observacion")
			: TEXT("4 Observation Modes"));
		S.Description = FText::FromString(bES
			? TEXT("MGOS opera en 4 modos configurables: "
				"Passive — sampling de baja frecuencia (1Hz), overhead minimo, recomendado para produccion.\n\n"
				"Active — sampling a 10Hz con deteccion de patrones, recomendado durante QA. "
				"Profiling — sampling a 60Hz con stack traces de allocacion, para investigar fugas especificas. "
				"Emergency — activa cuando se supera el umbral Critical, fuerza GC agresivo y limita allocaciones. "
				"El modo puede cambiar automaticamente via el profile engine, o manualmente con pgx.mgos.mode.")
			: TEXT("MGOS operates in 4 configurable modes: "
				"Passive — low-frequency sampling (1Hz), minimal overhead, recommended for production.\n\n"
				"Active — 10Hz sampling with pattern detection, recommended during QA. "
				"Profiling — 60Hz sampling with allocation stack traces, for investigating specific leaks. "
				"Emergency — activates when Critical threshold is exceeded, forces aggressive GC and limits allocations. "
				"The mode can change automatically via the profile engine, or manually with pgx.mgos.mode."));
		S.AccentColor = PGX::System::MGOS;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 5: Guide — 6-State Profile Engine
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Profile Engine de 6 Estados")
			: TEXT("6-State Profile Engine"));
		S.Description = FText::FromString(bES
			? TEXT("El Profile Engine de MGOS es una maquina de estados de 6 fases: "
				"Idle (no sesion activa) -> Monitoring (observacion normal) -> Warning (presion de memoria elevada) -> Alert (presion critica, GC mas frecuente) -> Critical (umbral superado, modo Emergency) -> Recovery (GC forzado, esperando estabilizacion).\n\n"
				"Las transiciones son automaticas basadas en los umbrales del config DA. "
				"BENEFICIO: El sistema reacciona a la presion de memoria sin intervencion del dev — solo configura los umbrales y MGOS se autogestiona.")
			: TEXT("The MGOS Profile Engine is a 6-phase state machine: "
				"Idle (no active session) -> Monitoring (normal observation) -> Warning (elevated memory pressure) -> Alert (critical pressure, more frequent GC) -> Critical (threshold exceeded, Emergency mode) -> Recovery (forced GC, waiting for stabilization).\n\n"
				"Transitions are automatic based on config DA thresholds. "
				"BENEFIT: The system reacts to memory pressure without developer intervention — just configure the thresholds and MGOS self-manages."));
		S.AccentColor = PGX::System::MGOS;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 6: Open DA
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Abrir y Configurar el MGOS Config")
			: TEXT("Open and Configure the MGOS Config"));
		S.Description = FText::FromString(bES
			? TEXT("Abre DA_MGOSConfig. Propiedades clave: InitialMode (modo al arrancar el engine), MemoryThresholds (Warning/Alert/Critical en MB), GCSamplingRate (Hz por modo), bEnableLeakDetection (activa heuristicas de deteccion de fuga). "
				"IMPORTANTE: MGOS es un UEngineSubsystem — NO usa GetTimerManager(). Usa FTSTicker internamente para el sampling.\n\n"
				"Ajusta los umbrales segun la plataforma objetivo: en Switch usa valores mas bajos (Warning=256MB) que en PC (Warning=1GB).")
			: TEXT("Open DA_MGOSConfig. Key properties: InitialMode (mode at engine startup), MemoryThresholds (Warning/Alert/Critical in MB), GCSamplingRate (Hz per mode), bEnableLeakDetection (activates leak detection heuristics). "
				"IMPORTANT: MGOS is a UEngineSubsystem — it does NOT use GetTimerManager(). It uses FTSTicker internally for sampling.\n\n"
				"Adjust thresholds according to the target platform: on Switch use lower values (Warning=256MB) than on PC (Warning=1GB)."));
		S.AccentColor = PGX::System::MGOS;
		S.Action = EPGXTutorialAction::OpenAsset;
		S.ActionPath = TEXT("MGOS");
		S.AssetName = TEXT("DA_MGOSConfig");
		Steps.Add(S);
	}

	// Step 7: Guide — Console Commands
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Comandos de Consola de MGOS")
			: TEXT("MGOS Console Commands"));
		S.Description = FText::FromString(bES
			? TEXT("Comandos disponibles: 'pgx.mgos.status' muestra modo actual, estado del profile engine, uso de memoria, y frecuencia de GC. "
				"'pgx.mgos.mode <Passive|Active|Profiling|Emergency>' cambia el modo de observacion en caliente.\n\n"
				"'pgx.mgos.profile' muestra el historial completo del profile engine con timestamps de cada transicion de estado. "
				"Esenciales durante sesiones de profiling — usa 'pgx.mgos.mode Profiling' para capturar una secuencia de memoria detallada.")
			: TEXT("Available commands: 'pgx.mgos.status' shows current mode, profile engine state, memory usage, and GC frequency. "
				"'pgx.mgos.mode <Passive|Active|Profiling|Emergency>' changes the observation mode on-the-fly.\n\n"
				"'pgx.mgos.profile' shows the complete profile engine history with timestamps of each state transition. "
				"Essential during profiling sessions — use 'pgx.mgos.mode Profiling' to capture a detailed memory sequence."));
		S.AccentColor = PGX::System::MGOS;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 8: Guide — System Observer
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXSystemObserver");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("MGOS en el System Observer")
			: TEXT("MGOS in the System Observer"));
		S.Description = FText::FromString(bES
			? TEXT("En el System Observer, MGOS aparece como UEngineSubsystem (distinto de los GameInstanceSubsystems del resto de sistemas PGX). "
				"Muestra: modo activo, estado del profile engine, uso de memoria actual, GCs totales registrados, y si la deteccion de fugas esta activa.\n\n"
				"NOTA: MGOS es el unico sistema de PGX que aparece como Engine-scope en el Observer — todos los demas son GameInstance-scope.")
			: TEXT("In the System Observer, MGOS appears as a UEngineSubsystem (distinct from the GameInstanceSubsystems of other PGX systems). "
				"Shows: active mode, profile engine state, current memory usage, total recorded GCs, and whether leak detection is active.\n\n"
				"NOTE: MGOS is the only PGX system that appears as Engine-scope in the Observer — all others are GameInstance-scope."));
		S.AccentColor = PGX::System::MGOS;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 9: Summary
	{
		FPGXTutorialStep S;
		S.TargetTabId = FName("PGXHub");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Resumen: MGOS Configurado — Tutoriales Completos")
			: TEXT("Summary: MGOS Configured — Tutorials Complete"));
		S.Description = FText::FromString(bES
			? TEXT("Has completado la configuracion del sistema MGOS. Resumen:\n\n"
				"- CREADO: DA_MGOSConfig (modo inicial, umbrales, sampling, deteccion de fugas)\n"
				"- EXPLORADO: MGOS Inspector (memoria en vivo, GC histogram, profile engine)\n"
				"- APRENDIDO: 4 modos de observacion (Passive/Active/Profiling/Emergency)\n"
				"- APRENDIDO: Profile Engine de 6 estados (Idle->Monitoring->Warning->Alert->Critical->Recovery)\n"
				"- APRENDIDO: Comandos pgx.mgos.* para profiling y debug de memoria\n"
				"- UNICO: MGOS es UEngineSubsystem — persiste durante toda la vida del engine\n\n"
				"FELICITACIONES — has completado los tutoriales de sistema incluidos. "
				"Regresa al PGX Hub para explorar los inspectores, validacion y PGX Docs.")
			: TEXT("You have completed the MGOS system configuration. Summary:\n\n"
				"- CREATED: DA_MGOSConfig (initial mode, thresholds, sampling, leak detection)\n"
				"- EXPLORED: MGOS Inspector (live memory, GC histogram, profile engine)\n"
				"- LEARNED: 4 observation modes (Passive/Active/Profiling/Emergency)\n"
				"- LEARNED: 6-state Profile Engine (Idle->Monitoring->Warning->Alert->Critical->Recovery)\n"
				"- LEARNED: pgx.mgos.* commands for memory profiling and debug\n"
				"- UNIQUE: MGOS is a UEngineSubsystem — persists for the entire engine lifetime\n\n"
				"CONGRATULATIONS — you have completed the included system tutorials. "
				"Return to the PGX Hub to explore inspectors, validation and PGX Docs."));
		S.AccentColor = PGX::Semantic::Good;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	return Steps;
}

// ============================================================================
// S14: Trade System (11 steps) — Constructor (Config DA + Settings-first)
// ============================================================================
// NOTE: Uses PGX::System::Tutorials accent color (Trade token not yet in VT).
TArray<FPGXTutorialStep> PGXSystemTutorials::GetS14_Trade(EPGXTutorialLanguage Lang)
{
	const bool bES = (Lang == EPGXTutorialLanguage::Spanish);
	TArray<FPGXTutorialStep> Steps;

	// Step 0: Intro
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Bienvenido al Sistema Trade")
			: TEXT("Welcome to the Trade System"));
		S.Description = FText::FromString(bES
			? TEXT("PGXTrade gestiona actores de comercio, ofertas, transacciones y reputacion. "
				"Todo es data-driven a traves de UPGXTradeConfig. "
				"En este tutorial crearas y configuraras un Trade Config DataAsset.")
			: TEXT("PGXTrade manages trade actors, offers, transactions, and reputation. "
				"Everything is data-driven via UPGXTradeConfig. "
				"In this tutorial you will create and configure a Trade Config DataAsset."));
		S.AccentColor = PGX::System::Tutorials;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 1: ConfigBasePath
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Configurar Ruta Base del Proyecto")
			: TEXT("Configure Project Base Path"));
		S.Description = FText::FromString(bES
			? TEXT("Define la carpeta raiz donde PGX guardara los assets de Trade. "
				"Ejemplo: /Game/PGX/Trade. "
				"PGXTrade usa esta ruta para localizar automaticamente UPGXTradeConfig.")
			: TEXT("Define the root folder where PGX will store Trade assets. "
				"Example: /Game/PGX/Trade. "
				"PGXTrade uses this path to automatically locate UPGXTradeConfig."));
		S.AccentColor = PGX::System::Tutorials;
		S.Action = EPGXTutorialAction::ConfigBasePath;
		Steps.Add(S);
	}

	// Step 2: CreateFolder
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Crear Carpeta de Trade")
			: TEXT("Create Trade Folder"));
		S.Description = FText::FromString(bES
			? TEXT("Crea la subcarpeta 'Trade' dentro de tu ruta base. "
				"Aqui vivira el DA_TradeConfig que configura actores, ofertas y reputacion.")
			: TEXT("Create the 'Trade' subfolder inside your base path. "
				"Here will live the DA_TradeConfig that configures actors, offers, and reputation."));
		S.AccentColor = PGX::System::Tutorials;
		S.Action = EPGXTutorialAction::CreateFolder;
		S.ActionPath = TEXT("Trade");
		Steps.Add(S);
	}

	// Step 3: CreateAsset — UPGXTradeConfig
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Crear Trade Config DataAsset")
			: TEXT("Create Trade Config DataAsset"));
		S.Description = FText::FromString(bES
			? TEXT("Content Browser > clic derecho > PGX > Trade > Trade Config. "
				"Esto crea DA_TradeConfig (clase UPGXTradeConfig) en /Game/PGX/Trade/. "
				"Nombralo DA_TradeConfig.")
			: TEXT("Content Browser > right-click > PGX > Trade > Trade Config. "
				"This creates DA_TradeConfig (class UPGXTradeConfig) at /Game/PGX/Trade/. "
				"Name it DA_TradeConfig."));
		S.AccentColor = PGX::System::Tutorials;
		S.Action = EPGXTutorialAction::CreateAsset;
		S.ActionPath = TEXT("DA_TradeConfig");
		S.AssetClass = TEXT("UPGXTradeConfig");
		S.AssetName = TEXT("DA_TradeConfig");
		Steps.Add(S);
	}

	// Step 4: OpenAsset
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Abrir Trade Config")
			: TEXT("Open Trade Config"));
		S.Description = FText::FromString(bES
			? TEXT("Doble clic en DA_TradeConfig para abrir su panel de propiedades. "
				"Veras: FairTradeTolerance, DefaultOfferExpirationSeconds, rango de reputacion, "
				"y DefaultInformationFreshness.")
			: TEXT("Double-click DA_TradeConfig to open its property panel. "
				"You will see: FairTradeTolerance, DefaultOfferExpirationSeconds, reputation range, "
				"and DefaultInformationFreshness."));
		S.AccentColor = PGX::System::Tutorials;
		S.Action = EPGXTutorialAction::OpenAsset;
		S.AssetName = TEXT("DA_TradeConfig");
		Steps.Add(S);
	}

	// Step 5: Configure — FairTradeTolerance
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Configurar Tolerancia de Comercio Justo")
			: TEXT("Configure Fair Trade Tolerance"));
		S.Description = FText::FromString(bES
			? TEXT("FairTradeTolerance define la banda de valuacion para comercio justo (default 0.25). "
				"Un valor de 0.25 significa que ofertas dentro del 25% del valor de mercado "
				"se consideran justas. Ajusta segun la economia de tu juego.")
			: TEXT("FairTradeTolerance defines the fair-trade valuation band (default 0.25). "
				"A value of 0.25 means offers within 25% of market value "
				"are considered fair. Adjust for your game's economy."));
		S.AccentColor = PGX::System::Tutorials;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 6: Configure — DefaultOfferExpirationSeconds
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Configurar Expiracion de Ofertas")
			: TEXT("Configure Offer Expiration"));
		S.Description = FText::FromString(bES
			? TEXT("DefaultOfferExpirationSeconds controla cuanto vive una oferta (default 300s = 5min). "
				"Tras este tiempo, las ofertas no aceptadas expiran automaticamente.")
			: TEXT("DefaultOfferExpirationSeconds controls how long an offer lives (default 300s = 5min). "
				"After this time, unaccepted offers expire automatically."));
		S.AccentColor = PGX::System::Tutorials;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 7: Configure — Reputation range
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Configurar Rango de Reputacion")
			: TEXT("Configure Reputation Range"));
		S.Description = FText::FromString(bES
			? TEXT("MinReputation (-100) y MaxReputation (100) definen los limites de reputacion. "
				"Los actores fuera de este rango no pueden comerciar. "
				"Ajusta para controlar quien participa en la economia.")
			: TEXT("MinReputation (-100) and MaxReputation (100) define reputation boundaries. "
				"Actors outside this range cannot trade. "
				"Adjust to control who participates in the economy."));
		S.AccentColor = PGX::System::Tutorials;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 8: Configure — DefaultInformationFreshness + Save
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Frescura de Informacion y Guardar")
			: TEXT("Information Freshness and Save"));
		S.Description = FText::FromString(bES
			? TEXT("DefaultInformationFreshness (0..1) controla el decaimiento de informacion de mercado. "
				"0 = siempre fresca, 1 = decaimiento maximo. "
				"Guarda el asset (Ctrl+S). IsPolicyValid() validara la configuracion al guardar.")
			: TEXT("DefaultInformationFreshness (0..1) controls market information decay. "
				"0 = always fresh, 1 = maximum decay. "
				"Save the asset (Ctrl+S). IsPolicyValid() validates the config on save."));
		S.AccentColor = PGX::System::Tutorials;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 9: Assign Config (Settings-first MANDATORY)
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Asignar Config en Project Settings")
			: TEXT("Assign Config in Project Settings"));
		S.Description = FText::FromString(bES
			? TEXT("CREAR EL DA NO ES SUFICIENTE. Ve a Project Settings > PGX Trade > ActiveConfig "
				"y asigna DA_TradeConfig. Este paso es OBLIGATORIO: el subsistema resuelve "
				"la configuracion desde aqui (AssetRegistry fallback fue removido en v0.6.0).")
			: TEXT("CREATING THE DA IS NOT ENOUGH. Go to Project Settings > PGX Trade > ActiveConfig "
				"and assign DA_TradeConfig. This step is MANDATORY: the subsystem resolves "
				"the configuration from here (AssetRegistry fallback was removed in v0.6.0)."));
		S.AccentColor = PGX::System::Tutorials;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 10: Blueprint entry point + Inspector
	{
		FPGXTutorialStep S;
		S.TargetTabId = TEXT("PGXTradeInspector");
		S.bOpenTab = true;
		S.Title = FText::FromString(bES
			? TEXT("Explorar API y Inspector Trade")
			: TEXT("Explore Trade API and Inspector"));
		S.Description = FText::FromString(bES
			? TEXT("Abre el inspector Trade (Window > PGX > Trade Inspector). "
				"Desde Blueprint, usa UPGXTradeBlueprintLibrary::GetTradeSubsystem() "
				"para acceder a RegisterTradeActor, CreateTradeOffer, y AcceptTradeOffer. "
				"Resumen:\n"
				"- CREADO: DA_TradeConfig con tolerancia, expiracion, reputacion y frescura\n"
				"- ASIGNADO: Config en Project Settings (Settings-first)\n"
				"- EXPLORADO: Trade Inspector y API Blueprint\n\n"
				"Siguiente paso recomendado: Tutorial S15 (Crafting) para sistemas de recetas y creacion.")
			: TEXT("Open the Trade inspector (Window > PGX > Trade Inspector). "
				"From Blueprint, use UPGXTradeBlueprintLibrary::GetTradeSubsystem() "
				"to access RegisterTradeActor, CreateTradeOffer, and AcceptTradeOffer. "
				"Summary:\n"
				"- CREATED: DA_TradeConfig with tolerance, expiration, reputation, and freshness\n"
				"- ASSIGNED: Config in Project Settings (Settings-first)\n"
				"- EXPLORED: Trade Inspector and Blueprint API\n\n"
				"Recommended next step: Tutorial S15 (Crafting) for recipe and creation systems."));
		S.AccentColor = PGX::Semantic::Good;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	return Steps;
}

// ============================================================================
// S15: Crafting System (9 steps) — Constructor (register-driven model)
// ============================================================================
// NOTE: PGX::System::Crafting token does not exist in PGXVisualTokens.h; steps use
//       PGX::System::Tutorials (the canonical default accent). A dedicated Crafting
//       token can be added in a future VT pass.
TArray<FPGXTutorialStep> PGXSystemTutorials::GetS15_Crafting(EPGXTutorialLanguage Lang)
{
	const bool bES = (Lang == EPGXTutorialLanguage::Spanish);
	TArray<FPGXTutorialStep> Steps;

	// Step 0: ConfigBasePath
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Configurar Ruta Base del Proyecto")
			: TEXT("Configure Project Base Path"));
		S.Description = FText::FromString(bES
			? TEXT("Define la carpeta donde se guardaran las recetas del sistema Crafting. "
				"Ejemplo: /Game/PGX/Crafting. Cada receta es un DataAsset UPGXRecipeDefinition independiente.")
			: TEXT("Define the folder where Crafting system recipes will be stored. "
				"Example: /Game/PGX/Crafting. Each recipe is an independent UPGXRecipeDefinition DataAsset."));
		S.AccentColor = PGX::System::Tutorials;
		S.Action = EPGXTutorialAction::ConfigBasePath;
		Steps.Add(S);
	}

	// Step 1: Create a Recipe Definition DA
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Crear una Recipe Definition")
			: TEXT("Create a Recipe Definition"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXRecipeDefinition es un DataAsset (no un Config) que define UNA receta authorada. "
				"Creas N recetas, una por cada receta del juego — no un config global unico.\n\n"
				"POR QUE EXISTE: separa el contenido (recetas) del codigo; los disenadores anaden recetas sin recompilar. "
				"BENEFICIO: cada receta vive en su propio asset, versionable y referenciable por tag.")
			: TEXT("UPGXRecipeDefinition is a DataAsset (not a Config) that defines ONE authored recipe. "
				"You create N recipes, one per game recipe — not a single global config.\n\n"
				"WHY IT EXISTS: separates content (recipes) from code; designers add recipes without recompiling. "
				"BENEFIT: each recipe lives in its own asset, versionable and referenceable by tag."));
		S.AccentColor = PGX::System::Tutorials;
		S.Action = EPGXTutorialAction::CreateAsset;
		S.ActionPath = TEXT("Crafting");
		S.AssetClass = TEXT("/Script/PGXCraftingRuntime.PGXRecipeDefinition");
		S.AssetName = TEXT("DA_PGXRecipe");
		Steps.Add(S);
	}

	// Step 2: Set the RecipeTag
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Asignar el RecipeTag")
			: TEXT("Set the RecipeTag"));
		S.Description = FText::FromString(bES
			? TEXT("En la propiedad Recipe (FPGXCraftingRecipeDefinition), asigna el RecipeTag (FGameplayTag). "
				"Es la clave por la que el subsistema resuelve la receta en runtime via FindRecipe(RecipeTag).")
			: TEXT("In the Recipe property (FPGXCraftingRecipeDefinition), set the RecipeTag (FGameplayTag). "
				"It is the key the subsystem uses to resolve the recipe at runtime via FindRecipe(RecipeTag)."));
		S.AccentColor = PGX::System::Tutorials;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 3: Fill the recipe payload
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Rellenar la receta")
			: TEXT("Fill the recipe payload"));
		S.Description = FText::FromString(bES
			? TEXT("Completa los campos de FPGXCraftingRecipeDefinition: inputs, outputs y requisitos de la receta. "
				"Estos definen que consume y produce el craft.")
			: TEXT("Complete the FPGXCraftingRecipeDefinition fields: recipe inputs, outputs and requirements. "
				"These define what the craft consumes and produces."));
		S.AccentColor = PGX::System::Tutorials;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 4: Save the asset
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Guardar el asset")
			: TEXT("Save the asset"));
		S.Description = FText::FromString(bES
			? TEXT("Guarda el DA_PGXRecipe. El asset ya es creable desde Content Browser > PGX > Crafting > Recipe Definition.")
			: TEXT("Save the DA_PGXRecipe. The asset is creatable from Content Browser > PGX > Crafting > Recipe Definition."));
		S.AccentColor = PGX::System::Tutorials;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 5: Register the recipe at runtime (CURRENT register-driven model)
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Registrar la receta en runtime")
			: TEXT("Register the recipe at runtime"));
		S.Description = FText::FromString(bES
			? TEXT("El UPGXCraftingSubsystem NO auto-carga recetas. Tras crear el asset, registralo en runtime con "
				"RegisterRecipeAsset(UPGXRecipeDefinition*) — por ejemplo desde tu GameMode/init. Tras registrar, "
				"FindRecipe(RecipeTag) la resuelve.\n\n"
				"NOTA: la auto-carga data-driven desde Project Settings (UPGXCraftingSettings) se anade en la tanda "
				"de alineacion runtime (T-RT). De momento el registro es explicito.")
			: TEXT("The UPGXCraftingSubsystem does NOT auto-load recipes. After creating the asset, register it at "
				"runtime via RegisterRecipeAsset(UPGXRecipeDefinition*) — e.g. from your GameMode/init. Once registered, "
				"FindRecipe(RecipeTag) resolves it.\n\n"
				"NOTE: data-driven auto-load from Project Settings (UPGXCraftingSettings) is added in the runtime-"
				"alignment tanda (T-RT). For now registration is explicit."));
		S.AccentColor = PGX::System::Tutorials;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 6: Subsystem entry points
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Puntos de entrada del subsistema")
			: TEXT("Subsystem entry points"));
		S.Description = FText::FromString(bES
			? TEXT("UPGXCraftingSubsystem (UGameInstanceSubsystem) expone: RegisterRecipeAsset, FindRecipe, HasRecipe, "
				"GetRegisteredRecipeCount, ValidateCraftRequest, SimulateCraft, StartCraft, CancelCraft. "
				"No hay BlueprintLibrary aun — usa el subsistema directamente.")
			: TEXT("UPGXCraftingSubsystem (UGameInstanceSubsystem) exposes: RegisterRecipeAsset, FindRecipe, HasRecipe, "
				"GetRegisteredRecipeCount, ValidateCraftRequest, SimulateCraft, StartCraft, CancelCraft. "
				"There is no BlueprintLibrary yet — use the subsystem directly."));
		S.AccentColor = PGX::System::Tutorials;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 7: Simulate vs Start a craft
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Simular vs iniciar un craft")
			: TEXT("Simulate vs start a craft"));
		S.Description = FText::FromString(bES
			? TEXT("SimulateCraft(FPGXCraftRequest) hace un dry-run sin cambiar estado; StartCraft inicia un trabajo real. "
				"Consulta los trabajos con GetActiveCraftJobCount / GetCraftJobsSnapshot.")
			: TEXT("SimulateCraft(FPGXCraftRequest) does a dry-run with no state change; StartCraft starts a real job. "
				"Query jobs with GetActiveCraftJobCount / GetCraftJobsSnapshot."));
		S.AccentColor = PGX::System::Tutorials;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	// Step 8: Current limitations
	{
		FPGXTutorialStep S;
		S.TargetTabId = NAME_None;
		S.bOpenTab = false;
		S.Title = FText::FromString(bES
			? TEXT("Limitaciones actuales")
			: TEXT("Current limitations"));
		S.Description = FText::FromString(bES
			? TEXT("La inspeccion en vivo del registro de recetas y la carga automatica desde Settings no estan disponibles en esta version. "
				"Las API actuales permiten crear y registrar recetas.")
			: TEXT("Live recipe-registry inspection and automatic loading from Settings are unavailable in this preview. "
				"The current APIs support creating and registering recipes."));
		S.AccentColor = PGX::System::Tutorials;
		S.Action = EPGXTutorialAction::None;
		Steps.Add(S);
	}

	return Steps;
}
