#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "InputActionValue.h"
#include "CharacterStates.h"
#include "Interfaces/PickupInterface.h"
#include "MainCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class AItem;
class ASoul;
class ATreasure;
class UAnimMontage;
class USlashOverlay;

UCLASS()
class SLASHANDSWORDRPG_API AMainCharacter : public ABaseCharacter, public IPickupInterface
{
	GENERATED_BODY()

public:
	AMainCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetHit_Implementation(const FVector& HitPoint, AActor* Attacker) override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void SetOverlappingItem(AItem* Item) override;
	virtual void AddSoul(ASoul* Soul) override;
	virtual void AddGold(ATreasure* Treasure) override;

	virtual void Jump() override;
	
protected:
	virtual void BeginPlay() override;

	void InitializeSlashOverlay(APlayerController* PlayerController);
	
	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);
	void Equip();
	
	virtual void Attack() override;
	virtual bool CanAttack() const override;
	virtual void AttackEnd() override;
	bool CanArm() const;
	bool CanDisarm() const;
	bool HasStamina() const;
	virtual void Die() override;
	
	void PlayEquipMontage(FName SectionName) const;
	
	UFUNCTION(BlueprintCallable)
	void Disarm();
	UFUNCTION(BlueprintCallable)
	void Arm();
	UFUNCTION(BlueprintCallable)
	void FinishEquipping();
	UFUNCTION(BlueprintCallable)
	void HitReactEnd();
	
	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* MainCharacterContext;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* MovementAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* EquipAction;

	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* AttackAction;
	
private:
	void SetHUDHealth();
	bool IsUnoccupied();
	
	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;
	
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* EquipMontage;

	UPROPERTY()
	USlashOverlay* SlashOverlay;
public:
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }
};
