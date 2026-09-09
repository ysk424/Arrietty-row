#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RowWaves.h"
#include "RowWater.generated.h"
class UProceduralMeshComponent;
class UMaterialInstanceDynamic;
class UTexture2D;
UCLASS()
class ARRIETTYROW_API ARowWater:public AActor {
    GENERATED_BODY()
public:
    ARowWater();
    void UpdateBoat(FVector Position,float Heading,float Speed,float Drive,float DeltaSeconds);
    virtual void BeginPlay() override;
private:
    void Upload();
    UPROPERTY() TObjectPtr<UProceduralMeshComponent> Patch;
    UPROPERTY() TObjectPtr<UTexture2D> Field;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> LocalMaterial;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> DistantMaterial;
    row::Waves Waves;
    double Accumulator=0,UploadTime=0,WakeTime=0;
    bool WasDriving=false;
};
