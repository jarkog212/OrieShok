// Fill out your copyright notice in the Description page of Project Settings.

#include "FishingPond.h"

#include "CustomARPawn.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

AFishingPond::~AFishingPond()
{
	// Disable Player motion sensing
	auto* Player = IsValid(UGameplayStatics::GetPlayerPawn(this, 0)) ?
		Cast<ACustomARPawn>(UGameplayStatics::GetPlayerPawn(this, 0)) :
		nullptr;

	if (IsValid(Player))
		Player->bIsProcessingMotion = false;
}

void AFishingPond::BeginPlay()
{
	Super::BeginPlay();

	// Enable motion processing of the player
	auto* Player = IsValid(UGameplayStatics::GetPlayerPawn(this, 0)) ?
		Cast<ACustomARPawn>(UGameplayStatics::GetPlayerPawn(this, 0)) :
		nullptr;

	if (IsValid(Player))
		Player->bIsProcessingMotion = true;
}

void AFishingPond::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Has to be done here as the PinComponent is assigned after BeginPlay()
	if (!IsValid(PlayerLure) && IsValid(LureClass) && !bIsClosing)
	{
		PlayerLure = Cast<AFishingLure>(GWorld->SpawnActor(LureClass));
		if (IsValid(PlayerLure))
			PlayerLure->PinComponent = PinComponent;
	}

	if (!bIsClosing)
		MockCoro_FishSpawner(DeltaTime);
}

AFish* AFishingPond::AddFish(const int ClassIndex, const FVector &RelativePosition, const FVector& PointOfInterest)
{
	if (CurrentFishCount >= MaxFish || ClassIndex < 0 || ClassIndex >= FishClasses.Num())
		return nullptr;

	auto* NewActor = Cast<AFish>(GWorld->SpawnActor(FishClasses[ClassIndex]));
	NewActor->PinComponent = PinComponent;
	NewActor->RelativeTransform.SetLocation(RelativePosition);
	NewActor->RelativePointOfInterest = PointOfInterest;
	CurrentFishCount++;
	return NewActor;
}

void AFishingPond::RemoveFish(AFish* FishToRemove)
{
	if (CurrentFishCount <= 0 || !IsValid(FishToRemove))
		return;

	//For removing fish that did not call this as a result of leaving the pond
	if (!FishToRemove->ShouldNotRemoveFromWorld())
		GWorld->DestroyActor(FishToRemove);

	CurrentFishCount--;
}

bool AFishingPond::GetValidLureRelativeLocation(FVector& LureRelativeLocation)
{
	if (!PlayerLure->IsDesirable())
		return false;

	LureRelativeLocation = PlayerLure->RelativeTransform.GetLocation();
	return true;
}

void AFishingPond::MockCoro_FishSpawner(const float DeltaTime)
{
	MockCoro_FishSpawner_Timer += DeltaTime;
	if (MockCoro_FishSpawner_Timer < FishSpawnTime)
		return;

	AddFish(
		FMath::RandRange(0, FishClasses.Num() - 1),
		FVector(FMath::RandRange(-100, 100), FMath::RandRange(-100, 100), -1),
		FVector(FMath::RandRange(-100, 100), FMath::RandRange(-100, 100), 0)
	);

	MockCoro_FishSpawner_Timer = 0.f;
}
