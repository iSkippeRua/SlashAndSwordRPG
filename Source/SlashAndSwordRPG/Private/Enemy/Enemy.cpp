#include "Enemy/Enemy.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

class UAnimInstance;

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	
	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AEnemy::GetHit(const FVector& HitPoint)
{
	DirectionalHitReact(HitPoint);

	if(HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			HitSound,
			HitPoint
		);
	}
}

void AEnemy::PlayHitReactMontage(const FName& SectionName) const
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

void AEnemy::DirectionalHitReact(const FVector& HitPoint)
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

	FName SectionName = FName("OnPlaceReactSecond");

	if(Theta >= -45.f && Theta < 45.f)
	{
		SectionName = FName("DynamicReactBackward");
	} else if(Theta >= -135.f && Theta < -45.f)
	{
		SectionName = FName("DynamicReactLeft");
	} else if(Theta >= 45.f && Theta < 135.f)
	{
		SectionName = FName("DynamicReactRight");
	}
	
	PlayHitReactMontage(SectionName);
}
