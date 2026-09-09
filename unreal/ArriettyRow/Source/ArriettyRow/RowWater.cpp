#include "RowWater.h"
#include "ProceduralMeshComponent.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EngineUtils.h"
#include "Math/Float16Color.h"

ARowWater::ARowWater() {
    Patch=CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("LocalWaveSurface")); RootComponent=Patch;
    Patch->SetCollisionEnabled(ECollisionEnabled::NoCollision); Patch->SetCastShadow(false);
}
void ARowWater::BeginPlay() {
    Super::BeginPlay();
    auto base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Row/Materials/M_RowWater.M_RowWater"));
    if(!base) { UE_LOG(LogTemp,Error,TEXT("ROW_WATER_MATERIAL_MISSING")); return; }
    Field=UTexture2D::CreateTransient(row::Waves::N,row::Waves::N,PF_FloatRGBA);
    Field->SRGB=false; Field->NeverStream=true; Field->AddressX=TA_Clamp; Field->AddressY=TA_Clamp;
    Field->Filter=TF_Bilinear; Field->UpdateResource();
    LocalMaterial=UMaterialInstanceDynamic::Create(base,this);
    DistantMaterial=UMaterialInstanceDynamic::Create(base,this);
    LocalMaterial->SetScalarParameterValue(TEXT("LocalPatch"),1);
    for(auto m:{LocalMaterial,DistantMaterial}) m->SetTextureParameterValue(TEXT("WakeField"),Field);
    int waterActors=0;
    for(TActorIterator<AStaticMeshActor> it(GetWorld());it;++it) {
        auto mesh=it->GetStaticMeshComponent();
        if(it->ActorHasTag(TEXT("RowWater"))) {
            for(int i=0;i<mesh->GetNumMaterials();++i) mesh->SetMaterial(i,DistantMaterial);
            mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); ++waterActors;
        }
    }
    constexpr int N=257; constexpr float Size=6400.f,Step=Size/(N-1);
    TArray<FVector> v,n; TArray<FVector2D> uv; TArray<int32> tri;
    for(int y=0;y<N;++y) for(int x=0;x<N;++x) { v.Add({x*Step-Size/2,y*Step-Size/2,0}); n.Add(FVector::UpVector); uv.Add({float(x)/(N-1),float(y)/(N-1)}); }
    for(int y=0;y<N-1;++y) for(int x=0;x<N-1;++x) { int k=y*N+x; tri.Append({k,k+1,k+N,k+1,k+N+1,k+N}); }
    Patch->CreateMeshSection_LinearColor(0,v,tri,n,uv,{}, {},false);
    Patch->SetMaterial(0,LocalMaterial);
    Upload();
    UE_LOG(LogTemp,Display,TEXT("ROW_WATER_READY far_actors=%d local_triangles=%d field=256"),waterActors,tri.Num()/3);
}
void ARowWater::UpdateBoat(FVector pos,float heading,float speed,float drive,float dt) {
    if(!LocalMaterial) return;
    const double x=pos.X*.01,y=pos.Y*.01;
    Waves.center(x,y); SetActorLocation(FVector(pos.X,pos.Y,0));
    for(auto m:{LocalMaterial,DistantMaterial}) {
        m->SetVectorParameterValue(TEXT("Boat"),FLinearColor(pos.X,pos.Y,0,0));
        m->SetVectorParameterValue(TEXT("BoatDirection"),FLinearColor(std::cos(heading),std::sin(heading),0,0));
        m->SetVectorParameterValue(TEXT("FieldOrigin"),FLinearColor(Waves.originX*100,Waves.originY*100,6400,0));
    }
    const double fx=std::cos(heading),fy=std::sin(heading),rx=-fy,ry=fx;
    // Bow/stern pressure points, deposited in world space, retain curved wakes.
    WakeTime+=dt;
    if(WakeTime>=.08 && speed>.12) {
        WakeTime=0; const float strength=FMath::Clamp(speed/3.f,0.f,1.f);
        Waves.disturb(x+fx*2,y+fy*2,.0025f*strength,.40f,.008f*strength);
        for(int side:{-1,1}) Waves.disturb(x-fx*1.8+rx*.45*side,y-fy*1.8+ry*.45*side,
            .0025f*strength,.40f,.025f*strength);
    }
    if(drive>.35 && !WasDriving) {
        for(int side:{-1,1}) Waves.disturb(x+rx*1.9,y+ry*1.9,.009f,.34f,.5f);
        WasDriving=true;
    } else if(drive<.08) WasDriving=false;
    Accumulator+=FMath::Min(dt,.1f);
    while(Accumulator>=row::Waves::Step) { Waves.tick(); Accumulator-=row::Waves::Step; }
    UploadTime+=dt; if(UploadTime>=1./30.) { UploadTime=0; Upload(); }
}
void ARowWater::Upload() {
    if(!Field) return;
    constexpr int N=row::Waves::N;
    auto pixels=new FFloat16Color[N*N];
    for(int y=0;y<N;++y) for(int x=0;x<N;++x) {
        const int k=y*N+x;
        const float dx=(Waves.height[y*N+FMath::Min(x+1,N-1)]-Waves.height[y*N+FMath::Max(x-1,0)])/(2*row::Waves::Cell);
        const float dy=(Waves.height[FMath::Min(y+1,N-1)*N+x]-Waves.height[FMath::Max(y-1,0)*N+x])/(2*row::Waves::Cell);
        pixels[k]=FFloat16Color(FLinearColor(Waves.height[k],dx,dy,Waves.foam[k]));
    }
    auto region=new FUpdateTextureRegion2D(0,0,0,0,N,N);
    Field->UpdateTextureRegions(0,1,region,N*sizeof(FFloat16Color),sizeof(FFloat16Color),
        reinterpret_cast<uint8*>(pixels),[](uint8* data,const FUpdateTextureRegion2D* r) {
            delete[] reinterpret_cast<FFloat16Color*>(data); delete r;
        });
}
