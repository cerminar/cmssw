#include <vector>
#include <string>
#include <ap_int.h>
#include <ap_fixed.h>
#include <TVector2.h>
#include <iostream>

#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/L1TParticleFlow/interface/PFCandidate.h" 
#include "DataFormats/L1TParticleFlow/interface/PFJet.h"

#include "DataFormats/L1TParticleFlow/interface/puppi.h"
#include "DataFormats/L1TParticleFlow/interface/sums.h"
#include "DataFormats/L1TParticleFlow/interface/jets.h"

#include "DataFormats/L1Trigger/interface/EtSum.h"
#include "DataFormats/Math/interface/LorentzVector.h"

using namespace l1t;

class L1MetCorrPfProducer : public edm::global::EDProducer<> {
    public:
      explicit L1MetCorrPfProducer(const edm::ParameterSet&);
      ~L1MetCorrPfProducer() override;
      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
    
    private:
      void produce(edm::StreamID, edm::Event& iEvent, const edm::EventSetup& iSetup) const override;
      // Tokens for raw MET (from the MET producer) and PF jets.
      // edm::EDGetTokenT<std::vector<l1t::EtSum>> metToken_;
      // edm::EDGetTokenT<std::vector<l1t::PFJet>> jetToken_;
      edm::EDGetTokenT<std::vector<l1t::EtSum>> metToken;
      edm::EDGetTokenT<std::vector<l1t::PFJet>> jetsToken;



      typedef ap_ufixed<14,12,AP_TRN,AP_SAT> pt_t;    // LSB = 0.25 GeV
      typedef ap_ufixed<28,24,AP_TRN,AP_SAT> pt2_t;     
      typedef ap_fixed<16,14,AP_TRN,AP_SAT> pxy_t;       
      typedef ap_int<12> eta_t;     // glbeta_t(12)           
      typedef ap_int<11> phi_t;     // glbphi_t(11)      

      typedef ap_fixed<22,12> proj_t;    // for x,y projections
      typedef ap_fixed<32, 22> proj2_t;
      typedef ap_fixed<32,2> poly_t;     
      typedef ap_fixed<32,2> poly2_t;

      typedef l1ct::Sum Met;
      typedef l1ct::Jet Jet;

      static constexpr float ptLSB_ = 0.25;               
      static constexpr float phiLSB_ = M_PI / 720;          
      static constexpr float maxPt_ = ((1 << pt_t::width) - 1) * ptLSB_;

      void Get_xy(pt_t pt, phi_t phi, proj_t& px, proj_t& py) const;
      void pxpy_to_ptphi(proj_t met_x, proj_t met_y, Met& hls_met) const;
      void Get_dPt(Jet jet, proj2_t& dpt_x2, proj2_t& dpt_y2) const;
      void CalcMetCorrHLS(l1t::EtSum& metVector,
                          std::vector<l1ct::Jet>& jets,
                          reco::Candidate::PolarLorentzVector& CorrmetVector) const;
      
      std::vector<l1ct::Jet> convertEDMToHW(std::vector<l1t::PFJet> edmJets) const;
      std::vector<l1t::EtSum> convertHWToEDM(l1ct::Sum hwSums) const;

      float minJetPt = 30;
      float maxJetEta = 2.4;
};


L1MetCorrPfProducer::L1MetCorrPfProducer(const edm::ParameterSet& cfg)
    : metToken(consumes<std::vector<l1t::EtSum>>(cfg.getParameter<edm::InputTag>("RawMET"))),
      jetsToken(consumes<std::vector<l1t::PFJet>>(cfg.getParameter<edm::InputTag>("L1PFJets"))) {
  produces<std::vector<l1t::EtSum>>();
}


void L1MetCorrPfProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("RawMET", edm::InputTag("l1tMETNewPFProducer"));
    desc.add<edm::InputTag>("L1PFJets", edm::InputTag("l1tSC4PFL1PuppiCorrectedEmulator"));
    descriptions.add("L1MetCorrPfProducer", desc);
  }


  void L1MetCorrPfProducer::produce(edm::StreamID, edm::Event& iEvent, const edm::EventSetup& iSetup) const {
    // Load Raw MET
    // edm::Handle<std::vector<l1t::EtSum>> metHandle;
    // edm::Handle<l1t::metCollection> metHandle;
    // iEvent.getByToken(metToken, metHandle);

    // const auto &rawMET = metHandle->at(0);

    l1t::EtSum rawMET = iEvent.get(metToken)[0];
    

    // Load Jets
    l1t::PFJetCollection edmJets = iEvent.get(jetsToken);

    std::vector<l1ct::Jet> hwJets = convertEDMToHW(edmJets);  // convert to the emulator format
    // Apply pT and eta selections
    std::vector<l1ct::Jet> hwJetsFiltered;
    std::copy_if(hwJets.begin(), hwJets.end(), std::back_inserter(hwJetsFiltered), [&](auto jet) {
    return jet.hwPt > l1ct::Scales::makePtFromFloat(minJetPt) &&
           std::abs(jet.hwEta) < l1ct::Scales::makeGlbEta(maxJetEta);
    });

    // MET Correction
    reco::Candidate::PolarLorentzVector CorrmetVector;
    CalcMetCorrHLS(rawMET, hwJetsFiltered, CorrmetVector);
    // CalcMetCorrHLS(rawMetVec, hwJets, CorrmetVector);

    l1t::EtSum theCorrMET(CorrmetVector, l1t::EtSum::EtSumType::kTotalHt, 0, 0, 0, 0);
    auto corrmetCollection = std::make_unique<std::vector<l1t::EtSum>>(0);
    corrmetCollection->push_back(theCorrMET);
    iEvent.put(std::move(corrmetCollection));

}

void L1MetCorrPfProducer::CalcMetCorrHLS(l1t::EtSum& metVector,
                                         std::vector<l1ct::Jet>& jets,
                                         reco::Candidate::PolarLorentzVector& CorrmetVector) const {
    // Corr MET Calculate
    int NJETS = jets.size();
    proj2_t dpt_x2[NJETS], dpt_y2[NJETS];
    proj2_t sum_dpx2 = 0, sum_dpy2 = 0;

    proj_t met_px = 0;
    proj_t met_py = 0;
    pt_t met_pt = metVector.pt();
    phi_t met_phi = l1ct::Scales::makeGlbPhi(metVector.phi());

    proj_t corr_px = 0;
    proj_t corr_py = 0;
    Met corr_met;
    Jet current_jet;

    Get_xy(met_pt, met_phi, met_px, met_py);

    for (int i=0; i<NJETS; ++i){
        current_jet.hwPt = jets[i].hwPt;
        current_jet.hwPhi = jets[i].hwPhi;
        current_jet.hwEta = jets[i].hwEta;
        Get_dPt(current_jet, dpt_x2[i], dpt_y2[i]);
        sum_dpx2 += dpt_x2[i];
        sum_dpy2 += dpt_y2[i];
    }

    corr_px = (met_px > 0) ? met_px + proj_t(sqrt(sum_dpx2.to_float())) : met_px - proj_t(sqrt(sum_dpx2.to_float()));
    corr_py = (met_py > 0) ? met_py + proj_t(sqrt(sum_dpy2.to_float())) : met_py - proj_t(sqrt(sum_dpy2.to_float()));

    pxpy_to_ptphi(corr_px, corr_py, corr_met);

    CorrmetVector.SetPt(corr_met.hwPt.to_double());
    CorrmetVector.SetPhi(corr_met.hwPhi.to_double() * phiLSB_);
    CorrmetVector.SetEta(0);
}



void L1MetCorrPfProducer::Get_xy(pt_t pt, phi_t phi, proj_t& px, proj_t& py) const {
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
    if      (phi < phi2_edges[1]) phibin = 0;
    else if (phi < phi2_edges[2]) phibin = 1;
    else if (phi < phi2_edges[3]) phibin = 2;
    else if (phi < phi2_edges[4]) phibin = 3;
    else if (phi < phi2_edges[5]) phibin = 4;
    else if (phi < phi2_edges[6]) phibin = 5;
    else if (phi < phi2_edges[7]) phibin = 6;
    else if (phi < phi2_edges[8]) phibin = 7;
    else if (phi < phi2_edges[9]) phibin = 8;
    else if (phi < phi2_edges[10]) phibin = 9;
    else if (phi < phi2_edges[11]) phibin = 10;
    else if (phi < phi2_edges[12]) phibin = 11;
    else if (phi < phi2_edges[13]) phibin = 12;
    else if (phi < phi2_edges[14]) phibin = 13;
    else if (phi < phi2_edges[15]) phibin = 14;
    else if (phi >= phi2_edges[15]) phibin = 15;
    
    poly_t cos_var = cos2_par0[phibin] + cos2_par1[phibin] * (phi - phi2_edges[phibin]) + cos2_par2[phibin] * (phi - phi2_edges[phibin]) * (phi - phi2_edges[phibin]);
    poly_t sin_var = sin2_par0[phibin] + sin2_par1[phibin] * (phi - phi2_edges[phibin]) + sin2_par2[phibin] * (phi - phi2_edges[phibin]) * (phi - phi2_edges[phibin]);
    px = pt * cos_var;
    py = pt * sin_var;

}

void L1MetCorrPfProducer::pxpy_to_ptphi(proj_t met_x, proj_t met_y, Met& hls_met) const {
  // convert x, y coordinate to pt, phi coordinate using math library

  hls_met.clear();
  hls_met.hwPt = hypot(met_x.to_float(), met_y.to_float());

  // Reduce Latency by not-using division.
  hls_met.hwPhi = phi_t(ap_fixed<26, 11>(atan2(met_y.to_float(), met_x.to_float())) * ap_fixed<26, 11>(229.29936)); // 720/pi
  // out_metphi = l1ct::Scales::makeGlbPhi(hls::atan2(mety, metx));

  return;

}

void L1MetCorrPfProducer::Get_dPt(Jet jet, proj2_t& dpt_x2, proj2_t& dpt_y2) const {
  ap_fixed<11, 1> eta_par1[5] = {0, 0.102, 0.130, 0.103, 0.049};
  ap_fixed<8, 5> eta_par2[5] = {0, 9.900, 13.184, 14.050, 21.763};

  eta_t eta_edges[4] = {298, 390, 573, 688};

  eta_t abseta = abs(jet.hwEta.to_float());
  int etabin = 0;
  if ( abseta == 0.00 )             etabin = 0;
  else if ( abseta < eta_edges[0] )  etabin = 1;
  else if ( abseta < eta_edges[1] )  etabin = 2;
  else if ( abseta < eta_edges[2] )  etabin = 3;
  else if ( abseta < eta_edges[3] )  etabin = 4;
  else etabin = 0;
  
  // Particle_xy dPt_xy;
  dpt_x2 = 0;
  dpt_y2 = 0;
  Jet jet_resolution;
  jet_resolution.hwPt = eta_par1[etabin]*jet.hwPt + eta_par2[etabin];
  jet_resolution.hwPhi = jet.hwPhi;
  proj_t dpt_x = 0;
  proj_t dpt_y = 0;
  Get_xy(jet_resolution.hwPt, jet_resolution.hwPhi, dpt_x, dpt_y);

  dpt_x2 = dpt_x * dpt_x;
  dpt_y2 = dpt_y * dpt_y;
}

std::vector<l1ct::Jet> L1MetCorrPfProducer::convertEDMToHW(std::vector<l1t::PFJet> edmJets) const {
  std::vector<l1ct::Jet> hwJets;
  std::for_each(edmJets.begin(), edmJets.end(), [&](l1t::PFJet jet) {
    l1ct::Jet hwJet = l1ct::Jet::unpack(jet.getHWJetCT());
    hwJets.push_back(hwJet);
  });
  return hwJets;
}

std::vector<l1t::EtSum> L1MetCorrPfProducer::convertHWToEDM(l1ct::Sum hwSums) const {
  std::vector<l1t::EtSum> edmSums;

  reco::Candidate::PolarLorentzVector htVector;
  l1gt::Sum gtSum = hwSums.toGT();
  htVector.SetPt(l1gt::Scales::floatPt(gtSum.scalar_pt));
  htVector.SetPhi(0);
  htVector.SetEta(0);

  reco::Candidate::PolarLorentzVector mhtVector;
  mhtVector.SetPt(l1gt::Scales::floatPt(gtSum.vector_pt));
  mhtVector.SetPhi(l1gt::Scales::floatPhi(gtSum.vector_phi));
  mhtVector.SetEta(0);

  l1t::EtSum ht(htVector, l1t::EtSum::EtSumType::kTotalHt, gtSum.scalar_pt.bits_to_uint64());
  l1t::EtSum mht(mhtVector, l1t::EtSum::EtSumType::kMissingHt, gtSum.vector_pt.bits_to_uint64(), 0, gtSum.vector_phi);

  edmSums.push_back(ht);
  edmSums.push_back(mht);
  return edmSums;
}



L1MetCorrPfProducer::~L1MetCorrPfProducer() {}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(L1MetCorrPfProducer);