// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "PGXDemoAuthorCommandlet.generated.h"
UCLASS()
class UPGXDemoAuthorCommandlet : public UCommandlet
{
    GENERATED_BODY()
public:
    UPGXDemoAuthorCommandlet();
    virtual int32 Main(const FString& Params) override;
};
