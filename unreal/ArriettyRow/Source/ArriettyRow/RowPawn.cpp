#include "RowPawn.h"
#include "RowPanel.h"
#include "RowWater.h"
#include "RowModule.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "ProceduralMeshComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "UnrealClient.h"
#include "Engine/Engine.h"
#include "IXRTrackingSystem.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "HAL/FileManager.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

ARowPawn::ARowPawn() {
    PrimaryActorTick.bCanEverTick=true; AutoPossessPlayer=EAutoReceiveInput::Player0;
    RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("BoatPosition"));
    Tracking=CreateDefaultSubobject<USceneComponent>(TEXT("XROrigin")); Tracking->SetupAttachment(RootComponent);
    Camera=CreateDefaultSubobject<UCameraComponent>(TEXT("HMD")); Camera->SetupAttachment(Tracking);
    Camera->bLockToHmd=true; Camera->bUsePawnControlRotation=false; Camera->SetFieldOfView(90);
    BoatRoot=CreateDefaultSubobject<USceneComponent>(TEXT("BoatVisuals")); BoatRoot->SetupAttachment(RootComponent);
    Hull=CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Hull")); Hull->SetupAttachment(BoatRoot);
    Hull->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Instruments=CreateDefaultSubobject<UWidgetComponent>(TEXT("RowInstruments")); Instruments->SetupAttachment(RootComponent);
    Instruments->SetWidgetSpace(EWidgetSpace::World); Instruments->SetWidgetClass(URowPanel::StaticClass());
    Instruments->SetDrawSize(FVector2D(1000,300)); Instruments->SetRelativeLocation(FVector(115,0,53));
    Instruments->SetRelativeRotation(FRotator(22,180,0)); Instruments->SetRelativeScale3D(FVector(.075));
    Instruments->SetTwoSided(true); Instruments->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Instruments->SetCastShadow(false);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> mesh(TEXT("/Engine/BasicShapes/Cube.Cube")); Cube=mesh.Object;
}
void ARowPawn::BeginPlay() {
    Super::BeginPlay(); Began=row::Devices::seconds();
    Home=GetActorTransform(); Home.SetLocation(FVector(GetActorLocation().X,GetActorLocation().Y,0)); SetActorTransform(Home);
    Model.heading=FMath::DegreesToRadians(GetActorRotation().Yaw);
    Offline=FParse::Param(FCommandLine::Get(),TEXT("RowOffline")); Demo=FParse::Param(FCommandLine::Get(),TEXT("RowDemo"));
    FParse::Value(FCommandLine::Get(),TEXT("RowQuitAfter="),QuitAfter);
    FParse::Value(FCommandLine::Get(),TEXT("RowScreenshotAt="),ScreenshotAt);
    FParse::Value(FCommandLine::Get(),TEXT("RowScreenshotPath="),ScreenshotPath);
    if(Demo) Offline=true;
    Chase=Offline && FParse::Param(FCommandLine::Get(),TEXT("RowChase"));
    if(Offline) { Camera->bLockToHmd=false; Camera->SetRelativeLocation(FVector(0,0,100)); Camera->SetRelativeRotation(FRotator(-7,0,0)); }
    else UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::LocalFloor);
    if(Chase) { Camera->SetRelativeLocation(FVector(-440,-440,310)); Camera->SetRelativeRotation(FRotator(-24,45,0)); Camera->SetFieldOfView(75); }
    if(!Offline) {
        FString configPath=FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()/TEXT("../../settings.local.json"));
        FParse::Value(FCommandLine::Get(),TEXT("RowSettings="),configPath);
        FString json; TSharedPtr<FJsonObject> cfg; row::DeviceConfig dc;
        if(FFileHelper::LoadFileToString(json,*configPath) && FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(json),cfg)) {
            FString serial,address; cfg->TryGetStringField(TEXT("tracker_serial"),serial); dc.trackerSerial=TCHAR_TO_UTF8(*serial);
            auto readAddress=[&](const TCHAR* key) { FString a; cfg->TryGetStringField(key,a); a.ReplaceInline(TEXT(":"),TEXT("")); a.ReplaceInline(TEXT("-"),TEXT("")); return FCString::Strtoui64(*a,nullptr,16); };
            dc.rowerAddress=readAddress(TEXT("rower_address")); dc.heartAddress=readAddress(TEXT("heart_rate_address"));
        } else Notice=TEXT("Local device settings missing");
        if(RowDeviceApiAvailable()) Devices=std::make_unique<row::Devices>(dc);
        else Notice=TEXT("OpenVR SDK missing / run bootstrap");
    }
    Instruments->InitWidget(); Panel=Cast<URowPanel>(Instruments->GetWidget());
    if(auto mat=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Row/Materials/M_Instruments.M_Instruments"))) Instruments->SetMaterial(0,mat);
    BuildBoat(); Water=GetWorld()->SpawnActor<ARowWater>();
    UE_LOG(LogTemp,Display,TEXT("ROW_READY offline=%d demo=%d reference_water_z_cm=0"),Offline,Demo);
}
void ARowPawn::SetupPlayerInputComponent(UInputComponent* input) {
    Super::SetupPlayerInputComponent(input);
    input->BindKey(EKeys::Enter,IE_Pressed,this,&ARowPawn::Toggle);
    input->BindKey(EKeys::NumPadZero,IE_Pressed,this,&ARowPawn::Stop);
    input->BindKey(EKeys::Insert,IE_Pressed,this,&ARowPawn::Stop);
    input->BindKey(EKeys::Escape,IE_Pressed,this,&ARowPawn::Stop);
}
row::Input ARowPawn::ReadInput() const {
    row::Input in; in.now=row::Devices::seconds(); in.bar=Snapshot.bar; in.head=Snapshot.head; in.telemetry=Snapshot.telemetry;
    if(Offline) {
        const double phase=std::fmod(SimTime,2.8);
        const double bar=Demo?(phase<1?.38*std::cos(row::Pi*phase):-.38*std::cos(row::Pi*(phase-1)/1.8)):.38;
        in.bar={{bar,0,.65},{1,0,0},true,in.now};
        in.head={{0,Demo?.11*std::sin(SimTime*.07):0,1},{1,0,0},true,in.now};
        if(Demo) in.telemetry.power.set(95,in.now);
    } else {
        // OpenVR head provides a common physical room frame for lean/bar input;
        // also require the actual OpenXR rendering pose to be tracked.
        auto xr=GEngine?GEngine->XRSystem:nullptr;
        in.head.valid=in.head.valid && xr.IsValid() && xr->IsTracking(IXRTrackingSystem::HMDDeviceId);
    }
    return in;
}
void ARowPawn::Toggle() {
    if(Model.state==row::State::Running) { Model.pause(); Record(TEXT("pause")); return; }
    const auto in=ReadInput();
    if(!Model.start(in)) { Notice=TEXT("Waiting for HMD + bar tracking"); return; }
    // Latch the horizontal view the rider actually sees; do not impose room yaw.
    Model.heading=FMath::DegreesToRadians(Chase?GetActorRotation().Yaw:Camera->GetComponentRotation().Yaw);
    if(!Offline) {
        const FVector roomEye=Camera->GetRelativeLocation();
        Tracking->SetRelativeLocation(FVector(-roomEye.X,-roomEye.Y,100-roomEye.Z));
    }
    const FRotator facing(0,FMath::RadiansToDegrees(Model.heading),0);
    BoatRoot->SetWorldRotation(facing); Instruments->SetWorldRotation(FRotator(22,facing.Yaw+180,0));
    Instruments->SetWorldLocation(GetActorLocation()+facing.RotateVector(FVector(115,0,53)));
    if(SessionFile.IsEmpty()) {
        const FString dir=FPaths::ProjectSavedDir()/TEXT("Sessions"); IFileManager::Get().MakeDirectory(*dir,true);
        SessionFile=dir/FString::Printf(TEXT("row-%s.csv"),*FDateTime::UtcNow().ToString(TEXT("%Y%m%dT%H%M%S%ss")));
        FFileHelper::SaveStringToFile(TEXT("utc,event,active_s,distance_m,speed_kmh,heart_bpm,strokes,source,machine_distance_m,machine_elapsed_s,machine_speed_kmh,power_w\n"),*SessionFile);
    }
    Notice.Empty(); Record(TEXT("start"));
}
void ARowPawn::Stop() {
    if(!SessionFile.IsEmpty()) Record(TEXT("stop")); SessionFile.Empty();
    Model.reset(); SimTime=0; DemoStarted=true; SetActorTransform(Home);
    Model.heading=FMath::DegreesToRadians(Home.Rotator().Yaw);
    BoatRoot->SetRelativeRotation(FRotator::ZeroRotator);
    Instruments->SetRelativeLocation(FVector(115,0,53)); Instruments->SetRelativeRotation(FRotator(22,180,0));
    Notice=TEXT("Stopped / Home");
}
void ARowPawn::Tick(float dt) {
    Super::Tick(dt); if(Devices) Snapshot=Devices->snapshot();
    if(Snapshot.bar.valid) ++ValidBarFrames;
    if(Demo && !DemoStarted && row::Devices::seconds()-Began>8) { Toggle(); DemoStarted=true; }
    const auto in=ReadInput(); const auto oldState=Model.state; const double oldHeading=Model.heading;
    // Pose derivatives use the real frame interval; a hitch invokes core watchdog.
    const double moved=Model.tick(in,dt);
    if(Model.state==row::State::Running) SimTime+=dt;
    if(oldState==row::State::Running && Model.state==row::State::TrackingLost) Record(TEXT("tracking_lost"));
    if(moved>0) {
        const FVector delta(std::cos(Model.heading)*moved*100,std::sin(Model.heading)*moved*100,0);
        FHitResult hit;
        FCollisionQueryParams params(SCENE_QUERY_STAT(RowBank),false,this);
        const FVector from=GetActorLocation()+FVector(0,0,35);
        if(GetWorld()->SweepSingleByChannel(hit,from,from+delta,FQuat::Identity,ECC_WorldStatic,FCollisionShape::MakeSphere(65),params)) {
            Model.distance-=moved; Model.pause(); Notice=TEXT("Shore / paused"); Record(TEXT("shore"));
        } else {
            AddActorWorldOffset(delta);
            AddActorWorldRotation(FRotator(0,FMath::RadiansToDegrees(Model.heading-oldHeading),0));
        }
    }
    // Wave following is visual only; the HMD root stays level and Z=0.
    const double time=row::Devices::seconds()-Began;
    BoatRoot->SetRelativeLocation(FVector(0,0,1.1*std::sin(time*1.4)));
    const auto br=BoatRoot->GetRelativeRotation();
    BoatRoot->SetRelativeRotation(FRotator(.25*std::sin(time*1.8),br.Yaw,.5*std::sin(time*1.3)));
    OarBlend=FMath::FInterpTo(OarBlend,float(Model.drive),dt,8);
    for(int i=0;i<Oars.Num();++i) {
        const int side=i==0?-1:1;
        Oars[i]->SetRelativeRotation(FRotator(0,side*(68+36*OarBlend),-side*(8-12*OarBlend)));
    }
    if(Water) Water->UpdateBoat(GetActorLocation(),Model.heading,Model.speed,Model.drive,dt);
    if(Panel) {
        Panel->Distance=FString::Printf(TEXT("%.0f m"),Model.distance);
        const int seconds=int(Model.elapsed); Panel->Time=FString::Printf(TEXT("%02d:%02d"),seconds/60,seconds%60);
        Panel->Speed=FString::Printf(TEXT("%.1f"),Model.speed*3.6);
        const auto hr=Snapshot.heart;
        Panel->Heart=hr.fresh(in.now,5) && hr.value>0?FString::Printf(TEXT("%.0f"),hr.value):TEXT("--");
        const TCHAR* state=Model.state==row::State::Running?TEXT("ROWING"):Model.state==row::State::Paused?TEXT("PAUSED"):
            Model.state==row::State::TrackingLost?TEXT("TRACKING LOST / ENTER"):TEXT("READY / ENTER");
        Panel->Status=Demo?TEXT("DEMO / synthetic strokes"):!Notice.IsEmpty()?Notice:state;
        Panel->Detail=FString::Printf(TEXT("%s   %.0f W   %u strokes   HR %s"),Model.usingBt?TEXT("BT power"):TEXT("Tracker estimate"),Model.power,Model.strokes,
            hr.fresh(in.now,5) && hr.value>0?TEXT("connected"):TEXT("--"));
    }
    if(!SessionFile.IsEmpty() && in.now>=NextRecord) { Record(TEXT("sample")); NextRecord=in.now+1; }
    if(ScreenshotAt>0 && time>ScreenshotAt && !ScreenshotPath.IsEmpty()) {
        FScreenshotRequest::RequestScreenshot(ScreenshotPath,false,false); ScreenshotAt=0;
        UE_LOG(LogTemp,Display,TEXT("ROW_PREVIEW_CAPTURE distance_m=%.2f state=%d"),Model.distance,int(Model.state));
    }
    if(QuitAfter>0 && time>QuitAfter) UKismetSystemLibrary::QuitGame(this,nullptr,EQuitPreference::Quit,false);
}
void ARowPawn::Record(const TCHAR* event) {
    if(SessionFile.IsEmpty()) return;
    const double now=row::Devices::seconds(); const auto& t=Snapshot.telemetry;
    auto value=[&](const row::Field& f,double scale=1.) { return f.fresh(now,5) && f.value>=0?FString::Printf(TEXT("%.3f"),f.value*scale):FString(); };
    const FString hr=Snapshot.heart.value>0?value(Snapshot.heart):FString();
    const FString speed=t.pace.fresh(now) && t.pace.value>=0?FString::Printf(TEXT("%.3f"),t.pace.value>0?1800./t.pace.value:0.):FString();
    const FString line=FString::Printf(TEXT("%s,%s,%.3f,%.3f,%.3f,%s,%u,%s,%s,%s,%s,%s\n"),
        *FDateTime::UtcNow().ToIso8601(),event,Model.elapsed,Model.distance,Model.speed*3.6,*hr,Model.strokes,
        Demo?TEXT("demo"):Model.usingBt?TEXT("bt"):TEXT("tracker_estimate"),*value(t.distance),*value(t.elapsed),*speed,*value(t.power));
    FFileHelper::SaveStringToFile(line,*SessionFile,FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM,&IFileManager::Get(),FILEWRITE_Append);
}
void ARowPawn::EndPlay(const EEndPlayReason::Type reason) {
    Record(TEXT("exit"));
    if(Devices) UE_LOG(LogTemp,Display,TEXT("ROW_DEVICE_SUMMARY tracker_frames=%llu rower_packets=%u heart_packets=%u rejected=%u"),
        ValidBarFrames,Snapshot.telemetry.packets,Snapshot.heartPackets,Snapshot.telemetry.rejected);
    Devices.reset(); Super::EndPlay(reason);
}
void ARowPawn::BuildBoat() {
    auto base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Row/Materials/M_Hull.M_Hull"));
    Hull->SetMaterial(0,base);
    // Open rowing shell: tapered keel and gunwales, with a second inner surface.
    TArray<FVector> v,n; TArray<FVector2D> uv; TArray<int32> idx;
    constexpr int rings=41,across=17;
    for(int inner=0;inner<2;++inner) for(int i=0;i<rings;++i) {
        const float along=float(i)/(rings-1),x=-230+460*along;
        const float width=FMath::Max(2.f,45.f*FMath::Pow(FMath::Max(0.f,FMath::Sin(PI*along)),.55f))-inner*1.3f;
        for(int j=0;j<across;++j) {
            const float a=float(j)/(across-1)*PI;
            v.Add(FVector(x,-width*FMath::Cos(a),22-38*FMath::Sin(a)+inner*2));
            n.Add(FVector(0,-FMath::Cos(a)*(inner?-1:1),-FMath::Sin(a)*(inner?-1:1)));
            uv.Add(FVector2D(along,float(j)/(across-1)));
        }
    }
    for(int side=0;side<2;++side) for(int i=0;i<rings-1;++i) for(int j=0;j<across-1;++j) {
        int k=side*rings*across+i*across+j;
        if(side) idx.Append({k,k+across,k+1,k+1,k+across,k+across+1});
        else idx.Append({k,k+1,k+across,k+1,k+across+1,k+across});
    }
    Hull->CreateMeshSection_LinearColor(0,v,idx,n,uv,{}, {},false);
    auto wood=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Row/Materials/M_Wood.M_Wood"));
    auto add=[&](const TCHAR* name,FVector p,FVector scale,USceneComponent* parent) {
        auto m=NewObject<UStaticMeshComponent>(this,name); m->SetupAttachment(parent); m->SetStaticMesh(Cube);
        m->SetRelativeLocation(p); m->SetRelativeScale3D(scale); m->SetMaterial(0,wood);
        m->SetCollisionEnabled(ECollisionEnabled::NoCollision); m->RegisterComponent(); return m;
    };
    add(TEXT("Seat"),FVector(-20,0,18),FVector(.32,.75,.035),BoatRoot);
    add(TEXT("Footboard"),FVector(75,0,5),FVector(.18,.55,.035),BoatRoot);
    for(int side:{-1,1}) {
        auto pivot=NewObject<UStaticMeshComponent>(this); pivot->SetupAttachment(BoatRoot); pivot->SetRelativeLocation(FVector(20,side*48,26)); pivot->RegisterComponent();
        add(side<0?TEXT("LeftShaft"):TEXT("RightShaft"),FVector(75,0,0),FVector(2.6,.035,.035),pivot);
        add(side<0?TEXT("LeftBlade"):TEXT("RightBlade"),FVector(188,0,0),FVector(.52,.22,.018),pivot);
        Oars.Add(pivot);
    }
}
