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
#include "DataFormats/L1TParticleFlow/interface/layer1_emulator.h"

#include "DataFormats/L1Trigger/interface/EtSum.h"
#include "DataFormats/Math/interface/LorentzVector.h"

#include "L1Trigger/Phase2L1ParticleFlow/interface/jetmet/L1PFJUMPEmulator.h"

using namespace l1t;

class L1JUMPProducer : public edm::global::EDProducer<> {
  // JUMP Producer
  // JUMP: Jet Uncertainty-aware MET Prediction
public:
  explicit L1JUMPProducer(const edm::ParameterSet&);
  ~L1JUMPProducer() override;

private:
  void produce(edm::StreamID, edm::Event& iEvent, const edm::EventSetup& iSetup) const override;
  edm::EDGetTokenT<std::vector<l1t::EtSum>> metToken;
  edm::EDGetTokenT<std::vector<l1t::PFJet>> jetsToken;

  typedef ap_ufixed<14, 12, AP_TRN, AP_SAT> pt_t;  // LSB = 0.25 GeV
  typedef ap_ufixed<28, 24, AP_TRN, AP_SAT> pt2_t;
  typedef ap_fixed<16, 14, AP_TRN, AP_SAT> pxy_t;
  typedef ap_int<12> eta_t;  // glbeta_t(12)
  typedef ap_int<11> phi_t;  // glbphi_t(11)

  typedef ap_fixed<22, 12> proj_t;  // for x,y projections
  typedef ap_fixed<32, 22> proj2_t;
  typedef ap_fixed<32, 2> poly_t;
  typedef ap_fixed<32, 2> poly2_t;

  typedef l1ct::Sum Met;
  typedef l1ct::Jet Jet;

  static constexpr float ptLSB_ = 0.25;
  static constexpr float phiLSB_ = M_PI / 720;
  static constexpr float maxPt_ = ((1 << pt_t::width) - 1) * ptLSB_;

  void CalcJUMP_HLS(l1t::EtSum& metVector,
                    std::vector<l1ct::Jet>& jets,
                    reco::Candidate::PolarLorentzVector& JUMPVector) const;

  std::vector<l1ct::Jet> convertEDMToHW(std::vector<l1t::PFJet> edmJets) const;
  std::vector<l1t::EtSum> convertHWToEDM(l1ct::Sum hwSums) const;

  float minJetPt = 30;
  float maxJetEta = 3.0;
};

L1JUMPProducer::L1JUMPProducer(const edm::ParameterSet& cfg)
    : metToken(consumes<std::vector<l1t::EtSum>>(cfg.getParameter<edm::InputTag>("RawMET"))),
      jetsToken(consumes<std::vector<l1t::PFJet>>(cfg.getParameter<edm::InputTag>("L1PFJets"))) {
  produces<std::vector<l1t::EtSum>>();
}

void L1JUMPProducer::produce(edm::StreamID, edm::Event& iEvent, const edm::EventSetup& iSetup) const {
  // Load Met
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

  // JUMP Algorithm
  reco::Candidate::PolarLorentzVector JUMPVector;
  CalcJUMP_HLS(rawMET, hwJetsFiltered, JUMPVector);
  // CalcJUMP_HLS(rawMET, hwJets, JUMPVector);

  l1t::EtSum theJUMP(JUMPVector, l1t::EtSum::EtSumType::kTotalHt, 0, 0, 0, 0);
  auto JUMPCollection = std::make_unique<std::vector<l1t::EtSum>>(0);
  JUMPCollection->push_back(theJUMP);
  iEvent.put(std::move(JUMPCollection));
}

void L1JUMPProducer::CalcJUMP_HLS(l1t::EtSum& metVector,
                                  std::vector<l1ct::Jet>& jets,
                                  reco::Candidate::PolarLorentzVector& outMet_Vector) const {
  // JUMP Calculate
  l1ct::Sum inMet;
  inMet.hwPt = metVector.pt();
  inMet.hwPhi = l1ct::Scales::makeGlbPhi(metVector.phi());

  l1ct::Sum outMet;

  JUMP_emu(inMet, jets, outMet);

  outMet_Vector.SetPt(outMet.hwPt.to_double());
  outMet_Vector.SetPhi(outMet.hwPhi.to_double() * phiLSB_);
  outMet_Vector.SetEta(0);
}

std::vector<l1ct::Jet> L1JUMPProducer::convertEDMToHW(std::vector<l1t::PFJet> edmJets) const {
  std::vector<l1ct::Jet> hwJets;
  std::for_each(edmJets.begin(), edmJets.end(), [&](l1t::PFJet jet) {
    l1ct::Jet hwJet = l1ct::Jet::unpack(jet.getHWJetCT());
    hwJets.push_back(hwJet);
  });
  return hwJets;
}

std::vector<l1t::EtSum> L1JUMPProducer::convertHWToEDM(l1ct::Sum hwSums) const {
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

L1JUMPProducer::~L1JUMPProducer() {}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(L1JUMPProducer);