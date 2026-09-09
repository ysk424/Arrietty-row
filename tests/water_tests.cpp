#include "RowKelvin.h"
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

namespace {
int checks=0;
void check(bool condition,const char* name) {
    ++checks;
    if(!condition) { std::cerr<<"FAIL "<<name<<'\n'; std::exit(1); }
}
double energy(const row::KelvinWake& w) {
    double e=0; for(float h:w.height) e+=double(h)*h; return e;
}
void dump(const row::KelvinWake& w,const std::filesystem::path& dir,const std::string& name,double boatX,double boatY,double heading=0) {
    if(dir.empty()) return;
    std::filesystem::create_directories(dir);
    std::ofstream out(dir/(name+".f32"),std::ios::binary);
    out.write(reinterpret_cast<const char*>(w.height.data()),std::streamsize(w.height.size()*sizeof(float)));
    check(bool(out),"synthetic height field exported");
    std::ofstream meta(dir/(name+".json"));
    meta<<"{\"origin\":["<<w.originX<<','<<w.originY<<"],\"boat\":["<<boatX<<','<<boatY
        <<"],\"heading\":"<<heading<<",\"cell\":0.25,\"size\":256}\n";
    check(bool(meta),"synthetic field coordinates exported");
}
}
int main(int argc,char** argv) {
    const std::filesystem::path output=argc>1?argv[1]:"";
    using Wake=row::KelvinWake;
    // Independent Fourier-mode oracle: two different wavelengths must oscillate
    // at sqrt(g*k), not the old constant-speed omega=c*k. Test actual solver.
    for(int mode:{4,16}) {
        Wake wave;
        const double k=2*Wake::Pi*mode/(Wake::N*Wake::Cell);
        for(int y=0;y<Wake::N;++y) for(int x=0;x<Wake::N;++x)
            wave.height[y*Wake::N+x]=.01f*float(std::cos(k*x*Wake::Cell));
        wave.tick(0,0,0,0);
        const double expected=.01*std::cos(std::sqrt(9.81*k)*Wake::Step)*std::exp(-.055*Wake::Step);
        check(std::abs(wave.height[128*Wake::N+128]-expected)<2e-7,"deep-water dispersion for independent wavelengths");
    }
    Wake idle;
    for(int i=0;i<30;++i) idle.tick(0,0,0,0);
    check(energy(idle)==0,"stationary/calibration boat generates no hull wake");
    idle.tick(0,0,0,std::numeric_limits<float>::quiet_NaN());
    check(energy(idle)==0,"invalid speed cannot contaminate water");

    const auto began=std::chrono::steady_clock::now();
    int ticks=0;
    for(float speed:{2.f,3.f}) {
        Wake wake;
        double x=0;
        for(int i=0;i<1200;++i) {
            x=speed*(i+.5)*Wake::Step;
            wake.center(x,0); wake.tick(x,0,0,speed); ++ticks;
        }
        double peak=0,mirror=0,sum=0;
        for(int y=48;y<208;++y) for(int i=48;i<208;++i) {
            const float h=wake.height[y*Wake::N+i];
            peak=std::max(peak,double(std::abs(h))); sum+=h;
            mirror=std::max(mirror,double(std::abs(h-wake.height[(256-y)*Wake::N+i])));
        }
        std::cout<<"WATER speed_mps="<<speed<<" peak_m="<<peak<<" mirror_error_m="<<mirror<<" mean_m="<<sum/(160*160)<<'\n';
        check(peak>.003 && peak<.15,"moving hull produces bounded centimetre gravity waves");
        check(mirror<.0005,"straight wake is symmetric about the travelled line");
        check(std::abs(sum/(160*160))<.001,"no sustained lift of reference water level");
        check(std::all_of(wake.height.begin(),wake.height.end(),[](float h){return std::isfinite(h)&&std::abs(h)<.15f;}),"long translating field remains finite and bounded");
        dump(wake,output,speed==2?"straight-2mps":"straight-3mps",x,0);
        // Sample translation at the same world position (no time advancement).
        const float before=wake.height[128*Wake::N+100];
        wake.center(x+Wake::Cell,0);
        check(wake.height[128*Wake::N+99]==before,"domain shift preserves wake world position exactly");
        const double movingEnergy=energy(wake);
        for(int i=0;i<900;++i) { wake.tick(x,0,0,0); ++ticks; }
        std::cout<<"WATER stopped_energy_ratio="<<energy(wake)/movingEnergy<<'\n';
        check(energy(wake)<movingEnergy*.1,"stopping removes pressure and old waves disperse/decay");
        wake.center(x+1000,1000);
        check(energy(wake)==0,"teleport clears the old wake");
        wake.tick(x+1000,1000,0,0);
        check(energy(wake)==0,"teleport also clears hidden surface potential");
    }
    Wake curve;
    double x=0,y=0;
    for(int i=0;i<1200;++i) {
        const double heading=i<600?0:(i-600)*Wake::Step*.065;
        x+=2.5*std::cos(heading)*Wake::Step; y+=2.5*std::sin(heading)*Wake::Step;
        curve.center(x,y); curve.tick(x,y,heading,2.5f); ++ticks;
    }
    dump(curve,output,"turn",x,y,599*Wake::Step*.065);
    check(std::all_of(curve.height.begin(),curve.height.end(),[](float h){return std::isfinite(h)&&std::abs(h)<.15f;}),"turning keeps a bounded world-space history");
    const auto old=curve.height;
    curve.center(x,y);
    check(curve.height==old,"centering alone never rotates or replaces an old wake");
    const double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count()/ticks;
    std::cout<<"WATER mean_step_ms="<<ms<<" ticks="<<ticks<<" (includes recentering and checks; no rendering)\n";
    std::cout<<"PASS "<<checks<<" water checks\n";
}
