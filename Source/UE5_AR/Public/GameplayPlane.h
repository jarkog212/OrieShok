// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlaceableActor.h"
#include "GameplayPlane.generated.h"

//! @brief Class representing the basic functionality of a gameplay plane, holds the game logic and visual representation
UCLASS()
class UE5_AR_API AGameplayPlane : public APlaceableActor
{
	GENERATED_BODY()

public:

	// Hierarchy

	//! Static mesh component for the ring border around the plane
	UPROPERTY(Category = "Hierarchy", VisibleAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* RingMeshComponent = nullptr;

	//! Static mesh compoennt for the ground texture of the plane
	UPROPERTY(Category = "Hierarchy", VisibleAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* TexturePlaneMeshComponent = nullptr;

	// Sets default values for this actor's properties
	AGameplayPlane();
	virtual ~AGameplayPlane() override;

protected:

	//! @brief Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	//! @brief Called every frame
	//! @param DeltaTime - time difference between frames.
	virtual void Tick(float DeltaTime) override;

	// Events

	//! @brief [Disabled] Function used to select an item, including highlighting it.
	//! Only select if selectable
	virtual void Select() override {};

	//! @brief [Disabled] Function used to deselect an item, including de-highlighting it.
	//! Only select if deselectible
	virtual void Deselect() override {};

	//! @brief [Disabled] Input event function used when the object is touched.
	//! Called once the type of touch is determined
	//! @param TouchPositionWorld - 2D position of the touch in screen-space.
	virtual void OnTouched(const FVector &TouchPositionWorld) override {};

	//! @brief [Disabled] Input event function used when the object is dragged on.
	//! Called once the type of touch is determined
	//! @param TouchPositionWorld - 2D position of the touch in screen-space.
	virtual void OnDrag(const FVector &TouchPositionWorld) override {};

	//! @brief Event function called when the game state changes
	//! @param NewMode - The mode that the application is switching into.
	virtual void OnDisplayModeChanged(const TEnumAsByte<EDisplayMode> NewMode) override;

	// Functions

	//! @brief Function called to help the player determine if the object can be added to UI as a member
	//! This is different for each plane, ex. Home only allows one object as UI member
	//! @returns true - If an object can be made an UI member
	//!	@returns false - otherwise
	UFUNCTION(BlueprintCallable, Category = "Gameplay Plane Functionality")
		virtual bool CanAddMeshToUI() { return false; };

	// Constants

	//! Scaling factor of the spawned plane on XY scale, Z is ignored
	UPROPERTY(Category = "Gameplay Plane Constants", EditAnywhere, BlueprintReadOnly)
		float RadiusXYScale = 1.0f;

	//! The speed modifier of the animation of the growing circle, used by materials 
	UPROPERTY(Category = "Gameplay Plane Constants", EditAnywhere, BlueprintReadOnly)
		float GrowthSpeed = 1.0f;

	//! The base colour of the plane area, used by materials
	UPROPERTY(Category = "Gameplay Plane Constants", EditAnywhere, BlueprintReadOnly)
		FColor BaseColor;

	//! The colour of the edge ring and wall, used by materials
	UPROPERTY(Category = "Gameplay Plane Constants", EditAnywhere, BlueprintReadOnly)
		FColor EdgeColor;

	//! Modifier of the emissivity of the rings, used by materials
	UPROPERTY(Category = "Gameplay Plane Constants", EditAnywhere, BlueprintReadOnly)
		float EdgeGlowPower = 1.0f;

	//! Opacity of the base of the plane, not the texture, used by materials
	UPROPERTY(Category = "Gameplay Plane Constants", EditAnywhere, BlueprintReadOnly)
		float BaseOpacity = 0.0f;

	//! Modifier of the overall opacity gradient of the edge wall, used by materials
	UPROPERTY(Category = "Gameplay Plane Constants", EditAnywhere, BlueprintReadOnly)
		float FinalEdgeOverallGradientIntensity = 0.2f;

	//! Scaling of the texture plane under the default one, used by materials
	UPROPERTY(Category = "Gameplay Plane Constants", EditAnywhere, BlueprintReadOnly)
		float TexturePlaneEdgeSize = 1.2f;

	// Assets

	//! Static mesh asset of a plane
	UPROPERTY(Category = "Gameplay Plane Assets", EditAnywhere, BlueprintReadWrite)
		UStaticMesh* RingMesh = nullptr;

	//! Static mesh asset of the ring wall
	UPROPERTY(Category = "Gameplay Plane Assets", EditAnywhere, BlueprintReadWrite)
		UMaterialInterface* RingMaterial = nullptr;

	//! Static mesh of the plane used to display the texture
	UPROPERTY(Category = "Gameplay Plane Assets", EditAnywhere, BlueprintReadWrite)
		UStaticMesh* TexturePlaneMesh = nullptr;

	//! Base material to be used for the texture plane mesh
	UPROPERTY(Category = "Gameplay Plane Assets", EditAnywhere, BlueprintReadWrite)
		UMaterialInterface* TexturePlaneMaterial = nullptr;

protected:

	//Hidden

	float MockCoro_InitialAnimation_UVRadius = 0.f;
	float MockCoro_InitialAnimation_CurrentOverallGradient = 1.f;
	//! @brief Mocked coroutine function describing the initial ring growth animation
	//! Variables above belong to the coroutine
	//! Called in the Tick
	//! @param DeltaTime - Time between frames.
	//! @returns true - If the animation is done.
	//! @returns false - otherwise.
	bool MockCoro_InitialAnimation(const float DeltaTime);

	float MockCoro_EndingProcess_UVRadius = 1.f;
	float MockCoro_EndingProcess_CurrentOverallGradient = 1.f;
	//! @brief Mocked coroutine function describing the closing ring animation
	//! Variables above belong to the coroutine
	//! Called in the Tick
	//! @param DeltaTime - Time between frames.
	//! @returns true - If the animation is done.
	//! @returns false - otherwise.
	bool MockCoro_EndingAnimation(const float DeltaTime);

	//! Flag noting the initialised status
	bool bIsInitialised = false;

	//! Flag noting the closing status
	bool bIsClosing = false;

	//Hidden properties

	//! Modifiable material instance applied to the ring wall.
	UPROPERTY()
		UMaterialInstanceDynamic* RingActualMaterial = nullptr;

	//! Modifiable material instance appllied to the texture plane 
	UPROPERTY()
		UMaterialInstanceDynamic* TexturePlaneActualMaterial = nullptr;
	
};
