#ifndef L1Trigger_Phase2L1ParticleFlow_MET_h
#define L1Trigger_Phase2L1ParticleFlow_MET_h

#include "DataFormats/L1TParticleFlow/interface/jets.h"
#include "DataFormats/L1TParticleFlow/interface/sums.h"
#include "DataFormats/L1TParticleFlow/interface/layer1_emulator.h"

#include "L1Trigger/Phase2L1ParticleFlow/interface/dbgPrintf.h"

#include <vector>
#include <numeric>
#include <algorithm>
#include "ap_int.h"
#include "ap_fixed.h"

namespace L1METEmu{
    // Define Data types for P2 L1 MET Emulator

    typedef l1ct::pt_t pt_t;
    typedef l1ct::glbphi_t phi_t;
    typedef l1ct::glbeta_t eta_t;  
    typedef l1ct::dpt_t pxy_t; 
    typedef l1ct::Sum Met;

    typedef ap_fixed<22, 12> proj_t;
    typedef ap_fixed<32, 22> proj2_t;
    typedef ap_fixed<32, 2> poly_t;
    typedef ap_fixed<32, 2> poly2_t;

        
    struct Particle_xy{
        // 44bits
            proj_t hwPt_x; // 22bits
            proj_t hwPt_y; // 22bits
        };
        

    inline Particle_xy Get_xy(l1ct::PuppiObjEmu in_particle) {
        /*
            Convert pt, phi to px, py
            Use 2nd order Polynomial interpolation for cos, sin with 16 points
          */
      
          poly2_t cos2_par0[16] = {-1.00007,-0.924181,-0.707596,-0.382902,-0.000618262,0.382137,0.707056,0.923708,1.00007,0.924181,0.707594,0.383285,0.000188727,-0.382139,-0.706719,-0.923708};
          poly2_t cos2_par1[16] = {9.164680268990924e-06, 0.0017064607695524156, 0.0031441321076514446, 0.004079929656016374, 0.004437063290882583, 0.004095969231842202, 0.0031107221424451436, 0.001689531075808071, -9.161756842493832e-06, -0.001706456406229286, -0.003143961938049376, -0.004103015998697129, -0.004411145151490469, -0.0040958165155326525, -0.0031310072316764474, -0.001689531075808071};
          poly2_t cos2_par2[16] = {9.319674765430664e-06, 7.871694899063284e-06, 5.222989318251642e-06, 2.0256106486379287e-06, -1.9299417402361656e-06, -5.35167113952279e-06, -7.740062096537953e-06, -9.348822844786505e-06, -9.319674765430664e-06, -7.871694899063284e-06, -5.225331064666252e-06, -1.780776301343235e-06, 1.6556927733433181e-06, 5.3495197789955455e-06, 7.954684107366423e-06, 9.348822844786505e-06};
      
          poly2_t sin2_par0[16] = {0.000524872,-0.382229,-0.706791,-0.923959,-1.00008,-0.924156,-0.707264,-0.383199,-0.000525527,0.382228,0.706792,0.923752,1.00013,0.924155,0.707535,0.3832};
          poly2_t sin2_par1[16] = {-0.004431478237276202, -0.00409041472149773, -0.0031267268116859314, -0.00167440343451641, 9.741773386162849e-06, 0.0017049641497188307, 0.00312406082125351, 0.0040978672774037465, 0.004431478237276202, 0.00409041472149773, 0.0031266351819002015, 0.0016868781753450394, -1.249302315254411e-05, -0.001704846339994321, -0.003140405829698437, -0.0040978672774037465};
          poly2_t sin2_par2[16] = {1.870674613498914e-06, 5.292404012785538e-06, 7.909829192302831e-06, 9.188746390688592e-06, 9.313525301268721e-06, 7.887020962996302e-06, 5.435897856093815e-06, 1.8358587462761668e-06, -1.870668901922293e-06, -5.292404012785538e-06, -7.908420336736317e-06, -9.320836119343602e-06, -9.284396260501616e-06, -7.88869635880513e-06, -5.262894200243701e-06, -1.835864457852788e-06};
      
          phi_t phi2_edges[16] = {-720, -630, -540, -450, -360, -270, -180, -90, 0, 90, 180, 270, 360, 450, 540, 630};
      
      
          int phibin = 0;
          if      (in_particle.hwPhi < phi2_edges[1]) phibin = 0;
          else if (in_particle.hwPhi < phi2_edges[2]) phibin = 1;
          else if (in_particle.hwPhi < phi2_edges[3]) phibin = 2;
          else if (in_particle.hwPhi < phi2_edges[4]) phibin = 3;
          else if (in_particle.hwPhi < phi2_edges[5]) phibin = 4;
          else if (in_particle.hwPhi < phi2_edges[6]) phibin = 5;
          else if (in_particle.hwPhi < phi2_edges[7]) phibin = 6;
          else if (in_particle.hwPhi < phi2_edges[8]) phibin = 7;
          else if (in_particle.hwPhi < phi2_edges[9]) phibin = 8;
          else if (in_particle.hwPhi < phi2_edges[10]) phibin = 9;
          else if (in_particle.hwPhi < phi2_edges[11]) phibin = 10;
          else if (in_particle.hwPhi < phi2_edges[12]) phibin = 11;
          else if (in_particle.hwPhi < phi2_edges[13]) phibin = 12;
          else if (in_particle.hwPhi < phi2_edges[14]) phibin = 13;
          else if (in_particle.hwPhi < phi2_edges[15]) phibin = 14;
          else if (in_particle.hwPhi >= phi2_edges[15]) phibin = 15;
          
          Particle_xy proj_xy;

          poly_t cos_var = cos2_par0[phibin] + cos2_par1[phibin] * (in_particle.hwPhi - phi2_edges[phibin]) + cos2_par2[phibin] * (in_particle.hwPhi - phi2_edges[phibin]) * (in_particle.hwPhi - phi2_edges[phibin]);
          poly_t sin_var = sin2_par0[phibin] + sin2_par1[phibin] * (in_particle.hwPhi - phi2_edges[phibin]) + sin2_par2[phibin] * (in_particle.hwPhi - phi2_edges[phibin]) * (in_particle.hwPhi - phi2_edges[phibin]);

          proj_xy.hwPt_x = in_particle.hwPt * cos_var;
          proj_xy.hwPt_y = in_particle.hwPt * sin_var;

          return proj_xy;
      
      }


    inline void Sum_Particles(std::vector<Particle_xy> particles_xy, Particle_xy &met_xy) {
        met_xy.hwPt_x = 0;
        met_xy.hwPt_y = 0;
      
        for (uint i=0; i<particles_xy.size(); ++i) {
          met_xy.hwPt_x -= particles_xy[i].hwPt_x;
          met_xy.hwPt_y -= particles_xy[i].hwPt_y;
        }
      }
      

    inline void pxpy_to_ptphi(Particle_xy met_xy, l1ct::Sum& hls_met) {
        // convert x, y coordinate to pt, phi coordinate using math library
        hls_met.clear();
        hls_met.hwPt = hypot(met_xy.hwPt_x.to_float(), met_xy.hwPt_y.to_float());
    
        // Reduce Latency by not-using division.
        hls_met.hwPhi = phi_t(ap_fixed<26, 11>(atan2(met_xy.hwPt_y.to_float(), met_xy.hwPt_x.to_float())) * ap_fixed<26, 11>(229.29936)); // 720/pi
        // out_metphi = l1ct::Scales::makeGlbPhi(hls::atan2(mety, metx));
    
        return;
    
    }


    inline void met_format(l1ct::Sum d, ap_uint<l1gt::Sum::BITWIDTH>& q) {
        // Change output formats to GT formats
          q = d.toGT().pack();
      }
      
}

inline void puppimet_emu(std::vector<l1ct::PuppiObjEmu> particles, l1ct::Sum& out_met) {
  
    std::vector<L1METEmu::Particle_xy> particles_xy;

    for (uint i=0; i < particles.size(); i++){
        particles_xy.push_back(L1METEmu::Get_xy(particles[i]));
    }

    L1METEmu::Particle_xy met_xy;
    L1METEmu::Sum_Particles(particles_xy, met_xy);
    L1METEmu::pxpy_to_ptphi(met_xy, out_met);

    return;
  }


#endif
  