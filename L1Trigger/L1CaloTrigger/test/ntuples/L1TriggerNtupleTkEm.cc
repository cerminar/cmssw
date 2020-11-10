#include "L1TCaloTriggerNtupleBase.h"

#include "DataFormats/L1TCorrelator/interface/TkEm.h"
#include "DataFormats/L1TCorrelator/interface/TkEmFwd.h"

class L1TriggerNtupleTkEm : public L1TCaloTriggerNtupleBase {
public:
  L1TriggerNtupleTkEm(const edm::ParameterSet& conf);
  ~L1TriggerNtupleTkEm() override{};
  void initialize(TTree&, const edm::ParameterSet&, edm::ConsumesCollector&&) final;
  void fill(const edm::Event& e, const edm::EventSetup& es) final;

private:
  void clear() final;

  edm::EDGetToken tkEm_token_;

  int tkEm_n_;
  std::vector<float> tkEm_pt_;
  std::vector<float> tkEm_energy_;
  std::vector<float> tkEm_eta_;
  std::vector<float> tkEm_phi_;
  std::vector<float> tkEm_hwQual_;
  std::vector<float> tkEm_tkIso_;
  std::vector<float> tkEm_tkIsoPV_;
  std::vector<float> tkEm_pfIso_;
  std::vector<float> tkEm_pfIsoPV_;

};

DEFINE_EDM_PLUGIN(HGCalTriggerNtupleFactory, L1TriggerNtupleTkEm, "L1TriggerNtupleTkEm");

L1TriggerNtupleTkEm::L1TriggerNtupleTkEm(const edm::ParameterSet& conf)
    : L1TCaloTriggerNtupleBase(conf) {}

void L1TriggerNtupleTkEm::initialize(TTree& tree,
                                            const edm::ParameterSet& conf,
                                            edm::ConsumesCollector&& collector) {
  tkEm_token_ = collector.consumes<l1t::TkEmCollection>(conf.getParameter<edm::InputTag>("TkEms"));

  tree.Branch(branch_name_w_prefix("n").c_str(), &tkEm_n_, branch_name_w_prefix("n/I").c_str());
  tree.Branch(branch_name_w_prefix("pt").c_str(), &tkEm_pt_);
  tree.Branch(branch_name_w_prefix("energy").c_str(), &tkEm_energy_);
  tree.Branch(branch_name_w_prefix("eta").c_str(), &tkEm_eta_);
  tree.Branch(branch_name_w_prefix("phi").c_str(), &tkEm_phi_);
  tree.Branch(branch_name_w_prefix("hwQual").c_str(), &tkEm_hwQual_);
  tree.Branch(branch_name_w_prefix("tkIso").c_str(), &tkEm_tkIso_);
  tree.Branch(branch_name_w_prefix("tkIsoPV").c_str(), &tkEm_tkIsoPV_);
  tree.Branch(branch_name_w_prefix("pfIso").c_str(), &tkEm_pfIso_);
  tree.Branch(branch_name_w_prefix("pfIsoPV").c_str(), &tkEm_pfIsoPV_);

}

void L1TriggerNtupleTkEm::fill(const edm::Event& e, const edm::EventSetup& es) {
  // retrieve towers
  edm::Handle<l1t::TkEmCollection> tkEm_h;
  e.getByToken(tkEm_token_, tkEm_h);
  const l1t::TkEmCollection& tkEm_collection = *tkEm_h;

  // triggerTools_.eventSetup(es);
  clear();
  for (auto tkEm_itr : tkEm_collection) {
    tkEm_n_++;
    tkEm_pt_.emplace_back(tkEm_itr.pt());
    tkEm_energy_.emplace_back(tkEm_itr.energy());
    tkEm_eta_.emplace_back(tkEm_itr.eta());
    tkEm_phi_.emplace_back(tkEm_itr.phi());
    tkEm_hwQual_.emplace_back(tkEm_itr.EGRef()->hwQual());
    tkEm_tkIso_.emplace_back(tkEm_itr.trkIsol());
    tkEm_tkIsoPV_.emplace_back(tkEm_itr.trkIsolPV());
    tkEm_pfIso_.emplace_back(tkEm_itr.pfIsol());
    tkEm_pfIsoPV_.emplace_back(tkEm_itr.pfIsolPV());
  }
}

void L1TriggerNtupleTkEm::clear() {
  tkEm_n_ = 0;
  tkEm_pt_.clear();
  tkEm_energy_.clear();
  tkEm_eta_.clear();
  tkEm_phi_.clear();
  tkEm_hwQual_.clear();
  tkEm_tkIso_.clear();
  tkEm_tkIsoPV_.clear();
  tkEm_pfIso_.clear();
  tkEm_pfIsoPV_.clear();

}
