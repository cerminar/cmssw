///
/// \class HGCalTowerMap2DImpl
///
/// \author: Thomas Strebler
///
/// Description: first iteration of HGCal Tower Maps

#include "FWCore/Utilities/interface/EDMException.h"

#include "L1Trigger/L1THGCal/interface/be_algorithms/HGCalTowerMap2DImpl.h"
#include "L1Trigger/L1THGCal/interface/HGCalTriggerGeometryBase.h"



HGCalTowerMap2DImpl::HGCalTowerMap2DImpl(const edm::ParameterSet& conf) : useLayerWeights_(conf.getParameter<bool>("useLayerWeights")),
                                                                          layerWeights_(conf.getParameter< std::vector<double> >("layerWeights")) { }

std::map<int, l1t::HGCalTowerMap> HGCalTowerMap2DImpl::newTowerMaps() {
  std::map<int, l1t::HGCalTowerMap> towerMaps;
  for(unsigned layer = 1; layer<=triggerTools_.lastLayerBH(); layer++) {
    // FIXME: this is hardcoded...quite ugly
    if (layer <= triggerTools_.lastLayerEE() && layer%2 == 0) continue;
    towerMaps[layer] = l1t::HGCalTowerMap(triggerTools_.getTriggerGeometry()->getTriggerTowers(), layer);
  }

  return towerMaps;

}




void HGCalTowerMap2DImpl::buildTowerMap2D(const std::vector<edm::Ptr<l1t::HGCalTriggerCell>> & triggerCellsPtrs,
					                                l1t::HGCalTowerMapBxCollection & towerMaps){

<<<<<<< HEAD
  for(const auto& tc : triggerCellsPtrs){

    unsigned layer = triggerTools_.layerWithOffset(tc->detId());
    int iEta = towerMapsTmp[layer-1].iEta(tc->eta());
    int iPhi = towerMapsTmp[layer-1].iPhi(tc->phi());

    double calibPt = tc->pt();
    if(useLayerWeights_) calibPt = layerWeights_[layer]*(tc->mipPt());
    math::PtEtaPhiMLorentzVector p4(calibPt,
				    tc->eta(),
				    tc->phi(),
				    0. );
=======
  std::map<int, l1t::HGCalTowerMap> towerMapsTmp = newTowerMaps();

  for(auto tc:  triggerCellsPtrs) {
    unsigned layer = triggerTools_.layerWithOffset(tc->detId());
    // FIXME: should actually sum the energy not the Et...
    double calibPt = tc->pt();
    if(useLayerWeights_) calibPt = layerWeights_[layer] * tc->mipPt();
>>>>>>> get the trigger tower grid and mapping from the geometry and switch to iX iY mapping

    double etEm = layer<=triggerTools_.lastLayerEE() ? calibPt : 0;
    double etHad = layer>triggerTools_.lastLayerEE() ? calibPt : 0;

    towerMapsTmp[layer].addEt(triggerTools_.getTriggerGeometry()->getTriggerTowerFromTriggerCell(tc->detId()), etEm, etHad);

  }

  /* store towerMaps in the persistent collection */
  towerMaps.resize(0, towerMapsTmp.size());
  int i=0;
  for(auto towerMap : towerMapsTmp){
    towerMaps.set( 0, i, towerMap.second);
    i++;
  }


}
