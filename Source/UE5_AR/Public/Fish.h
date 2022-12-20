// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlaceableActor.h"
#include "Components/SphereComponent.h"
#include "Fish.generated.h"

class AFishingLure;
class USoundBase;

//! @brief Class of the fishing pond fish actors
UCLASS()
class UE5_AR_API AFish : public APlaceableActor
{
	GENERATED_BODY()

public:

	// Hierarchy

	//! Static mesh component for the actual fish 3D model
	UPROPERTY(Category = "Hierarchy", VisibleAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* RealFishMeshComponent = nullptr;

	//! Collision sphere used for lure detection
	UPROPERTY(Category = "Hierarchy", VisibleAnywhere, BlueprintReadWrite)
		USphereComponent* SwimmingCollider = nullptr;

	AFish();
	//~AFish();

protected:

	//! @brief Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	//! @brief Called every frame
	//! @param DeltaTime - time difference between frames.
	virtual void Tick(float DeltaTime) override;

	// Assets

	//! Actual 3D model of the fish, WITH applied material
	UPROPERTY(Category = "Fish Assets", EditAnywhere, BlueprintReadWrite)
		UStaticMesh* FishMesh = nullptr;

	//! Particle effect for water splash
	UPROPERTY(Category = "Fish Assets", EditAnywhere, BlueprintReadWrite)
		USoundBase* SplashSfx = nullptr;

	// Constants

	//! Fade in animation custom factor, used in materials
	UPROPERTY(Category = "Fish Cosntant", EditAnywhere, BlueprintReadWrite)
		float FadeInOutSpeed = 1.0f;

	//! The width of the silhouette, applied as a base static mesh transform
	UPROPERTY(Category = "Fish Cosntant", EditAnywhere, BlueprintReadWrite)
		float ScaleWidth = 1.0f;

	//! The height of the silhouette, applied as a base static mesh transform
	UPROPERTY(Category = "Fish Cosntant", EditAnywhere, BlueprintReadWrite)
		float ScaleHeight = 1.0f;

	//! The maximum angle offset fot the wiggle animation
	UPROPERTY(Category = "Fish Cosntant", EditAnywhere, BlueprintReadWrite)
		float MaxWiggleAngle = 1.0f;

	//! The speed factor of the wiggle animation
	UPROPERTY(Category = "Fish Cosntant", EditAnywhere, BlueprintReadWrite)
		float WiggleSpeedFactor = 0.25f;

	//! Maximum speed the fish can swim
	UPROPERTY(Category = "Fish Cosntant", EditAnywhere, BlueprintReadWrite)
		float MaxSpeed = 1.0f;

	//! Maximum turning velocity the fish can turn per second
	UPROPERTY(Category = "Fish Cosntant", EditAnywhere, BlueprintReadWrite)
		float MaxAngularVelocity = 1.0f;

	//! Maximum movement difference in a second for a lure to not break when catching fish
	UPROPERTY(Category = "Fish Cosntant", EditAnywhere, BlueprintReadWrite)
		float CatchingMovementRangeTop = 150;

	//! Time interval in seconds for a fish to change the target position 
	UPROPERTY(Category = "Fish Cosntant", EditAnywhere, BlueprintReadWrite)
		float TargetChangeTime = 4.f;

	//! The probability that the fish will choose the lure (if available) as its next target
	UPROPERTY(Category = "Fish Cosntant", EditAnywhere, BlueprintReadWrite)
		int ChoosingLureProbability = 30;

	//! How close the fish can swim towards the target
	UPROPERTY(Category = "Fish Cosntant", EditAnywhere, BlueprintReadWrite)
		float LureDetectionRadius = 35.f;

	//! How many times the fish can change target before leaving the pond for good
	UPROPERTY(Category = "Fish Cosntant", EditAnywhere, BlueprintReadWrite)
		int IterationLifespan = 4;

	//! The class of the placeable actor representation of the fish to be placed in the house etc.
	UPROPERTY(Category = "Fish Cosntant", EditAnywhere, BlueprintReadWrite)
		TSubclassOf<APlaceableActor> PlaceableFishClass;

	// Functionality data

	//! Flag noting whether the fish is guaranteed to escape reel in next frame
	UPROPERTY(Category = "Fish Functionality", EditAnywhere, BlueprintReadWrite)
		bool GuaranteeEscape = false;

	//! The position relative to the center of the pond the fish is trying to reach
	UPROPERTY(Category = "Fish Functionality", VisibleAnywhere, BlueprintReadWrite)
		FVector RelativePointOfInterest;

	// Events

	//! @brief Input event function used when the object is touched.
	//! Called once the type of touch is determined
	//! @param TouchPositionWorld - 2D position of the touch in screen-space.
	virtual void OnTouched(const FVector& TouchPositionWorld) override;

	//! @brief Event function called when the game state changes
	//! @param NewMode - The mode that the application is switching into.
	virtual void OnDisplayModeChanged(const TEnumAsByte<EDisplayMode> NewMode) override;

	//! @brief Input event function called when the player moves suddenly
	//! Called only when sudden movement is being tracked
	//! @param MovementDelta - The direction of the movement in World space.
	virtual void OnSuddenPlayerMove(const FVector& MovementDelta) override;

	//! @brief [UNUSED] Input event function called when the player rotates suddenly
	//! Called only when sudden rotation is being tracked
	//! @param RotationDelta - The rotation difference.
	virtual void OnSuddenPlayerRotate(const FRotator& RotationDelta) override;

	//! @brief Collider response function on entry
	//! @param OverlappedComp - Our component that got overlapped.
	//! @param OtherActor - The actor that collided with us.
	//! @param OtherComp - The component of the other actor that overlapped us.
	//! @param OtherBodyIndex - Index of the other body.
	//! @param bFromSweep - Whether the overlap was caused as a result of our teleportation.
	//! @param SweepResult - HitResult of our teleportation movement.
	UFUNCTION()
		virtual void OnColliderEnter(
			UPrimitiveComponent* OverlappedComp, 
			AActor* OtherActor, 
			UPrimitiveComponent* OtherComp, 
			int32 OtherBodyIndex, 
			bool bFromSweep, 
			const FHitResult& SweepResult
		);

	//! @brief Collider response function on leave
	//! @param OverlappedComp - Our component that got overlapped.
	//! @param OtherActor - The actor that collided with us.
	//! @param OtherComp - The component of the other actor that overlapped us.
	//! @param OtherBodyIndex - Index of the other body.
	UFUNCTION()
		virtual void OnColliderLeave(
			UPrimitiveComponent* OverlappedComp,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex
		);

	// Functions

	//! @brief Function that causes the fish to start leaving the pond
	//! Switches the fish state
	UFUNCTION(BlueprintCallable, Category = "Fish Functionality")
		virtual void LeavePond();

	//! @brief Function that starts reeling in the fish
	//! Will guarantee escape if the lure is already "in use"
	//! Changes the fish state
	UFUNCTION(BlueprintCallable, Category = "Fish Functionality")
		virtual void Catch();

	//! @brief Function that causes the fish to get spooked and starts leaving the pond.
	//! Changes the fish state
	UFUNCTION(BlueprintCallable, Category = "Fish Functionality")
		virtual void SpookFish(const FVector& SpookSource);

	//! @brief Function that checks whether the fish can be Destroyed externally
	//! @returns true - If the fish is not already leaving the pond or escaping reel in.
	//! @returns false - otherwise.
	UFUNCTION(BlueprintCallable, Category = "Fish Functionality")
		bool ShouldNotRemoveFromWorld() const;

protected:

	//Hidden

	//! @brief Update function moving the fish toward the point of interest on the pond
	//! @param DeltaTime - Time between frames
	virtual void MoveTowardsInterest(const float DeltaTime);

	//! @brief Function that checks whether the fish should escape reel in or not.
	//! Only used while being reeled in.
	//! @returns true - If the fish should escape or is guaranteed to escape.
	//! @returns false - otherwise.
	virtual bool ShouldEscape();

	float MockCoro_ConsiderChangingTarget_Timer = 0.f;
	//! @brief Mocked coroutine function chencking and changing the fish target at the specified intervals
	//! Variables above belong to the coroutine
	//! Called in the Tick
	//! @param DeltaTime - Time between frames.
	void MockCoro_ConsiderChangingTarget(const float DeltaTime);

	float MockCoro_FadeInAnimation_CurrentOpacity = 0.f;
	//! @brief Mocked coroutine function describing the fading in animation
	//! Variables above belong to the coroutine
	//! Called in the Tick
	//! @param DeltaTime - Time between frames.
	//! @returns true - If the animation is done.
	//! @returns false - otherwise.
	bool MockCoro_FadeInAnimation(const float DeltaTime);

	float MockCoro_FadeOutAnimation_CurrentOpacity = 1.f;
	//! @brief Mocked coroutine function describing the fading out animation
	//! Variables above belong to the coroutine
	//! Called in the Tick
	//! @param DeltaTime - Time between frames.
	//! @returns true - If the animation is done.
	//! @returns false - otherwise.
	bool MockCoro_FadeOutAnimation(const float DeltaTime);

	bool MockCoro_ReelInAnimation_FirstRun = true;
	//! @brief Mocked coroutine function describing the reeling in animation
	//! Variables above belong to the coroutine
	//! Called in the Tick
	//! @param DeltaTime - Time between frames.
	//! @returns true - If the animation is done.
	//! @returns false - otherwise.
	bool MockCoro_ReelInAnimation(const float DeltaTime);

	float MockCoro_WiggleAnimation_SineInput = 0.f;
	//! @brief Mocked coroutine function describing the wiggling animation
	//! Variables above belong to the coroutine
	//! Called in the Tick
	//! @param DeltaTime - Time between frames.
	void MockCoro_WiggleAnimation(const float DeltaTime);

	//! @brief Enumerator describing the state of the fish
	enum FishState
	{
		Entering,
		Swimming,
		Leaving,
		ReelingIn,
		Caught,
		Escaped,

		//Category flags
		Interactive = ReelingIn
	}State = Entering;

	//! Current swimming speed of the fish in units per second
	float Speed = 20;

	//! Number of times the fish has changed the target
	int Iterations = 0;

	//! Current angular velocity of the fish per second
	float ActualAngularVelocity = 1.f;

	//Hidden properties

	//! The pointer to the colliding fishing lure, can be nullptr
	UPROPERTY()
		AFishingLure* LureInVicinity = nullptr;

	//! Actual material instance from the fish mesh asset
	UPROPERTY()
		UMaterialInstanceDynamic* FishActualMaterial = nullptr;
};
