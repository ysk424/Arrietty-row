#include "RowPanel.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
int32 URowPanel::NativePaint(const FPaintArgs&,const FGeometry& g,const FSlateRect&,
    FSlateWindowElementList& e,int32 layer,const FWidgetStyle&,bool) const {
    const auto brush=FCoreStyle::Get().GetBrush("WhiteBrush");
    auto box=[&](float x,float y,float w,float h,FLinearColor c) {
        FSlateDrawElement::MakeBox(e,layer,g.ToPaintGeometry(FVector2f(w,h),FSlateLayoutTransform(FVector2f(x,y))),brush,ESlateDrawEffect::None,c);
    };
    auto text=[&](float x,float y,FString s,int size,FLinearColor c) {
        FSlateDrawElement::MakeText(e,layer+1,g.ToPaintGeometry(FVector2f(1000,300),FSlateLayoutTransform(FVector2f(x,y))),s,
            FCoreStyle::GetDefaultFontStyle("Regular",size),ESlateDrawEffect::None,c);
    };
    const FLinearColor white(.91,.97,.94),muted(.42,.61,.57),mint(.33,1,.75);
    box(0,0,1000,300,FLinearColor(.008,.022,.024,1)); box(0,0,5,300,mint);
    text(25,14,TEXT("ARRIETTY / ROW"),22,mint); text(450,18,Status,18,white);
    const FString labels[]{TEXT("DISTANCE"),TEXT("TIME"),TEXT("SPEED  km/h"),TEXT("HEART  bpm")};
    const FString values[]{Distance,Time,Speed,Heart};
    for(int i=0;i<4;++i) { text(25+247*i,73,labels[i],18,muted); text(25+247*i,110,values[i],43,white); }
    box(25,195,950,1,FLinearColor(.07,.17,.16));
    text(25,215,Detail,18,muted);
    text(25,267,TEXT("NUM ENTER Start / Pause     NUM 0 Stop / Home      Lean left / right to steer"),16,muted);
    return layer+2;
}
