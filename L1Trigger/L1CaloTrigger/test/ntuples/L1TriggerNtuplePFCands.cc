#include "L1TCaloTriggerNtupleBase.h"

#include "DataFormats/L1TParticleFlow/interface/PFCandidate.h"

class L1TriggerNtuplePFCands : public L1TCaloTriggerNtupleBase {
public:
  L1TriggerNtuplePFCands(const edm::ParameterSet& conf);
  ~L1TriggerNtuplePFCands() override{};
  void initialize(TTree&, const edm::ParameterSet&, edm::ConsumesCollector&&) final;
  void fill(const edm::Event& e, const edm::EventSetup& es) final;

private:
  void clear() final;

  edm::EDGetToken pfCand_token_;

  int pfCand_n_;
  std::vector<float> pfCand_pt_;
  std::vector<float> pfCand_energy_;
  std::vector<float> pfCand_eta_;
  std::vector<float> pfCand_phi_;
  std::vector<float> pfCand_id_;
  std::vector<float> pfCand_z0_;
  std::vector<float> pfCand_caloEta_;
  std::vector<float> pfCand_caloPhi_;
  
  
  
  
};

DEFINE_EDM_PLUGIN(HGCalTriggerNtupleFactory, L1TriggerNtuplePFCands, "L1TriggerNtuplePFCands");

L1TriggerNtuplePFCands::L1TriggerNtuplePFCands(const edm::ParameterSet& conf)
    : L1TCaloTriggerNtupleBase(conf) {}

void L1TriggerNtuplePFCands::initialize(TTree& tree,
                                            const edm::ParameterSet& conf,
                                            edm::ConsumesCollector&& collector) {
  pfCand_token_ = collector.consumes<std::vector<l1t::PFCandidate>>(conf.getParameter<edm::InputTag>("L1PFObjects"));

  tree.Branch(branch_name_w_prefix("n").c_str(), &pfCand_n_, branch_name_w_prefix("n/I").c_str());
  tree.Branch(branch_name_w_prefix("pt").c_str(), &pfCand_pt_);
  tree.Branch(branch_name_w_prefix("energy").c_str(), &pfCand_energy_);
  tree.Branch(branch_name_w_prefix("eta").c_str(), &pfCand_eta_);
  tree.Branch(branch_name_w_prefix("phi").c_str(), &pfCand_phi_);
  tree.Branch(branch_name_w_prefix("id").c_str(), &pfCand_id_);
  tree.Branch(branch_name_w_prefix("z0").c_str(), &pfCand_z0_);
  tree.Branch(branch_name_w_prefix("caloEta").c_str(), &pfCand_caloEta_);
  tree.Branch(branch_name_w_prefix("caloPhi").c_str(), &pfCand_caloPhi_);
  
}

void L1TriggerNtuplePFCands::fill(const edm::Event& e, const edm::EventSetup& es) {
  // retrieve towers
  edm::Handle<l1t::PFCandidateCollection> pfCand_h;
  e.getByToken(pfCand_token_, pfCand_h);
  const l1t::PFCandidateCollection& pfCand_collection = *pfCand_h;

  // triggerTools_.eventSetup(es);
  clear();
  // std::cout << branch_name_w_prefix("") << " # ele: " << pfCand_collection.size() << std::endl;
  for (auto pfCand_itr : pfCand_collection) {
    pfCand_n_++;
    pfCand_pt_.emplace_back(pfCand_itr.pt());
    pfCand_energy_.emplace_back(pfCand_itr.energy());
    pfCand_eta_.emplace_back(pfCand_itr.eta());
    pfCand_phi_.emplace_back(pfCand_itr.phi());
    pfCand_id_.emplace_back(pfCand_itr.id());
    pfCand_z0_.emplace_back(pfCand_itr.z0());
    pfCand_caloEta_.emplace_back(pfCand_itr.caloEta());
    pfCand_caloPhi_.emplace_back(pfCand_itr.caloPhi());
    
  }
}

void L1TriggerNtuplePFCands::clear() {
  pfCand_n_ = 0;
  pfCand_pt_.clear();
  pfCand_energy_.clear();
  pfCand_eta_.clear();
  pfCand_phi_.clear();
  pfCand_id_.clear();
  pfCand_z0_.clear();
  pfCand_caloEta_.clear();
  pfCand_caloPhi_.clear();
}
