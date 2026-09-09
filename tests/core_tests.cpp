#include "RowCore.h"
#include "RowWaves.h"
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
    row::Model m; check(m.start(input(0)),"start with tracking");
    for(int i=1;i<=300;++i) m.tick(input(i*.01),.01);
    check(m.distance==0 && m.speed==0,"stationary tracker with BT power stays still");
    double time=3;
    m.start(input(time,.4));
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
    check(m.start(input(time+1)),"resume");
    auto lost=input(time+1.01); lost.bar.valid=false; m.tick(lost,.01);
    check(m.state==row::State::TrackingLost && m.speed==0,"tracking loss stops");
    m.tick(input(time+1.02),.01); check(m.state==row::State::TrackingLost,"no automatic restart");
    m.reset(); check(m.distance==0 && m.elapsed==0 && m.state==row::State::Ready,"reset");
    auto invalid=input(time); invalid.head.position.x=std::numeric_limits<double>::quiet_NaN();
    check(!m.start(invalid),"NaN pose rejected");
    m.start(input(time)); m.tick(input(time+.2),.2); check(m.state==row::State::TrackingLost,"frame hitch stops");
    m.reset(); m.start(input(0)); m.speed=2;
    for(int i=1;i<=100;++i) m.tick(input(i*.01,0,.22),.01);
    check(m.heading>0,"right lean turns right");
    m.reset(); m.start(input(0)); m.speed=2;
    for(int i=1;i<=100;++i) { auto v=input(i*.01); v.head.forward={0,1,0}; m.tick(v,.01); }
    check(m.heading==0,"head yaw does not steer");
    row::Waves waves; waves.center(100,100); waves.disturb(100,100,.02f,.4f,.5f);
    double peak=0; for(float h:waves.height) peak=std::max(peak,double(std::abs(h)));
    check(peak>.01,"oar pressure enters wave field");
    for(int i=0;i<900;++i) { waves.tick(); if(i%6==0) waves.center(100+i*.005,100); }
    check(std::all_of(waves.height.begin(),waves.height.end(),[](float h){return std::isfinite(h)&&std::abs(h)<.1;}),"stable translating ripple simulation");
    waves.center(1000,1000); check(std::all_of(waves.height.begin(),waves.height.end(),[](float h){return h==0;}),"teleport clears distant local domain");
    std::cout<<"PASS "<<checks<<" checks\n";
}
