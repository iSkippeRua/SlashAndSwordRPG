#include "Items/Item.h"
#include "Components/SphereComponent.h"
#include "Interfaces/PickupInterface.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); 
	RootComponent = ItemMesh;
	
	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(GetRootComponent());

	PickupEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara Pickup Effect"));
	PickupEffect->SetupAttachment(GetRootComponent());
}

void AItem::BeginPlay()
{
	Super::BeginPlay();

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);
}

float AItem::TransformedSin()
{
	return Amplitude * FMath::Sin(RunningTime * TimeConstant);
}

float AItem::TransformedCos()
{
	return Amplitude * FMath::Cos(RunningTime * TimeConstant);
}

void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor))
	{
		PickupInterface->SetOverlappingItem(this);
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor))
	{
		PickupInterface->SetOverlappingItem(nullptr);
	}
}

void AItem::SpawnPickupSystem()
{
	if(SoulPickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			SoulPickupEffect,
			GetActorLocation()
			);
	}
}

void AItem::SpawnPickupSound()
{
	if(PickupSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			PickupSound,
			GetActorLocation()
		);
	}
}

void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	RunningTime += DeltaTime;
	
	if(ItemState == EItemState::EIS_Hovering)
	{
		AddActorWorldOffset(FVector(0.f, 0.f, TransformedSin()));
	}
}

