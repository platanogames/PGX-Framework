# PGX Framework

### Professional Game Extensions for Unreal Engine 5

> **v0.4.0** | **UE 5.6.1** | **25 modular plugins** | **13 production-ready systems** | **1,300+ source files**

---

## What Is PGX?

**PGX (Professional Game Extensions)** is a modular C++ framework for Unreal Engine 5. It provides a complete, production-ready architectural layer between raw UE5 and your game code.

PGX is **NOT** a game template -- it's **middleware**. Every system wraps the corresponding UE5 subsystem with a clean, documented, observable facade. You bring the game design; PGX provides the engineering foundation.

Think of it as the missing layer between "empty UE5 project" and "production-ready game architecture."

---

## Documentation

Comprehensive documentation is available through the **[PGX Wiki](../../wiki)**:

| Page | Description |
|------|-------------|
| **[FAQ](../../wiki/FAQ)** | Common questions from evaluators and potential users |
| **[Statistics](../../wiki/Statistics)** | Hard numbers verified against the current codebase |
| **[Glossary](../../wiki/Glossary)** | Key terms used throughout PGX documentation |
| **[Licensing & Distribution](../../wiki/Licensing-and-Distribution)** | Full license model, tiers, terms, and professional services |

Licensed users also receive 90+ documentation files, including architecture documents, usage guides, testing guides, extension guides, and 18 bilingual interactive tutorials.

---

## Why PGX Exists

I built PGX because after years of developing products for clients and auditing codebases across studios of every size, I kept seeing the same thing: every single project reinvents the same foundational systems from scratch. Save systems, audio management, loading screens, state machines, configuration pipelines -- rebuilt from zero, every time, by every team.

It frustrated me. In other ecosystems this was solved long ago. Python developers have Django. JavaScript developers have Next.js. Ruby developers have Rails. These frameworks handle the repetitive architectural work so you can focus on what makes your product unique. But Unreal Engine -- despite being one of the most powerful engines in the world -- had no equivalent. No middleware layer. No shared foundation.

And the cost of that gap is something I saw firsthand, project after project:

- **No architectural parity between studios.** Every team builds their own conventions, their own patterns, their own solutions to the same problems. A developer switching studios has to relearn everything from scratch.
- **Accumulated technical debt.** Custom implementations of common systems rarely get the polish, testing, and documentation they deserve under production pressure. They work "well enough" until they don't.
- **Slow onboarding.** New team members spend weeks understanding proprietary systems that could have been standardized from the start.
- **The same bugs, rediscovered.** Save corruption edge cases. Loading screens that destroy themselves during level transitions. Shader compilation hitches in the first 30 seconds. I kept seeing the same problems solved poorly in different ways.

I decided it was time to build what should have existed from the beginning: a standardized, extensible, production-tested layer that sits **on top of** Unreal Engine -- never replacing it, always complementing and expanding it. Every PGX system wraps the corresponding UE5 subsystem with a clean, documented, observable facade. The architecture, the patterns, the tooling, the documentation -- built once, built right, so every team that uses it can skip the years of foundational work and focus entirely on what makes their game different.

### Who Benefits

**For solo developers:** PGX gives you the architecture of a 20-person studio from day one.

**For teams:** PGX provides a shared foundation so everyone speaks the same language -- same patterns, same conventions, same tools.

**For studios:** PGX is a reusable investment. Build once, use across every project.

---

## The Plugin Ecosystem

### Foundation

| Plugin | Description |
|--------|-------------|
| **PGXCore** | Base classes, message bus, event handler bus, state machine, configuration, data registry, object pool, interfaces, construction system |
| **PGXEditorTools** | Asset auditing, system observer, test dashboard, editor productivity tools |

### Production-Ready Systems

| Plugin | What It Does |
|--------|-------------|
| **PGXGameFlow** | Multi-channel game state machine with GameplayTag-driven phases and validation rules |
| **PGXSave** | Complete save/load with slots, auto-save, versioning, compression, migration chains, async I/O |
| **PGXAudio** | Dual-backend audio (Legacy + Audio Modulation), 5-layer mix, ducking, HDR, music manager, dialogue, sound pool |
| **PGXPSO** | Pipeline State Object management with automatic Loading screen coordination -- eliminates shader compilation hitches during level transitions. Includes recording tools and optional catalog-based pre-warming for dynamic spawn scenarios |
| **PGXLoading** | Loading screen system with PSO-aware progress tracking, configurable visual profiles via DataAsset, and deterministic timing. Widget persists across level transitions -- zero code required |
| **PGXProfile** | Platform profile system -- capabilities, budgets, features, build validation, platform overrides |
| **PGXLog** | Polymorphic observability -- 13 domain renderers, structured log entries, domain filter bar, Live/Manual modes |
| **PGXDocs** | In-editor Markdown documentation viewer with native Slate rendering, full-text search, live reload |
| **PGXMGOS** | Garbage collection observability -- inference-based monitoring, leak detection, behavioral profiling |
| **PGXVersionControl** | Git integration with editor workflow, changelists, version management |

### Infrastructure Systems (L1 Core)

| System | What It Does |
|--------|-------------|
| **Message Bus** | Typed pub/sub messaging with GameplayTag channels, template-based broadcast, async listeners, history recording |
| **Event Handler** | Data-driven behavior resolution bus with GameplayTag routing, typed payloads, lifecycle management (Singleton/Cached/Ephemeral), telemetry and blackbox |
| **Data Registry** | DataTable-first persistent store with compiled queries, cache management, and runtime statistics |
| **Construction** | Actor assembly system with 8 DA types covering actors, components, widgets, game modes, and more |

### Tool Plugins

| Plugin | What It Does |
|--------|-------------|
| **PGXSimHarness** | Simulation harness -- injects realistic test data across all 13 systems (~160 API calls, ~90% coverage) |
| **PGXScaffold** | Automated project scaffolding -- templates, folder creation, Data Asset generation, guided workflow |

### Scaffolded Systems (Coming Soon)

| Plugin | What It Does |
|--------|-------------|
| **PGXInput** | Enhanced Input abstraction, input contexts via Data Assets |
| **PGXCamera** | Camera modes, transitions, data-driven configuration |
| **PGXUI** | Screen management, widget pool, notifications, loading screens |
| **PGXAbility** | GAS facade with developer-friendly API and observability |
| **PGXInventory** | Items, containers, equipment |
| **PGXInteraction** | Component-based world interaction |
| **PGXSpawn** | Spawn management, wave definitions |
| **PGXAI** | AI controllers, behavior tree library |
| **PGXAnimation** | AnimInstance base, data-driven montages |
| **PGXMaterials** | Material management, dynamic parameters |
| **PGXVFX** | Visual effects management (Niagara wrapper) |
| **PGXCinematic** | Cinematic system (Level Sequence wrapper) |
| **PGXMultiplayer** | Replication framework, state sync |
| **PGXOnline** | Online services abstraction (EOS, Steam) |

---

## Every System Follows the Same Pattern

Each PGX system implements a consistent, proven architecture:

| Component | What It Provides |
|-----------|-----------------|
| **Subsystem** | Core logic (GameInstance, World, or Engine scope) |
| **Config Data Asset** | All configuration via Data Assets -- no hardcoded values. Deterministic resolution via Project Settings with one-click DA assignment |
| **Console Commands** | Runtime inspection and debugging |
| **Blueprint Library** | Full Blueprint access with tiered categories (Core, Query, Advanced, Debug) |
| **Test Utility** | Standardized test functions per system |
| **Editor Inspector** | Dockable panel with live PIE data, built on shared visual tokens |
| **Base Classes** | 10 L1-integrated base classes with 79 overridable Blueprint events |
| **Documentation** | Architecture + Usage + Testing + Extension guides |

This pattern has been validated across **13 production systems** and **8 Construction DA types**.

---

## Editor Integration

PGX integrates deeply into the Unreal Editor with native Slate UI:

| Feature | Count |
|---------|-------|
| **Editor Inspector Panels** | 22 dockable NomadTabs |
| **Data Asset Factories** | 58 (with distinct colors per system) |
| **Blueprint Factories** | 23 |
| **Quick Access Pins** | 19 |
| **Custom SVG Icons** | 26 |
| **Console Commands** | 95+ across all systems |
| **Blueprint Nodes** | 170+ across all BP Libraries |
| **Gameplay Tags** | 180+ production tags with branch ownership |
| **Test Functions** | 95+ (standardized per system + domain tests) |

### Editor Panels

- **Hub Dashboard** -- Central navigation with 12 system cards and quick status overview
- **System Observer** -- Live dashboard showing all subsystem states with sparkline graphs
- **Test Dashboard** -- Run all system tests with color-coded results and history
- **Log Viewer** -- Polymorphic domain rendering, 13 domain renderers, domain filter bar, detail windows, Live/Manual modes
- **Save Inspector** -- Slot browser, data viewer, version inspector
- **GameFlow Inspector** -- 8-channel state visualization with transition rate graphs
- **PSO Inspector** -- Pipeline status, configs, recording, progress visualization
- **LevelFlow Inspector** -- Loading pipeline with color-coded states and progress graphs
- **Loading Inspector** -- Transition management with PSO coordination view
- **Profile Inspector** -- 6 panels with platform simulation controls
- **Audio Inspector** -- 12 sections covering all audio subsystems
- **Data Registry Browser** -- Database explorer with search, stats, and cache management
- **Config Dashboard** -- System grouping with validation and quick navigation
- **Message Inspector** -- Live pub/sub channel monitoring, listener tracking, message history
- **Event Debugger** -- Handler browser, telemetry dashboard, blackbox recorder, execution graphs
- **Platform Health Dashboard** -- Per-system budget overview, platform comparison, simulation
- **Documentation Viewer** -- Full Markdown rendering with search and navigation
- **Visual Showcase** -- Design token and atomic widget reference panel
- **Simulation Harness** -- Test data injection controls with live simulation ticker
- **Scaffold** -- Guided project scaffolding with template selection and preview
- **MGOS Inspector** -- GC observability with memory graphs and health indicators
- **Registry Validation** -- Data integrity checks and cross-reference analysis

---

## Design Principles

### UE5-First
We never reinvent what Epic provides. Every PGX system wraps the corresponding UE5 system with a clean facade. Enhanced Input, GAS, Audio Modulation, Niagara, Level Sequence -- all wrapped, never replaced.

### Data-Driven
Configuration through Data Assets, not hardcoded values. Every system is configurable without recompilation. Two DA contexts: **Config DAs** (system settings, assigned via Project Settings) and **Object DAs** (game content). Deterministic config resolution -- one slot per system, zero ambiguity.

### Blueprint-Friendly
Over 170 Blueprint nodes cover the full API surface with progressive disclosure. C++ for architecture, Blueprints for content. 10 base classes with 79 overridable events ensure Blueprint developers have first-class access.

### Decoupled
Systems communicate through a three-bus model (Message Bus, Event Handler, Data Registry), not direct references. You can remove any plugin without affecting others.

### Observable
Every system has console commands, editor inspectors, and traceability. Debug any subsystem at runtime without code changes. The Log system provides 13 domain-specific renderers for structured visualization.

### Zero-Configuration Start
Every system works out of the box with sensible defaults. Create a Data Asset, configure it in the Details panel, done. No documentation required to get started.

### Progressive Disclosure -- UX at Every Layer

PGX follows the same progressive disclosure philosophy that Epic uses throughout Unreal Engine. A developer who opens PGX for the first time should find the essentials immediately -- without noise, without intimidation, without reading documentation.

This principle is applied systematically across three layers:

| Layer | Problem | PGX Solution |
|-------|---------|-------------|
| **Blueprint Palette** | Dozens of nodes per system, impossible to find the 3 you need | Tiered categories: Core shows daily-use nodes, Query/Advanced/Debug subcategories for the rest |
| **Blueprint Nodes** | Too many pins on commonly-used nodes | Minimal signatures for core operations, following the same pattern as Lyra's messaging |
| **Data Asset Properties** | 20+ properties per Config DA overwhelm new users | Non-essential properties hidden by default -- same pattern Epic uses on Actor, Component, and every UE base class |

The result: a junior developer can create a save system by filling 3 visible fields on a Data Asset. A senior developer expands "Advanced" and finds full control over slots, versioning, compression, and migration chains. **Same asset, two experiences, zero compromises.**

Every classification decision (which properties are visible, which are advanced) is documented with explicit reasoning so the intent survives across the project's lifetime.

---

## The Numbers

| Metric | Value |
|--------|-------|
| Total plugins | **25** |
| Source files | **1,300+** |
| Production-ready systems | **13** |
| UE modules | **28+** |
| Data Asset types | **58** |
| Editor panels (NomadTabs) | **22** |
| Console commands | **95+** |
| Blueprint nodes | **170+** |
| Gameplay tags | **180+** |
| Test functions | **95+** |
| Custom SVG icons | **26** |
| Quick access pins | **19** |
| Hub dashboard cards | **12** |
| Design tokens | **60+** |
| Atomic widget instances | **167** |
| Overridable Blueprint events | **79** |
| Documentation files | **90+** |
| clang-tidy warnings fixed | **957** |
| Panel UX audits completed | **22** |

---

## Tech Stack

| Component | Technology |
|-----------|------------|
| **Engine** | Unreal Engine 5.6.1 |
| **Language** | C++20 (UE5 standard) |
| **Configuration** | Data Assets (Data-Driven) |
| **Editor** | Native Slate UI |
| **Visual System** | Design tokens + 11 atomic widget types |
| **Platforms** | Windows (primary), expandable |

---

## Repository Structure

PGX is developed as a **monorepo** containing all 25 plugins. The source code, documentation, editor tooling, and automation scripts live together in a single private repository.

Individual plugin repositories have been consolidated and archived. The monorepo is the single source of truth.

### Multi-Engine Version Strategy

PGX supports multiple Unreal Engine versions through a branch strategy:

- **`main`** -- Always targets the active development UE version
- **`ue/5.6`** -- Frozen snapshot when a new UE version is adopted
- **Release tags**: `v0.4.0-ue5.6`, `v0.5.0-ue5.7`, etc.

---

## Development Status

| Phase | Status | Key Deliverable |
|-------|--------|-----------------|
| 0-1 | Complete | Project structure + Core infrastructure |
| 2 | Complete | Log System v1.0 |
| 3 | Complete | Save System v1.0 |
| 4 | Complete | GameFlow v1.0 |
| 5 | Complete | PSO System v2.0 |
| 6 | Complete | Infrastructure Upgrades (Path + Trace + Observer) |
| 7.5 | Complete | LevelFlow v1.0 |
| 8.1 | Complete | Profile System v1.0 |
| 8.1.5 | Complete | PGXDocs v1.1 (in-editor Markdown viewer) |
| 8.2 | Complete | Audio v1.1 + Data Registry v2.0 + Construction v1.3 |
| 8.5 | Complete | Test Dashboard + Quality Audit Remediation |
| 8.6 | Complete | Test Harness v1.0 |
| 8.7 | Complete | Monorepo Workspace + CI/CD |
| 8.8 | Complete | Message Bus v1.0 + Event Handler v1.0 |
| 8.9 | Complete | Profile v2.0 -- Platform-Aware Configuration |
| 9.0 | Complete | GameplayTags Standardization + Branch Ownership |
| 9.5 | Complete | Simulation Harness v2.0 (13-system injection, ~160 API calls) |
| 9.6 | Complete | Panel UX Refactoring (22 panels, standardized professional UX) |
| 9.7 | Complete | Log v3.0 -- Polymorphic Observability (13 domain renderers) |
| 9.8 | Complete | Visual Construction -- Design tokens, 11 atomic widgets, 27 panel migrations |
| 9.9 | Complete | Interactive Tutorials -- 18 bilingual tutorials (5 onboarding + 13 per-system) |
| 10 | Complete | L1 Integration + Blueprint Reflection Pass + clang-tidy Remediation |
| 10.3 | Complete | Settings-first Config Resolution (deterministic DA discovery for 11 subsystems) |
| 11 | Planned | Template integration |
| 12 | Planned | Polish + v1.0 release |

---

## Licensing

PGX Framework uses an **Open Core** model with a **Source-Available Proprietary** license. Full source code is provided to licensees -- this is not obfuscated middleware. Access is granted by invitation to the private GitHub repository.

| Tier | Revenue Threshold | Annual Fee | What You Get |
|------|-------------------|------------|-------------|
| **Community** | Under EUR 50,000/year | **Free** | Full framework, all updates, community support |
| **Growth** | EUR 50,000 -- EUR 100,000/year | Contact us | Full framework + priority support + extended license |
| **Studio** | Over EUR 100,000/year | Contact us | Full framework + priority support + custom terms |

Revenue means **gross revenue from products using PGX** -- not total company revenue. The EUR 50,000 threshold means most independent projects will never pay a license fee.

Key terms: source code in full (not compiled), modify freely, cannot redistribute the framework, per-studio license covering all team members, all engine version updates included. Educational licenses available at no cost for academic institutions.

Full licensing details, vertical extension policy, and professional services information are available on the **[Licensing & Distribution](../../wiki/Licensing-and-Distribution)** wiki page.

---

## How to Access

PGX Framework is available through the **Platano Games Academy**.

**Website:** [www.platanogames.es](https://www.platanogames.es)

The academy provides:
- **Full source code** of PGX Framework via private GitHub repository
- **Documentation** (90+ files) through the academy platform
- **Updates** as new systems reach production status
- **Support** and community access

### For Studios and Teams

Institutional licenses, team access, and professional services available. Contact us through the academy website.

---

## Also by Platano Games

### [Lyra Architectural Study](https://github.com/platanogames/Lyra-Architectural-Study)
Complete architectural dissection of Epic's LyraStarterGame -- 668 C++ files commented, 29 textbook chapters, 106 documentation files.

### VRScan3D
Professional 3D scanning simulator built on UE 5.5 for technical training, with cloud integration via PlayFab.

### [Docs Converter](https://github.com/platanogames/docsconverter)
Open-source Markdown to HTML/PDF/DOCX/EPUB converter with Pandoc and PySide6 dashboard.

---

## License

PGX Framework is proprietary software. All rights reserved.

**Copyright (c) 2024-2026 Platano Games.**

The framework, source code, documentation, editor tools, and all associated materials are distributed exclusively through the Platano Games Academy. See the **[Licensing & Distribution](../../wiki/Licensing-and-Distribution)** wiki page for full terms.

---

<p align="center">
  <strong><a href="https://www.platanogames.es">www.platanogames.es</a></strong><br>
  <em>Professional game development tools and education</em>
</p>

---

<!-- ES: Seccion en Espanol -->

## Version en Espanol

PGX (Professional Game Extensions) es un framework modular profesional C++ para Unreal Engine 5. 25 plugins, 13 sistemas production-ready, 1,300+ archivos fuente, y herramientas de editor integradas en Slate nativo.

No es un template -- es la capa arquitectonica entre UE5 y tu juego. Cada sistema wrappea el subsistema UE5 correspondiente con una fachada limpia, documentada y observable.

### Por que existe PGX

Cree PGX porque tras anos desarrollando productos para clientes y auditando codebases de estudios de todos los tamanos, seguia viendo lo mismo: cada proyecto reinventa los mismos sistemas fundamentales desde cero. Guardado, audio, pantallas de carga, maquinas de estado, configuracion -- reconstruidos de cero, cada vez, por cada equipo.

Me frustraba. En otros ecosistemas esto se resolvio hace anos. Python tiene Django. JavaScript tiene Next.js. Ruby tiene Rails. Pero Unreal Engine -- pese a ser uno de los motores mas potentes del mundo -- no tenia equivalente. Sin capa de middleware. Sin fundacion compartida.

Y el coste de esa carencia lo vi de primera mano, proyecto tras proyecto: no hay paridad arquitectonica entre estudios, cada equipo inventa sus patrones, la deuda tecnica se acumula porque las implementaciones custom nunca reciben el pulido que merecen bajo presion de produccion, el onboarding de nuevos miembros es lento, y los mismos bugs -- corrupcion de saves, pantallas de carga que se autodestruyen, hitches de compilacion de shaders -- se redescubren una y otra vez.

Decidi que era momento de construir lo que deberia haber existido desde el principio: una capa estandarizada, extensible y probada en produccion que se situa **encima de** Unreal Engine -- nunca lo reemplaza, siempre lo complementa y amplia. La arquitectura, los patrones, las herramientas, la documentacion -- construidos una vez, construidos bien, para que cada equipo que lo use pueda saltarse anos de trabajo fundacional y enfocarse en lo que hace unico a su juego.

### Ecosistema completo

- **PGX Framework** -- 25 plugins C++ para UE 5.6.1
- **22 paneles de editor** integrados en Slate nativo
- **95+ comandos de consola** para inspeccion en runtime
- **170+ nodos Blueprint** para disenadores
- **58 tipos de Data Asset** configurables sin recompilacion
- **180+ gameplay tags** nativos con branch ownership
- **60+ design tokens** y 11 tipos de widgets atomicos
- **18 tutoriales interactivos** bilingues (EN/ES)

### Licencia

Modelo **Open Core, Source-Available Propietario**. Gratis para desarrolladores independientes (ingresos menores a 50.000 EUR/ano). Acceso por invitacion al repositorio privado. Detalles completos en la **[wiki de Licensing & Distribution](../../wiki/Licensing-and-Distribution)**.

### Acceso

Disponible a traves de la academia: [www.platanogames.es](https://www.platanogames.es)
