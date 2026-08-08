// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Onboarding tutorial content — T1-T5 step definitions (bilingual EN/ES).
// ES: Contenido de tutoriales onboarding — definiciones de pasos T1-T5 (bilingue EN/ES).

#include "Tutorials/PGXOnboardingTutorials.h"
#include "Style/PGXVisualTokens.h"

// ============================================================================
// T1: What is PGX? — Guide Agent (11 steps)
// EN: Complete tour of every major PGX editor tool. No assets created.
// ES: Tour completo de cada herramienta principal del editor PGX. No crea assets.
// ============================================================================

TArray<FPGXTutorialStep> PGXOnboardingTutorials::GetT1_WhatIsPGX(EPGXTutorialLanguage Lang)
{
	const bool bES = (Lang == EPGXTutorialLanguage::Spanish);
	TArray<FPGXTutorialStep> Steps;
	Steps.Reserve(11);

	// --- Step 1: Welcome (no tab target) ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = NAME_None;
		Step.bOpenTab = false;
		Step.Title = bES
			? FText::FromString(TEXT("Bienvenido al Framework PGX"))
			: FText::FromString(TEXT("Welcome to PGX Framework"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"PGX es un framework modular C++ para Unreal Engine 5 con una constelacion publica de plugins en Development Preview. "
				"Los sistemas runtime dependen de PGXCoreRuntime y se coordinan mediante contratos compartidos.\n\n"
				"La configuracion es completamente data-driven: creas un DataAsset en el Content Browser, llenas sus propiedades en el panel Details, y el sistema se auto-configura. "
				"Este tour te guiara por cada panel principal del editor para que conozcas todas las herramientas disponibles antes de empezar a construir tu juego."))
			: FText::FromString(TEXT(
				"PGX is a modular C++ framework for Unreal Engine 5 with a public plugin constellation in Development Preview. "
				"Runtime systems depend on PGXCoreRuntime and coordinate through shared contracts.\n\n"
				"Configuration is fully data-driven: you create a DataAsset in the Content Browser, fill its properties in the Details panel, and the system auto-configures. "
				"This tour will walk you through every major editor panel so you know what tools are available before you start building your game."));
		Step.AccentColor = PGX::Semantic::Info;
		Steps.Add(Step);
	}

	// --- Step 2: PGX Hub ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXHub"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("PGX Hub — Panel Central"))
			: FText::FromString(TEXT("PGX Hub — Central Dashboard"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El PGX Hub es tu centro de comando. Muestra todos los plugins PGX instalados como tarjetas con su estado, version, y botones de acceso rapido. "
				"Desde aqui puedes abrir cualquier inspector de sistema con un solo click, ver cuales estan inicializados, y acceder a la documentacion.\n\n"
				"Piensa en el como la cabina de tu proyecto — un vistazo te dice la salud de toda tu configuracion PGX. "
				"Puedes fijar tus inspectores mas usados en la barra Quick Access del toolbar para navegacion aun mas rapida."))
			: FText::FromString(TEXT(
				"The PGX Hub is your central command center. It displays all installed PGX plugins as cards with their status, version, and quick-access buttons. "
				"From here you can open any system inspector with a single click, see which systems are initialized, and access documentation.\n\n"
				"Think of it as your project's cockpit — one glance tells you the health of your entire PGX setup. "
				"You can pin your most-used inspectors in the Quick Access bar at the top of the toolbar for even faster navigation."));
		Step.AccentColor = PGX::Semantic::Info;
		Steps.Add(Step);
	}

	// --- Step 3: System Observer ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXSystemObserver"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("System Observer — Sistemas Publicos de un Vistazo"))
			: FText::FromString(TEXT("System Observer — Public Systems at a Glance"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El System Observer te da visibilidad en tiempo real de los 13 subsistemas PGX simultaneamente. "
				"Cada fila muestra estado de inicializacion, resultado de descubrimiento de config, estado activo, y contadores runtime clave.\n\n"
				"El orden de inicializacion (Profile > GameFlow > Log > Save > PSO > Widget > Audio > Registry > Message > EventHandler) se aplica automaticamente. "
				"Usa este panel para diagnosticar problemas — si un sistema aparece como no inicializado, su DataAsset de configuracion puede faltar o estar mal configurado."))
			: FText::FromString(TEXT(
				"The System Observer gives you real-time visibility into all 13 PGX subsystems simultaneously. "
				"Each row displays initialization status, config discovery result, active state, and key runtime counters.\n\n"
				"The initialization order (Profile > GameFlow > Log > Save > PSO > Widget > Audio > Registry > Message > EventHandler) is enforced automatically. "
				"Use this panel to diagnose issues — if a system shows as uninitialized, its configuration DataAsset may be missing or misconfigured."));
		Step.AccentColor = PGX::Semantic::Info;
		Steps.Add(Step);
	}

	// --- Step 4: Profile Inspector ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXProfileInspector"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Profile Inspector — Constitucion del Proyecto"))
			: FText::FromString(TEXT("Profile Inspector — Project Constitution"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El sistema Profile es la base de PGX — se inicializa primero y establece la identidad de tu proyecto y sus plataformas objetivo. "
				"A traves de UPGXProjectProfileConfig defines capacidades de plataforma, presupuestos de rendimiento, y restricciones que todos los demas sistemas respetan.\n\n"
				"El inspector muestra el perfil resuelto, la plataforma detectada, y el estado de aplicacion de presupuestos en los 11 subsistemas sensibles a budgets. "
				"Aqui es donde estableces las reglas de tu proyecto: limites de memoria, canales de audio, topes de slots de guardado, y mas."))
			: FText::FromString(TEXT(
				"The Profile system is PGX's foundation — it initializes first and establishes your project's identity and platform targets. "
				"Through UPGXProjectProfileConfig you define platform capabilities, performance budgets, and constraints that all other systems respect.\n\n"
				"The inspector shows the resolved profile, detected platform, and budget enforcement status across all 11 budget-aware subsystems. "
				"This is where you set the rules of your project: memory limits, audio channel counts, save slot caps, and more."));
		Step.AccentColor = PGX::System::Profile;
		Steps.Add(Step);
	}

	// --- Step 5: GameFlow Inspector ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXGameFlowInspector"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("GameFlow Inspector — Estados de Juego con Tags"))
			: FText::FromString(TEXT("GameFlow Inspector — Tag-Driven Game States"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"GameFlow maneja la maquina de estados de tu juego usando GameplayTags a traves de 8 canales independientes. "
				"En lugar de enums o estados hardcodeados, las transiciones son basadas en tags — haciendo el sistema infinitamente extensible sin cambios de codigo.\n\n"
				"El inspector muestra el estado actual de cada canal, historial de transiciones, y reglas de flujo activas. "
				"Este diseno desacoplado permite que tus sistemas de UI, audio, save y loading reaccionen a cambios de estado sin conocerse entre si."))
			: FText::FromString(TEXT(
				"GameFlow manages your game's state machine using GameplayTags across 8 independent channels. "
				"Instead of enums or hardcoded states, transitions are tag-based — making the system infinitely extensible without code changes.\n\n"
				"The inspector shows the current state of each channel, transition history, and active flow rules. "
				"This decoupled design means your UI, audio, save, and loading systems can all react to state changes without knowing about each other."));
		Step.AccentColor = PGX::System::GameFlow;
		Steps.Add(Step);
	}

	// --- Step 6: Save Inspector ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXSaveInspector"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Save Inspector — Sistema de Persistencia"))
			: FText::FromString(TEXT("Save Inspector — Persistence System"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El sistema Save provee persistencia estructurada con slots, dominios, y programacion automatica de guardado. "
				"Los dominios particionan tus datos logicamente (progreso del jugador, configuraciones, estado del mundo), y cada uno puede guardarse independientemente.\n\n"
				"El inspector muestra todos los save slots, sus metadatos, desglose por dominio, y estado del auto-save. "
				"Los objetos implementan la interfaz IPGXSaveable para participar en el ciclo save/load — el sistema maneja serializacion, compresion, y file I/O."))
			: FText::FromString(TEXT(
				"The Save system provides structured persistence with slots, domains, and automatic save scheduling. "
				"Domains partition your save data logically (player progress, settings, world state), and each can be saved independently.\n\n"
				"The inspector displays all save slots, their metadata, domain breakdown, and auto-save status. "
				"Objects implement the IPGXSaveable interface to participate in the save/load cycle — the system handles serialization, compression, and file I/O."));
		Step.AccentColor = PGX::System::Save;
		Steps.Add(Step);
	}

	// --- Step 7: Log Viewer ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXLogViewer"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Log Viewer — Observabilidad Polimorfica"))
			: FText::FromString(TEXT("Log Viewer — Polymorphic Observability"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"PGX Log v3.0 es un sistema de observabilidad polimorfico con renderers especificos por dominio. "
				"A diferencia del Output Log estandar, entiende las categorias de log de PGX y renderiza las entradas con formato enriquecido — colores, iconos, y datos estructurados.\n\n"
				"Puedes filtrar por sistema, severidad, y rango de tiempo. Cada sistema PGX tiene su propia categoria de log (LogPGXSave, LogPGXGameFlow, etc.) para filtrado preciso. "
				"Las capacidades de exportacion te permiten capturar sesiones diagnosticas para debugging o reportes offline."))
			: FText::FromString(TEXT(
				"PGX Log v3.0 is a polymorphic observability system with domain-specific renderers. "
				"Unlike the standard Output Log, it understands PGX log categories and renders entries with rich formatting — colors, icons, and structured data.\n\n"
				"You can filter by system, severity, and time range. Each PGX system has its own log category (LogPGXSave, LogPGXGameFlow, etc.) for precise filtering. "
				"Export capabilities let you capture diagnostic sessions for debugging or offline reporting."));
		Step.AccentColor = PGX::System::Log;
		Steps.Add(Step);
	}

	// --- Step 8: Config Dashboard ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXConfigDashboard"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Config Dashboard — Todas las Configuraciones en una Vista"))
			: FText::FromString(TEXT("Config Dashboard — All Configurations in One View"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El Config Dashboard agrega todos los DataAssets de configuracion PGX de tu proyecto, agrupados por sistema. "
				"En lugar de buscar por el Content Browser, ves todas las configs de un vistazo — que sistemas tienen configs, cuales faltan, y sus valores resueltos.\n\n"
				"Doble click en cualquier entrada para abrir el asset y editarlo. El dashboard auto-descubre configs usando el AssetRegistry, igual que los subsistemas en runtime. "
				"Este es tu panel de referencia para responder la pregunta: 'Esta todo configurado correctamente?'"))
			: FText::FromString(TEXT(
				"The Config Dashboard aggregates every PGX configuration DataAsset in your project, grouped by system. "
				"Instead of hunting through the Content Browser, you see all configs at a glance — which systems have configs, which are missing, and their resolved values.\n\n"
				"Double-click any entry to open the asset for editing. The dashboard auto-discovers configs using the AssetRegistry, just like the subsystems do at runtime. "
				"This is your go-to panel for answering the question: 'Is everything configured correctly?'"));
		Step.AccentColor = PGX::System::Config;
		Steps.Add(Step);
	}

	// --- Step 9: Data Registry Browser ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXDataRegistryBrowser"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Data Registry Browser — Catalogo de Datos Tipados"))
			: FText::FromString(TEXT("Data Registry Browser — Typed Data Catalog"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El Data Registry es el catalogo de datos tipados de PGX con lookup indexado por tags. "
				"Provee un lugar centralizado para registrar, descubrir, y consultar datos de juego — items, abilities, spawns, y cualquier contenido basado en DataTable.\n\n"
				"El browser te permite explorar entradas registradas, ver sus tags, previsualizar datos, y verificar que todo esta correctamente indexado. "
				"Al desacoplar el acceso a datos del conocimiento del sistema, el Registry permite que cualquier sistema consulte datos sin dependencias directas."))
			: FText::FromString(TEXT(
				"The Data Registry is PGX's typed data catalog with tag-indexed lookup. "
				"It provides a centralized place to register, discover, and query game data — items, abilities, spawns, and any DataTable-based content.\n\n"
				"The browser lets you explore registered entries, see their tags, preview data, and verify that everything is properly indexed. "
				"By decoupling data access from system knowledge, the Registry enables any system to query data without direct dependencies."));
		Step.AccentColor = PGX::System::DataRegistry;
		Steps.Add(Step);
	}

	// --- Step 10: Test Dashboard ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXTestDashboard"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Test Dashboard — Verificacion de Sistemas"))
			: FText::FromString(TEXT("Test Dashboard — System Verification"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El Test Dashboard te permite ejecutar tests de sistemas PGX y ver resultados por sistema. "
				"Los sistemas incluidos aportan utilidades de verificacion para sus contratos principales.\n\n"
				"Usa las pruebas disponibles para obtener evidencia enfocada antes y despues de un cambio. "
				"Usa este panel para verificar la integracion PGX de tu proyecto despues de cambios de configuracion o actualizaciones del framework."))
			: FText::FromString(TEXT(
				"The Test Dashboard lets you run PGX system tests and view per-system results. "
				"Included systems provide verification utilities for their principal contracts.\n\n"
				"Use the available tests to collect focused evidence before and after a change. "
				"Use this panel to verify your project's PGX integration after making configuration changes or updating the framework."));
		Step.AccentColor = PGX::Semantic::Info;
		Steps.Add(Step);
	}

	// --- Step 11: Summary ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = NAME_None;
		Step.bOpenTab = false;
		Step.Title = bES
			? FText::FromString(TEXT("Tour Completado!"))
			: FText::FromString(TEXT("Tour Complete!"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"Felicidades! Has explorado todas las herramientas principales del editor PGX. "
				"Ahora sabes donde encontrar estado de sistemas (System Observer), configuracion (Config Dashboard), persistencia (Save Inspector), "
				"estados de juego (GameFlow Inspector), diagnosticos (Log Viewer), y gestion de datos (Data Registry Browser).\n\n"
				"Para profundizar en cualquier sistema, usa el panel PGX Docs accesible desde el toolbar. "
				"Siguiente tutorial recomendado: T2 'Zero-Config Game Loop' para crear tu primera configuracion PGX desde cero."))
			: FText::FromString(TEXT(
				"Congratulations! You have explored all the major PGX editor tools. "
				"You now know where to find system status (System Observer), configuration (Config Dashboard), persistence (Save Inspector), "
				"game states (GameFlow Inspector), diagnostics (Log Viewer), and data management (Data Registry Browser).\n\n"
				"For deep dives into any system, use the PGX Docs panel accessible from the toolbar. "
				"Next recommended tutorial: T2 'Zero-Config Game Loop' to create your first PGX configuration from scratch."));
		Step.AccentColor = PGX::Semantic::Info;
		Steps.Add(Step);
	}

	return Steps;
}

// ============================================================================
// T2: Zero-Config Game Loop — Constructor Agent (10 steps)
// EN: Creates a basic game loop configuration: Profile + GameFlow + Save.
// ES: Crea una configuracion basica de game loop: Profile + GameFlow + Save.
// ============================================================================

TArray<FPGXTutorialStep> PGXOnboardingTutorials::GetT2_ZeroConfigGameLoop(EPGXTutorialLanguage Lang)
{
	const bool bES = (Lang == EPGXTutorialLanguage::Spanish);
	TArray<FPGXTutorialStep> Steps;
	Steps.Reserve(10);

	// --- Step 1: ConfigBasePath ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = NAME_None;
		Step.bOpenTab = false;
		Step.Action = EPGXTutorialAction::ConfigBasePath;
		Step.Title = bES
			? FText::FromString(TEXT("Configurar Ruta Base del Tutorial"))
			: FText::FromString(TEXT("Configure Tutorial Base Path"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"Antes de crear cualquier asset, necesitas establecer la carpeta base donde se organizara todo el contenido del tutorial. "
				"Esta ruta se usara como raiz para todas las carpetas y DataAssets creados durante este tutorial.\n\n"
				"Elige una ubicacion dentro del directorio Content de tu proyecto — por ejemplo '/Game/PGX/' o '/Game/Config/PGX/'. "
				"Todos los assets creados por el tutorial se pueden limpiar facilmente eliminando esta unica carpeta cuando termines."))
			: FText::FromString(TEXT(
				"Before creating any assets, you need to set the base folder where all tutorial content will be organized. "
				"This path will be used as the root for all folders and DataAssets created during this tutorial.\n\n"
				"Choose a location inside your project's Content directory — for example '/Game/PGX/' or '/Game/Config/PGX/'. "
				"All tutorial-created assets can be easily cleaned up by deleting this single folder when you are done."));
		Step.AccentColor = PGX::Semantic::Info;
		Steps.Add(Step);
	}

	// --- Step 2: Create base folder ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = NAME_None;
		Step.bOpenTab = false;
		Step.Action = EPGXTutorialAction::CreateFolder;
		Step.ActionPath = TEXT("");
		Step.Title = bES
			? FText::FromString(TEXT("Crear Carpeta de Configuracion del Proyecto"))
			: FText::FromString(TEXT("Create Project Configuration Folder"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"Vamos a crear la carpeta raiz para los assets de configuracion PGX de tu proyecto. "
				"PGX sigue una filosofia data-driven: cada sistema descubre su configuracion via el AssetRegistry al inicializarse.\n\n"
				"Esto significa que puedes organizar tus assets de config en cualquier parte del proyecto — PGX los encontrara automaticamente. "
				"Sin embargo, mantenerlos organizados en una sola jerarquia hace tu proyecto mas limpio y facil de manejar."))
			: FText::FromString(TEXT(
				"We will create the root folder for your PGX configuration assets. "
				"PGX follows a data-driven philosophy: every system discovers its configuration via the AssetRegistry at initialization.\n\n"
				"This means you can organize your config assets anywhere in the project — PGX will find them automatically. "
				"However, keeping them organized in a single hierarchy makes your project cleaner and easier to manage."));
		Step.AccentColor = PGX::Semantic::Info;
		Steps.Add(Step);
	}

	// --- Step 3: Create ProfileConfig DA ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = NAME_None;
		Step.bOpenTab = false;
		Step.Action = EPGXTutorialAction::CreateAsset;
		Step.ActionPath = TEXT("");
		Step.AssetClass = TEXT("/Script/PGXCoreRuntime.PGXProjectProfileConfig");
		Step.AssetName = TEXT("TutorialProfileConfig");
		Step.Title = bES
			? FText::FromString(TEXT("Crear Configuracion de Profile"))
			: FText::FromString(TEXT("Create Profile Configuration"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El sistema Profile es el primero en inicializarse y establece la identidad de tu proyecto. "
				"UPGXProjectProfileConfig define plataformas objetivo, presupuestos de rendimiento, y restricciones de capacidad que todos los demas sistemas PGX respetan.\n\n"
				"Con un config por defecto, PGX auto-detecta tu plataforma y aplica valores sensatos. "
				"Despues puedes ajustar presupuestos de memoria, canales de audio, slots de guardado, y mas — todo aplicado automaticamente en 11 subsistemas."))
			: FText::FromString(TEXT(
				"The Profile system is the first to initialize and establishes your project's identity. "
				"UPGXProjectProfileConfig defines platform targets, performance budgets, and capability constraints that all other PGX systems respect.\n\n"
				"With a default config, PGX auto-detects your platform and applies sensible defaults. "
				"Later you can fine-tune budgets for memory, audio channels, save slots, and more — all enforced automatically across 11 subsystems."));
		Step.AccentColor = PGX::System::Profile;
		Steps.Add(Step);
	}

	// --- Step 4: Open Profile Inspector ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXProfileInspector"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Verificar Configuracion de Profile"))
			: FText::FromString(TEXT("Verify Profile Configuration"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El Profile Inspector ahora muestra tu configuracion recien creada resuelta y activa. "
				"Nota como el sistema automaticamente detecto tu plataforma de desarrollo y aplico el perfil de presupuesto correspondiente.\n\n"
				"La seccion de enforcement de presupuestos muestra que subsistemas estan respetando las restricciones del perfil. "
				"Este es el poder del enfoque zero-config de PGX: creas el DataAsset, y el sistema se configura solo."))
			: FText::FromString(TEXT(
				"The Profile Inspector now shows your newly created configuration resolved and active. "
				"Notice how the system automatically detected your development platform and applied the matching budget profile.\n\n"
				"The budget enforcement section shows which subsystems are respecting the profile's constraints. "
				"This is the power of PGX's zero-config approach: create the DataAsset, and the system configures itself."));
		Step.AccentColor = PGX::System::Profile;
		Steps.Add(Step);
	}

	// --- Step 5: Create GameFlowConfig DA ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = NAME_None;
		Step.bOpenTab = false;
		Step.Action = EPGXTutorialAction::CreateAsset;
		Step.ActionPath = TEXT("GameFlow");
		Step.AssetClass = TEXT("/Script/PGXGameFlowRuntime.PGXGameFlowConfig");
		Step.AssetName = TEXT("TutorialGameFlowConfig");
		Step.Title = bES
			? FText::FromString(TEXT("Crear Configuracion de GameFlow"))
			: FText::FromString(TEXT("Create GameFlow Configuration"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"GameFlow es la maquina de estados de tu juego — rastrea en que fase esta (MainMenu, Loading, InGame, Paused, etc.) a traves de 8 canales. "
				"UPGXGameFlowConfig define estados iniciales, transiciones validas, y configuracion de canales.\n\n"
				"Con un config por defecto obtienes rastreo de estados inmediato y eventos de transicion que otros sistemas reciben via el bus de Mensajes. "
				"Sin enums, sin switch statements — solo GameplayTags que extiendes con tus propios estados especificos del juego."))
			: FText::FromString(TEXT(
				"GameFlow is your game's state machine — it tracks what phase your game is in (MainMenu, Loading, InGame, Paused, etc.) across 8 channels. "
				"UPGXGameFlowConfig defines initial states, valid transitions, and channel configuration.\n\n"
				"With a default config you get immediate state tracking and transition events that other systems receive via the Message bus. "
				"No enums, no switch statements — just GameplayTags that you extend with your own game-specific states."));
		Step.AccentColor = PGX::System::GameFlow;
		Steps.Add(Step);
	}

	// --- Step 6: Open GameFlow Inspector ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXGameFlowInspector"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Explorar Estados de GameFlow"))
			: FText::FromString(TEXT("Explore GameFlow States"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El GameFlow Inspector muestra el estado actual de los 8 canales y el historial de transiciones. "
				"Cada canal opera independientemente, permitiendote rastrear fase de juego, estado de UI, contexto de audio, y mas simultaneamente.\n\n"
				"En este punto los canales muestran sus estados iniciales por defecto. En un juego en ejecucion verias transiciones en tiempo real conforme tu logica las dispara. "
				"Las Flow Rules (configurables via UPGXFlowRulesConfig) pueden aplicar restricciones como 'No puede entrar a InGame sin completar Loading'."))
			: FText::FromString(TEXT(
				"The GameFlow Inspector displays the current state of all 8 channels and the transition history. "
				"Each channel operates independently, allowing you to track game phase, UI state, audio context, and more simultaneously.\n\n"
				"At this point channels show their default initial states. In a running game you would see real-time transitions as your game logic fires them. "
				"Flow Rules (configurable via UPGXFlowRulesConfig) can enforce constraints like 'Cannot enter InGame without completing Loading'."));
		Step.AccentColor = PGX::System::GameFlow;
		Steps.Add(Step);
	}

	// --- Step 7: Create SaveConfig DA ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = NAME_None;
		Step.bOpenTab = false;
		Step.Action = EPGXTutorialAction::CreateAsset;
		Step.ActionPath = TEXT("Save");
		Step.AssetClass = TEXT("/Script/PGXSaveRuntime.PGXSaveConfig");
		Step.AssetName = TEXT("TutorialSaveConfig");
		Step.Title = bES
			? FText::FromString(TEXT("Crear Configuracion de Save"))
			: FText::FromString(TEXT("Create Save Configuration"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El sistema Save provee persistencia estructurada del juego con slots configurables, dominios, y programacion de auto-save. "
				"UPGXSaveConfig define el numero de save slots, particionamiento de dominios, configuracion de compresion, e intervalos de auto-save.\n\n"
				"Los dominios te permiten separar progreso del jugador de configuraciones y estado del mundo — cada uno guardable independientemente. "
				"Con el config por defecto inmediatamente tienes un sistema de save funcional en el que tus objetos participan via IPGXSaveable."))
			: FText::FromString(TEXT(
				"The Save system provides structured game persistence with configurable slots, domains, and auto-save scheduling. "
				"UPGXSaveConfig defines the number of save slots, domain partitioning, compression settings, and auto-save intervals.\n\n"
				"Domains let you separate player progress from settings from world state — each saveable independently. "
				"With the default config you immediately get a working save system that your game objects can participate in via IPGXSaveable."));
		Step.AccentColor = PGX::System::Save;
		Steps.Add(Step);
	}

	// --- Step 8: Open Save Inspector ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXSaveInspector"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Verificar Sistema de Save"))
			: FText::FromString(TEXT("Verify Save System"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El Save Inspector ahora refleja tu nueva configuracion con sus slots y estructura de dominios visibles. "
				"Puedes ver los save slots disponibles, su estado actual (vacio/ocupado), y la programacion de auto-save.\n\n"
				"Durante gameplay este inspector se actualiza en tiempo real mostrando operaciones de guardado, flags de dominio dirty, y metricas de serializacion. "
				"Has creado las tres configs fundamentales: Profile define las reglas, GameFlow rastrea el estado, y Save lo persiste."))
			: FText::FromString(TEXT(
				"The Save Inspector now reflects your new save configuration with its slots and domain structure visible. "
				"You can see the available save slots, their current state (empty/occupied), and the auto-save schedule.\n\n"
				"During gameplay this inspector updates in real-time showing save operations, domain dirty flags, and serialization metrics. "
				"You have now created the three foundational configs: Profile defines the rules, GameFlow tracks the state, and Save persists it."));
		Step.AccentColor = PGX::System::Save;
		Steps.Add(Step);
	}

	// --- Step 9: Open System Observer ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXSystemObserver"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Ve tus Sistemas Funcionando Juntos"))
			: FText::FromString(TEXT("See Your Systems Running Together"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El System Observer ahora muestra Profile, GameFlow, y Save todos inicializados con sus configuraciones descubiertas. "
				"Nota como cada sistema reporta su estado de config independientemente — la topologia estrella de PGX significa que no dependen entre si.\n\n"
				"El orden de inicializacion asegura que los presupuestos de Profile esten disponibles antes de que GameFlow inicie, y el estado de GameFlow este listo antes de que Save comience. "
				"Con solo tres DataAssets tienes una base completa de game loop: identidad, gestion de estado, y persistencia."))
			: FText::FromString(TEXT(
				"The System Observer now shows Profile, GameFlow, and Save all initialized with their configurations discovered. "
				"Notice how each system reports its config status independently — PGX's star topology means they don't depend on each other.\n\n"
				"The initialization order ensures Profile's budgets are available before GameFlow starts, and GameFlow's state is ready before Save begins. "
				"With just three DataAssets you have a complete game loop foundation: identity, state management, and persistence."));
		Step.AccentColor = PGX::Semantic::Info;
		Steps.Add(Step);
	}

	// --- Step 10: Summary ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = NAME_None;
		Step.bOpenTab = false;
		Step.Title = bES
			? FText::FromString(TEXT("Game Loop Listo!"))
			: FText::FromString(TEXT("Game Loop Ready!"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"Has creado un game loop zero-config completo con solo 3 DataAssets: Profile, GameFlow, y Save. "
				"Cada sistema auto-descubrio su configuracion y se inicializo en el orden correcto sin ningun cambio de codigo.\n\n"
				"Los assets del tutorial fueron creados en tu ruta base configurada y pueden eliminarse cuando ya no los necesites. "
				"Siguiente tutorial recomendado: T3 'Data-Driven Workflow' para agregar configuraciones de audio, loading, y PSO a tu proyecto."))
			: FText::FromString(TEXT(
				"You have created a complete zero-config game loop with just 3 DataAssets: Profile, GameFlow, and Save. "
				"Each system auto-discovered its configuration and initialized in the correct order without any code changes.\n\n"
				"The tutorial assets were created in your configured base path and can be deleted when no longer needed. "
				"Next recommended tutorial: T3 'Data-Driven Workflow' to add audio, loading, and PSO configurations to your project."));
		Step.AccentColor = PGX::Semantic::Info;
		Steps.Add(Step);
	}

	return Steps;
}

// ============================================================================
// T3: Data-Driven Workflow — Constructor Agent (8 steps)
// EN: Shows how PGX uses DataAssets for everything: Audio, Loading, PSO configs.
// ES: Muestra como PGX usa DataAssets para todo: Audio, Loading, PSO configs.
// ============================================================================

TArray<FPGXTutorialStep> PGXOnboardingTutorials::GetT3_DataDrivenWorkflow(EPGXTutorialLanguage Lang)
{
	const bool bES = (Lang == EPGXTutorialLanguage::Spanish);
	TArray<FPGXTutorialStep> Steps;
	Steps.Reserve(8);

	// --- Step 1: ConfigBasePath ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = NAME_None;
		Step.bOpenTab = false;
		Step.Action = EPGXTutorialAction::ConfigBasePath;
		Step.Title = bES
			? FText::FromString(TEXT("Configurar Ruta Base del Tutorial"))
			: FText::FromString(TEXT("Configure Tutorial Base Path"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"Establece la carpeta base donde se crearan los DataAssets de este tutorial. "
				"Si completaste T2, puedes reutilizar la misma ruta base para mantener todo el contenido tutorial organizado junto.\n\n"
				"La filosofia data-driven de PGX significa que el comportamiento de cada sistema es controlado por DataAssets, no por codigo. "
				"En este tutorial crearas configs para Audio, Loading, y PSO para ver como cada sistema se auto-configura."))
			: FText::FromString(TEXT(
				"Set the base folder where this tutorial's DataAssets will be created. "
				"If you completed T2, you can reuse the same base path to keep all tutorial content organized together.\n\n"
				"PGX's data-driven philosophy means every system's behavior is controlled by DataAssets, not code. "
				"In this tutorial you will create configs for Audio, Loading, and PSO to see how each system self-configures."));
		Step.AccentColor = PGX::Semantic::Info;
		Steps.Add(Step);
	}

	// --- Step 2: Create AudioConfig DA ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = NAME_None;
		Step.bOpenTab = false;
		Step.Action = EPGXTutorialAction::CreateAsset;
		Step.ActionPath = TEXT("Audio");
		Step.AssetClass = TEXT("/Script/PGXAudioRuntime.PGXAudioConfig");
		Step.AssetName = TEXT("TutorialAudioConfig");
		Step.Title = bES
			? FText::FromString(TEXT("Crear Configuracion de Audio"))
			: FText::FromString(TEXT("Create Audio Configuration"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"PGX Audio es un sistema de dual-backend soportando tanto MetaSound como Sound Cues legacy a traves de un API unificado. "
				"UPGXAudioConfig define presupuestos de canales, configuracion de crossfade de musica, politicas de cola de dialogos, y seleccion de backend.\n\n"
				"El sistema respeta los presupuestos de Profile automaticamente — si tu config de plataforma limita canales de audio, Audio lo aplica. "
				"Con este unico DataAsset obtienes una capa de gestion de audio completamente funcional incluyendo transiciones de musica, cola de dialogos, y audio espacial."))
			: FText::FromString(TEXT(
				"PGX Audio is a dual-backend system supporting both MetaSound and legacy Sound Cues through a unified API. "
				"UPGXAudioConfig defines channel budgets, music crossfade settings, dialogue queue policies, and backend selection.\n\n"
				"The system respects Profile budgets automatically — if your platform config limits audio channels, Audio enforces it. "
				"With this single DataAsset you get a fully functional audio management layer including music transitions, dialogue queuing, and spatial audio."));
		Step.AccentColor = PGX::System::Audio;
		Steps.Add(Step);
	}

	// --- Step 3: Create LoadingConfig DA ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = NAME_None;
		Step.bOpenTab = false;
		Step.Action = EPGXTutorialAction::CreateAsset;
		Step.ActionPath = TEXT("Loading");
		Step.AssetClass = TEXT("/Script/PGXLoadingRuntime.PGXLoadingConfig");
		Step.AssetName = TEXT("TutorialLoadingConfig");
		Step.Title = bES
			? FText::FromString(TEXT("Crear Configuracion de Loading"))
			: FText::FromString(TEXT("Create Loading Configuration"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El sistema Loading maneja pantallas de carga, rastreo de progreso, y coordinacion de transiciones de nivel. "
				"UPGXLoadingConfig define widgets de pantalla de carga, tiempo minimo de display, frecuencia de actualizacion de progreso, y rotacion de tips.\n\n"
				"Loading trabaja de cerca con GameFlow y PSO: espera las transiciones de estado de GameFlow y el warm-up de pipelines PSO antes de ocultar la pantalla de carga. "
				"Estas dependencias L2-a-L2 son las unicas excepciones a la topologia estrella de PGX, validadas y documentadas en la arquitectura."))
			: FText::FromString(TEXT(
				"The Loading system manages loading screens, progress tracking, and level transition coordination. "
				"UPGXLoadingConfig defines loading screen widgets, minimum display time, progress update frequency, and tip rotation.\n\n"
				"Loading works closely with GameFlow and PSO: it waits for GameFlow state transitions and PSO pipeline warm-up before hiding the loading screen. "
				"These L2-to-L2 dependencies are the only exceptions to PGX's star topology, validated and documented in the architecture."));
		Step.AccentColor = PGX::System::Loading;
		Steps.Add(Step);
	}

	// --- Step 4: Create PSOWarmUpConfig DA ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = NAME_None;
		Step.bOpenTab = false;
		Step.Action = EPGXTutorialAction::CreateAsset;
		Step.ActionPath = TEXT("PSO");
		Step.AssetClass = TEXT("/Script/PGXPSORuntime.PGXPSOWarmUpConfig");
		Step.AssetName = TEXT("TutorialPSOConfig");
		Step.Title = bES
			? FText::FromString(TEXT("Crear Configuracion de Warm-Up PSO"))
			: FText::FromString(TEXT("Create PSO Warm-Up Configuration"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El sistema PSO (Pipeline State Object) elimina los stutters de compilacion de shaders pre-cacheando pipelines antes de que sean necesarios. "
				"UPGXPSOWarmUpConfig define que contextos de pipeline precalentar, configuracion del modo de grabacion, y umbrales de timeout de warm-up.\n\n"
				"Durante pantallas de carga PSO se coordina con el sistema Loading para compilar todos los shaders necesarios antes de que el jugador vea el nivel. "
				"Esto es critico para juegos de produccion — los stutters de PSO son la queja de calidad visual mas comun en proyectos UE5."))
			: FText::FromString(TEXT(
				"The PSO (Pipeline State Object) system eliminates shader compilation stutters by pre-caching pipelines before they are needed. "
				"UPGXPSOWarmUpConfig defines which pipeline contexts to warm up, recording mode settings, and warm-up timeout thresholds.\n\n"
				"During loading screens PSO coordinates with the Loading system to compile all needed shaders before the player sees the level. "
				"This is critical for shipping games — PSO stutters are the most common visual quality complaint in UE5 projects."));
		Step.AccentColor = PGX::System::PSO;
		Steps.Add(Step);
	}

	// --- Step 5: Open Config Dashboard ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXConfigDashboard"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Ver Todas las Configuraciones"))
			: FText::FromString(TEXT("View All Configurations"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El Config Dashboard ahora muestra las configuraciones de Audio, Loading, y PSO que acabas de crear, junto con cualquier config del T2. "
				"Nota como cada entrada esta agrupada por sistema con su color de acento — esta organizacion visual hace obvio que sistemas estan configurados.\n\n"
				"Cada DataAsset de configuracion PGX sigue el mismo patron: crealo en cualquier parte de Content, y el sistema lo encuentra via escaneo del AssetRegistry. "
				"Este es el nucleo de la filosofia data-driven de PGX — sin cambios de codigo, sin llamadas de registro, solo crea el DataAsset y funciona."))
			: FText::FromString(TEXT(
				"The Config Dashboard now shows the Audio, Loading, and PSO configurations you just created, alongside any configs from T2. "
				"Notice how each entry is grouped by system with its accent color — this visual organization makes it obvious which systems are configured.\n\n"
				"Every PGX configuration DataAsset follows the same pattern: create it anywhere in Content, and the system finds it via AssetRegistry scan. "
				"This is the core of PGX's data-driven philosophy — no code changes, no registration calls, just create the DataAsset and it works."));
		Step.AccentColor = PGX::System::Config;
		Steps.Add(Step);
	}

	// --- Step 6: Open Data Registry Browser ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXDataRegistryBrowser"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Explorar el Data Registry"))
			: FText::FromString(TEXT("Explore the Data Registry"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El Data Registry Browser muestra como PGX cataloga datos tipados usando una arquitectura DataTable-first con lookup indexado por tags. "
				"Mientras que los DataAssets de configuracion controlan el comportamiento del sistema, el Data Registry maneja contenido de juego — items, abilities, definiciones de spawn, y mas.\n\n"
				"Ambos siguen el mismo principio: data-driven, auto-descubiertos, y desacoplados de los sistemas que los consumen. "
				"Conforme construyes tu juego registraras tus tipos de datos custom aqui para acceso centralizado desde cualquier sistema."))
			: FText::FromString(TEXT(
				"The Data Registry Browser shows how PGX catalogs typed data using a DataTable-first architecture with tag-indexed lookup. "
				"While configuration DataAssets control system behavior, the Data Registry handles game content — items, abilities, spawn definitions, and more.\n\n"
				"Both follow the same principle: data-driven, auto-discovered, and decoupled from the systems that consume them. "
				"As you build your game you will register your custom data types here for centralized access from any system."));
		Step.AccentColor = PGX::System::DataRegistry;
		Steps.Add(Step);
	}

	// --- Step 7: Navigate CB to tutorial folder ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = NAME_None;
		Step.bOpenTab = false;
		Step.Action = EPGXTutorialAction::NavigateCB;
		Step.ActionPath = TEXT("");
		Step.Title = bES
			? FText::FromString(TEXT("Revisar Assets Creados"))
			: FText::FromString(TEXT("Review Created Assets"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"Naveguemos al Content Browser para ver todos los DataAssets creados en esta sesion en tu carpeta de tutorial. "
				"Cada asset sigue la convencion de nombres de PGX: el nombre de la clase indica su sistema, y la estructura de carpetas refleja la organizacion del sistema.\n\n"
				"Puedes hacer doble click en cualquier asset para abrirlo en el panel Details y editar sus propiedades — todos los cambios toman efecto en la siguiente inicializacion. "
				"Este es el workflow completo de PGX: crea el DataAsset, configura sus propiedades, y el sistema maneja el resto."))
			: FText::FromString(TEXT(
				"Let's navigate the Content Browser to see all the DataAssets created in this session in your tutorial folder. "
				"Each asset follows the PGX naming convention: the class name indicates its system, and the folder structure mirrors the system organization.\n\n"
				"You can double-click any asset to open it in the Details panel and edit its properties — all changes take effect at the next initialization. "
				"This is the full PGX workflow: create the DataAsset, configure its properties, and the system handles the rest."));
		Step.AccentColor = PGX::Semantic::Info;
		Steps.Add(Step);
	}

	// --- Step 8: Summary ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = NAME_None;
		Step.bOpenTab = false;
		Step.Title = bES
			? FText::FromString(TEXT("Workflow Data-Driven Completado!"))
			: FText::FromString(TEXT("Data-Driven Workflow Complete!"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"Has experimentado la filosofia data-driven de PGX de primera mano creando configuraciones de Audio, Loading, y PSO. "
				"Cada sistema PGX sigue el mismo patron: crea un DataAsset, establece propiedades, y el sistema se auto-configura.\n\n"
				"Sin codigo de registro, sin llamadas de inicializacion, sin cableado de dependencias — solo datos. "
				"Los assets del tutorial se pueden limpiar eliminando la carpeta base. Siguiente: T4 'Full Observability' para explorar todas las herramientas diagnosticas."))
			: FText::FromString(TEXT(
				"You have experienced PGX's data-driven philosophy firsthand by creating Audio, Loading, and PSO configurations. "
				"Every PGX system follows the same pattern: create a DataAsset, set properties, and the system auto-configures.\n\n"
				"No registration code, no initialization calls, no dependency wiring — just data. "
				"The tutorial assets can be cleaned up by deleting the tutorial base folder. Next: T4 'Full Observability' to explore all diagnostic tools."));
		Step.AccentColor = PGX::Semantic::Info;
		Steps.Add(Step);
	}

	return Steps;
}

// ============================================================================
// T4: Full Observability — Guide Agent (9 steps)
// EN: Tour of all diagnostic and monitoring tools.
// ES: Tour de todas las herramientas de diagnostico y monitoreo.
// ============================================================================

TArray<FPGXTutorialStep> PGXOnboardingTutorials::GetT4_FullObservability(EPGXTutorialLanguage Lang)
{
	const bool bES = (Lang == EPGXTutorialLanguage::Spanish);
	TArray<FPGXTutorialStep> Steps;
	Steps.Reserve(9);

	// --- Step 1: Welcome ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = NAME_None;
		Step.bOpenTab = false;
		Step.Title = bES
			? FText::FromString(TEXT("Observabilidad Completa — Ve Todo"))
			: FText::FromString(TEXT("Full Observability — See Everything"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"PGX provee observabilidad completa para cada sistema — nunca deberias tener que adivinar que esta pasando bajo el capo. "
				"Cada subsistema expone su estado interno a traves de paneles inspectores dedicados, metricas en tiempo real, y logging estructurado.\n\n"
				"Este tutorial te lleva por cada herramienta de observabilidad para que sepas exactamente donde mirar cuando debuggeas, perfilas, o verificas comportamiento. "
				"El principio es simple: si PGX hace algo, puedes verlo sucediendo."))
			: FText::FromString(TEXT(
				"PGX provides full observability for every system — you should never have to guess what is happening under the hood. "
				"Every subsystem exposes its internal state through dedicated inspector panels, real-time metrics, and structured logging.\n\n"
				"This tutorial walks you through every observability tool so you know exactly where to look when debugging, profiling, or verifying behavior. "
				"The principle is simple: if PGX does something, you can see it happening."));
		Step.AccentColor = PGX::Semantic::Info;
		Steps.Add(Step);
	}

	// --- Step 2: System Observer ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXSystemObserver"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("System Observer — La Vision General"))
			: FText::FromString(TEXT("System Observer — The Big Picture"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El System Observer es tu primera parada para diagnosticar cualquier problema — muestra el estado en tiempo real de los 13 subsistemas PGX en una sola vista. "
				"Cada fila muestra: estado de inicializacion, resultado de descubrimiento de config, estado activo, y contadores runtime clave.\n\n"
				"La columna de orden de inicializacion muestra la secuencia de boot forzada, haciendo los problemas de dependencia inmediatamente visibles. "
				"Si un sistema esta en rojo o no inicializado, aqui es donde empiezas a investigar — luego profundizas en el inspector dedicado de ese sistema."))
			: FText::FromString(TEXT(
				"The System Observer is your first stop for diagnosing any issue — it shows the real-time state of all 13 PGX subsystems in a single view. "
				"Each system row displays: initialization status, config discovery result, active state, and key runtime counters.\n\n"
				"The initialization order column shows the enforced boot sequence, making dependency issues immediately visible. "
				"If a system is red or uninitialized, this is where you start investigating — then drill down into that system's dedicated inspector."));
		Step.AccentColor = PGX::Semantic::Info;
		Steps.Add(Step);
	}

	// --- Step 3: Log Viewer ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXLogViewer"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Log Viewer — Diagnosticos Estructurados"))
			: FText::FromString(TEXT("Log Viewer — Structured Diagnostics"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"PGX Log v3.0 va mas alla de la salida de texto — provee renderers polimorficos especificos por dominio que entienden la estructura de cada entrada de log. "
				"Filtra por categoria de sistema (LogPGXSave, LogPGXGameFlow, etc.), nivel de severidad, y rango de tiempo para encontrar exactamente lo que necesitas.\n\n"
				"La funcionalidad de busqueda soporta patrones regex en todos los campos, y la funcion de exportacion captura sesiones diagnosticas para analisis offline. "
				"Cada sistema PGX usa su propia categoria de log (regla S7: Logging Hygiene), asi que nunca tienes que navegar entre ruido irrelevante de LogTemp."))
			: FText::FromString(TEXT(
				"PGX Log v3.0 goes beyond text output — it provides polymorphic domain-specific renderers that understand the structure of each log entry. "
				"Filter by system category (LogPGXSave, LogPGXGameFlow, etc.), severity level, and time range to find exactly what you need.\n\n"
				"The search functionality supports regex patterns across all fields, and the export feature captures diagnostic sessions for offline analysis. "
				"Every PGX system uses its own log category (S7: Logging Hygiene rule), so you never have to wade through irrelevant LogTemp noise."));
		Step.AccentColor = PGX::System::Log;
		Steps.Add(Step);
	}

	// --- Step 4: MGOS Inspector ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXMGOSInspector"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("MGOS Inspector — Observacion de Memoria y GC"))
			: FText::FromString(TEXT("MGOS Inspector — Memory & GC Observation"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"MGOS (Memory and GC Observation System) es el subsistema a nivel engine de PGX que monitorea garbage collection y presion de memoria. "
				"Opera en 4 modos: Passive (monitoreo de bajo overhead), Active (rastreo de frecuencia GC), Profiling (analisis detallado de allocations), y Emergency (deteccion de leaks).\n\n"
				"El motor de perfil de 6 estados transiciona automaticamente basado en condiciones detectadas — desde Idle pasando por Warning hasta Critical. "
				"Usa este panel para entender el comportamiento de memoria de tu juego, identificar picos de GC, y atrapar memory leaks antes de que se shippen."))
			: FText::FromString(TEXT(
				"MGOS (Memory and GC Observation System) is PGX's engine-level subsystem that monitors garbage collection and memory pressure. "
				"It operates in 4 modes: Passive (low overhead monitoring), Active (GC frequency tracking), Profiling (detailed allocation analysis), and Emergency (leak detection).\n\n"
				"The 6-state profile engine transitions automatically based on detected conditions — from Idle through Warning to Critical. "
				"Use this panel to understand your game's memory behavior, identify GC spikes, and catch memory leaks before they ship."));
		Step.AccentColor = PGX::System::MGOS;
		Steps.Add(Step);
	}

	// --- Step 5: Test Dashboard ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXTestDashboard"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Test Dashboard — Verifica Todo"))
			: FText::FromString(TEXT("Test Dashboard — Verify Everything"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El Test Dashboard provee ejecucion de tests por sistema con resultados claros de pass/fail y salida detallada. "
				"Los sistemas incluidos aportan utilidades de test para los contratos que implementan.\n\n"
				"Ejecuta la evidencia disponible en el target correspondiente y revisa cada resultado en su contexto. "
				"Ejecuta tests despues de cambios de configuracion o actualizaciones del framework para verificar que tu integracion es saludable — verde en todas las filas significa que todo funciona."))
			: FText::FromString(TEXT(
				"The Test Dashboard provides per-system test execution with clear pass/fail results and detailed output. "
				"Included systems provide test utilities for the contracts they implement.\n\n"
				"Run the available evidence in the corresponding target and review each result in context. "
				"Run tests after configuration changes or framework updates to verify your integration is healthy — green across the board means everything works."));
		Step.AccentColor = PGX::Semantic::Info;
		Steps.Add(Step);
	}

	// --- Step 6: Profile Inspector + Platform Health ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXPlatformHealthDashboard"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Platform Health — Aplicacion de Presupuestos"))
			: FText::FromString(TEXT("Platform Health — Budget Enforcement"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El Platform Health Dashboard muestra como el sistema de presupuestos de Profile v2.0 aplica limites de recursos en los 11 subsistemas sensibles a budgets. "
				"Cada subsistema muestra su uso actual de recursos contra el presupuesto definido por el perfil — canales de audio, save slots, pools de memoria, y mas.\n\n"
				"La columna de deteccion de plataforma muestra la plataforma auto-detectada y que tier de presupuesto esta activo (Desktop, Console, Mobile). "
				"Si algun subsistema excede su presupuesto el dashboard lo resalta inmediatamente — previniendo problemas de rendimiento de llegar a produccion."))
			: FText::FromString(TEXT(
				"The Platform Health Dashboard shows how Profile v2.0's budget system enforces resource limits across all 11 budget-aware subsystems. "
				"Each subsystem displays its current resource usage against the profile's defined budget — audio channels, save slots, memory pools, and more.\n\n"
				"The platform detection column shows the auto-detected platform and which budget tier is active (Desktop, Console, Mobile). "
				"If any subsystem exceeds its budget the dashboard highlights it immediately — preventing performance issues from reaching production."));
		Step.AccentColor = PGX::System::Profile;
		Steps.Add(Step);
	}

	// --- Step 7: PSO Inspector ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXPSOInspector"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("PSO Inspector — Rastreo de Estado de Pipelines"))
			: FText::FromString(TEXT("PSO Inspector — Pipeline State Tracking"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El PSO Inspector te da visibilidad sobre el proceso de warm-up de Pipeline State Objects que previene stutters de compilacion de shaders. "
				"Muestra contextos de warm-up (por nivel, por categoria de material), progreso de compilacion, tasas de cache hit, y sesiones de grabacion activas.\n\n"
				"Durante una sesion de grabacion el inspector rastrea que pipelines se estan compilando y los guarda para warm-up futuro. "
				"Los controles de pause/resume te permiten manejar el timing de warm-up, y la vista de contextos muestra que conjuntos de pipelines estan listos para cada escenario de juego."))
			: FText::FromString(TEXT(
				"The PSO Inspector gives you visibility into the Pipeline State Object warm-up process that prevents shader compilation stutters. "
				"It shows warm-up contexts (per-level, per-material category), compilation progress, cache hit rates, and active recording sessions.\n\n"
				"During a recording session the inspector tracks which pipelines are being compiled and saves them for future warm-up. "
				"The pause/resume controls let you manage warm-up timing, and the context view shows which pipeline sets are ready for each game scenario."));
		Step.AccentColor = PGX::System::PSO;
		Steps.Add(Step);
	}

	// --- Step 8: Event Debugger ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXEventDebugger"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Event Debugger — Telemetria de Comportamiento"))
			: FText::FromString(TEXT("Event Debugger — Behavior Telemetry"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El Event Debugger expone el pipeline de resolucion de comportamiento del sistema EventHandler, mostrando registros de handlers, orden de ejecucion, y telemetria. "
				"Cada dispatch de evento se registra con su cadena de handlers, resolucion de prioridad, y tiempo de ejecucion — dandote una imagen completa del comportamiento del sistema.\n\n"
				"La funcionalidad de blackbox graba los ultimos N dispatches de eventos para debugging post-mortem cuando algo sale mal. "
				"Combinado con el Message Inspector tienes visibilidad completa sobre los dos sistemas de comunicacion de PGX: pub/sub (Messages) y resolucion de comportamiento (Events)."))
			: FText::FromString(TEXT(
				"The Event Debugger exposes the EventHandler system's behavior resolution pipeline, showing handler registrations, execution order, and telemetry. "
				"Every event dispatch is logged with its handler chain, priority resolution, and execution time — giving you a complete picture of system behavior.\n\n"
				"The blackbox feature records the last N event dispatches for post-mortem debugging when something goes wrong. "
				"Combined with the Message Inspector you have full visibility into PGX's two communication systems: pub/sub (Messages) and behavior resolution (Events)."));
		Step.AccentColor = PGX::System::EventHandler;
		Steps.Add(Step);
	}

	// --- Step 9: Summary ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = NAME_None;
		Step.bOpenTab = false;
		Step.Title = bES
			? FText::FromString(TEXT("Observabilidad Completa!"))
			: FText::FromString(TEXT("Full Observability Complete!"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"Has recorrido cada herramienta de observabilidad en PGX: System Observer para la vision general, Log Viewer para diagnosticos estructurados, "
				"MGOS para memoria y GC, Test Dashboard para verificacion, Platform Health para aplicacion de presupuestos, PSO para rastreo de pipelines, y Event Debugger para telemetria de comportamiento.\n\n"
				"La conclusion clave: PGX nunca oculta lo que esta haciendo. Cada sistema, cada decision, cada transicion de estado es visible e inspeccionable. "
				"Siguiente tutorial recomendado: T5 'Decoupled Architecture' para entender como los sistemas PGX se comunican sin dependencias."))
			: FText::FromString(TEXT(
				"You have toured every observability tool in PGX: System Observer for the big picture, Log Viewer for structured diagnostics, "
				"MGOS for memory and GC, Test Dashboard for verification, Platform Health for budget enforcement, PSO for pipeline tracking, and Event Debugger for behavior telemetry.\n\n"
				"The key takeaway: PGX never hides what it is doing. Every system, every decision, every state transition is visible and inspectable. "
				"Next recommended tutorial: T5 'Decoupled Architecture' to understand how PGX systems communicate without dependencies."));
		Step.AccentColor = PGX::Semantic::Info;
		Steps.Add(Step);
	}

	return Steps;
}

// ============================================================================
// T5: Decoupled Architecture — Guide Agent (8 steps)
// EN: Shows how PGX systems communicate without cross-system dependencies.
// ES: Muestra como los sistemas PGX se comunican sin dependencias cruzadas.
// ============================================================================

TArray<FPGXTutorialStep> PGXOnboardingTutorials::GetT5_DecoupledArchitecture(EPGXTutorialLanguage Lang)
{
	const bool bES = (Lang == EPGXTutorialLanguage::Spanish);
	TArray<FPGXTutorialStep> Steps;
	Steps.Reserve(8);

	// --- Step 1: Welcome ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = NAME_None;
		Step.bOpenTab = false;
		Step.Title = bES
			? FText::FromString(TEXT("Topologia Estrella — Cero Dependencias Cruzadas"))
			: FText::FromString(TEXT("Star Topology — Zero Cross-Dependencies"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"PGX usa una topologia estrella donde los 21 plugins L2 dependen unicamente de PGXCoreRuntime — el centro L1. "
				"Ningun sistema L2 conoce a ningun otro sistema L2 a nivel de codigo. Se comunican exclusivamente a traves del bus de Mensajes y el sistema EventHandler en L1.\n\n"
				"Esta arquitectura significa que puedes agregar, remover, o reemplazar cualquier sistema sin romper los demas. "
				"Este tutorial te muestra los tres pilares del desacoplamiento: Mensajes basados en tags para pub/sub, EventHandlers data-driven para comportamiento, y DataAssets para configuracion."))
			: FText::FromString(TEXT(
				"PGX uses a star topology where all 21 L2 plugins depend only on PGXCoreRuntime — the L1 center. "
				"No L2 system knows about any other L2 system at the code level. They communicate exclusively through the Message bus and EventHandler system in L1.\n\n"
				"This architecture means you can add, remove, or replace any system without breaking others. "
				"This tutorial shows you the three pillars of decoupling: tag-based Messages for pub/sub, data-driven EventHandlers for behavior, and DataAssets for configuration."));
		Step.AccentColor = PGX::Semantic::Info;
		Steps.Add(Step);
	}

	// --- Step 2: Message Inspector ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXMessageInspector"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Message Inspector — Comunicacion Pub/Sub"))
			: FText::FromString(TEXT("Message Inspector — Pub/Sub Communication"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El sistema de Mensajes es el mecanismo principal de comunicacion entre sistemas de PGX — un bus pub/sub tipado con canales basados en tags. "
				"Cualquier sistema puede transmitir un mensaje en un canal GameplayTag sin saber quien esta escuchando. Cualquier sistema puede suscribirse sin saber quien esta enviando.\n\n"
				"El inspector muestra suscripciones activas, trafico reciente de mensajes, estadisticas de canales, y metricas de entrega. "
				"Asi es como GameFlow notifica a Audio de cambios de estado, como Save dispara actualizaciones de UI, y como Loading se coordina con PSO — todo sin referencias directas."))
			: FText::FromString(TEXT(
				"The Message system is PGX's primary cross-system communication mechanism — a typed pub/sub bus with tag-based channels. "
				"Any system can broadcast a message on a GameplayTag channel without knowing who is listening. Any system can subscribe without knowing who is sending.\n\n"
				"The inspector shows active subscriptions, recent message traffic, channel statistics, and delivery metrics. "
				"This is how GameFlow notifies Audio of state changes, how Save triggers UI updates, and how Loading coordinates with PSO — all without direct references."));
		Step.AccentColor = PGX::System::Message;
		Steps.Add(Step);
	}

	// --- Step 3: Event Debugger ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXEventDebugger"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Event Debugger — Comportamiento Data-Driven"))
			: FText::FromString(TEXT("Event Debugger — Data-Driven Behavior"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"Mientras que los Mensajes son broadcasts de tipo fire-and-forget, el sistema EventHandler provee resolucion de comportamiento data-driven con prioridades. "
				"Multiples handlers pueden registrarse para el mismo tag de evento, y el sistema resuelve el orden de ejecucion basado en prioridad y configuracion del handler.\n\n"
				"Los handlers se definen como DataAssets — configuras comportamiento sin escribir codigo de dispatch o switch statements. "
				"El debugger muestra el registro de handlers, la cadena de resolucion para cada evento, y telemetria de ejecucion — haciendo lo invisible visible."))
			: FText::FromString(TEXT(
				"While Messages are fire-and-forget broadcasts, the EventHandler system provides data-driven behavior resolution with priorities. "
				"Multiple handlers can register for the same event tag, and the system resolves execution order based on priority and handler configuration.\n\n"
				"Handlers are defined as DataAssets — you configure behavior without writing dispatch code or switch statements. "
				"The debugger shows the handler registry, resolution chain for each event, and execution telemetry — making the invisible visible."));
		Step.AccentColor = PGX::System::EventHandler;
		Steps.Add(Step);
	}

	// --- Step 4: System Observer ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXSystemObserver"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Sistemas Independientes Corriendo en Paralelo"))
			: FText::FromString(TEXT("Independent Systems Running in Parallel"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"Mira el System Observer con el lente de desacoplamiento: cada sistema conserva su propio estado y expone evidencia mediante Core. "
				"Los sistemas runtime publicos dependen de PGXCoreRuntime en lugar de compilar directamente contra plugins hermanos.\n\n"
				"La coordinacion opcional utiliza mensajes tipados, Gameplay Tags, interfaces y registries compartidos."))
			: FText::FromString(TEXT(
				"Look at the System Observer through the decoupling lens: each system owns its state and exposes evidence through Core. "
				"Public runtime systems depend on PGXCoreRuntime rather than compiling directly against sibling plugins.\n\n"
				"Optional coordination uses typed messages, Gameplay Tags, interfaces and shared registries."));
		Step.AccentColor = PGX::Semantic::Info;
		Steps.Add(Step);
	}

	// --- Step 5: GameFlow Inspector ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXGameFlowInspector"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Modelo de Estado Solo con Tags"))
			: FText::FromString(TEXT("Tag-Only State Model"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"GameFlow demuestra la filosofia de solo-tags de PGX: los estados de juego son GameplayTags, no enums ni clases. "
				"Esto significa que puedes agregar nuevos estados de juego simplemente definiendo nuevos tags — sin recompilacion, sin nuevas clases, sin registro.\n\n"
				"Otros sistemas reaccionan a cambios de estado a traves del bus de Mensajes: Audio cambia tracks, UI actualiza pantallas, Save dispara checkpoints. "
				"Ninguno de ellos referencia GameFlow directamente — se suscriben a canales basados en tags y reaccionan a transiciones independientemente."))
			: FText::FromString(TEXT(
				"GameFlow demonstrates PGX's tag-only philosophy: game states are GameplayTags, not enums or classes. "
				"This means you can add new game states by simply defining new tags — no code recompilation, no new classes, no registration.\n\n"
				"Other systems react to state changes through the Message bus: Audio changes tracks, UI updates screens, Save triggers checkpoints. "
				"None of them reference GameFlow directly — they subscribe to tag-based channels and react to transitions independently."));
		Step.AccentColor = PGX::System::GameFlow;
		Steps.Add(Step);
	}

	// --- Step 6: Config Dashboard ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXConfigDashboard"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Los DataAssets Eliminan Dependencias de Codigo"))
			: FText::FromString(TEXT("DataAssets Eliminate Code Dependencies"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El Config Dashboard ilustra como la configuracion data-driven elimina la necesidad de referencias de codigo entre sistemas. "
				"Cada sistema descubre su propio DataAsset de config a traves del AssetRegistry — ningun sistema necesita conocer el tipo de config de otro sistema.\n\n"
				"Profile establece presupuestos que Audio respeta, pero Audio lee esos presupuestos desde su propio contexto de inicializacion — no incluyendo headers de Profile. "
				"Esta es la filosofia de 'la caja y la forma': el DataAsset es la interfaz universal entre el desarrollador y el sistema."))
			: FText::FromString(TEXT(
				"The Config Dashboard illustrates how data-driven configuration eliminates the need for cross-system code references. "
				"Each system discovers its own config DataAsset through the AssetRegistry — no system needs to know about another system's config type.\n\n"
				"Profile sets budgets that Audio respects, but Audio reads those budgets from its own initialization context — not by including Profile headers. "
				"This is the 'box and shape' philosophy: the DataAsset is the universal interface between the developer and the system."));
		Step.AccentColor = PGX::System::Config;
		Steps.Add(Step);
	}

	// --- Step 7: Data Registry Browser ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = FName(TEXT("PGXDataRegistryBrowser"));
		Step.bOpenTab = true;
		Step.Title = bES
			? FText::FromString(TEXT("Catalogo Tipado — Acceso a Datos Desacoplado"))
			: FText::FromString(TEXT("Typed Catalog — Decoupled Data Access"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"El Data Registry completa el panorama de desacoplamiento proveyendo un catalogo de datos centralizado e indexado por tags. "
				"Cualquier sistema puede consultar datos por tipo y tag sin saber cual sistema los registro o donde vive el DataTable.\n\n"
				"Esto significa que tu sistema de Inventario puede buscar definiciones de items, tu sistema de Spawn puede encontrar configuraciones de spawn, y tu sistema de AI puede acceder datos de comportamiento — todo a traves del mismo API del Registry. "
				"El Registry es el equivalente en la capa de datos del bus de Mensajes: desacopla productores de datos de consumidores de datos."))
			: FText::FromString(TEXT(
				"The Data Registry completes the decoupling picture by providing a centralized, tag-indexed data catalog. "
				"Any system can query data by type and tag without knowing which system registered it or where the DataTable lives.\n\n"
				"This means your Inventory system can look up item definitions, your Spawn system can find spawn configurations, and your AI system can access behavior data — all through the same Registry API. "
				"The Registry is the data layer equivalent of the Message bus: it decouples data producers from data consumers."));
		Step.AccentColor = PGX::System::DataRegistry;
		Steps.Add(Step);
	}

	// --- Step 8: Summary ---
	{
		FPGXTutorialStep Step;
		Step.TargetTabId = NAME_None;
		Step.bOpenTab = false;
		Step.Title = bES
			? FText::FromString(TEXT("Arquitectura Desacoplada Comprendida!"))
			: FText::FromString(TEXT("Decoupled Architecture Understood!"));
		Step.Description = bES
			? FText::FromString(TEXT(
				"Ahora entiendes los tres pilares de desacoplamiento de PGX: Mensajes para comunicacion entre sistemas, EventHandlers para resolucion de comportamiento, y DataAssets para configuracion. "
				"La topologia estrella asegura que ningun sistema L2 dependa de ningun otro sistema L2 — solo del core L1.\n\n"
				"Esta arquitectura te da la libertad de adoptar sistemas PGX incrementalmente, reemplazar sistemas individuales, o extenderlos sin miedo a romper el resto. "
				"Has completado los 5 tutoriales de onboarding! Visita el PGX Hub para explorar inspectores de sistemas individuales, o el panel PGX Docs para documentacion de arquitectura profunda."))
			: FText::FromString(TEXT(
				"You now understand PGX's three pillars of decoupling: Messages for cross-system communication, EventHandlers for behavior resolution, and DataAssets for configuration. "
				"The star topology ensures no L2 system depends on any other L2 system — only on the L1 core.\n\n"
				"This architecture gives you the freedom to adopt PGX systems incrementally, replace individual systems, or extend them without fear of breaking the rest. "
				"You have completed all 5 onboarding tutorials! Visit the PGX Hub to explore individual system inspectors, or the PGX Docs panel for deep architecture documentation."));
		Step.AccentColor = PGX::Semantic::Info;
		Steps.Add(Step);
	}

	return Steps;
}
