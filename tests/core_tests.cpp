#include "RowCore.h"
#include "RowWaves.h"
#include "RowCalibration.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <limits>
static int checks=0;
void check(bool ok,const char* name) { ++checks; if(!ok) { std::cerr<<"FAIL "<<name<<'\n'; std::exit(1); } }
row::Input input(double now,double bar=0,double lean=0) {
    row::Input in; in.now=now; in.head={{0,lean,1},{1,0,0},true,now}; in.bar={{bar,0,.65},{1,0,0},true,now};
    in.telemetry.power.set(80,now); return in;
}
bool start(row::Model& model,const row::Input& in) {
    return model.start(in,{{1,0,0},in.head.position,true});
}
int main() {
    row::Telemetry t;
    // Synthetic values in the two fragment layouts observed on Q1S. No user data.
    std::vector<uint8_t> part{0xff,0,40,0xe8,3,0,0xfa,0,4,1,80,0,70,0,6,0};
    std::vector<uint8_t> last{0,0x0b,44,10,0,5,0,0xff,0xff,0xff,0,30,0};
    check(row::parseRower(part.data(),part.size(),10,t),"multipart first");
    check(t.distance.value==1000 && t.pace.value==250 && t.power.value==80,"SI field layout");
    check(row::parseRower(last.data(),last.size(),10.1,t),"multipart final");
    check(t.strokeRate.value==22 && t.strokeCount.value==10 && t.elapsed.value==30,"final fields");
    check(t.power.value==80 && t.power.received==10,"partial merge retains timestamp");
    row::parseRower(last.data(),last.size(),20,t);
    check(!t.power.fresh(20) && t.elapsed.fresh(20),"unrelated fragment cannot revive power");
    for(size_t n=0;n<part.size();++n) { auto copy=t; check(!row::parseRower(part.data(),n,21,copy),"reject truncation"); check(copy.power.received==10,"atomic failure"); }
    const uint8_t h8[]{6,90},h16[]{7,100,0},contactLost[]{4,90},short16[]{1,90};
    check(row::parseHeart(h8,2)==90 && row::parseHeart(h16,3)==100,"8/16 bit HR");
    check(!row::parseHeart(contactLost,2) && !row::parseHeart(short16,2),"unknown HR");
    row::Model m; check(start(m,input(0)),"start with tracking");
    for(int i=1;i<=300;++i) m.tick(input(i*.01),.01);
    check(m.distance==0 && m.speed==0,"stationary tracker with BT power stays still");
    double time=3;
    start(m,input(time,.4));
    for(int cycle=0;cycle<8;++cycle) {
        // Smooth 80 cm full stroke: 0.8 s drive, 1.6 s recovery.
        for(int i=0;i<240;++i) {
            time+=.01; const double phase=i*.01;
            const double bar=phase<.8?.4*std::cos(row::Pi*phase/.8):-.4*std::cos(row::Pi*(phase-.8)/1.6);
            m.tick(input(time,bar),.01);
        }
    }
    check(m.distance>10 && m.speed>0 && m.strokes==8,"strokes propel boat");
    const double before=m.speed; time+=.01; const double pos=m.previousBar;
    for(int i=0;i<200;++i) { time+=.01; m.tick(input(time,pos),.01); }
    check(m.speed<before,"coast down after stroke");
    const auto dist=m.distance,elapsed=m.elapsed; m.pause(); m.tick(input(time+1),.01);
    check(m.distance==dist && m.elapsed==elapsed && m.speed==0,"pause freezes session");
    check(start(m,input(time+1)),"resume");
    auto lost=input(time+1.01); lost.bar.valid=false; m.tick(lost,.01);
    check(m.state==row::State::TrackingLost && m.speed==0,"tracking loss stops");
    m.tick(input(time+1.02),.01); check(m.state==row::State::TrackingLost,"no automatic restart");
    m.reset(); check(m.distance==0 && m.elapsed==0 && m.state==row::State::Ready,"reset");
    auto invalid=input(time); invalid.head.position.x=std::numeric_limits<double>::quiet_NaN();
    check(!start(m,invalid),"NaN pose rejected");
    start(m,input(time)); m.tick(input(time+.2),.2); check(m.state==row::State::TrackingLost,"frame hitch stops");
    m.reset(); start(m,input(0)); m.speed=2;
    for(int i=1;i<=100;++i) m.tick(input(i*.01,0,.22),.01);
    check(m.heading>0,"right lean turns right");
    m.reset(); start(m,input(0)); m.speed=2;
    for(int i=1;i<=100;++i) { auto v=input(i*.01); v.head.forward={0,1,0}; m.tick(v,.01); }
    check(m.heading==0,"head yaw does not steer");
    for(double side:{-1.,1.}) {
        m.reset(); start(m,input(0)); m.speed=2;
        for(int i=1;i<=200;++i) m.tick(input(i*.01,0,side*.079),.01);
        check(m.heading==0 && m.steer==0,"8 cm margin on both sides");
    }
    auto turn=[](double lean) {
        row::Model boat; start(boat,input(0)); boat.speed=2;
        for(int i=1;i<=200;++i) boat.tick(input(i*.01,0,lean),.01);
        return boat;
    };
    const auto leftTurn=turn(-.18),rightTurn=turn(.18),gentleTurn=turn(.10);
    check(std::abs(leftTurn.heading+rightTurn.heading)<1e-12,"symmetric left and right steering");
    check(gentleTurn.heading>0 && gentleTurn.heading<rightTurn.heading*.08,"gentle onset outside margin");
    m=rightTurn;
    for(int i=1;i<=200;++i) m.tick(input(2+i*.01),.01);
    const double settledHeading=m.heading;
    for(int i=1;i<=100;++i) m.tick(input(4+i*.01),.01);
    check(m.yawRate==0 && m.heading==settledHeading,"center has no lingering turn");

    // The machine is rotated in the room, while gaze is another 20 degrees off.
    const double angle=.61;
    const row::Vec3 axis{std::cos(angle),std::sin(angle),0},right{-axis.y,axis.x,0};
    auto physical=[&](double now,double along,double lateral=0.) {
        auto v=input(now);
        v.bar.position={3+axis.x*along,-2+axis.y*along,.65};
        v.head.position={3+right.x*lateral,-2+right.y*lateral,1};
        v.head.forward={std::cos(angle+.35),std::sin(angle+.35),0};
        return v;
    };
    row::Calibration calibration; auto reached=physical(0,.4,.2);
    check(calibration.begin(reached),"calibration accepts tracking");
    m.reset(); m.distance=12; m.elapsed=7; m.speed=2; m.calibrate();
    double calibrationTime=0,axisTime=0;
    row::Input calibratedInput;
    for(int i=1;i<=1200 && calibration.phase!=row::CalibrationPhase::Complete;++i) {
        calibrationTime=i*.01;
        const bool fitting=calibration.phase==row::CalibrationPhase::Axis;
        if(fitting) axisTime+=.01;
        calibratedInput=physical(calibrationTime,fitting?.4*std::cos(2*row::Pi*axisTime/2.8):.4,calibrationTime<1?.2:0);
        calibration.tick(calibratedInput,.01); m.tick(calibratedInput,.01);
    }
    check(calibration.phase==row::CalibrationPhase::Complete && calibration.strokes==2,"two out-and-back strokes complete calibration");
    check(m.distance==12 && m.elapsed==7 && m.speed==0 && m.strokes==0,"calibration cannot propel or count exercise");
    check(std::hypot(calibration.frame.center.x-3,calibration.frame.center.y+2)<1e-8,"keypad reach excluded from averaged neutral");
    check(row::dot(calibration.frame.forward,axis)>.99999,"machine axis comes from bar, not gaze");
    check(m.start(calibratedInput,calibration.frame),"start with measured frame");
    m.heading=0; m.speed=2;
    for(int i=1;i<=600;++i) {
        auto v=physical(calibrationTime+i*.01,row::dot(calibratedInput.bar.position-row::Vec3{3,-2,0},axis));
        const double along=.4*std::sin(i*.02);
        v.head.position.x+=axis.x*along; v.head.position.y+=axis.y*along;
        v.head.forward={0,1,0}; m.tick(v,.01);
    }
    check(std::abs(m.heading)<1e-10 && std::abs(m.lean)<1e-10,"40 cm fore-aft head travel never becomes a turn");
    check(!m.start(input(0),{}),"no fallback to uncalibrated gaze axis");
    auto invalidFrame=calibration.frame; invalidFrame.center.x=std::numeric_limits<double>::quiet_NaN();
    check(!m.start(input(0),invalidFrame),"invalid calibration frame rejected");

    row::Calibration quiet; quiet.begin(input(0));
    for(int i=1;i<=240;++i) quiet.tick(input(i*.01),.01);
    quiet.tick(input(2.41,0,.12),.01);
    check(quiet.phase==row::CalibrationPhase::Center && quiet.remaining==1,"moving posture restarts quiet interval");
    for(int i=242;i<=290;++i) quiet.tick(input(i*.01,0,.12),.01);
    check(quiet.phase==row::CalibrationPhase::Center,"partial quiet interval cannot latch center");
    for(int i=291;i<=4200;++i) quiet.tick(input(i*.01,0,.12),.01);
    check(quiet.phase==row::CalibrationPhase::Failed && quiet.issue==row::CalibrationIssue::Timeout,"stationary bar cannot calibrate; timeout permits retry");
    row::Calibration lostCalibration; lostCalibration.begin(input(0));
    auto missing=input(.01); missing.bar.valid=false; lostCalibration.tick(missing,.01);
    check(lostCalibration.phase==row::CalibrationPhase::Failed && !lostCalibration.frame.valid,"tracking loss cancels calibration");
    lostCalibration.begin(input(0)); lostCalibration.tick(input(.2),.2);
    check(lostCalibration.phase==row::CalibrationPhase::Failed,"hitch cannot silently contaminate calibration");

    row::Calibration circular; circular.begin(input(0));
    for(int i=1;i<=1800;++i) {
        auto v=input(i*.01,.4);
        if(i>300) { const double phase=(i-300)*.01*row::Pi; v.bar.position={.4*std::cos(phase),.4*std::sin(phase),.65}; }
        circular.tick(v,.01);
    }
    check(circular.phase==row::CalibrationPhase::Axis && circular.issue==row::CalibrationIssue::KeepStraight && !circular.frame.valid,
        "nonlinear bar motion is not accepted as machine axis");
    row::Waves waves; waves.center(100,100); waves.disturb(100,100,.02f,.4f,.5f);
    double peak=0; for(float h:waves.height) peak=std::max(peak,double(std::abs(h)));
    check(peak>.01,"oar pressure enters wave field");
    for(int i=0;i<900;++i) { waves.tick(); if(i%6==0) waves.center(100+i*.005,100); }
    check(std::all_of(waves.height.begin(),waves.height.end(),[](float h){return std::isfinite(h)&&std::abs(h)<.1;}),"stable translating ripple simulation");
    waves.center(1000,1000); check(std::all_of(waves.height.begin(),waves.height.end(),[](float h){return h==0;}),"teleport clears distant local domain");
    std::cout<<"PASS "<<checks<<" checks\n";
}
