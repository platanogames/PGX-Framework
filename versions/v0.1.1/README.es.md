# PGX Framework

**Extensiones profesionales para videojuegos en Unreal Engine 5**

> [!WARNING]
> **PGX se encuentra en desarrollo activo.** Esta vista previa pública permite
> que desarrolladores de Unreal Engine inspeccionen la arquitectura y las API,
> evalúen los sistemas incluidos y aporten comentarios técnicos útiles.
>
> Las API, el empaquetado, los límites de compatibilidad y los flujos de
> contribución pueden cambiar entre versiones `0.x`. En esta etapa PGX está
> destinado a evaluación, aprendizaje y desarrollo, y
> **todavía no se recomienda para proyectos en producción**.

[![Estado: Development Preview](https://img.shields.io/badge/estado-development_preview-orange.svg)](#vista-previa-de-desarrollo)
[![Licencia: Apache-2.0](https://img.shields.io/badge/licencia-Apache--2.0-green.svg)](LICENSE.md)

## Vista previa de desarrollo

[English](README.md) | Español

PGX reúne plugins para proyectos de Unreal Engine 5 que necesitan configuración
compartida, mensajería, persistencia, carga, audio y diagnóstico desde el
editor. Se apoya directamente en subsistemas, Data Assets y Gameplay Tags de
Unreal, con puntos de extensión visibles en C++.

## Estado de la versión preliminar

| Área | Política actual |
|---|---|
| Versión actual | `0.1.1` (`v0.1.1`) |
| Motor validado | Unreal Engine 5.7.4, con objetivos Windows Development |
| Etapa de publicación | Pre-release / Development Preview (`0.x`) |
| Uso previsto | Evaluación de arquitectura y API, aprendizaje, pruebas y comentarios |
| Estabilidad de API | Puede haber cambios incompatibles entre versiones preliminares |
| Uso en producción | No recomendado actualmente |
| Licencia | Apache-2.0, sujeta a los avisos y términos de terceros descritos en este repositorio |

Las versiones compatibles del motor, los pasos de instalación y los resultados
de compilación se publican únicamente cuando han sido validados para una versión
concreta. Consulta sus notas antes de evaluar PGX en un proyecto
de Unreal Engine.

La instantánea preliminar verificada de `v0.1.1` contiene **26 plugins y 48 módulos de Unreal**
en una única constelación compatible. En Unreal Engine
5.7.4, con objetivos Windows Development, el candidato exportado superó estos
controles:

- compilación limpia de Editor: 792/792 acciones superadas;
- compilación limpia de Game: 414/414 acciones superadas;
- Automation de Editor: 242/242 pruebas superadas;
- Automation de Game: 115/115 pruebas superadas;
- `PGX.Demo`: 3/3 pruebas superadas.

## Modelo de desarrollo y publicación

El desarrollo diario se realiza en un monorepo privado canónico. Este repositorio
público recibe versiones revisadas que contienen únicamente
el código fuente, documentación, pruebas y archivos de proyecto aprobados para
distribución pública.

El repositorio público utiliza una única rama `release` y conserva cada
instantánea aprobada en `versions/vX.Y.Z/`. Las etiquetas y las publicaciones
de GitHub son límites inmutables. Las correcciones avanzan mediante una nueva
versión en lugar de reescribir una existente.

El material privado de trabajo, los componentes sin comportamiento, los archivos
generados, las cachés y el contenido sin un límite de redistribución verificado
quedan fuera. Un plugin incompleto puede incluirse cuando su alcance implementado
y sus limitaciones se describan de forma explícita como parte de esta versión preliminar.

Las incidencias y pull requests públicas forman parte del proceso de revisión. Tras la
revisión, los cambios aceptados se reconcilian con la línea canónica de desarrollo
y se incluyen en una versión pública posterior.

## Qué contiene el monorepo público

```text
PGX-Framework/
├── versions/
│   ├── v0.1.0/           Primera instantánea pública conservada
│   └── v0.1.1/
│       ├── Plugins/      Constelación de plugins de esta versión
│       ├── Samples/
│       │   └── PGXDemo/  Proyecto de ejemplo listo para abrir
│       └── docs/         Documentación vinculada a la instantánea
├── .github/              Flujos de contribución y del repositorio
├── README.md             Presentación del proyecto en inglés
├── README.es.md          Presentación del proyecto en español
├── CHANGELOG.md          Historial público de cambios
├── CONTRIBUTING.md       Guía de contribución
├── SECURITY.md           Política para reportar vulnerabilidades
├── SUPPORT.md            Guía para solicitudes de soporte
├── LICENSE.md            Apache License 2.0
└── NOTICE                Atribución y límites de terceros
```

El contenido de cada directorio versionado define su API pública y su límite de
soporte. Los componentes ausentes de `versions/v0.1.1/` no forman parte de
`v0.1.1`.

## Plugins incluidos

La versión preliminar pública `v0.1.1` contiene **26 plugins y 48 módulos de Unreal**.
El repositorio combina sistemas consolidados con versiones preliminares; la
presencia de un plugin no significa que todas sus capacidades previstas estén implementadas.

| Grupo de madurez | Plugins |
|---|---|
| Fundación | `PGXCore` |
| Sistemas preliminares consolidados | `PGXAudio`, `PGXGameFlow`, `PGXLoading`, `PGXMGOS`, `PGXPSO`, `PGXSave` |
| Versiones preliminares funcionales | `PGXAbility`, `PGXCrafting`, `PGXInput`, `PGXInteraction`, `PGXInventory`, `PGXSpawn`, `PGXTrade` |
| Versiones preliminares estructuradas | `PGXAI`, `PGXCamera`, `PGXColony`, `PGXEnvironment`, `PGXUI`, `PGXVehicles` |
| Herramientas de editor y verificación | `PGXDocs`, `PGXEditorTools`, `PGXScaffold`, `PGXSimHarness`, `PGXTutorials`, `PGXVersionControl` |

La mayoría de plugins funcionales dependen de `PGXCore`. `PGXEditorTools` es una
capa de agregación exclusiva del editor que reúne los inspectores de los sistemas
seleccionados. Consulta el [catálogo de plugins](docs/plugins/catalog.md)
y el [mapa de dependencias](docs/architecture/modules-and-dependencies.md) antes de
habilitar un subconjunto.

## Arquitectura de un vistazo

PGX se organiza como un conjunto de plugins de Unreal Engine, no como una
plantilla de juego obligatoria.

- El código de ejecución y las herramientas de editor viven en módulos separados.
- La configuración y la comunicación utilizan Data Assets, subsistemas y
  Gameplay Tags de Unreal.
- El código fuente expone API de C++ y Blueprint, además de inspectores y
  validadores para los sistemas incluidos.

Cada plugin declara sus dependencias. La instantánea completa de 26 plugins ha
superado los controles documentados de Editor y Game, pero cada subconjunto debe
comprobarse contra sus descriptores y reglas de módulo. Consulta
[Verificación](docs/validation/verification.md) para conocer la evidencia exacta
y sus límites.

## Instalar o explorar v0.1.1

Para instalar con un límite inequívoco, descarga el artefacto específico de la
versión adjunto a la publicación `v0.1.1` de GitHub. Los archivos de código
fuente generados automáticamente por GitHub contienen el catálogo acumulado del
monorepo, incluidas todas las versiones conservadas.

1. Abre [`versions/v0.1.1/`](./).
2. Revisa [`versions/v0.1.1/Plugins/`](Plugins/).
3. Copia los plugins necesarios al directorio `Plugins/` de tu proyecto sin
   renombrarlos.
4. Incluye `PGXCore` y todas las dependencias declaradas por los descriptores.
5. Regenera los archivos de proyecto y compila el objetivo previsto.

Para inspeccionar una integración configurada, abre
[`versions/v0.1.1/Samples/PGXDemo/PGXDemo.uproject`](Samples/PGXDemo/PGXDemo.uproject).
El ejemplo demuestra Message, GameFlow, Save e InputBuffer. Los otros 22 plugins
están presentes para validar dependencias, carga y enlazado; su presencia no
significa que el ejemplo demuestre su comportamiento durante la ejecución.

Para revisar el código fuente:

1. Revisa el directorio versionado `Plugins/`.
2. Abre el descriptor de un plugin para inspeccionar sus módulos y dependencias.
3. Consulta su árbol `Source/` y la documentación incluida con ese plugin.
4. Revisa el [`CHANGELOG`](CHANGELOG.md) público para conocer los cambios de cada versión.
5. Utiliza los formularios de incidencias para preguntas o hallazgos reproducibles.

Para evaluar la ruta de editor actual en Windows, sigue la
[guía de primeros pasos](docs/getting-started/quickstart.md). La
validación empaquetada y Shipping permanece fuera del alcance actual.

## Documentación

La documentación de cada versión se distribuye junto al código fuente público
correspondiente siempre que es posible. De este modo, la guía arquitectónica y de
API permanece vinculada a la instantánea que describe.

Empieza por:

- [`versions/v0.1.1/docs/getting-started/quickstart.md`](docs/getting-started/quickstart.md): lista de comprobación para evaluar la versión preliminar
- [`versions/v0.1.1/docs/architecture/overview.md`](docs/architecture/overview.md): estructura del monorepo y límites entre plugins
- [`versions/v0.1.1/docs/architecture/system-map.md`](docs/architecture/system-map.md): capas y flujos entre plugins
- [`versions/v0.1.1/docs/architecture/modules-and-dependencies.md`](docs/architecture/modules-and-dependencies.md): topología exacta de plugins y módulos
- [`versions/v0.1.1/docs/plugins/catalog.md`](docs/plugins/catalog.md): responsabilidades implementadas y límites de los 26 plugins
- [`versions/v0.1.1/docs/validation/verification.md`](docs/validation/verification.md): comprobaciones realizadas y sus límites
- [Wiki del proyecto](https://github.com/platanogames/PGX-Framework/wiki): arquitectura y flujos de trabajo ampliados
- [`versions/v0.1.1/Plugins/`](Plugins/): descriptores, código fuente y documentación incluida
- [`versions/v0.1.1/Samples/PGXDemo/`](Samples/PGXDemo/): proyecto de ejemplo configurado
- [`CHANGELOG.md`](CHANGELOG.md): cambios públicos e historial de versiones
- [`KNOWN_ISSUES.md`](KNOWN_ISSUES.md): limitaciones verificadas de la versión preliminar
- [`ROADMAP.md`](ROADMAP.md): hitos públicos hacia un contrato estable
- [`docs/releasing/public-release-model.md`](docs/releasing/public-release-model.md): controles y política de instantáneas
- [`CONTRIBUTING.md`](CONTRIBUTING.md): flujo de contribución
- [`SUPPORT.md`](SUPPORT.md): información necesaria para solicitudes útiles
- [`SECURITY.md`](SECURITY.md): guía para informar vulnerabilidades de forma privada

## Camino hacia 1.0

La serie `0.x` es el periodo en el que PGX consolidará su contrato público. El
trabajo hacia `1.0` incluye:

- instalación reproducible desde un clon limpio;
- una matriz de compatibilidad de Unreal Engine respaldada por evidencia;
- ejemplos y flujos de validación mantenidos;
- una API pública y una política de deprecación documentadas;
- procesos estables de contribución y publicación;
- notas de migración para cambios incompatibles.

`1.0` significará que el contrato público seleccionado es estable. No significará
que todos los sistemas o extensiones futuras imaginables para PGX estén terminados.

## Contribuir

PGX se publica antes de `1.0` para que otros desarrolladores revisen la
arquitectura y las API antes de que esos cambios resulten costosos.

Antes de abrir una incidencia o pull request, lee [`CONTRIBUTING.md`](CONTRIBUTING.md).
Mantén los informes reproducibles y elimina credenciales, código privado de otros
proyectos, recursos con licencia y registros no relacionados de los ejemplos.

## Seguridad y soporte

No publiques posibles vulnerabilidades en una incidencia pública. Sigue
[`SECURITY.md`](SECURITY.md) para utilizar el canal privado.

Para preguntas de integración y solicitudes de soporte, sigue
[`SUPPORT.md`](SUPPORT.md) e incluye la revisión exacta de PGX, la versión de
Unreal Engine, la plataforma objetivo, los pasos de reproducción, el resultado
esperado y el resultado observado.

## Licencia y procedencia

PGX Framework se distribuye bajo la [Apache License 2.0](LICENSE.md).

El material de terceros, cuando exista, continúa regido por sus términos
respectivos. Consulta [`NOTICE`](NOTICE) y las cabeceras de cada archivo para
conocer la atribución y el límite de licencia aplicables.

Copyright 2024-2026 Platano Games.
