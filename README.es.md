# PGX Framework

> **Vista previa de desarrollo** — PGX continúa en desarrollo activo. Las instantáneas publicadas están destinadas a revisar la arquitectura, explorar la API y evaluar el framework. Las API pueden cambiar de forma incompatible y todavía no se recomienda ninguna versión para producción.

PGX es un framework modular y de código abierto para Unreal Engine, mantenido como un monorepo versionado. Cada versión se conserva como una instantánea autónoma para que los desarrolladores puedan inspeccionar, comparar y evaluar superficies exactas de plugins, documentación y proyectos de ejemplo sin cambiar de rama.

[English](README.md) | Español

## Versiones

| Versión | Estado | Unreal Engine | Instantánea |
|---|---|---|---|
| [`v0.1.1`](versions/v0.1.1/README.es.md) | **Publicada** | 5.7.4 | [Plugins](versions/v0.1.1/Plugins/) · [Documentación](versions/v0.1.1/docs/) · [PGXDemo](versions/v0.1.1/Samples/PGXDemo/) |
| [`v0.1.0`](versions/v0.1.0/README.es.md) | Publicada | 5.6 | [Plugins](versions/v0.1.0/Plugins/) · [Documentación](versions/v0.1.0/docs/) |

PGX publicó `v0.1.1` como versión preliminar en GitHub el 10 de agosto de 2026. La publicación incluye el [archivo específico de la versión](https://github.com/platanogames/PGX-Framework/releases/tag/v0.1.1).

Los metadatos del catálogo legibles por herramientas están en [`RELEASES.json`](RELEASES.json). Cada versión también contiene su propio `RELEASE.json` y un fichero [`SHA256SUMS`](versions/v0.1.1/SHA256SUMS).

## Instantánea actual: `v0.1.1`

La instantánea publicada contiene **26 plugins**, su código y documentación públicos y un proyecto de demostración listo para abrir con Unreal Engine 5.7.4.

Controles validados:

- Compilación de Unreal Editor: **792 acciones**, correcta.
- Compilación de Unreal Game: **414 acciones**, correcta.
- Automatización de PGXDemo: **3/3**.
- Automatización de Editor: **242/242**.
- Automatización de Game: **115/115**.
- Recursos de demostración: **7/7** verificados.
- Hallazgos del saneador y de secretos: **0**.
- Dependencias ausentes en el cierre y ciclos de dependencias: **0**.

Consulta el [README de la versión](versions/v0.1.1/README.es.md), sus [límites de verificación](versions/v0.1.1/docs/validation/verification.md) y sus [problemas conocidos](versions/v0.1.1/KNOWN_ISSUES.md) antes de evaluarla.

## Cómo utilizar una instantánea

1. Selecciona una versión en la tabla anterior.
2. Lee su README, sus problemas conocidos y sus límites de verificación.
3. Copia los plugins necesarios desde `versions/<versión>/Plugins/` al directorio `Plugins/` de tu proyecto de Unreal, o abre el proyecto de ejemplo incluido cuando esté disponible.
4. Activa solo los plugins que necesites y revisa sus dependencias declaradas.
5. Genera los archivos del proyecto y compila con la versión de Unreal Engine documentada en esa instantánea.
6. Comprueba los archivos descargados con el fichero `SHA256SUMS` de la versión.

## Política del repositorio

- La rama `release` funciona como catálogo de versiones.
- Las instantáneas publicadas dentro de `versions/` son inmutables.
- Las nuevas versiones proceden de exportaciones saneadas y validadas.
- Las etiquetas identifican publicaciones y nunca se desplazan después de publicarse.
- La infraestructura privada de desarrollo y los documentos internos de trabajo no forman parte de este repositorio.

## Enlaces del proyecto

- [Historial de versiones](CHANGELOG.md)
- [Índice de problemas conocidos](KNOWN_ISSUES.md)
- [Roadmap público](ROADMAP.md)
- [Cómo contribuir](CONTRIBUTING.md)
- [Soporte](SUPPORT.md)
- [Política de seguridad](SECURITY.md)
- [Código de conducta](CODE_OF_CONDUCT.md)

## Licencia

PGX Framework se distribuye bajo la [Apache License 2.0](LICENSE.md). Los componentes de terceros conservan sus propias condiciones; consulta [`NOTICE`](NOTICE) y los avisos incluidos en cada instantánea.
