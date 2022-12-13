// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayPlane.h"
#include "CustomGameMode.h"
#include "ARPin.h"
#include "CustomARPawn.h"
#include "ARBlueprintLibrary.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "ARBlueprintLibrary.h"

// Sets default values
AGameplayPlane::AGameplayPlane()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RingMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RingMeshComponent"));
	RingMeshComponent->SetupAttachment(StaticMeshComponent);

	TexturePlaneMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TexturePlaneMeshComponent"));
	TexturePlaneMeshComponent->SetupAttachment(StaticMeshComponent);
}

AGameplayPlane::~AGameplayPlane()
{
	if (IsValid(PinComponent))
	{
		PinComponent->RemoveFromRoot();
		UARBlueprintLibrary::RemovePin(PinComponent);
	}
}

// Called when the game starts or when spawned
void AGameplayPlane::BeginPlay()
{
	Super::BeginPlay();

	// Init Ring
	if (IsValid(RingMesh))
		RingMeshComponent->SetStaticMesh(RingMesh);
	else
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("No Mesh (AGameplayPlane::BeginPlay())"));

	if (IsValid(RingMaterial))
		RingActualMaterial = UMaterialInstanceDynamic::Create(RingMaterial, NULL);
	else
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("No Material (AGamplayPlane::BeginPlay())"));

	if (IsValid(RingActualMaterial))
	{
		RingMeshComponent->SetMaterial(0, RingActualMaterial);
		RingActualMaterial->SetVectorParameterValue("Color", EdgeColor);
		RingActualMaterial->SetScalarParameterValue("OverallGradientIntensity", 1.0);
		RingActualMaterial->SetScalarParameterValue("EmissivityPower", EdgeGlowPower);
	}

	// Init Texture Plane
	if (IsValid(TexturePlaneMesh))
		TexturePlaneMeshComponent->SetStaticMesh(TexturePlaneMesh);
	else
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("No Mesh (AGameplayPlane::BeginPlay())"));

	if (IsValid(TexturePlaneMaterial))
		TexturePlaneActualMaterial = UMaterialInstanceDynamic::Create(TexturePlaneMaterial, NULL);
	else
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("No Material (AGamplayPlane::BeginPlay())"));

	if (IsValid(TexturePlaneActualMaterial))
	{
		TexturePlaneMeshComponent->SetMaterial(0, TexturePlaneActualMaterial);
		TexturePlaneActualMaterial->SetScalarParameterValue("CircleSharpness", 2);
		TexturePlaneActualMaterial->SetScalarParameterValue("UVDiameter", 0.0);
	}

	// Transform and other aspect changes
	StaticMeshComponent->SetRelativeScale3D(FVector(RadiusXYScale, RadiusXYScale, 1.0f));
	TexturePlaneMeshComponent->SetRelativeScale3D(FVector(TexturePlaneEdgeSize, TexturePlaneEdgeSize, 1.0f));
	TexturePlaneMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Material Init
	if (IsValid(ActualMaterial))
	{
		ActualMaterial->SetVectorParameterValue("BaseColor", BaseColor);
		ActualMaterial->SetVectorParameterValue("EdgeColor", EdgeColor);
		ActualMaterial->SetScalarParameterValue("BaseOpacityFactor", BaseOpacity);
		ActualMaterial->SetScalarParameterValue("EdgeOpacityFactor", 1.0);
		ActualMaterial->SetScalarParameterValue("CircleSharpness", 310);
		ActualMaterial->SetScalarParameterValue("UVDiameter", 0.0);
		ActualMaterial->SetScalarParameterValue("EmissivityPower", EdgeGlowPower);
	}
}

bool AGameplayPlane::MockCoro_InitialAnimation(const float DeltaTime)
{
	//Function written like a coroutine in case it ever gets support
	if (MockCoro_InitialAnimation_UVRadius < 1.0f)
	{
		MockCoro_InitialAnimation_UVRadius += DeltaTime * GrowthSpeed;

		if (MockCoro_InitialAnimation_UVRadius > 1.0f)
			MockCoro_InitialAnimation_UVRadius = 1.0f;

		if (IsValid(ActualMaterial))
			ActualMaterial->SetScalarParameterValue("UVDiameter", MockCoro_InitialAnimation_UVRadius);

		if (IsValid(TexturePlaneActualMaterial))
			TexturePlaneActualMaterial->SetScalarParameterValue("UVDiameter", MockCoro_InitialAnimation_UVRadius);

		return false;
	}
	else if (MockCoro_InitialAnimation_CurrentOverallGradient > FinalEdgeOverallGradientIntensity)
	{
		MockCoro_InitialAnimation_CurrentOverallGradient -= DeltaTime * (GrowthSpeed / 2);

		if (IsValid(RingActualMaterial))
			RingActualMaterial->SetScalarParameterValue("OverallGradientIntensity", MockCoro_InitialAnimation_CurrentOverallGradient);

		return false;
	}
	else
	{
		MockCoro_EndingProcess_CurrentOverallGradient = MockCoro_InitialAnimation_CurrentOverallGradient;
		return true;
	}
}

bool AGameplayPlane::MockCoro_EndingAnimation(const float DeltaTime)
{
	if (MockCoro_EndingProcess_UVRadius > 0.f || MockCoro_EndingProcess_CurrentOverallGradient < 1.f)
	{
		MockCoro_EndingProcess_UVRadius -= DeltaTime * GrowthSpeed * 2;

		if (MockCoro_EndingProcess_UVRadius < 0.f)
			MockCoro_EndingProcess_UVRadius = 0.f;

		if (IsValid(ActualMaterial))
			ActualMaterial->SetScalarParameterValue("UVDiameter", MockCoro_EndingProcess_UVRadius);

		if (IsValid(TexturePlaneActualMaterial))
			TexturePlaneActualMaterial->SetScalarParameterValue("UVDiameter", MockCoro_EndingProcess_UVRadius);

		MockCoro_EndingProcess_CurrentOverallGradient += DeltaTime * GrowthSpeed * 2;

		if (MockCoro_EndingProcess_CurrentOverallGradient > 1.f)
			MockCoro_EndingProcess_CurrentOverallGradient = 1.f;

		if (IsValid(RingActualMaterial))
			RingActualMaterial->SetScalarParameterValue("OverallGradientIntensity", MockCoro_EndingProcess_CurrentOverallGradient);

		return false;
	}

	return true;
}

// Called every frame
void AGameplayPlane::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsInitialised)
		bIsInitialised = MockCoro_InitialAnimation(DeltaTime);

	if (bIsClosing)
		if (MockCoro_EndingAnimation(DeltaTime))
			GWorld->DestroyActor(this);
}

void AGameplayPlane::OnDisplayModeChanged(const TEnumAsByte<EDisplayMode> NewMode)
{
	bIsClosing = true;
}
