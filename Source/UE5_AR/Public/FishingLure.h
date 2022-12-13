// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlaceableActor.h"
#include "FishingLure.generated.h"

class UNiagaraSystem;
class USoundCue;

//! @brief Class handling the fishing lure of the fishing game
UCLASS()
class UE5_AR_API AFishingLure : public APlaceableActor
{
	GENERATED_BODY()

public:
	// Hierarchy

	//! Static mesh component for the actual mesh of the lure
	UPROPERTY(Category = "Hierarchy", VisibleAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* RealLureMeshComponent;

	AFishingLure();
	//~AFishingLure()

protected:

	// Called when the game starts or when spawned
	//virtual void BeginPlay() override;

public:

	//! @brief Called every frame
	//! @param DeltaTime - time difference between frames.
	virtual void Tick(float DeltaTime) override;

	// Assets

	//! [UNUSED] Particle effect asset used for splash effects in water
	UPROPERTY(Category = "Fishing Lure Assets", EditAnywhere, BlueprintReadWrite)
		UNiagaraSystem* SplashSystem = nullptr;

	//! [UNUSED] Particle effect asset displayed when fish is catchable
	UPROPERTY(Category = "Fishing Lure Assets", EditAnywhere, BlueprintReadWrite)
		UNiagaraSystem* LureAlert = nullptr;

	//! Sound "bank" asset for splash sounds
	UPROPERTY(Category = "Fishing Lure Assets", EditAnywhere, BlueprintReadWrite)
		USoundCue* SplashSfx = nullptr;

	// Constants

	//! Height offset unit in world units for the lure to start the falling animation at
	UPROPERTY(Category = "Fishing Lure Constant", EditAnywhere, BlueprintReadWrite)
		float InitialRelativeHeightOffset = 100.0;

	//! Floating animation maximum altitude offset, from the water surface
	UPROPERTY(Category = "Fishing Lure Constant", EditAnywhere, BlueprintReadWrite)
		float FloatingHeightOffset = 3.0;

	//! Floating movement speed towards target
	UPROPERTY(Category = "Fishing Lure Constant", EditAnywhere, BlueprintReadWrite)
		float FloatMovingSpeed = 50.0;

	// Events

	//! @brief [DISABLED] Input event function used when the object is touched.
	//! Called once the type of touch is determined
	//! @param TouchPositionWorld - 2D position of the touch in screen-space.
	virtual void OnTouched(const FVector& TouchPositionWorld) override {};

	//! @brief Input event function called when the player moves suddenly
	//! Called only when sudden movement is being tracked
	//! @param MovementDelta - The direction of the movement in World space.
	virtual void OnSuddenPlayerMove(const FVector& MovementDelta) override;

	// Functions

	//! @brief Function that returns whether the lure should spook the fish
	//! @returns true - If the fish that detect this lure should get scared
	//! @returns false - otherwise
	UFUNCTION(BlueprintCallable, Category = "Fishing Lure State")
		bool IsSpooky() const { return State == Casting; }

	//! @brief Function that returns whether the lure is desirable to the fish
	//! @returns true - If the fish that detect this can be caught
	//! @returns false - otherwise
	UFUNCTION(BlueprintCallable, Category = "Fishing Lure State")
		bool IsDesirable() const { return State == Floating; }

	//! @brief Function that returns whether the lure can be casted
	//! @returns true - If the lure can be casted
	//! @returns false - otherwise
	UFUNCTION(BlueprintCallable, Category = "Fishing Lure State")
		bool IsCastable() const { return bIsCastable; }

	//! @brief Function that start the falling animation, by switching the lure state 
	UFUNCTION(BlueprintCallable, Category = "Fishing Lure State")
		void StartFalling();

	//! @brief Function that starts the reel in animation, by switching the lure state
	UFUNCTION(BlueprintCallable, Category = "Fishing Lure Functionality")
		void ReelIn();

	//! @brief Function that starts the casting animation, by switching the lure state
	//! Switches visibility of both meshes
	UFUNCTION(BlueprintCallable, Category = "Fishing Lure Functionality")
		void CastLure();

	//! @brief Function that allows the fish to (un)mark the lure as "in use"
	//! @param IsCatching - Whether the lure should be considered "in use". 
	UFUNCTION(BlueprintCallable, Category = "Fishing Lure Functionality")
		void SetIsCatchingFish(const bool IsCatching) { bIsCatchingAFish = IsCatching; };

	//! @brief Function that returns whether the lure is considered "in use"
	//! @returns true - If the lure is "in use"
	//! @returns false - otherwise
	UFUNCTION(BlueprintCallable, Category = "Fishing Lure Functionality")
		bool IsCatchingFish() const { return bIsCatchingAFish; };

protected:

	// Hidden

	FVector MockCoro_FallingAnimation_FinalRelativePosition;
	float MockCoro_FallingAnimation_Speed = 150;
	float MockCoro_FallingAnimation_Acceleration = 70;
	int MockCoro_FallingAnimation_AnimationStep = 0;
	//! @brief Mocked coroutine function describing the falling lure animation
	//! Variables above belong to the coroutine
	//! Called in the Tick
	//! @param DeltaTime - Time between frames.
	//! @returns true - If the animation is done.
	//! @returns false - otherwise.
	bool MockCoro_FallingAnimation(const float DeltaTime);

	float MockCoro_FloatingAnimation_SineInput = 0.f;
	//! @brief Mocked coroutine function describing the floating lure animation
	//! Variables above belong to the coroutine
	//! Called in the Tick
	//! @param DeltaTime - Time between frames.
	void MockCoro_FloatingAnimation(const float DeltaTime);

	//! @brief Function performing an update each frame the lure is in visualisation mode
	//! Performs line trace and lure movement
	void VisualisationUpdate();

	//! @brief Function performing an update each frame the lure is in floating mode
	//! @param DeltaTime - Time between frames.
	void FloatingUpdate(const float DeltaTime);

	//! @brief Enumerator specifying the lure mode
	enum LureState
	{
		Visualisation,
		Casting,
		Floating,
	}State = Visualisation;

	//! The position, relative to the pond center, where the lure should float to
	FVector FloatTowardsPoint;

	//! Flag noting whether the lure is "in use"
	bool bIsCatchingAFish = false;

	//! Flag noting whether the lure can be cast
	bool bIsCastable = false;
};
