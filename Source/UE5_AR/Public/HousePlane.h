// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayPlane.h"
#include "HousePlane.generated.h"

//! @brief Gameplay plane class used for the house state, allowing placing the items and saving the layout
UCLASS()
class UE5_AR_API AHousePlane : public AGameplayPlane
{
	GENERATED_BODY()

public:
	// Hierarchy

	//! Static mesh component of the house mesh
	UPROPERTY(Category = "Hierarchy", VisibleAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* HouseMeshComponent = nullptr;

	AHousePlane();
	// virtual AGameplayPlane() override;

protected:

	//! @brief Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	
	//! @brief Called every frame
	//! @param DeltaTime - time difference between frames.
	virtual void Tick(float DeltaTime) override;

	// Events

	//! @brief Input event function used when the object is touched.
	//! Called once the type of touch is determined
	//! @param TouchPositionWorld - 2D position of the touch in screen-space.
	virtual void OnTouched(const FVector& TouchPositionWorld) override;

	//! @brief Input event function used when the object is dragged on.
	//! Called once the type of touch is determined
	//! @param TouchPositionWorld - 2D position of the touch in screen-space.
	virtual void OnDrag(const FVector& TouchPositionWorld) override;

	// Functions

	//! @brief Function called to help the player determine if the object can be added to UI as a member
	//! This is different for each plane, ex. Home only allows one object as UI member
	//! @returns true - If an object can be made an UI member
	//!	@returns false - otherwise
	virtual bool CanAddMeshToUI() override;

	//! @brief Function that when called, ensure the layout of the house will be saved.
	//! Saving is performed next frame
	UFUNCTION(BlueprintCallable, Category = "House Plane Functionality")
		void ForceStoreLayout() { bForceSaveNextTick = true; };

	// Constants

	//! [UNUSED] Maximum number of placed object in the scene
	UPROPERTY(Category = "House Plane Constants", EditAnywhere, BlueprintReadOnly)
		int MaxObjects = 10;

	//! Seconds to wait between saving the layouts automatically
	UPROPERTY(Category = "House Plane Constants", EditAnywhere, BlueprintReadOnly)
		int AutosaveFrequencySeconds = 300;

	//! Height offset to start the house mesh at for the animation
	UPROPERTY(Category = "House Plane Constants", EditAnywhere, BlueprintReadOnly)
		float InitialHouseRelativeHeightOffset = 30.f;

	//! Rotation offset to start the house mesh at for the animation
	UPROPERTY(Category = "House Plane Constants", EditAnywhere, BlueprintReadOnly)
		float InitialHouseRelativeRotationOffset = 180.f;

	//! Duration of the intro animation, in seconds 
	UPROPERTY(Category = "House Plane Constants", EditAnywhere, BlueprintReadOnly)
		float AnimationDuration = 2.f;

	// Assets

	//! House static mesh asset
	UPROPERTY(Category = "House Plane Assets", EditAnywhere, BlueprintReadWrite)
		UStaticMesh* DefaultHouse = nullptr;

protected:

	//Hidden

	float MockCoro_HouseIntroAnimation_Completion = 0.f;
	//! @brief Mocked coroutine function describing the falling house intro animation
	//! Variables above belong to the coroutine
	//! Called in the Tick
	//! @param DeltaTime - Time between frames.
	//! @returns true - If the animation is done.
	//! @returns false - otherwise.
	bool MockCoro_HouseIntroAnimation(const float DeltaTime);

	//! @brief Function that accesses stored layout and spawns the objects back to scene with correct transforms
	void LoadLayout();

	//! @brief Function that iterates through spawned objects and saves their class and transform into layout array
	void StoreLayout();

	//! Timer used to track when to autosave
	float AutosaveTimer = 0.f;

	//! Flag noting whether to force an autosave next frame
	bool bForceSaveNextTick = false;

	//! Flag noting whehter the house plane is initialized
	bool bIsHouseInitialized = false;
};
