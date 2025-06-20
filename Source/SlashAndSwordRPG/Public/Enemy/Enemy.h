#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "Character/CharacterStates.h"
#include "Enemy.generated.h"

class UHealthBarComponent;
class UPawnSensingComponent;

UCLASS()
class SLASHANDSWORDRPG_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;

	virtual void GetHit_Implementation(const FVector& HitPoint, AActor* Attacker) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void PawnSeen(APawn* SeenPawn);
	
	virtual void Attack() override;
	virtual bool CanAttack() const override;
	virtual void AttackEnd() override;
	bool InTargetRange(AActor* Target, double Radius) const;
	void MoveToTarget(AActor* Target) const;
	
	virtual void HandleDamage(float DamageAmount) override;
	virtual void Die() override;
	void SpawnSoul();

	UPROPERTY(EditAnywhere, Category = Combat)
	float DeathLifeSpan = 8.f;

	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;
	
	AActor* ChoosePatrolTarget();
	
private:
	/* --- AI Behaviour --- */
	void InitializeEnemy();
	void CheckCombatTarget();
	void CheckPatrolTarget();
	void PatrolTimerFinished() const;
	void HideHealthBar() const;
	void ShowHealthBar() const;
	void LoseInterest();
	void StartPatrolling();
	void StartChasing();
	bool IsOutsideCombatRadius() const;
	bool IsOutsideAttackRadius() const;
	bool IsInsideAttackRadius() const;
	bool IsChasing() const;
	bool IsAttacking() const;
	bool IsEngaged() const;
	bool IsDead() const;
	void ClearPatrolTimer();
	void StartAttackTimer();
	void ClearAttackTimer();
	
	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;
	
	UPROPERTY(VisibleAnywhere);
	UPawnSensingComponent* PawnSensing;

	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<class AWeapon> WeaponClass;

	UPROPERTY(EditAnywhere, Category = Combat)
	double PatrolRadius = 200.f;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	double CombatRadius = 500.f;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	double AttackRadius = 150.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	double AcceptanceRadius = 65.f;

	UPROPERTY()
	class AAIController* EnemyController;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	AActor* PatrolTarget;
	
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	float PatrollingSpeed = 30.f;

	FTimerHandle PatrolTimer;
	FTimerHandle AttackTimer;
	
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float MinWaitTime = 5.f;
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float MaxWaitTime = 10.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float AttackMin = 1.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float AttackMax = 1.5f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ChasingSpeed = 300.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<class ASoul> SoulClass;
};
