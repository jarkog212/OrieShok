// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomARPawn.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "ARBlueprintLibrary.h"
#include "Camera/CameraComponent.h"
#include "CustomGameMode.h"
#include "HousePlane.h"
#include "PlaceableActor.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"

// Sets default values
ACustomARPawn::ACustomARPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SceneComponent);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BGMAudio"));
	AudioComponent->SetupAttachment(SceneComponent);
};

// Called when the game starts or when spawned
void ACustomARPawn::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(AudioComponent) && IsValid(BgmCue))
	{
		AudioComponent->SetSound(BgmCue);
		AudioComponent->Play();
	}
}

void ACustomARPawn::OnScreenTouchReleaseMandatory(const ETouchIndex::Type FingerIndex, const FVector ScreenPos)
{
	if (InputTouch == TouchType::Deliberating)
		OnScreenTouchRelease(FingerIndex, ScreenPos);

	InputTouch = TouchType::None;
}

// Called every frame
void ACustomARPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Touch Drag Event
	ProcessTouchInput();

	if (bIsProcessingMotion)
		ProcessMotionInput(DeltaTime);

	LastCameraPosition = CameraComponent->GetComponentLocation();
	LastCameraRotation = CameraComponent->GetComponentRotation();
}

// Called to bind functionality to input
void ACustomARPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	//Bind various player inputs to functions
	PlayerInputComponent->BindTouch(IE_Released, this, &ACustomARPawn::OnScreenTouchReleaseMandatory);
}

void ACustomARPawn::OnUIAddNewActorToUI(const TSubclassOf<APlaceableActor> ClassToSpawn)
{
	const auto GM = Cast<ACustomGameMode>(GetWorld()->GetAuthGameMode());
	if (!IsValid(GM) || (IsValid(GM->GetGameplayPlane()) && !GM->GetGameplayPlane()->CanAddMeshToUI()))
		return;

	auto SpawningLocation = CameraComponent->GetComponentLocation();
	auto* NewActor = Cast<APlaceableActor>(GWorld->SpawnActor(ClassToSpawn, &SpawningLocation));
	NewActor->SetAsUIMember(true, this);
	NewActor->Select();
}

void ACustomARPawn::OnUIRemoveSelected()
{
	const auto GM = Cast<ACustomGameMode>(GetWorld()->GetAuthGameMode());

	if (!IsValid(GM) || !IsValid(GM->GetSelectedActor()))
		return;

	auto* ToRemove = GM->GetSelectedActor();

	if (UIMembers.Contains(ToRemove))
		UIMembers.RemoveSingle(ToRemove);

	GWorld->DestroyActor(ToRemove);
}

void ACustomARPawn::OnUISwitchStateTo(const TEnumAsByte<EDisplayMode> NewDisplayMode)
{
	const auto GM = Cast<ACustomGameMode>(GetWorld()->GetAuthGameMode());

	if (!IsValid(GM))
		return;

	//Destructors of individual objects are called by their respective State change handler or GC
	UIMembers.Empty();
	SingleScreenInventory.Empty();
	GM->SetDisplayType(NewDisplayMode);
	SwitchBgm(0);
}

void ACustomARPawn::OnUIStartGame()
{
	const auto GM = Cast<ACustomGameMode>(GetWorld()->GetAuthGameMode());

	if (!IsValid(GM))
		return;

	GM->StartGame();
	GM->SetDisplayType(EDisplayMode::Default);
}

void ACustomARPawn::OnUIStoreItem(const TSubclassOf<APlaceableActor> ActorToStore, int Quantity)
{
	AddToInventory(ActorToStore, Quantity);
}

void ACustomARPawn::OnUISellItemType(const TSubclassOf<APlaceableActor> ActorClassToSell, int Quantity)
{
	const auto* ActorCDO = Cast<APlaceableActor>(ActorClassToSell->GetDefaultObject());
	const auto GM = Cast<ACustomGameMode>(GetWorld()->GetAuthGameMode());

	if (!IsValid(ActorCDO) || !IsValid(GM))
		return;

	const int SellPrice = std::max(1.f, ActorCDO->BuyPrice * ActorCDO->SellPriceDiscount);
	GM->AddMoney(SellPrice * Quantity);
}

void ACustomARPawn::OnUISellActualItem(APlaceableActor* ActorToSell)
{
	const auto GM = Cast<ACustomGameMode>(GetWorld()->GetAuthGameMode());

	if (!IsValid(GM))
		return;

	if (IsInTempInventory(ActorToSell))
		RemoveFromTempInventory(0, ActorToSell);

	const int SellPrice = std::max(1.f,ActorToSell->BuyPrice * ActorToSell->SellPriceDiscount);
	GM->AddMoney(SellPrice);
	GWorld->DestroyActor(ActorToSell);
}

void ACustomARPawn::OnUISetSelectedActorRelativeRotation(const FRotator& Offset)
{
	const auto GM = Cast<ACustomGameMode>(GetWorld()->GetAuthGameMode());

	if (!IsValid(GM) || !IsValid(GM->GetSelectedActor()))
		return;

	auto SelectedActorRotation = GM->GetSelectedActor()->RelativeTransform.GetRotation().Rotator();
	SelectedActorRotation += Offset;
	GM->GetSelectedActor()->RelativeTransform.SetRotation(SelectedActorRotation.Quaternion());
}

void ACustomARPawn::OnUISetSelectedActorRelativeScale(const FVector& Offset)
{
	const auto GM = Cast<ACustomGameMode>(GetWorld()->GetAuthGameMode());

	if (!IsValid(GM) || !IsValid(GM->GetSelectedActor()))
		return;

	auto SelectedActorScale = GM->GetSelectedActor()->StaticMeshComponent->GetComponentScale();
	SelectedActorScale += Offset;
	GM->GetSelectedActor()->StaticMeshComponent->SetWorldScale3D(SelectedActorScale);
}

void ACustomARPawn::OnUISaveHouseLayout()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("OnUISaveHouseLayout()"));

	const auto GM = Cast<ACustomGameMode>(GetWorld()->GetAuthGameMode());

	if (!IsValid(GM) || GM->GetDisplayType() != EDisplayMode::Default || !IsValid(GM->GetGameplayPlane()))
		return;

	auto* Plane = Cast<AHousePlane>(GM->GetGameplayPlane());

	if (IsValid(Plane))
		Plane->ForceStoreLayout();
}

void ACustomARPawn::OnUIChangeBgmVolume(float const VolumeDelta)
{
	BgmAudioVolume += VolumeDelta;
	BgmAudioVolume = FMath::Clamp(BgmAudioVolume, 0, 1);
	AudioComponent->SetFloatParameter("BGMVolume", BgmAudioVolume);
}

void ACustomARPawn::OnUIChangeBgfVolume(float const VolumeDelta)
{
	BgfAudioVolume += VolumeDelta;
	BgfAudioVolume = FMath::Clamp(BgfAudioVolume, 0, 1);
	AudioComponent->SetFloatParameter("BGFVolume", BgfAudioVolume);
}

void ACustomARPawn::OnUIAudioSetRadioEffect(const bool State)
{
	AudioComponent->SetTriggerParameter(State ? "RadioEnable" : "RadioDisable");
}

void ACustomARPawn::AddNewToTempInventory(APlaceableActor* ToAdd)
{
	ToAdd->PinComponent = nullptr;
	ToAdd->SetActorHiddenInGame(true);
	SingleScreenInventory.AddUnique(ToAdd);
}

APlaceableActor* ACustomARPawn::RemoveFromTempInventory(const int Index, APlaceableActor* ItemToRemove, UARPin* NewARPin, const FVector NewWorldLocation)
{
	if (Index >= SingleScreenInventory.Num())
		return nullptr;

	APlaceableActor* Out;

	if(IsValid(ItemToRemove) && IsInTempInventory(ItemToRemove))
	{
		SingleScreenInventory.Remove(ItemToRemove);
		Out = ItemToRemove;
	}
	else
	{
		Out = SingleScreenInventory[Index];
		SingleScreenInventory.Remove(Out);
	}	

	Out->SetActorHiddenInGame(false);
	Out->PinComponent = NewARPin;
	Out->SetARPosition(NewWorldLocation);
	return Out;
}

bool ACustomARPawn::IsInTempInventory(APlaceableActor* ToCheck) const
{
	return SingleScreenInventory.Contains(ToCheck);
}

APlaceableActor* ACustomARPawn::GetAtTempInventory(const int Index) const
{
	if (SingleScreenInventory.Num() <= Index)
		return nullptr;

	return SingleScreenInventory[Index];
}

int ACustomARPawn::QuantityInTempInventory() const
{
	return SingleScreenInventory.Num();
}

void ACustomARPawn::AddToInventory(const TSubclassOf<APlaceableActor> ActorClass, const int Quantity)
{
	if (Quantity <= 0)
		return;

	if (!Inventory.Contains(ActorClass))
		Inventory.Add(ActorClass, Quantity);
	else
		Inventory[ActorClass] += Quantity;
}

void ACustomARPawn::RemoveFromInventory(const TSubclassOf<APlaceableActor> ActorClass,
	const int Quantity)
{
	if (!Inventory.Contains(ActorClass))
		return;

	Inventory[ActorClass] -= Quantity;
	if (Inventory[ActorClass] <= 0)
		Inventory.FindAndRemoveChecked(ActorClass);
}

int ACustomARPawn::QuantityInInventory(const TSubclassOf<APlaceableActor> ActorClass) const
{
	if (!Inventory.Contains(ActorClass))
		return 0;

	return Inventory[ActorClass];
}

const TArray<TSubclassOf<APlaceableActor>> ACustomARPawn::GetAllInventoryItemTypes() const
{
	TArray<TSubclassOf<APlaceableActor>> Out;
	Inventory.GetKeys(Out);
	return Out;
}

void ACustomARPawn::SwitchBgm(const int32 StateParam)
{
	AudioComponent->SetIntParameter("State", StateParam);
}

void ACustomARPawn::SilenceBGMForSFX(float SilenceTime)
{
	AudioComponent->SetFloatParameter("SFXTimeDelay", SilenceTime);
	AudioComponent->SetTriggerParameter("SFXSilence");
}

void ACustomARPawn::OnScreenTouchRelease(const ETouchIndex::Type FingerIndex, const FVector &ScreenPos)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Screen Touch Reached"));
	InputTouch = TouchType::Press;
	HandleTouchInput(FingerIndex, ScreenPos);
}

void ACustomARPawn::OnScreenDrag(const ETouchIndex::Type FingerIndex, const FVector &ScreenPos)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Screen Drag Reached"));
	InputTouch = TouchType::Drag;
	HandleTouchInput(FingerIndex, ScreenPos);
}

void ACustomARPawn::OnSuddenMovement(const FVector& MovementDelta)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Sudden Movement"));
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlaceableActor::StaticClass(), FoundActors);

	for (auto* It : FoundActors)
	{
		auto* FoundActor = IsValid(It) ? Cast<APlaceableActor>(It) : nullptr;

		if (IsValid(FoundActor))
			FoundActor->OnSuddenPlayerMove(MovementDelta);
	}
}

void ACustomARPawn::OnSuddenRotation(const FRotator& RotationDelta)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("SuddentRotation"));
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlaceableActor::StaticClass(), FoundActors);

	for (auto* It : FoundActors)
	{
		auto* FoundActor = IsValid(It) ? Cast<APlaceableActor>(It) : nullptr;

		if (IsValid(FoundActor))
			FoundActor->OnSuddenPlayerRotate(RotationDelta);
	}
}

void ACustomARPawn::HandleTouchInput(const ETouchIndex::Type FingerIndex, const FVector &ScreenPos)
{
	const auto GM = Cast<ACustomGameMode>(GetWorld()->GetAuthGameMode());

	if (!IsValid(GM) || GM->GetDisplayType() == EDisplayMode::Intro)
		return;

	TArray<FARTraceResult> ARHits;
	FHitResult HitResult;
	FVector HitDirection;
	GM->AskForLineTrace(ScreenPos, ARHits, HitResult, HitDirection);

	if (HitResult.IsValidBlockingHit())
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Game Object touched"));
		auto* HitObject = IsValid(HitResult.GetActor()) ? Cast<APlaceableActor>(HitResult.GetActor()) : nullptr;

		if (IsValid(HitObject))
		{
			switch(InputTouch)
			{
				case TouchType::Press:
					HitObject->OnTouched(HitResult.ImpactPoint);
					break;

				case TouchType::Drag:
					HitObject->OnDrag(HitResult.ImpactPoint);
					break;
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Failed Cast To APlaceableActor"));
		}
	}
	else
	{
		if (!ARHits.IsValidIndex(0))
			return;

		GM->HandleGeneralLineTraceResult(ARHits[0], HitDirection);
	}
}

void ACustomARPawn::ProcessTouchInput()
{
	if (const auto* PlayerController = Cast<APlayerController>(GetController()))
	{
		float LocX, LocY;
		bool bIsPressed;
		PlayerController->GetInputTouchState(ETouchIndex::Type::Touch1, LocX, LocY, bIsPressed);

		if (bIsPressed)
		{
			if (InputTouch == TouchType::None)
			{
				TouchTimestamp = FDateTime::Now();
				InputTouch = TouchType::Deliberating;
			}

			const int DeltaTicks = FDateTime::Now().GetTicks() - TouchTimestamp.GetTicks();

			if (DeltaTicks > HoldFrames)
			{
				const FVector CurrentTouchLocation = FVector(LocX, LocY, 0.0f);
				OnScreenDrag(ETouchIndex::Type::Touch1, CurrentTouchLocation);
			}
		}
	}
}

void ACustomARPawn::ProcessMotionInput(const float DeltaTime)
{
	// Possible optimization oof combining the loops within the functions
	const auto PositionDelta = CameraComponent->GetComponentLocation() - LastCameraPosition;

	if (PositionDelta.Length() > MovementMotionSensitivity * DeltaTime)
		OnSuddenMovement(PositionDelta);

	const auto RotationDelta = CameraComponent->GetComponentRotation() - LastCameraRotation;

	if ((RotationDelta.Pitch + RotationDelta.Yaw + RotationDelta.Roll) * DeltaTime > RotationMotionSensitivity)
		OnSuddenRotation(RotationDelta);
}
