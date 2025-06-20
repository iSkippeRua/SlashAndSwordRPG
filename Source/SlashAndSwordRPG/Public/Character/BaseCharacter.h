#pragma once

#include "CoreMinimal.h"
#include "CharacterStates.h"
#include "Interfaces/HitInterface.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

class AWeapon;
class UAnimMontage;
class UAttributeComponent;

UCLASS()
class SLASHANDSWORDRPG_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void GetHit_Implementation(const FVector& HitPoint, AActor* Attacker) override;
	bool IsAlive() const;
	virtual bool CanAttack() const;
	virtual void Attack();
	virtual void Die();
	void DirectionalHitReact(const FVector& HitPoint) const;
	virtual void HandleDamage(float DamageAmount);
	void PlayHitSound(const FVector& HitPoint) const;
	void SpawnHitParticles(const FVector& HitPoint);
	void DisableCapsule() const;
	void DisableMeshCollision();

	int32 PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames) const;
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName) const;
	virtual int32 PlayAttackMontage();
	void PlayHitReactMontage(const FName& SectionName) const;
	virtual int32 PlayDeathMontage();
	void StopAttackMontage();

	UFUNCTION(BlueprintCallable)
	FVector GetTranslationWarpTarget();
	UFUNCTION(BlueprintCallable)
	FVector GetRotationWarpTarget();

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollision(ECollisionEnabled::Type CollisionEnabled);

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();
	
	UPROPERTY(VisibleAnywhere, Category = Weapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UAttributeComponent* Attributes;

	UPROPERTY(BlueprintReadOnly, Category = Combat)
	AActor* CombatTarget;

	UPROPERTY(EditAnywhere, Category = Combat)
	double WarpTargetDistance = 75.f;

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EDeathState> DeathState;
private:
	UPROPERTY(EditAnywhere, Category = Sounds)
	USoundBase* HitSound;
	UPROPERTY(EditAnywhere, Category = "Visual Effects")
	UParticleSystem* HitParticles;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* AttackMontage;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* HitReactMontage;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* DeathMontage;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FName> AttackMontageSections;
	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FName> DeathMontageSections;

public:
	FORCEINLINE TEnumAsByte<EDeathState> GetDeathState() const { return DeathState; }
};
