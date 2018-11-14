#include "DataFormats/ForwardDetId/interface/HGCalDetId.h"
#include "DataFormats/L1THGCal/interface/HGCalTriggerCell.h"
#include "DataFormats/L1THGCal/interface/HGCalTowerMap.h"
#include "DataFormats/L1THGCal/interface/HGCalTower.h"

#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "L1Trigger/L1THGCal/interface/HGCalTriggerGeometryBase.h"
#include "L1Trigger/L1THGCal/interface/HGCalTriggerBackendAlgorithmBase.h"
#include "L1Trigger/L1THGCal/interface/fe_codecs/HGCalTriggerCellBestChoiceCodec.h"
#include "L1Trigger/L1THGCal/interface/fe_codecs/HGCalTriggerCellThresholdCodec.h"
#include "L1Trigger/L1THGCal/interface/be_algorithms/HGCalTriggerCellCalibration.h"
#include "L1Trigger/L1THGCal/interface/be_algorithms/HGCalTowerMap2DImpl.h"
#include "L1Trigger/L1THGCal/interface/be_algorithms/HGCalTowerMap3DImpl.h"

#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "L1Trigger/L1THGCal/interface/HGCalTriggerTools.h"

using namespace HGCalTriggerBackend;


template<typename FECODEC, typename DATA>
class HGCSimFractionTowerAlgo : public Algorithm<FECODEC>
{
    public:
        using Algorithm<FECODEC>::name;

    protected:
        using Algorithm<FECODEC>::codec_;



    public:

        HGCSimFractionTowerAlgo(const edm::ParameterSet& conf, edm::ConsumesCollector& cc) :
        Algorithm<FECODEC>(conf, cc),
        trgcell_product_( new l1t::HGCalTriggerCellBxCollection ),
        towermap_product_( new l1t::HGCalTowerMapBxCollection ),
        tower_product_( new l1t::HGCalTowerBxCollection ),
        calibration_( conf.getParameterSet("calib_parameters") ),
        towermap2D_( conf.getParameterSet("towermap_parameters") ),
        towermap3D_( ),
        simClusters_(cc.consumes<std::vector<SimCluster>>(conf.getParameter<edm::InputTag>("simClusters"))),
        simClusterSelection_(conf.getParameter<std::string>("simClusterSelection")),
        tcLabel_(conf.getParameter<std::string>("tcLabel")),
        towerMapLabel_(conf.getParameter<std::string>("towerMapLabel")),
        towerLabel_(conf.getParameter<std::string>("towerLabel")),
        debug_(conf.getUntrackedParameter<bool>("debug", false))
        {
        }


        void setProduces(edm::stream::EDProducer<>& prod) const final
        {
            prod.produces<l1t::HGCalTriggerCellBxCollection>(tcLabel_);
            prod.produces<l1t::HGCalTowerMapBxCollection>(towerMapLabel_);
            prod.produces<l1t::HGCalTowerBxCollection>(towerLabel_);
        }


        void run(const l1t::HGCFETriggerDigiCollection& coll, const edm::EventSetup& es, edm::Event&evt ) final;


        void putInEvent(edm::Event& evt) final
        {

        }


        void reset() final
        {
            trgcell_product_.reset( new l1t::HGCalTriggerCellBxCollection );
            towermap_product_.reset( new l1t::HGCalTowerMapBxCollection );
            tower_product_.reset( new l1t::HGCalTowerBxCollection );
        }


    private:

        /* pointers to collections of trigger-cells, towerMaps and towers */
        std::unique_ptr<l1t::HGCalTriggerCellBxCollection> trgcell_product_;
        std::unique_ptr<l1t::HGCalTowerMapBxCollection> towermap_product_;
        std::unique_ptr<l1t::HGCalTowerBxCollection> tower_product_;

        edm::ESHandle<HGCalTriggerGeometryBase> triggerGeometry_;

        /* algorithms instances */
        HGCalTriggerCellCalibration calibration_;
        HGCalTowerMap2DImpl towermap2D_;
        HGCalTowerMap3DImpl towermap3D_;
        edm::EDGetTokenT<std::vector<SimCluster>> simClusters_;
        StringCutObjectSelector<SimCluster> simClusterSelection_;
        HGCalTriggerTools triggerTools_;
        std::string tcLabel_;
        std::string towerMapLabel_;
        std::string towerLabel_;
        bool debug_;
};


template<typename FECODEC, typename DATA>
void HGCSimFractionTowerAlgo<FECODEC,DATA>::run(const l1t::HGCFETriggerDigiCollection & coll,
                                       const edm::EventSetup & es,
                                       edm::Event & evt )
{
    es.get<CaloGeometryRecord>().get("", triggerGeometry_);
    calibration_.eventSetup(es);
    towermap2D_.eventSetup(es);
    triggerTools_.eventSetup(es);

    edm::Handle<std::vector<SimCluster>> simClusters;
    evt.getByToken(simClusters_, simClusters);

    std::set<uint32_t> tcs_with_signaldeposit;


    // FIXME: we fill a map TCID fractions
    for (const auto & sc : *simClusters) {
        bool pass = simClusterSelection_(sc);
        if(debug_)
          printf("SimCluster from particle of pdgId %+d et %.2f eta %+.2f phi %+.2f from event %d bx %+d (selected: %c)\n",
            sc.pdgId(), sc.et(), sc.eta(), sc.phi(), sc.eventId().event(), sc.eventId().bunchCrossing(), (pass ? 'Y': 'n'));
        if(pass) {
          for (const auto & pair : sc.hits_and_fractions()) {
              if (pair.second == 0) continue;
              try {
                unsigned triggerCellId = triggerTools_.getTriggerGeometry()->getTriggerCellFromCell(pair.first);
                  //FIXME: very rough approx: if there is a fraction > 0 in a cell of this TC we will use it
                if(pair.second > 0) {
                    tcs_with_signaldeposit.insert(triggerCellId);
                }
              } catch (cms::Exception & ex) {
                      printf("\thit on detid %12u (no associated TC), fraction %.4f\n",
                              pair.first, pair.second);
              }
          }
        }
    }



    for( const auto& digi : coll ){

        HGCalDetId module_id( digi.id() );


        DATA data;
        data.reset();
        digi.decode(codec_, data);

        for(const auto& triggercell : data.payload)
        {

            if( triggercell.hwPt() > 0 && (tcs_with_signaldeposit.find(triggercell.detId()) != tcs_with_signaldeposit.end()))
            {
                l1t::HGCalTriggerCell calibratedtriggercell( triggercell );
                calibration_.calibrateInGeV( calibratedtriggercell);
                trgcell_product_->push_back( 0, calibratedtriggercell );

                if(debug_) std::cout << "TC: " << triggercell.detId() << " hwPt: " << triggercell.hwPt() << " MIPT: " << triggercell.mipPt() << " eta: " << triggercell.eta() << " phi: " << triggercell.phi() << "selected YES!" << std::endl;

            } else {
              if(debug_) std::cout << "TC: " << triggercell.detId() << " hwPt: " << triggercell.hwPt() << " MIPT: " << triggercell.mipPt() << " eta: " << triggercell.eta() << " phi: " << triggercell.phi() << " selected NO!" << std::endl;
            }
        }

    }

    /* orphan handles to the collections of trigger-cells, towermaps and towers */
    edm::OrphanHandle<l1t::HGCalTriggerCellBxCollection> triggerCellsHandle;
    edm::OrphanHandle<l1t::HGCalTowerMapBxCollection> towerMapsHandle;
    edm::OrphanHandle<l1t::HGCalTowerBxCollection> towersHandle;

    /* retrieve the orphan handle to the trigger-cells collection and put the collection in the event */
    triggerCellsHandle = evt.put( std::move( trgcell_product_ ), tcLabel_);

    /* create a persistent vector of pointers to the trigger-cells */
    std::vector<edm::Ptr<l1t::HGCalTriggerCell>> triggerCellsPtrs;
    for( unsigned i = 0; i < triggerCellsHandle->size(); ++i ) {
        edm::Ptr<l1t::HGCalTriggerCell> ptr(triggerCellsHandle,i);
        triggerCellsPtrs.push_back(ptr);
    }

    /* call to towerMap2D clustering */
    towermap2D_.buildTowerMap2D( triggerCellsPtrs, *towermap_product_);

    /* retrieve the orphan handle to the towermaps collection and put the collection in the event */
    towerMapsHandle = evt.put( std::move( towermap_product_ ), towerMapLabel_);

    /* create a persistent vector of pointers to the towerMaps */
    std::vector<edm::Ptr<l1t::HGCalTowerMap>> towerMapsPtrs;
    for( unsigned i = 0; i < towerMapsHandle->size(); ++i ) {
        edm::Ptr<l1t::HGCalTowerMap> ptr(towerMapsHandle,i);
        towerMapsPtrs.push_back(ptr);
    }

    /* call to towerMap3D clustering */
    towermap3D_.buildTowerMap3D( towerMapsPtrs, *tower_product_);

    /* retrieve the orphan handle to the tower collection and put the collection in the event */
    towersHandle = evt.put( std::move( tower_product_ ), towerLabel_);

}

typedef HGCSimFractionTowerAlgo<HGCalTriggerCellBestChoiceCodec, HGCalTriggerCellBestChoiceCodec::data_type> HGCSimFractionTowerAlgoBestChoice;
typedef HGCSimFractionTowerAlgo<HGCalTriggerCellThresholdCodec, HGCalTriggerCellThresholdCodec::data_type> HGCSimFractionTowerAlgoThreshold;


DEFINE_EDM_PLUGIN(HGCalTriggerBackendAlgorithmFactory,
        HGCSimFractionTowerAlgoBestChoice,
        "HGCSimFractionTowerAlgoBestChoice");

DEFINE_EDM_PLUGIN(HGCalTriggerBackendAlgorithmFactory,
        HGCSimFractionTowerAlgoThreshold,
        "HGCSimFractionTowerAlgoThreshold");
