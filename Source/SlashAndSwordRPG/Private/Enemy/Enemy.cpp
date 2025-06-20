#include "Enemy/Enemy.h"
#include "Components/AttributeComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HUD/HealthBarComponent.h"
#include "AIController.h"
#include "Character/MainCharacter.h"
#include "Items/Soul.h"
#include "Items/Weapon/Weapon.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/PawnSensingComponent.h"

class UAnimInstance;

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	
	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);
	
	
	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	
	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("Pawn Sensing"));
	PawnSensing->SightRadius = 1500.f;
	PawnSensing->SetPeripheralVisionAngle(45.f);
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	if(PawnSensing) PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	
	InitializeEnemy();
	Tags.Add(FName("Enemy"));
}

void AEnemy::InitializeEnemy()
{
	EnemyController = Cast<AAIController>(GetController());

	HideHealthBar();
	MoveToTarget(PatrolTarget);
	
	UWorld* World = GetWorld();
	if(World && WeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("WeaponSocket"), this, this);
		EquippedWeapon = DefaultWeapon;
	}
}


void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(IsDead()) return;
	if(EnemyState > EEnemyState::EES_Patrolling)
	{
		CheckCombatTarget();
	} else
	{
		CheckPatrolTarget();
	}
}

void AEnemy::GetHit_Implementation(const FVector& HitPoint, AActor* Attacker)
{
	Super::GetHit_Implementation(HitPoint, Attacker);
	if(!IsDead()) ShowHealthBar();
	ClearPatrolTimer();
	ClearAttackTimer();

	SetWeaponCollision(ECollisionEnabled::NoCollision);
	StopAttackMontage();
	if(IsInsideAttackRadius())
	{
		if(!IsDead()) StartAttackTimer();
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	CombatTarget = EventInstigator->GetPawn();
	if(IsInsideAttackRadius())
	{
		EnemyState = EEnemyState::EES_Attacking;
	} else if(IsOutsideAttackRadius())
	{
		StartChasing();
	}
	return DamageAmount;
}

void AEnemy::Attack()
{
	Super::Attack();
	if(CombatTarget == nullptr) return;
	EnemyState = EEnemyState::EES_Engaged;
	PlayAttackMontage();
}

bool AEnemy::CanAttack() const
{
	bool bCanAttack =
		IsInsideAttackRadius() &&
		!IsAttacking() &&
		!IsEngaged() &&
		!IsDead();
	return bCanAttack;
}

void AEnemy::AttackEnd()
{
	EnemyState = EEnemyState::EES_NoState;
	CheckCombatTarget();
}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);
	if(Attributes && HealthBarWidget)
	{
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());
	}
}

void AEnemy::Die()
{
	Super::Die();
	EnemyState = EEnemyState::EES_Dead;
	ClearAttackTimer();
	HideHealthBar();
	DisableCapsule();
	SetLifeSpan(DeathLifeSpan);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	SetWeaponCollision(ECollisionEnabled::NoCollision);
	SpawnSoul();
}

void AEnemy::SpawnSoul()
{
	UWorld* World = GetWorld();
	if(World && SoulClass && Attributes)
	{
		const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 40.f, 0.f);
		ASoul* SpawnedSoul = World->SpawnActor<ASoul>(SoulClass, SpawnLocation, GetActorRotation());
		if(SpawnedSoul)
		{
			SpawnedSoul->SetSouls(Attributes->GetSouls());
		}
	}
}

bool AEnemy::IsDead() const
{
	return EnemyState == EEnemyState::EES_Dead;
}

void AEnemy::Destroyed()
{
	if(EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
}

bool AEnemy::InTargetRange(AActor* Target, double Radius) const
{
	if(Target == nullptr) return false;
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	return DistanceToTarget <= Radius;
}

void AEnemy::PatrolTimerFinished() const
{
	MoveToTarget(PatrolTarget);
}

void AEnemy::HideHealthBar() const
{
	if(HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);
	}
}

void AEnemy::ShowHealthBar() const
{
	if(HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(true);
	}
}

void AEnemy::LoseInterest()
{
	CombatTarget = nullptr;
	HideHealthBar();
}

void AEnemy::StartPatrolling()
{
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
	MoveToTarget(PatrolTarget);
}

void AEnemy::StartChasing()
{
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	MoveToTarget(CombatTarget);
}

bool AEnemy::IsOutsideCombatRadius() const
{
	return !InTargetRange(CombatTarget, CombatRadius);
}

bool AEnemy::IsOutsideAttackRadius() const
{
	return !InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsInsideAttackRadius() const
{
	return InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsChasing() const
{
	return EnemyState == EEnemyState::EES_Chasing;
}

bool AEnemy::IsAttacking() const
{
	return EnemyState == EEnemyState::EES_Attacking;
}

bool AEnemy::IsEngaged() const
{
	return EnemyState == EEnemyState::EES_Engaged;
}

void AEnemy::StartAttackTimer()
{
	EnemyState = EEnemyState::EES_Attacking;
	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
}

void AEnemy::MoveToTarget(AActor* Target) const
{
	if(EnemyController == nullptr || Target == nullptr) return;
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	EnemyController->MoveTo(MoveRequest);
}

void AEnemy::CheckCombatTarget()
{
	if(IsOutsideCombatRadius())
	{
		ClearAttackTimer();
		LoseInterest();
		if(!IsEngaged())
		{
			StartPatrolling();
		}
	}
	else if(IsOutsideAttackRadius() && !IsChasing())
	{
		ClearAttackTimer();
		if(!IsEngaged())
		{
			StartChasing();
		}
	}
	else if(CanAttack())
	{
		StartAttackTimer();
	}
}

void AEnemy::CheckPatrolTarget()
{
	if(InTargetRange(PatrolTarget, PatrolRadius))
	{
		PatrolTarget = ChoosePatrolTarget();
		const float RandomWaitTime = FMath::RandRange(MinWaitTime, MaxWaitTime);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, RandomWaitTime);
	}
}

AActor* AEnemy::ChoosePatrolTarget()
{
	TArray<AActor*> ValidTargets;
	for(AActor* Target : PatrolTargets)
	{
		if(Target != PatrolTarget)
		{
			ValidTargets.AddUnique(Target);
		}
	}
			
	const int32 NumberOfPatrolTargets = ValidTargets.Num();
	if(NumberOfPatrolTargets > 0)
	{
		const int32 TargetSelection = FMath::RandRange(0, NumberOfPatrolTargets - 1);
		return ValidTargets[TargetSelection];
	}
	
	return nullptr;
}

void AEnemy::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

void AEnemy::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	const bool bShouldChaseTarget =
			EnemyState != EEnemyState::EES_Dead &&
			EnemyState != EEnemyState::EES_Chasing &&
			EnemyState < EEnemyState::EES_Attacking &&
			SeenPawn->ActorHasTag(FName("EngageableTarget"));

	if(bShouldChaseTarget)
	{
		CombatTarget = SeenPawn;
		ClearPatrolTimer();
		StartChasing();
	}
}