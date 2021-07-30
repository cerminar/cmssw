#include "L1Trigger/L1THGCal/interface/HGCalTriggerTools.h"

#include "L1Trigger/L1TTrackMatch/interface/pTFrom2Stubs.h"

#include "DataFormats/L1TrackTrigger/interface/TTTrack.h"
#include "DataFormats/L1TrackTrigger/interface/TTTypes.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/ParticleFlowReco/interface/PFCluster.h"
#include "DataFormats/L1TParticleFlow/interface/PFTrack.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include "CommonTools/BaseParticlePropagator/interface/BaseParticlePropagator.h"
#include "FWCore/Framework/interface/ESWatcher.h"

#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"

#include "L1TCaloTriggerNtupleBase.h"

class L1TriggerNtupleTrackTrigger : public L1TCaloTriggerNtupleBase {
public:
  L1TriggerNtupleTrackTrigger(const edm::ParameterSet& conf);
  ~L1TriggerNtupleTrackTrigger() override{};
  void initialize(TTree&, const edm::ParameterSet&, edm::ConsumesCollector&&) final;
  void fill(const edm::Event& e, const edm::EventSetup& es) final;
  typedef TTTrack<Ref_Phase2TrackerDigi_> L1TTTrackType;

private:
  void clear() final;
  // HGCalTriggerTools triggerTools_;
  std::pair<float, float> propagateToCalo(const math::XYZTLorentzVector& iMom,
                                          const math::XYZTLorentzVector& iVtx,
                                          double iCharge,
                                          double iBField);

  edm::EDGetToken track_token_;
  edm::EDGetToken decoded_track_token_;
  bool fill_decoded_pftracks;

  int l1track_n_;
  std::vector<float> l1track_pt_;
  std::vector<float> l1track_pt2stubs_;

  // std::vector<float> l1track_energy_;
  std::vector<float> l1track_eta_;
  std::vector<float> l1track_phi_;
  std::vector<float> l1track_curv_;
  std::vector<float> l1track_chi2_;
  std::vector<float> l1track_chi2Red_;
  std::vector<int> l1track_nStubs_;
  std::vector<float> l1track_z0_;
  std::vector<int> l1track_charge_;
  std::vector<float> l1track_caloeta_;
  std::vector<float> l1track_calophi_;

  int pfdtk_n_;
  std::vector<float> pfdtk_pt_;
  std::vector<float> pfdtk_eta_;
  std::vector<float> pfdtk_phi_;
  std::vector<float> pfdtk_caloeta_;
  std::vector<float> pfdtk_calophi_;
  std::vector<float> pfdtk_z0_;
  std::vector<float> pfdtk_simpt_;
  std::vector<float> pfdtk_simeta_;
  std::vector<float> pfdtk_simphi_;
  std::vector<float> pfdtk_simcaloeta_;
  std::vector<float> pfdtk_simcalophi_;
  std::vector<float> pfdtk_simz0_;



  edm::ESWatcher<IdealMagneticFieldRecord> magfield_watcher_;
  HGCalTriggerTools triggerTools_;
};

DEFINE_EDM_PLUGIN(HGCalTriggerNtupleFactory, L1TriggerNtupleTrackTrigger, "L1TriggerNtupleTrackTrigger");

L1TriggerNtupleTrackTrigger::L1TriggerNtupleTrackTrigger(const edm::ParameterSet& conf)
    : L1TCaloTriggerNtupleBase(conf) {}

void L1TriggerNtupleTrackTrigger::initialize(TTree& tree,
                                             const edm::ParameterSet& conf,
                                             edm::ConsumesCollector&& collector) {
  track_token_ =
      collector.consumes<std::vector<TTTrack<Ref_Phase2TrackerDigi_>>>(conf.getParameter<edm::InputTag>("TTTracks"));

  decoded_track_token_ = 
    collector.consumes<std::vector<l1t::PFTrack>>(conf.getParameter<edm::InputTag>("PFDecodedTracks"));

  fill_decoded_pftracks = conf.getParameter<bool>("fillPFDecodedTracks");

  tree.Branch(branch_name_w_prefix("n").c_str(), &l1track_n_, branch_name_w_prefix("n/I").c_str());
  tree.Branch(branch_name_w_prefix("pt").c_str(), &l1track_pt_);
  tree.Branch(branch_name_w_prefix("pt2stubs").c_str(), &l1track_pt2stubs_);

  // tree.Branch("l1track_energy", &l1track_energy_);
  tree.Branch(branch_name_w_prefix("eta").c_str(), &l1track_eta_);
  tree.Branch(branch_name_w_prefix("phi").c_str(), &l1track_phi_);
  tree.Branch(branch_name_w_prefix("curv").c_str(), &l1track_curv_);
  tree.Branch(branch_name_w_prefix("chi2").c_str(), &l1track_chi2_);
  tree.Branch(branch_name_w_prefix("chi2Red").c_str(), &l1track_chi2Red_);
  tree.Branch(branch_name_w_prefix("nStubs").c_str(), &l1track_nStubs_);
  tree.Branch(branch_name_w_prefix("z0").c_str(), &l1track_z0_);
  tree.Branch(branch_name_w_prefix("charge").c_str(), &l1track_charge_);
  tree.Branch(branch_name_w_prefix("caloeta").c_str(), &l1track_caloeta_);
  tree.Branch(branch_name_w_prefix("calophi").c_str(), &l1track_calophi_);
  
  if(fill_decoded_pftracks) {
    tree.Branch("pfdtk_n", &pfdtk_n_, "pfdtk_n/I");
    tree.Branch("pfdtk_pt", &pfdtk_pt_);
    tree.Branch("pfdtk_eta", &pfdtk_eta_);
    tree.Branch("pfdtk_phi", &pfdtk_phi_);
    tree.Branch("pfdtk_caloeta", &pfdtk_caloeta_);
    tree.Branch("pfdtk_calophi", &pfdtk_calophi_);
    tree.Branch("pfdtk_z0", &pfdtk_z0_);
    tree.Branch("pfdtk_simpt", &pfdtk_simpt_);
    tree.Branch("pfdtk_simeta", &pfdtk_simeta_);
    tree.Branch("pfdtk_simphi", &pfdtk_simphi_);
    tree.Branch("pfdtk_simcaloeta", &pfdtk_simcaloeta_);
    tree.Branch("pfdtk_simcalophi", &pfdtk_simcalophi_);
    tree.Branch("pfdtk_simz0", &pfdtk_simz0_);
  }
  
}

void L1TriggerNtupleTrackTrigger::fill(const edm::Event& ev, const edm::EventSetup& es) {
  // the L1Tracks
  edm::Handle<std::vector<L1TTTrackType>> l1TTTrackHandle;
  ev.getByToken(track_token_, l1TTTrackHandle);

  edm::Handle<std::vector<l1t::PFTrack>> pfDecodedTrackHandle;
  ev.getByToken(decoded_track_token_, pfDecodedTrackHandle);


  float fBz = 0;
  if (magfield_watcher_.check(es)) {
    edm::ESHandle<MagneticField> magfield;
    es.get<IdealMagneticFieldRecord>().get(magfield);
    // aField_ = &(*magfield);
    fBz = magfield->inTesla(GlobalPoint(0, 0, 0)).z();
  }

  // geometry needed to call pTFrom2Stubs
  edm::ESHandle<TrackerGeometry> geomHandle;
  es.get<TrackerDigiGeometryRecord>().get("idealForDigi", geomHandle);
  const TrackerGeometry* tGeom = geomHandle.product();

  triggerTools_.eventSetup(es);

  clear();
  for (auto trackIter = l1TTTrackHandle->begin(); trackIter != l1TTTrackHandle->end(); ++trackIter) {
    l1track_n_++;
    l1track_pt_.emplace_back(trackIter->momentum().perp());
    l1track_pt2stubs_.emplace_back(pTFrom2Stubs::pTFrom2(trackIter, tGeom));

    // l1track_energy_.emplace_back(trackIter->energy());
    l1track_eta_.emplace_back(trackIter->momentum().eta());
    l1track_phi_.emplace_back(trackIter->momentum().phi());
    l1track_curv_.emplace_back(trackIter->rInv());
    l1track_chi2_.emplace_back(trackIter->chi2());
    l1track_chi2Red_.emplace_back(trackIter->chi2Red());
    l1track_nStubs_.emplace_back(trackIter->getStubRefs().size());
    // FIXME: need to be configuratble?
    // int nParam_ = 4;
    float z0 = trackIter->POCA().z();  //cm
    int charge = trackIter->rInv() > 0 ? +1 : -1;

    reco::Candidate::PolarLorentzVector p4p(
        trackIter->momentum().perp(), trackIter->momentum().eta(), trackIter->momentum().phi(), 0);  // no mass ?
    reco::Particle::LorentzVector p4(p4p.X(), p4p.Y(), p4p.Z(), p4p.E());
    reco::Particle::Point vtx(0., 0., z0);

    auto caloetaphi = propagateToCalo(p4, math::XYZTLorentzVector(0., 0., z0, 0.), charge, fBz);

    l1track_z0_.emplace_back(z0);
    l1track_charge_.emplace_back(charge);
    l1track_caloeta_.emplace_back(caloetaphi.first);
    l1track_calophi_.emplace_back(caloetaphi.second);
  }
  
  if(fill_decoded_pftracks) {
    for(auto dtk: *pfDecodedTrackHandle) {
      pfdtk_n_++;
      pfdtk_pt_.emplace_back(dtk.pt());
      pfdtk_eta_.emplace_back(dtk.eta());
      pfdtk_phi_.emplace_back(dtk.phi());
      pfdtk_caloeta_.emplace_back(dtk.caloEta());
      pfdtk_calophi_.emplace_back(dtk.caloPhi());
      pfdtk_z0_.emplace_back(dtk.vz());

      auto tk = dtk.track();
      
      float z0 = tk->POCA().z();  //cm
      int charge = tk->rInv() > 0 ? +1 : -1;
      reco::Candidate::PolarLorentzVector p4p(
          tk->momentum().perp(), tk->momentum().eta(), tk->momentum().phi(), 0);  // no mass ?
      reco::Particle::LorentzVector p4(p4p.X(), p4p.Y(), p4p.Z(), p4p.E());
      reco::Particle::Point vtx(0., 0., z0);

      auto caloetaphi = propagateToCalo(p4, math::XYZTLorentzVector(0., 0., z0, 0.), charge, fBz);
      
      pfdtk_simpt_.emplace_back(tk->momentum().perp());
      pfdtk_simeta_.emplace_back(tk->momentum().eta());
      pfdtk_simphi_.emplace_back(tk->momentum().phi());

      pfdtk_simcaloeta_.emplace_back(caloetaphi.first);
      pfdtk_simcalophi_.emplace_back(caloetaphi.second);
      pfdtk_simz0_.emplace_back(z0);
    }  
  }
}

void L1TriggerNtupleTrackTrigger::clear() {
  l1track_n_ = 0;
  l1track_pt_.clear();
  l1track_pt2stubs_.clear();
  // l1track_energy_.clear();
  l1track_eta_.clear();
  l1track_phi_.clear();
  l1track_curv_.clear();
  l1track_chi2_.clear();
  l1track_chi2Red_.clear();
  l1track_nStubs_.clear();
  l1track_z0_.clear();
  l1track_charge_.clear();
  l1track_caloeta_.clear();
  l1track_calophi_.clear();
  
  pfdtk_n_ = 0;
  pfdtk_pt_.clear();
  pfdtk_eta_.clear();
  pfdtk_phi_.clear();
  pfdtk_caloeta_.clear();
  pfdtk_calophi_.clear();
  pfdtk_z0_.clear();
  pfdtk_simpt_.clear();
  pfdtk_simeta_.clear();
  pfdtk_simphi_.clear();
  pfdtk_simcaloeta_.clear();
  pfdtk_simcalophi_.clear();
  pfdtk_simz0_.clear();

  
}

// #include "FastSimulation/Particle/interface/RawParticle.h"

std::pair<float, float> L1TriggerNtupleTrackTrigger::propagateToCalo(const math::XYZTLorentzVector& iMom,
                                                                     const math::XYZTLorentzVector& iVtx,
                                                                     double iCharge,
                                                                     double iBField) {
  BaseParticlePropagator prop = BaseParticlePropagator(RawParticle(iMom, iVtx, iCharge), 0., 0., iBField);
  prop.setPropagationConditions(129.0, triggerTools_.getLayerZ(1), false);
  prop.propagate();
  double ecalShowerDepth = reco::PFCluster::getDepthCorrection(prop.particle().momentum().E(), false, false);
  math::XYZVector point = math::XYZVector(prop.particle().vertex()) +
                          math::XYZTLorentzVector(prop.particle().momentum()).Vect().Unit() * ecalShowerDepth;
  // math::XYZVector point  = particle.vertex();
  // math::XYZVector point = math::XYZVector(particle.vertex())+math::XYZTLorentzVector(particle.momentum()).Vect().Unit()*ecalShowerDepth;
  return std::make_pair(point.eta(), point.phi());
}
