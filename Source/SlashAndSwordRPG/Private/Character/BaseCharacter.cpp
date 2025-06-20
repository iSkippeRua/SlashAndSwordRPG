#include "Character/BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "Items/Weapon/Weapon.h"
#include "Components/AttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseCharacter::GetHit_Implementation(const FVector& HitPoint, AActor* Attacker)
{
	if(IsAlive() && Attacker)
	{
		DirectionalHitReact(Attacker->GetActorLocation());
	} else Die();
	
	PlayHitSound(HitPoint);
	SpawnHitParticles(HitPoint);
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseCharacter::SetWeaponCollision(ECollisionEnabled::Type CollisionEnabled)
{
	if(EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeapon->IgnoreActorsList.Empty();
	}
}

void ABaseCharacter::Attack()
{
	if(CombatTarget && CombatTarget->ActorHasTag(FName("Dead")))
	{
		CombatTarget = nullptr;
	}
}

bool ABaseCharacter::CanAttack() const
{
	return false;
}

void ABaseCharacter::AttackEnd()
{
	
}

bool ABaseCharacter::IsAlive() const
{
	return Attributes && Attributes->IsAlive();
}

void ABaseCharacter::Die()
{
	Tags.Add(FName("Dead"));
	PlayDeathMontage();
}

void ABaseCharacter::PlayHitReactMontage(const FName& SectionName) const
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

void ABaseCharacter::DirectionalHitReact(const FVector& HitPoint) const
{
	DrawDebugSphere(GetWorld(), HitPoint, 5.0f, 12, FColor::Cyan, false, 10.f);

	const FVector Forward = GetActorForwardVector();
	const FVector HitPointLowered(HitPoint.X, HitPoint.Y, GetActorLocation().Z);
	// finding vector from character to the hit point
	const FVector ToHit = (HitPointLowered - GetActorLocation()).GetSafeNormal();
	// finding the angle between forward vector of the character and the HitVector
	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	double Theta = FMath::Acos(CosTheta);
	Theta = FMath::RadiansToDegrees(Theta);
	// finding CrossProduct to understand the direction of the hit (whether right or left side hit)
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if(CrossProduct.Z < 0)
	{
		Theta *= -1.f;
	}

	FName SectionName = FName("BackHitReact");

	if(Theta >= -45.f && Theta < 45.f)
	{
		SectionName = FName("FrontHitReact");
	} else if(Theta >= -135.f && Theta < -45.f)
	{
		SectionName = FName("LeftHitReact");
	} else if(Theta >= 45.f && Theta < 135.f)
	{
		SectionName = FName("RightHitReact");
	}
	
	PlayHitReactMontage(SectionName);
}

void ABaseCharacter::PlayHitSound(const FVector& HitPoint) const
{
	if(HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			HitSound,
			HitPoint
		);
	}
}

void ABaseCharacter::SpawnHitParticles(const FVector& HitPoint)
{
	if(HitParticles && GetWorld())
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			this,
			HitParticles,
			HitPoint
		);
	}
}

void ABaseCharacter::HandleDamage(float DamageAmount)
{
	if(Attributes)
	{
		Attributes->ReceiveDamage(DamageAmount);
	}
}

void ABaseCharacter::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName) const
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
}

int32 ABaseCharacter::PlayAttackMontage()
{
	return PlayRandomMontageSection(AttackMontage, AttackMontageSections);
}

int32 ABaseCharacter::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames) const
{
	if(SectionNames.Num() <= 0) return -1;
	const int32 MaxSectionIndex = SectionNames.Num() - 1;
	const int32 Selection = FMath::RandRange(0, MaxSectionIndex);
	PlayMontageSection(Montage, SectionNames[Selection]);
	return Selection;
}

int32 ABaseCharacter::PlayDeathMontage()
{
	const int32 Selection = PlayRandomMontageSection(DeathMontage, DeathMontageSections);
	TEnumAsByte<EDeathState> Pose(Selection);
	if(Pose < EDeathState::EDS_MAX)
	{
		DeathState = Pose;
	}
	
	return Selection;
}

void ABaseCharacter::StopAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance)
	{
		AnimInstance->Montage_Stop(0.25f, AttackMontage);
	}
}

FVector ABaseCharacter::GetTranslationWarpTarget()
{
	if(CombatTarget == nullptr) return FVector();

	const FVector CombatTargetLocation = CombatTarget->GetActorLocation();
	const FVector Location = GetActorLocation();
	
	FVector TargetToActor = (Location - CombatTargetLocation).GetSafeNormal();
	TargetToActor *= WarpTargetDistance;

	return CombatTargetLocation + TargetToActor;
}

FVector ABaseCharacter::GetRotationWarpTarget()
{
	if(CombatTarget)
	{
		return CombatTarget->GetActorLocation();
	}
	return FVector();
}

void ABaseCharacter::DisableCapsule() const
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::DisableMeshCollision()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}