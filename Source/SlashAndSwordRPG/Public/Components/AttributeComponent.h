#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SLASHANDSWORDRPG_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAttributeComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category="Actor Attributes")
	float Health;
	UPROPERTY(EditAnywhere, Category="Actor Attributes")
	float MaxHealth;

	UPROPERTY(EditAnywhere, Category="Actor Attributes")
	float Stamina;
	UPROPERTY(EditAnywhere, Category="Actor Attributes")
	float MaxStamina;
	UPROPERTY(EditAnywhere, Category="Actor Attributes")
	float StaminaRegenRate = 3.f;
	
	UPROPERTY(EditAnywhere, Category="Actor Attributes")
	float AttackCost = 15.f;

	UPROPERTY(EditAnywhere, Category="Actor Attributes")
	int32 Gold;
	UPROPERTY(EditAnywhere, Category="Actor Attributes")
	int32 Souls;
	
public:
	void ReceiveDamage(float Damage);
	void UseStamina(float StaminaCost);
	float GetHealthPercent() const;
	float GetStaminaPercent() const;
	bool IsAlive() const;
	void AddGold(int32 AmountOfGold);
	void AddSouls(int32 NumberOfSouls);
	void RegenStamina(float DeltaTime);
	
	FORCEINLINE int32 GetGold() const { return Gold; }
	FORCEINLINE int32 GetSouls() const { return Souls; }
	FORCEINLINE float GetAttackCost() const { return AttackCost; }
	FORCEINLINE float GetStamina() const { return Stamina; }
};
