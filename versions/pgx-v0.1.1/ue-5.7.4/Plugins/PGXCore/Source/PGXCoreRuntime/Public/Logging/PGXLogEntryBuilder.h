// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Logging/PGXLogTypes.h"
#include "GameplayTagContainer.h"

// ═══════════════════════════════════════════════════════════════
// EN: Type-Adaptive Log Entry System.
//     Contains: Formatter (template specializations), Builder (RAII),
//     and NullBuilder (shipping no-op). Consolidated for cohesion.
// ES: Sistema de Entrada de Log Adaptativo por Tipo.
//     Contiene: Formatter (especializaciones template), Builder (RAII),
//     y NullBuilder (no-op para shipping). Consolidado para cohesion.
// ═══════════════════════════════════════════════════════════════

// ─────────────────────────────────────────────────────────────
// SECTION 1: TPGXLogFormatter - Type Dispatch
// SECCION 1: TPGXLogFormatter - Despacho de Tipo
// ─────────────────────────────────────────────────────────────

/**
 * EN: Template specializations that convert any supported type to FString + EPGXLogDataType.
 *     The builder calls TPGXLogFormatter<T>::Format() for automatic type detection.
 *     Default fallback uses LexToString for any type that supports it.
 * ES: Especializaciones de template que convierten cualquier tipo soportado a FString + EPGXLogDataType.
 *     El builder llama a TPGXLogFormatter<T>::Format() para deteccion automatica de tipo.
 *     Fallback default usa LexToString para cualquier tipo que lo soporte.
 */
template<typename T>
struct TPGXLogFormatter
{
	static FPGXLogParam Format(const FString& Key, const T& Value)
	{
		FPGXLogParam Param;
		Param.Key = Key;
		Param.Value = LexToString(Value);
		Param.DataType = EPGXLogDataType::Custom;
		return Param;
	}
};

// ── String types / Tipos string ──

template<>
struct TPGXLogFormatter<FString>
{
	static FPGXLogParam Format(const FString& Key, const FString& V)
	{
		return { Key, V, EPGXLogDataType::String };
	}
};

template<>
struct TPGXLogFormatter<FName>
{
	static FPGXLogParam Format(const FString& Key, const FName& V)
	{
		return { Key, V.ToString(), EPGXLogDataType::String };
	}
};

template<>
struct TPGXLogFormatter<FText>
{
	static FPGXLogParam Format(const FString& Key, const FText& V)
	{
		return { Key, V.ToString(), EPGXLogDataType::String };
	}
};

// ── Numeric types / Tipos numericos ──

template<>
struct TPGXLogFormatter<int32>
{
	static FPGXLogParam Format(const FString& Key, int32 V)
	{
		return { Key, FString::FromInt(V), EPGXLogDataType::Integer };
	}
};

template<>
struct TPGXLogFormatter<int64>
{
	static FPGXLogParam Format(const FString& Key, int64 V)
	{
		return { Key, FString::Printf(TEXT("%lld"), V), EPGXLogDataType::Integer };
	}
};

template<>
struct TPGXLogFormatter<uint32>
{
	static FPGXLogParam Format(const FString& Key, uint32 V)
	{
		return { Key, FString::Printf(TEXT("%u"), V), EPGXLogDataType::Integer };
	}
};

template<>
struct TPGXLogFormatter<float>
{
	static FPGXLogParam Format(const FString& Key, float V)
	{
		return { Key, FString::SanitizeFloat(V), EPGXLogDataType::Float };
	}
};

template<>
struct TPGXLogFormatter<double>
{
	static FPGXLogParam Format(const FString& Key, double V)
	{
		return { Key, FString::Printf(TEXT("%.6f"), V), EPGXLogDataType::Float };
	}
};

template<>
struct TPGXLogFormatter<bool>
{
	static FPGXLogParam Format(const FString& Key, bool V)
	{
		return { Key, V ? TEXT("true") : TEXT("false"), EPGXLogDataType::Boolean };
	}
};

// ── Spatial types / Tipos espaciales ──

template<>
struct TPGXLogFormatter<FVector>
{
	static FPGXLogParam Format(const FString& Key, const FVector& V)
	{
		return { Key, V.ToString(), EPGXLogDataType::Vector };
	}
};

template<>
struct TPGXLogFormatter<FRotator>
{
	static FPGXLogParam Format(const FString& Key, const FRotator& V)
	{
		return { Key, V.ToString(), EPGXLogDataType::Rotator };
	}
};

template<>
struct TPGXLogFormatter<FTransform>
{
	static FPGXLogParam Format(const FString& Key, const FTransform& V)
	{
		return { Key, V.ToString(), EPGXLogDataType::Transform };
	}
};

// ── Tag type / Tipo tag ──

template<>
struct TPGXLogFormatter<FGameplayTag>
{
	static FPGXLogParam Format(const FString& Key, const FGameplayTag& V)
	{
		return { Key, V.ToString(), EPGXLogDataType::GameplayTag };
	}
};

// ── Object type / Tipo objeto ──

template<>
struct TPGXLogFormatter<UObject*>
{
	static FPGXLogParam Format(const FString& Key, const UObject* V)
	{
		return { Key, V ? V->GetName() : TEXT("null"), EPGXLogDataType::Object };
	}
};

// ── Color types / Tipos color ──

template<>
struct TPGXLogFormatter<FLinearColor>
{
	static FPGXLogParam Format(const FString& Key, const FLinearColor& V)
	{
		return { Key, V.ToString(), EPGXLogDataType::Color };
	}
};

template<>
struct TPGXLogFormatter<FColor>
{
	static FPGXLogParam Format(const FString& Key, const FColor& V)
	{
		return { Key, V.ToString(), EPGXLogDataType::Color };
	}
};

// ─────────────────────────────────────────────────────────────
// SECTION 2: FPGXLogEntryBuilder - RAII Builder
// SECCION 2: FPGXLogEntryBuilder - Builder RAII
// ─────────────────────────────────────────────────────────────

/**
 * EN: RAII builder that collects typed parameters and fires the log on destruction.
 *     Created by PGX_LOG_ENTRY macro. Auto-submits when the statement ends.
 *     The .Add() method returns *this for chaining.
 *
 * ES: Builder RAII que recolecta parametros tipados y dispara el log al destruirse.
 *     Creado por la macro PGX_LOG_ENTRY. Auto-envia cuando la sentencia termina.
 *     El metodo .Add() retorna *this para encadenamiento.
 *
 * Usage / Uso:
 *     PGX_LOG_ENTRY(LogPGXSave, EPGXLogVerbosity::Info, "Save completed")
 *         .Add("slot", SlotName)
 *         .Add("bytes", ByteCount)
 *         .Add("position", PlayerLocation);
 *     // ↑ Destructor fires here, submitting the entry
 */
class PGXCORERUNTIME_API FPGXLogEntryBuilder
{
public:
	FPGXLogEntryBuilder(FName InCategory, EPGXLogVerbosity InVerbosity, FString&& InMessage);
	~FPGXLogEntryBuilder();

	// EN: Non-copyable, movable / ES: No copiable, movible
	FPGXLogEntryBuilder(const FPGXLogEntryBuilder&) = delete;
	FPGXLogEntryBuilder& operator=(const FPGXLogEntryBuilder&) = delete;
	FPGXLogEntryBuilder(FPGXLogEntryBuilder&& Other) noexcept;
	FPGXLogEntryBuilder& operator=(FPGXLogEntryBuilder&&) = delete;

	/**
	 * EN: Add a typed parameter. Type is auto-detected via TPGXLogFormatter<T>.
	 * ES: Agregar un parametro tipado. El tipo se auto-detecta via TPGXLogFormatter<T>.
	 */
	template<typename T>
	FPGXLogEntryBuilder& Add(const FString& Key, const T& Value)
	{
		Params.Add(TPGXLogFormatter<T>::Format(Key, Value));
		return *this;
	}

	/**
	 * EN: Add a single GameplayTag to this entry. Appended to auto-tags on Submit.
	 * ES: Agregar un solo GameplayTag a esta entrada. Se agrega a los auto-tags en Submit.
	 */
	FPGXLogEntryBuilder& Tag(const FGameplayTag& InTag);

	/**
	 * EN: Add multiple GameplayTags to this entry. Appended to auto-tags on Submit.
	 * ES: Agregar multiples GameplayTags a esta entrada. Se agregan a los auto-tags en Submit.
	 */
	FPGXLogEntryBuilder& Tags(const FGameplayTagContainer& InTags);

	/**
	 * EN: Set the domain tag for this entry. Used by PGX_LOG_DOMAIN macro.
	 *     If not set, the Log Subsystem will auto-infer from the Category.
	 * ES: Establecer el tag de dominio para esta entrada. Usado por macro PGX_LOG_DOMAIN.
	 *     Si no se establece, el Log Subsystem auto-infiere de la Category.
	 */
	FPGXLogEntryBuilder& Domain(const FGameplayTag& InDomainTag);

private:
	FName Category;
	EPGXLogVerbosity Verbosity;
	FString Message;
	TArray<FPGXLogParam> Params;
	FGameplayTagContainer CustomTags;
	FGameplayTag DomainTag;
	bool bSubmitted = false;

	/** EN: Submit the entry to UPGXLogSubsystem / ES: Enviar la entrada a UPGXLogSubsystem */
	void Submit();

	/**
	 * EN: Build formatted message string: "msg | key=value | key2=value2"
	 * ES: Construir string de mensaje formateado: "msg | key=value | key2=value2"
	 */
	FString BuildFormattedMessage() const;
};

// ─────────────────────────────────────────────────────────────
// SECTION 3: FPGXLogNullBuilder - Shipping No-Op
// SECCION 3: FPGXLogNullBuilder - No-Op para Shipping
// ─────────────────────────────────────────────────────────────

/**
 * EN: Null builder for Shipping builds. All .Add() calls are no-ops.
 *     Compiler optimizes this entirely away (zero-cost).
 * ES: Builder nulo para builds Shipping. Todas las llamadas .Add() son no-ops.
 *     El compilador optimiza esto completamente (coste cero).
 */
class FPGXLogNullBuilder
{
public:
	template<typename T>
	FPGXLogNullBuilder& Add(const FString&, const T&) { return *this; }
	FPGXLogNullBuilder& Tag(const FGameplayTag&) { return *this; }
	FPGXLogNullBuilder& Tags(const FGameplayTagContainer&) { return *this; }
	FPGXLogNullBuilder& Domain(const FGameplayTag&) { return *this; }
};
