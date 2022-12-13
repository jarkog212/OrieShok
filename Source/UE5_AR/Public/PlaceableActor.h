// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "GameFramework/Actor.h"
#include "CustomGameMode.h"
#include "PlaceableActor.generated.h"

class UARPin;
class ACustomARPawn;
class UNiagaraSystem;

//! @brief Base class for AR spawnable Actors, handles interaction with AR manager
UCLASS()
class UE5_AR_API APlaceableActor : public AActor
{
	GENERATED_BODY()

public:

	// Hierarchy
	
	//! Root component
	UPROPERTY(Category = "Hierarchy", VisibleAnywhere, BlueprintReadWrite)
		USceneComponent* SceneComponent;

	//! Default static mesh used for visual representation of the actor
	UPROPERTY(Category = "Hierarchy", VisibleAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* StaticMeshComponent;

	//! Pin, to which the transform should be relative to
	UARPin* PinComponent;

	// Sets default values for this actor's properties
	APlaceableActor();
	~APlaceableActor() override;

protected:
	//! @brief Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	//! @brief Called every frame
	//! @param DeltaTime - time difference between frames.
	virtual void Tick(float DeltaTime) override;

	// Asset Slots

	//! Material asset class, used to build a dynamic version for modifications of to apply on the mesh
	UPROPERTY(Category = "Placeable Actor Assets", EditAnywhere, BlueprintReadWrite)
		UMaterialInterface* Material = nullptr;

	//! Mesh asset to set as the mesh representation of the class
	UPROPERTY(Category = "Placeable Actor Assets", EditAnywhere, BlueprintReadWrite)
		UStaticMesh* Mesh = nullptr;

	//! 2D Texture/Image used for representing the class/object on the UI
	UPROPERTY(Category = "Placeable Actor Assets", EditAnywhere, BlueprintReadWrite)
		UTexture* TextureRepresentation = nullptr;

	//! Particle effect asset to use when spawning
	UPROPERTY(Category = "Placeable Actor Assets", EditAnywhere, BlueprintReadWrite)
		UNiagaraSystem* SpawnPuff = nullptr;

	// Placeable Actor Constants
	
	//! Transformation matrix relative to the AR pin of the object
	UPROPERTY(BlueprintReadWrite)
		FTransform RelativeTransform;

	//! Speed of the rotation when the actor is in 3D mode and "in UI"
	UPROPERTY(Category = "Placeable Actor Constants", EditAnywhere, BlueprintReadWrite)
		float PlacingActorAngularVelocity = 30.0f;

	//! Position relative to the camera offset for the object to follow when in 3D mode and "in UI"
	UPROPERTY(Category = "Placeable Actor Constants", EditAnywhere, BlueprintReadWrite)
		FVector UIObjectCameraOffset = FVector(20, 0, -10);

	//! String name of the object, used in inventory etc.
	UPROPERTY(Category = "Placeable Actor Constants", EditAnywhere, BlueprintReadWrite)
		FString Name = "";

	//! Modifier of the purchase price for calculating the selling price of the item
	UPROPERTY(Category = "Placeable Actor Constants", EditAnywhere, BlueprintReadWrite)
		float SellPriceDiscount = 0.25;

	//! The price of the item in the store
	UPROPERTY(Category = "Placeable Actor Constants", EditAnywhere, BlueprintReadWrite)
		int BuyPrice = 1;

	// Events

	//! @brief Input event function used when the object is touched.
	//! Called once the type of touch is determined
	//! @param TouchPositionWorld - 2D position of the touch in screen-space.
	UFUNCTION(BlueprintCallable, Category = "Placeable Actor Events")
		virtual void OnTouched(const FVector& TouchPositionWorld);

	//! @brief Input event function used when the object is dragged on.
	//! Called once the type of touch is determined
	//! @param TouchPositionWorld - 2D position of the touch in screen-space.
	UFUNCTION(BlueprintCallable, Category = "Placeable Actor Events")
		virtual void OnDrag(const FVector& TouchPositionWorld) {};

	//! @brief Input event function called when the player moves suddenly
	//! Called only when sudden movement is being tracked
	//! @param MovementDelta - The direction of the movement in World space.
	UFUNCTION(BlueprintCallable, Category = "Placeable Actor Events")
		virtual void OnSuddenPlayerMove(const FVector& MovementDelta) {};

	//! @brief [UNUSED] Input event function called when the player rotates suddenly
	//! Called only when sudden rotation is being tracked
	//! @param RotationDelta - The rotation difference.
	UFUNCTION(BlueprintCallable, Category = "Placeable Actor Events")
		virtual void OnSuddenPlayerRotate(const FRotator& RotationDelta) {};

	//! @brief Event function called when the game state changes
	//! @param NewMode - The mode that the application is switching into.
	UFUNCTION(BlueprintCallable, Category = "Placeable Actor Events")
		virtual void OnDisplayModeChanged(const TEnumAsByte<EDisplayMode> NewMode);

	// Functions

	//! @brief Function used to select an item, including highlighting it.
	//! Only select if selectible
	UFUNCTION(BlueprintCallable, Category = "Placeable Actor States")
		virtual void Select();

	//! @brief Function used to deselect an item, including de-highlighting it.
	//! Only select if deselectible
	UFUNCTION(BlueprintCallable, Category = "Placeable Actor States")
		virtual void Deselect();

	//! @brief Function accesing the selection status of the object
	//! @returns true - When selected
	//! @returns false - otherwise
	UFUNCTION(BlueprintCallable, Category = "Placeable Actor States")
		bool GetIsSelected() const { return bIsSelected; };

	//! @brief Function setting the object position from real world to be relative to the tracked pin
	//! Simply sets the world position if no pin is available
	//! @param WorldPosition - 3D world position of the new placement
	UFUNCTION(BlueprintCallable, Category = "Placeable Actor Tranform")
		void SetARPosition(const FVector& WorldPosition);

	//! @brief Function that sets/usets the object to be tranformed based onn the camera transform, "adding" it to UI
	//! If setting to UI mode, Player pawn is required, for the player to add it to UI array
	//! @param State - Whether to set to UI mode or not
	//! @param CurrentUIPlayer - Player pawn handling the UI members
	//! @param RelativeCameraOffset - [Optional] Offset relative to the camera when in UI mode 
	UFUNCTION(BlueprintCallable, Category = "Placeable Actor States")
		void SetAsUIMember(const bool State, ACustomARPawn* CurrentUIPlayer, const FVector RelativeCameraOffset = FVector(20, 0, -10));

	//! @brief Function accessing the UI member status of the object
	//! @returns true - If the object is considered part of UI
	//!	@returns false - otherwise
	UFUNCTION(BlueprintCallable, Category = "Placeable Actor States")
		bool GetIsUIMember() const { return bIsUIMember; };

protected:

	//! @brief Update function called when the object is part of the UI
	//! @param DeltaTime - Time difference between frames.
	virtual void UIMemberUpdate(const float DeltaTime);

	// Hidden

	//! Flag noting the selection status
	bool bIsSelected = false;

	//! Flag noting the UI member status
	bool bIsUIMember = false;

	// Hidden properties
	
	//! Pointer to the player managing the UI members
	UPROPERTY()
		ACustomARPawn* UIPlayer = nullptr;

	//! Dynamic instance of the material applied on the static mesh of the object
	UPROPERTY()
		UMaterialInstanceDynamic* ActualMaterial = nullptr;
};
