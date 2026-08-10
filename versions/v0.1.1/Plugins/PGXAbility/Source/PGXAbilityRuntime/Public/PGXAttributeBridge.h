// Copyright PGX Framework. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "PGXAttributeBridge.generated.h"

class UAbilitySystemComponent;

/**
 * EN: Internal bridge connecting PGX types to GAS types.
 *     Handles the translation between PGX data structures and GAS AttributeSets,
 *     Effects, and Abilities.
 *     This class is internal to PGXAbility and should not be used by other plugins.
 *     It is the single point of coupling between PGX abstractions and raw GAS types,
 *     making it easier to adapt if GAS APIs change in future engine versions.
 *
 *     Resolution convention: GAS has no
 *     built-in Tag->FGameplayAttribute resolver (an `FGameplayAttribute` points at a
 *     specific `UPROPERTY` inside a project-authored `UAttributeSet` subclass — the
 *     generic framework cannot hardcode project attribute names). This bridge resolves
 *     by reflection: the LEAF segment of the attribute tag (e.g. `PGX.Attribute.Identity.Health`
 *     -> `Health`) must match the `UPROPERTY` name of an `FGameplayAttributeData` field on
 *     one of the AttributeSets already spawned on the ASC. Zero project registration code
 *     required (consistent with design section 5 "Core preconfigurable" — no project code needed for
 *     basic operation), at the cost of a naming convention the project must follow.
 *
 * ES: Bridge interno conectando tipos PGX a tipos GAS.
 *     Esta clase es interna de PGXAbility y no debe ser usada por otros plugins.
 *
 *     Convencion de resolucion: el segmento final (leaf) del tag de atributo
 *     (ej. `PGX.Attribute.Identity.Health` -> `Health`) debe coincidir con el nombre de
 *     la UPROPERTY `FGameplayAttributeData` en algun AttributeSet ya spawneado en el ASC.
 *     Sin codigo de registro de proyecto requerido, a cambio de una convencion de naming.
 */
UCLASS()
class PGXABILITYRUNTIME_API UPGXAttributeBridge : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * EN: Constructor. Sets default values.
	 * ES: Constructor. Establece valores por defecto.
	 */
	UPGXAttributeBridge();

	/**
	 * EN: Resolve a PGX attribute tag to a GAS `FGameplayAttribute` by searching the leaf tag
	 *     name against `FGameplayAttributeData` properties on all AttributeSets spawned on the
	 *     given ASC. Returns an invalid (`!IsValid()`) attribute if no match is found.
	 * ES: Resuelve un tag de atributo PGX a un `FGameplayAttribute` de GAS buscando el nombre
	 *     leaf del tag contra propiedades `FGameplayAttributeData` en los AttributeSets
	 *     spawneados en el ASC dado. Retorna un atributo invalido si no hay match.
	 */
	static FGameplayAttribute ResolveAttributeByTag(const UAbilitySystemComponent* ASC, FGameplayTag AttributeTag);

	/**
	 * EN: Reverse direction — enumerate all `FGameplayAttributeData` properties on all
	 *     AttributeSets spawned on the ASC, paired with a best-effort reconstructed
	 *     `FGameplayTag` (`PGX.Attribute.Identity.<PropertyName>`, requested non-erroring via
	 *     `RequestGameplayTag(..., false)`). The tag is invalid in the result if the project did
	 *     not register that exact tag — callers (Inspector) should treat an invalid tag as
	 *     "attribute exists, tag convention not registered" rather than failing.
	 * ES: Direccion inversa — enumera todas las propiedades `FGameplayAttributeData` en los
	 *     AttributeSets spawneados, emparejadas con un tag reconstruido best-effort. El tag es
	 *     invalido en el resultado si el proyecto no registro ese tag exacto.
	 */
	static TArray<TPair<FGameplayTag, FGameplayAttribute>> GetAllAttributesOnASC(const UAbilitySystemComponent* ASC);

	/**
	 * EN: Find the spawned `UAttributeSet` instance that owns a given attribute. Public-API
	 *     replacement for `UAbilitySystemComponent::GetAttributeSubobject` (protected in UE5.7) —
	 *     iterates `GetSpawnedAttributes()` and matches by `IsA(Attribute.GetAttributeSetClass())`.
	 * ES: Encuentra la instancia `UAttributeSet` spawneada que posee un atributo dado.
	 *     Reemplazo de API publica para `GetAttributeSubobject` (protected en UE5.7).
	 */
	static const class UAttributeSet* FindOwningAttributeSet(const UAbilitySystemComponent* ASC, const FGameplayAttribute& Attribute);

private:
	/** EN: Extracts the last dot-separated segment of a GameplayTag name. / ES: Extrae el ultimo segmento de un GameplayTag. */
	static FString GetLeafSegment(FGameplayTag Tag);
};
