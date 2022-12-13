// Fill out your copyright notice in the Description page of Project Settings.

#include "HousePlane.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "CustomARPawn.h"
#include "ARPin.h"

AHousePlane::AHousePlane()
{
	PrimaryActorTick.bCanEverTick = true;

	HouseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HouseMeshComponent"));
	HouseMeshComponent->SetupAttachment(SceneComponent);
	HouseMeshComponent->SetVisibility(false);
}

void AHousePlane::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(DefaultHouse))
		HouseMeshComponent->SetStaticMesh(DefaultHouse);
}

void AHousePlane::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsHouseInitialized && MockCoro_HouseIntroAnimation(DeltaTime))
	{
		 bIsHouseInitialized = true;
		 LoadLayout();
	}

	AutosaveTimer += DeltaTime;

	if (AutosaveTimer >= AutosaveFrequencySeconds || bForceSaveNextTick)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Tick()"));

		bForceSaveNextTick = false;
		AutosaveTimer = 0.f;
		StoreLayout();
	} 
}

void AHousePlane::OnTouched(const FVector& TouchPositionWorld)
{
	const auto* GM = Cast<ACustomGameMode>(GetWorld()->GetAuthGameMode());
	if (!IsValid(GM))
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("AGameplayPlane::OnTouched - No GM"));
		return;
	}

	auto* SelectedActor = GM->GetSelectedActor();

	if (!IsValid(SelectedActor) || !bIsHouseInitialized)
		return;

	if (!IsValid(SelectedActor->PinComponent))
		SelectedActor->PinComponent = this->PinComponent;

	SelectedActor->SetARPosition(TouchPositionWorld);
	SelectedActor->SetAsUIMember(false, nullptr);
}

void AHousePlane::OnDrag(const FVector& TouchPositionWorld)
{
	OnTouched(TouchPositionWorld);
}

bool AHousePlane::CanAddMeshToUI()
{
	auto* Player = IsValid(UGameplayStatics::GetPlayerPawn(this, 0)) ?
		Cast<ACustomARPawn>(UGameplayStatics::GetPlayerPawn(this, 0)) :
		nullptr;

	if (IsValid(Player))
		return Player->UIMembers.IsEmpty();

	return false;
}

bool AHousePlane::MockCoro_HouseIntroAnimation(const float DeltaTime)
{
	if (!(MockCoro_HouseIntroAnimation_Completion > 0.f))
	{
		auto CurrentRelativeLocation = HouseMeshComponent->GetRelativeLocation();
		CurrentRelativeLocation.Z = InitialHouseRelativeHeightOffset;
		HouseMeshComponent->SetRelativeLocation(CurrentRelativeLocation);
		HouseMeshComponent->SetRelativeRotation(FRotator(0, 90 + InitialHouseRelativeRotationOffset, 0));
		HouseMeshComponent->SetVisibility(true);
	}

	MockCoro_HouseIntroAnimation_Completion += DeltaTime / AnimationDuration;

	if (MockCoro_HouseIntroAnimation_Completion > 1.f)
		MockCoro_HouseIntroAnimation_Completion = 1.f;

	float RelativeTimeLeft = 1.f - MockCoro_HouseIntroAnimation_Completion;
	float CurrentHeight = RelativeTimeLeft * InitialHouseRelativeHeightOffset;
	float CurrentRotation = RelativeTimeLeft * InitialHouseRelativeRotationOffset;
	auto CurrentRelativeLocation = HouseMeshComponent->GetRelativeLocation();
	CurrentRelativeLocation.Z = CurrentHeight;
	HouseMeshComponent->SetRelativeLocation(CurrentRelativeLocation);
	HouseMeshComponent->SetRelativeRotation(FRotator(0, 90 + CurrentRotation, 0));

	return MockCoro_HouseIntroAnimation_Completion >= 1.f;
}

void AHousePlane::LoadLayout()
{
	auto* GM = Cast<ACustomGameMode>(UGameplayStatics::GetGameMode(this));
	if (!IsValid(GM) || !IsValid(GetWorld()))
		return;

	auto StoredLayout = GM->GetLayoutData();

	for(auto& It : StoredLayout)
	{
		auto SpawnTransform = FTransform(It.GeneralRelativeTransform * PinComponent->GetLocalToWorldTransform());
		auto SpawnedActor = GetWorld()->SpawnActor(It.Class, &SpawnTransform);
		auto SpawnedPlaceable = Cast<APlaceableActor>(SpawnedActor);

		if (!IsValid(SpawnedPlaceable) || !IsValid(PinComponent))
		{
			GetWorld()->DestroyActor(SpawnedActor);
			continue;
		}

		SpawnedPlaceable->PinComponent = PinComponent;
		SpawnedPlaceable->RelativeTransform = It.GeneralRelativeTransform;
		SpawnedPlaceable->StaticMeshComponent->SetRelativeTransform(It.StaticMeshTransform);
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Loaded Object!!"));

	}
}

void AHousePlane::StoreLayout()
{
	auto* GM = Cast<ACustomGameMode>(UGameplayStatics::GetGameMode(this));
	if (!IsValid(GM))
		return;

	TArray<FLayoutData> LayoutData;
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(this, APlaceableActor::StaticClass(), FoundActors);

	for (auto* It : FoundActors)
	{
		if (IsValid(Cast<AGameplayPlane>(It)))
			continue;

		const auto* ActorToStore = Cast<APlaceableActor>(It);

		if (!IsValid(ActorToStore) || ActorToStore->GetIsUIMember())
			continue;

		FLayoutData ArrayElement;
		ArrayElement.Class = ActorToStore->GetClass();
		ArrayElement.GeneralRelativeTransform = ActorToStore->RelativeTransform;
		ArrayElement.StaticMeshTransform = ActorToStore->StaticMeshComponent->GetRelativeTransform();

		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Saved Object!!"));

		LayoutData.Add(ArrayElement);
	}

	GM->StoreLayoutData(LayoutData);
}
