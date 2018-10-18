#include "DataFormats/L1THGCal/interface/HGCalTower.h"
#include "DataFormats/L1THGCal/interface/HGCalTowerMap.h"
#include "DataFormats/ForwardDetId/interface/HGCalDetId.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "L1Trigger/L1THGCal/interface/HGCalTriggerGeometryBase.h"
#include "L1Trigger/L1THGCal/interface/HGCalTriggerNtupleBase.h"
#include "L1Trigger/L1THGCal/interface/HGCalTriggerTools.h"



class HGCalTriggerNtupleHGCTowers : public HGCalTriggerNtupleBase
{

  public:
    HGCalTriggerNtupleHGCTowers(const edm::ParameterSet& conf);
    ~HGCalTriggerNtupleHGCTowers() override{};
    void initialize(TTree&, const edm::ParameterSet&, edm::ConsumesCollector&&) final;
    void fill(const edm::Event& e, const edm::EventSetup& es) final;

  private:
    void clear() final;
    HGCalTriggerTools triggerTools_;

    edm::EDGetToken towers_token_;
    edm::EDGetToken tower_maps_token_;

    int tower_n_ ;
    std::vector<float> tower_pt_;
    std::vector<float> tower_energy_;
    std::vector<float> tower_eta_;
    std::vector<float> tower_phi_;
    std::vector<float> tower_etEm_;
    std::vector<float> tower_etHad_;
    std::vector<int> tower_iEta_;
    std::vector<int> tower_iPhi_;
    std::vector<std::vector<float> > tower_etLayers_;

    std::string branch_name_prefix_;

};

DEFINE_EDM_PLUGIN(HGCalTriggerNtupleFactory,
    HGCalTriggerNtupleHGCTowers,
    "HGCalTriggerNtupleHGCTowers" );


HGCalTriggerNtupleHGCTowers::
HGCalTriggerNtupleHGCTowers(const edm::ParameterSet& conf):HGCalTriggerNtupleBase(conf)
{
}

void
HGCalTriggerNtupleHGCTowers::
initialize(TTree& tree, const edm::ParameterSet& conf, edm::ConsumesCollector&& collector)
{
  towers_token_ = collector.consumes<l1t::HGCalTowerBxCollection>(conf.getParameter<edm::InputTag>("Towers"));
  tower_maps_token_ = collector.consumes<l1t::HGCalTowerMapBxCollection>(conf.getParameter<edm::InputTag>("TowerMaps"));
  branch_name_prefix_ = conf.getUntrackedParameter<std::string>("BranchNamePrefix", "tower");

  tree.Branch((branch_name_prefix_+"_n").c_str(), &tower_n_, (branch_name_prefix_+"_n/I").c_str());
  tree.Branch((branch_name_prefix_+"_pt").c_str(), &tower_pt_);
  tree.Branch((branch_name_prefix_+"_energy").c_str(), &tower_energy_);
  tree.Branch((branch_name_prefix_+"_eta").c_str(), &tower_eta_);
  tree.Branch((branch_name_prefix_+"_phi").c_str(), &tower_phi_);
  tree.Branch((branch_name_prefix_+"_etEm").c_str(), &tower_etEm_);
  tree.Branch((branch_name_prefix_+"_etHad").c_str(), &tower_etHad_);
  tree.Branch((branch_name_prefix_+"_iEta").c_str(), &tower_iEta_);
  tree.Branch((branch_name_prefix_+"_iPhi").c_str(), &tower_iPhi_);
  tree.Branch((branch_name_prefix_+"_etLayers").c_str(), &tower_etLayers_);

}



void
HGCalTriggerNtupleHGCTowers::
fill(const edm::Event& e, const edm::EventSetup& es)
{

  // retrieve towers
  edm::Handle<l1t::HGCalTowerBxCollection> towers_h;
  e.getByToken(towers_token_, towers_h);
  const l1t::HGCalTowerBxCollection& towers = *towers_h;

  // retrieve geometry
  edm::ESHandle<HGCalTriggerGeometryBase> geometry;
  es.get<CaloGeometryRecord>().get(geometry);

  edm::Handle<l1t::HGCalTowerMapBxCollection> towers_maps_h;
  e.getByToken(tower_maps_token_, towers_maps_h);
  const l1t::HGCalTowerMapBxCollection& tower_maps = *towers_maps_h;

  triggerTools_.eventSetup(es);

  clear();
  for(auto tower_itr=towers.begin(0); tower_itr!=towers.end(0); tower_itr++)
  {
    tower_n_++;
    // physical values
    tower_pt_.emplace_back(tower_itr->pt());
    tower_energy_.emplace_back(tower_itr->energy());
    tower_eta_.emplace_back(tower_itr->eta());
    tower_phi_.emplace_back(tower_itr->phi());
    tower_etEm_.emplace_back(tower_itr->etEm());
    tower_etHad_.emplace_back(tower_itr->etHad());

    tower_iEta_.emplace_back(tower_itr->id().iEta());
    tower_iPhi_.emplace_back(tower_itr->id().iPhi());
    std::vector<float> et_layers;
    et_layers.reserve(triggerTools_.lastLayerBH());
    for(auto tower_map: tower_maps ) {
      et_layers[tower_map.layer()-1] = tower_map.towers().find(tower_itr->id().rawId())->second.pt();
    }
    tower_etLayers_.push_back(et_layers);
  }
}


void
HGCalTriggerNtupleHGCTowers::
clear()
{
  tower_n_ = 0;
  tower_pt_.clear();
  tower_energy_.clear();
  tower_eta_.clear();
  tower_phi_.clear();
  tower_etEm_.clear();
  tower_etHad_.clear();
  tower_iEta_.clear();
  tower_iPhi_.clear();
  tower_etLayers_.clear();
}
