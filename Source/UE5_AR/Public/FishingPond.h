// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayPlane.h"
#include "Fish.h"
#include "FishingLure.h"
#include "FishingPond.generated.h"

//! @brief Gameplay plane used for fishing state, handles the fishing minigame game logic 
UCLASS()
class UE5_AR_API AFishingPond : public AGameplayPlane
{
	GENERATED_BODY()

public:

	//AFishingPond();
	virtual ~AFishingPond() override;

protected:

	//! @brief Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	//! @brief Called every frame
	//! @param DeltaTime - time difference between frames.
	virtual void Tick(float DeltaTime) override;

	// Functions

	//! @brief Function that spawns a new fish in the lake and specifies its first target.
	//! @param ClassIndex - Index of the stored fish class to spawn.
	//! @param RelativePosition - The initial spawning position, relative to the center of the pond.
	//! @param PointOfInterest - The fist position the fish will target relative to the center of the pond.
	//! @returns [value] - Pointer to the spawned fish actor, if spawned.
	//! @returns nullptr - otherwise. 
	UFUNCTION(BlueprintCallable, Category = "Fishing Pond Functionality")
		AFish* AddFish(const int ClassIndex, const FVector& RelativePosition, const FVector& PointOfInterest = FVector(0,1, 0));

	//! @brief Remove the provided fish from the lake by destroying it.
	//! Used to keep track of the fish in the pond.
	//! Will not destroy the fish if its is leaving the pond or escaping reel in on its own.
	//! @param FishToRemove - Fish actor to leave the pond.
	UFUNCTION(BlueprintCallable, Category = "Fishing Pond Functionality")
		void RemoveFish(AFish* FishToRemove);

	//! @brief Function to gather the position of the lure in the pond, if it can be gathered.
	//! @param LureRelativeLocation - [OUT] Reference used to return the lure position, if available.
	//! @returns true - if the position can be/was gathered.
	//! @returns false - otherwise
	UFUNCTION(BlueprintCallable, Category = "Fishing Pond Functionality")
		bool GetValidLureRelativeLocation(FVector& LureRelativeLocation);

	// Assets

	//! The array of fish classes spawnable on the lake
	UPROPERTY(Category = "Fishing Pond Assets", EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<AFish>> FishClasses;

	//! The blueprint class of the lure object to spawn
	UPROPERTY(Category = "Fishing Pond Assets", EditAnywhere, BlueprintReadWrite)
		TSubclassOf<AFishingLure> LureClass;

	// Constants

	//! Maximum number of fish at a time in the pond
	UPROPERTY(Category = "Fishing Pond Constants", EditAnywhere, BlueprintReadOnly)
		int MaxFish = 5;

	//! Time interval for the pond to try to spawn new fish
	UPROPERTY(Category = "Fishing Pond Constants", EditAnywhere, BlueprintReadOnly)
		float FishSpawnTime = 2.f;

	//! Maximum number of caught fish in the temporary inventory
	UPROPERTY(Category = "Fishing Pond Constants", EditAnywhere, BlueprintReadOnly)
		int MaxCaughtFishCapacity = 3;

protected:

	//Hidden

	float MockCoro_FishSpawner_Timer = 0.f;
	//! @brief Mocked coroutine function spawning the fish at given intervals
	//! Variables above belong to the coroutine
	//! Called in the Tick
	//! @param DeltaTime - Time between frames.
	void MockCoro_FishSpawner(const float DeltaTime); 

	//! The number of fish currently present in the pond
	int CurrentFishCount = 0;

	//Hidden properties

	//! Pointer to the fishing lure object
	UPROPERTY()
		AFishingLure* PlayerLure = nullptr;
};
