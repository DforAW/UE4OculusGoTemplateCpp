// Copyright(c) 2018 PaperSloth

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HMDPawn.generated.h"

UCLASS()
class GOTEMPLATE_API AHMDPawn : public APawn
{
	GENERATED_BODY()

public:
	AHMDPawn();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* VROrigin;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UArrowComponent* TraceDirection;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* MotionController;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ControllerMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* TeleportPin;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* TeleportRing;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* TeleportArrow;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultPlayerHeight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsCurrentLocationValid;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsLocationPinned;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsLocationFound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector PinnedLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CurrentLookAtLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionController")
	EControllerHand ControllerHand;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	UMaterialInstanceDynamic* MIRingGlow;

protected:
	FHitResult ProjectilePath();
	bool GetTeleportDestination();
	void OnPressResetHMD();
	void OnPressTeleport();
	void OnReleaseTeleport();
};
