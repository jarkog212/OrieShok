// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "Misc/DateTime.h"
#include "CustomGameMode.h"
#include "CustomARPawn.generated.h"

class UCameraComponent;
class APlaceableActor;
class USoundBase;
class UAudioComponent;

//! @brief The customized pawn class used to represent the player
UCLASS()
class UE5_AR_API ACustomARPawn : public APawn
{
	GENERATED_BODY()

public:

	//Hierarchy

	//! Root component
	UPROPERTY(Category = "Hierarchy", VisibleAnywhere, BlueprintReadWrite)
		USceneComponent* SceneComponent;

	//! Camera component
	UPROPERTY(Category = "Hierarchy", VisibleAnywhere, BlueprintReadWrite)
		UCameraComponent* CameraComponent;

	//! Background music / effects audio component
	UPROPERTY(Category = "Hierarchy", VisibleAnywhere, BlueprintReadWrite)
		UAudioComponent* AudioComponent;

	// Sets default values for this pawn's properties
	ACustomARPawn();

protected:

	//! @brief Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	//! @brief Called every frame
	//! @param DeltaTime - time difference between frames.
	virtual void Tick(float DeltaTime) override;

	//! @brief  Called to bind functionality to input
	//! @param PlayerInputComponent - The input component used for the setup.
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// UI Events

	//! @brief UI event function, called when a new actor is to be spawned as an UI member
	//! @param ClassToSpawn - The Placeable actor class to spawn
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn UI Events")
		void OnUIAddNewActorToUI(const TSubclassOf<APlaceableActor> ClassToSpawn);

	//! @brief UI event function, called to destroy the selected actor
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn UI Events")
		void OnUIRemoveSelected();

	//! @brief UI event function, called to switch the game state
	//! @param NewDisplayMode - The state the application is to switch to.
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn UI Events")
		void OnUISwitchStateTo(const TEnumAsByte<EDisplayMode> NewDisplayMode);

	//! @brief UI event function, called to start the game proper
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn UI Events")
		void OnUIStartGame();

	//! @brief UI event function, function called to store an item into the player's inventory
	//! @param ActorToStore - Placeable actor subclass of the actor to be stored.
	//! @param Quantity - Quantity of actors to store into the inventory.
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn UI Events")
		void OnUIStoreItem(const TSubclassOf<APlaceableActor> ActorToStore, int Quantity);

	//! @brief UI event function, function called to sell the item
	//! @param ActorClassToSell - Placeable actor subclass of the actor.
	//! @param Quantity - Quantity of the items to sell.
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn UI Events")
		void OnUISellItemType(const TSubclassOf<APlaceableActor> ActorClassToSell, int Quantity);

	//! @brief, UI event function, function called to sell the actual instance of the item
	//! Destroys the actor
	//! @param ActorToSell - Pointer to the actual instance to be sold and destroyed.
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn UI Events")
		void OnUISellActualItem(APlaceableActor* ActorToSell);

	//! @brief UI event function, function called to change the selected actor's relative rotation
	//! Rotation transformation of its base static mesh component
	//! @param Offset - Rotation offset to apply.
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn UI Events")
		void OnUISetSelectedActorRelativeRotation(const FRotator& Offset);

	//! @brief UI event function, function called to change the selected actor's relative scale
	//! Scale transformation of the base static mesh component
	//! @param Offset - Scale offset to apply.
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn UI Events")
		void OnUISetSelectedActorRelativeScale(const FVector& Offset);

	//! @brief UI event function, function to save the current house scene layout
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn UI Events")
		void OnUISaveHouseLayout();

	//! @brief UI event function, function to change the BGM volume levels
	//! @param VolumeDelta - The amount by which to change the volume [-1,1]
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn UI Events")
		void OnUIChangeBgmVolume(const float VolumeDelta);

	//! @brief UI event function, function to change the BGF volume levels
	//! @param VolumeDelta - The amount by which to change the volume [-1,1]
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn UI Events")
		void OnUIChangeBgfVolume(const float VolumeDelta);

	//! @brief UI event function, function that switches radio effect of BGM
	//! @param State - Whether to enable or disable the effect.
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn UI Events")
		void OnUIAudioSetRadioEffect(const bool State);

	// Inventory

	//! @brief Function adding a specific placeable actor instance into the temporary inventory
	//! Moves the actor away and sets it invisible
	//! @param ToAdd - Actor instance to be added
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn Inventory")
		void AddNewToTempInventory(APlaceableActor* ToAdd);

	//! @brief Function removing item from the temporary inventory
	//! Actor is placed in the specified location depending on whether a pin is supplied or not
	//! Actor is set to be visible
	//! @param Index - Index of the object in the temporary inventory.
	//! @param ItemToRemove - A pointer to the actor to be removed from inventory.
	//! @param NewArPin - [Optional] AR pin to give ot the actor.
	//! @param NewWorldLocation - [Optional] Location where to put the actor.
	//! @returns [value] - The pointer to the removed object, if available.
	//!	@returns nullptr - otherwise.
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn Inventory")
		APlaceableActor* RemoveFromTempInventory(const int Index, APlaceableActor* ItemToRemove = nullptr, UARPin* NewARPin = nullptr, const FVector NewWorldLocation = FVector(0, 0, 0));

	//! @brief Function to check whether an actor is in the temporary inventory
	//! @param ToCheck - Pointer to the actor whose placement we want to affirm.
	//! @returns true - Actor is present in the temporary inventory.
	//!	@returns false - otherwise.
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn Inventory")
		bool IsInTempInventory(APlaceableActor* ToCheck) const;

	//! @brief Function to return an object at an index from temporary inventory
	//! @param Index - Valid index of the object.
	//! @returns [value] - Pointer to the stored object.
	//! @returns nullptr - Object at an index is not existing, unavailable or wrong index
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn Inventory")
		APlaceableActor* GetAtTempInventory(const int Index) const;

	//! @brief Function to check the number of items in the temporary inventory
	//! @returns [value] - Number of items stored in the temporary inventory.
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn Inventory")
		int QuantityInTempInventory() const;

	//! @brief Function to add an object type into the player's inventory.
	//! @param ActorClass - Placeable actor subclass to store into the inventory.
	//! @param Quantity - Number of items to store.
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn Inventory")
		void AddToInventory(const TSubclassOf<APlaceableActor> ActorClass, const int Quantity = 1);

	//! @brief Function to remove an item from players inventory
	//! Changes quantity of the stored type
	//! @param ActorClass - Placeable actor subclass to store into the inventory.
	//! @param Quantity - Number of items to remove
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn Inventory")
		void RemoveFromInventory(const TSubclassOf<APlaceableActor> ActorClass, const int Quantity = 1);

	//! @brief Function to lookup the quantity of the item type in the inventory
	//! @param ActorClass - Placeable actor subclass to lookup.
	//! @returns [value] - The number of items stored of the type/class.
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn Inventory")
		int QuantityInInventory(const TSubclassOf<APlaceableActor> ActorClass) const;

	//! @brief Function to get the number of different item types store in the player's inventory
	//! @returns [value] - The number of different item types.
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn Inventory")
		const TArray<TSubclassOf<APlaceableActor>> GetAllInventoryItemTypes() const;

	// Functions

	//! @brief Function to switch the BGM/BGF audio.
	//! @param StateParam - Numeric representation of the game state (DisplayType)
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn Audio")
		void SwitchBgm(const int32 StateParam);

	//! @brief Function to access the volume of the BGM
	//! @returns [0,1] - BGM volume level
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn Audio")
		float GetBgmVolume() const { return BgmAudioVolume; };

	//! @brief Function to access the volume of the BGF
	//! @returns [0,1] - BGF volume level
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn Audio")
		float GetBgfVolume() const { return BgfAudioVolume; };

	//! @brief Function that triggers BGM to be silenced (and then re-enabled)
	//! @param SilenceTime - [Optional] The amount of seconds the BGM will be completely silent (not in Attack nor Release)
	UFUNCTION(BlueprintCallable, Category = "Custom AR Pawn Audio")
		void SilenceBGMForSFX(float SilenceTime = 1.5);

	// Constants

	//! The number of frames to wait when determining the touch type
	UPROPERTY(Category = "Custom AR Pawn Constants", EditAnywhere, BlueprintReadWrite)
		int HoldFrames = 10;

	//! The minimum movement delta for a movement to be considered sudden and fire relevant events
	UPROPERTY(Category = "Custom AR Pawn Constants", EditAnywhere, BlueprintReadWrite)
		float MovementMotionSensitivity = 25.f;

	//! The minimum rotation delta for a rotation to be considered sudden and fire relevant events
	UPROPERTY(Category = "Custom AR Pawn Constants", EditAnywhere, BlueprintReadWrite)
		float RotationMotionSensitivity = 20.f;

	// Assets

	//! Sound "Bank" that handles the background music and background effects
	UPROPERTY(Category = "Custom AR Pawn Audio", EditAnywhere, BlueprintReadWrite)
		USoundBase* BgmCue = nullptr;

	// Functionality

	//! A flag noting whether sudden movement and rotation should be tracked and responded to
	UPROPERTY(Category = "Custom AR Pawn Functionality", EditAnywhere, BlueprintReadWrite)
		bool bIsProcessingMotion = false;

	//! An array of objects considered to be a part of UI.
	//! The members get transformed in relation to the camera.
	UPROPERTY(Category = "Custom AR Pawn Functionality", VisibleAnywhere, BlueprintReadWrite)
		TArray<APlaceableActor*> UIMembers;

protected:

	// Hidden

	// Events

	//! @brief Function to be bound for the release touch input. Handles resetting to correct touch type.
	//! @param FingerIndex - The touch index in case multiple touches happen.
	//! @param ScreenPos - Screen space position of the touch.
	void OnScreenTouchReleaseMandatory(const ETouchIndex::Type FingerIndex, const FVector ScreenPos);

	//! @brief Function called when a touch is considered to be non-dragging.
	//! @param FingerIndex - The touch index in case multiple touches happen.
	//! @param ScreenPos - Screen space position of the touch.
	virtual void OnScreenTouchRelease(const ETouchIndex::Type FingerIndex, const FVector &ScreenPos);

	//! @brief Function called when a touch is considered to be dragging.
	//! @param FingerIndex - The touch index in case multiple touches happen.
	//! @param ScreenPos - Screen space position of the touch.
	virtual void OnScreenDrag(const ETouchIndex::Type FingerIndex, const FVector &ScreenPos);

	//! @brief Function called as a response to sudden movement.
	//! @param MovementDelta - The movement difference from last frame in world-space.
	virtual void OnSuddenMovement(const FVector &MovementDelta);

	//! @brief Function called as a response to sudden rotation.
	//! @param RotationDelta - The rotation difference from last frame in world-space.
	virtual void OnSuddenRotation(const FRotator &RotationDelta);

	// Input Processing

	//! @brief Function that propagates the touch input to the correct objects
	//! Performs a line trace
	//! Calls relevant event function of objects or game mode
	//! @param FingerIndex - The touch index in case multiple touches happen.
	//! @param ScreenPos - Screen space position of the touch.
	virtual void HandleTouchInput(const ETouchIndex::Type FingerIndex, const FVector& ScreenPos);

	//! @brief Function used to process the touch input to determine its type.
	void ProcessTouchInput();

	//! @brief Function used to track the movement differences and determine whether they were sudden
	//! Calls relevant event responses if the movements were sudden
	//! @param DeltaTime - Time between frames
	void ProcessMotionInput(const float DeltaTime);

	// Data

	//! @brief Enumerator specifying the type of touch 
	enum class TouchType
	{
		None,
		Deliberating,
		Press,
		Drag
	}InputTouch = TouchType::None;

	//! Timestamp, in system tick count, of the last touch input 
	FDateTime TouchTimestamp = FDateTime::Now();

	//! Cached player rotation form the last frame
	FRotator LastCameraRotation = FRotator(0, 0, 0);

	//! Cached player location from the last frame
	FVector LastCameraPosition = FVector(0, 0, 0);

	//! The volume of the background music
	float BgmAudioVolume = 1.0f;

	//! The volume of the background effects
	float BgfAudioVolume = 1.0f;

	// Hidden properties

	//! Array of actor instances, serves as the temporary inventory
	UPROPERTY()
		TArray<APlaceableActor*> SingleScreenInventory;

	//! Map of the actor types and quantities, serves as the player inventory
	UPROPERTY()
		TMap<TSubclassOf<APlaceableActor>, int> Inventory;
};
