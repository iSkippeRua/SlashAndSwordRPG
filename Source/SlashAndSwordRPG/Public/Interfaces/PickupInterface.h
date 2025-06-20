#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PickupInterface.generated.h"

UINTERFACE(MinimalAPI)
class UPickupInterface : public UInterface
{
	GENERATED_BODY()
};

class SLASHANDSWORDRPG_API IPickupInterface
{
	GENERATED_BODY()

public:
	virtual void SetOverlappingItem(class AItem* Item);
	virtual void AddSoul(class ASoul* Soul);
	virtual void AddGold(class ATreasure* Treasure);
};
