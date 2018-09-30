// Copyright(c) 2018 PaperSloth

#include "HMDPawn.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "MotionControllerComponent.h"
#include "NavigationSystem.h"
#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AHMDPawn::AHMDPawn()
	: DefaultPlayerHeight(180.0f),
	  ControllerHand(EControllerHand::Right)
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	TraceDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("TraceDirection"));
	MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController"));
	ControllerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OculusGoContoller"));
	TeleportPin = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TeleportPin"));
	TeleportRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TeleportRing"));
	TeleportArrow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TeleportArrow"));

	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, false);
	Camera->AttachToComponent(VROrigin, AttachmentRules);
	TraceDirection->AttachToComponent(Camera, AttachmentRules);
	TraceDirection->RelativeRotation = FRotator(-10.0f, 0.0f, 0.0f);
	MotionController->AttachToComponent(VROrigin, AttachmentRules);
	MotionController->SetTrackingSource(ControllerHand);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ControllerMeshObj(TEXT("/OculusVR/Meshes/OculusGoController"));
	if (ControllerMeshObj.Succeeded())
	{
		ControllerMesh->SetStaticMesh(ControllerMeshObj.Object);
	}
	ControllerMesh->AttachToComponent(MotionController, AttachmentRules);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PinMeshObj(TEXT("/Engine/BasicShapes/Cylinder"));
	if (PinMeshObj.Succeeded())
	{
		TeleportPin->SetStaticMesh(PinMeshObj.Object);
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PinMaterialObj(TEXT("/Game/GoTemplate/Materials/Characters/MI_TeleportCylinderPreview"));
	if (PinMaterialObj.Succeeded())
	{
		TeleportPin->SetMaterial(0, PinMaterialObj.Object);
	}
	TeleportPin->SetRelativeScale3D(FVector(0.75f, 0.75f, 1.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> RingMeshObj(TEXT("/Game/GoTemplate/Meshes/SM_Cylinder"));
	if (RingMeshObj.Succeeded())
	{
		TeleportRing->SetStaticMesh(RingMeshObj.Object);
	}
	TeleportRing->AttachToComponent(TeleportPin, AttachmentRules);
	TeleportRing->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.15f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ArrowMeshObj(TEXT("/Game/GoTemplate/Meshes/SM_Beacon"));
	if (ArrowMeshObj.Succeeded())
	{
		TeleportArrow->SetStaticMesh(ArrowMeshObj.Object);
	}
	TeleportArrow->AttachToComponent(TeleportPin, AttachmentRules);
}

void AHMDPawn::BeginPlay()
{
	Super::BeginPlay();

	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye);
	VROrigin->AddLocalOffset(FVector(0.0f, 0.0f, DefaultPlayerHeight));

	MIRingGlow = TeleportPin->CreateDynamicMaterialInstance(0);
}

void AHMDPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	IsCurrentLocationValid = GetTeleportDestination();
	if (IsLocationPinned == false)
	{
		TeleportPin->SetWorldLocation(CurrentLookAtLocation, false, nullptr, ETeleportType::TeleportPhysics);
		TeleportPin->SetVisibility(IsCurrentLocationValid);
	}

	TeleportArrow->SetVisibility(IsLocationPinned);
	const float HeightScale = (IsLocationPinned) ? 1.0f : 0.35f;
	MIRingGlow->SetScalarParameterValue(TEXT("HeightScale"), HeightScale);

	FRotator deviceRotation;
	FVector  devicePosition;
	UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(deviceRotation, devicePosition);
	TeleportArrow->SetWorldRotation(FRotator(0.0f, deviceRotation.Yaw, 0.0f));
}

void AHMDPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetHMD", EInputEvent::IE_Pressed, this, &AHMDPawn::OnPressResetHMD);
	PlayerInputComponent->BindAction("Teleport", EInputEvent::IE_Pressed, this, &AHMDPawn::OnPressTeleport);
	PlayerInputComponent->BindAction("Teleport", EInputEvent::IE_Released, this, &AHMDPawn::OnReleaseTeleport);
}

FHitResult AHMDPawn::ProjectilePath()
{
	const FVector StartPos = TraceDirection->GetComponentLocation();
	const float TraceDistance = 10000.0f;
	const FVector LaunchVelocity = StartPos + (TraceDirection->GetForwardVector() * TraceDistance);

	FPredictProjectilePathParams Params = FPredictProjectilePathParams(0.0f, StartPos, LaunchVelocity, 2.0f, ECollisionChannel::ECC_WorldStatic);
	Params.DrawDebugTime = 0.0f;
	Params.SimFrequency = 30.0f;
	Params.ObjectTypes.Emplace(EObjectTypeQuery::ObjectTypeQuery1);
	Params.bTraceWithChannel = false;

	FPredictProjectilePathResult PredictResult;
	UGameplayStatics::PredictProjectilePath(this, Params, PredictResult);
	return PredictResult.HitResult;
}

bool AHMDPawn::GetTeleportDestination()
{
	auto const NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavigationSystem == nullptr)
	{
		return false;
	}
	auto const NavigationData = NavigationSystem->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
	if (NavigationData == nullptr)
	{
		return false;
	}
	const FHitResult HitResult = ProjectilePath();
	const FVector ProjectNavExtends = FVector(100.0f);
	FNavLocation OutNavLocation;
	const bool HitNavigation = NavigationSystem->ProjectPointToNavigation(
		HitResult.Location, OutNavLocation, ProjectNavExtends, NavigationData,
		UNavigationQueryFilter::GetQueryFilter(*NavigationData, this, nullptr)
	);
	CurrentLookAtLocation = OutNavLocation.Location;

	return HitResult.bBlockingHit && HitNavigation;
}

void AHMDPawn::OnPressResetHMD()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AHMDPawn::OnPressTeleport()
{
	if (IsCurrentLocationValid == false)
	{
		return;
	}
	IsLocationPinned = true;
	IsLocationFound = IsCurrentLocationValid;
	if (IsLocationFound)
	{
		PinnedLocation = CurrentLookAtLocation;
	}
}

void AHMDPawn::OnReleaseTeleport()
{
	if (IsLocationPinned == false)
	{
		return;
	}
	IsLocationPinned = false;
	if (IsLocationFound)
	{
		FRotator deviceRotation;
		FVector  devicePosition;
		UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(deviceRotation, devicePosition);

		devicePosition.Z = 0.0f;
		const FVector DestLocation = PinnedLocation - GetActorRotation().RotateVector(devicePosition);
		TeleportTo(DestLocation, GetActorRotation());
	}
}
