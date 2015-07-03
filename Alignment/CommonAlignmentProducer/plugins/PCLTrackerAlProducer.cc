/// \file PCLTrackerAlProducer.cc
///
///  \author    : Frederic Ronga
///  Revision   : $Revision: 1.68 $
///  last update: $Date: 2012/08/10 09:25:23 $
///  by         : $Author: flucke $

// TODO: Update Doxygen description



/*** Header file ***/
#include "PCLTrackerAlProducer.h"

/*** Core framework functionality ***/
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESTransientHandle.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/Parse.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"

/*** Alignment ***/
#include "Alignment/TrackerAlignment/interface/TrackerScenarioBuilder.h"
#include "Alignment/MuonAlignment/interface/MuonScenarioBuilder.h"
#include "Alignment/CommonAlignmentAlgorithm/interface/AlignmentAlgorithmPluginFactory.h"
#include "Alignment/CommonAlignmentAlgorithm/interface/IntegratedCalibrationPluginFactory.h"
#include "Alignment/CommonAlignmentMonitor/interface/AlignmentMonitorPluginFactory.h"
#include "Alignment/CommonAlignmentParametrization/interface/RigidBodyAlignmentParameters.h"
#include "Alignment/CommonAlignmentParametrization/interface/BeamSpotAlignmentParameters.h"
#include "Alignment/CommonAlignmentAlgorithm/interface/AlignmentParameterSelector.h"
#include "Alignment/CommonAlignment/interface/SurveyDet.h"
#include "Alignment/LaserAlignment/interface/TsosVectorCollection.h"
#include "TrackingTools/PatternTools/interface/TrajTrackAssociation.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"

/*** Geometry ***/
#include "Geometry/TrackingGeometryAligner/interface/GeometryAligner.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeomBuilderFromGeometricDet.h"
#include "Geometry/DTGeometry/interface/DTGeometry.h"
#include "Geometry/CSCGeometry/interface/CSCGeometry.h"
#include "Geometry/DTGeometryBuilder/src/DTGeometryBuilderFromDDD.h"
#include "Geometry/CSCGeometryBuilder/src/CSCGeometryBuilderFromDDD.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "Geometry/Records/interface/MuonNumberingRecord.h"

/*** Logging ***/
#define CLASS_NAME typeid(*this).name()
#define FUNC_NAME  __FUNCTION__

template <typename T>
void buildMessage(std::ostream& oss, T t)
{
  oss << t;
}

template<typename T, typename... Args>
void buildMessage(std::ostream& oss, T t, Args... args)
{
  buildMessage(oss, t);
  buildMessage(oss, args...);
}

template<typename... Args>
void PRINT_INFO(std::string className, std::string funcName, Args... args)
{
  //std::string className = "PCLTrackerAlProducer";

  std::ostringstream oss;
  buildMessage(oss, args...);

  edm::LogInfo("Alignment") << "@SUB=" << className.c_str() << "::"
                            << funcName.c_str() << ": "
                            << oss.str() << std::endl;
  printf("%s::%s: %s\n", className.c_str(),
                         funcName.c_str(),
                         oss.str().c_str());
}

//TODO: void PRINT_WARNING()
//TODO: void PRINT_ERROR()



//_____________________________________________________________________________
PCLTrackerAlProducer
::PCLTrackerAlProducer(const edm::ParameterSet& config) :
  theAlignmentAlgo(0),
  theAlignmentParameterStore(0),
  theTrackerAlignables(0),
  theMuonAlignables(0),
  theExtraAlignables(0),
  globalPositions_(0),

  /* Steering parameters */
  theParameterSet(config),
  stNFixAlignables_        (config.getParameter<int>          ("nFixAlignables")),
  stRandomShift_           (config.getParameter<double>       ("randomShift")),
  stRandomRotation_        (config.getParameter<double>       ("randomRotation")),
  applyDbAlignment_        (config.getUntrackedParameter<bool>("applyDbAlignment")),
  checkDbAlignmentValidity_(config.getUntrackedParameter<bool>("checkDbAlignmentValidity")),
  doMisalignmentScenario_  (config.getParameter<bool>         ("doMisalignmentScenario")),
  saveToDB_                (config.getParameter<bool>         ("saveToDB")),
  saveApeToDB_             (config.getParameter<bool>         ("saveApeToDB")),
  saveDeformationsToDB_    (config.getParameter<bool>         ("saveDeformationsToDB")),
  doTracker_               (config.getUntrackedParameter<bool>("doTracker") ),
  doMuon_                  (config.getUntrackedParameter<bool>("doMuon") ),
  useExtras_               (config.getUntrackedParameter<bool>("useExtras")),
  useSurvey_               (config.getParameter<bool>         ("useSurvey")),

  /* Event input tags */
  tjTkAssociationMapTag_   (config.getParameter<edm::InputTag>("tjTkAssociationMapTag")),
  beamSpotTag_             (config.getParameter<edm::InputTag>("beamSpotTag")),
  tkLasBeamTag_            (config.getParameter<edm::InputTag>("tkLasBeamTag")),
  clusterValueMapTag_      (config.getParameter<edm::InputTag>("hitPrescaleMapTag"))
{
  PRINT_INFO(CLASS_NAME, FUNC_NAME, "called");

  createAlignmentAlgorithm(config);
  createCalibrations      (config);
  createMonitors          (config);
}

//_____________________________________________________________________________
PCLTrackerAlProducer
::~PCLTrackerAlProducer()
{
  PRINT_INFO(CLASS_NAME, FUNC_NAME, "called");

  delete theAlignmentAlgo;

  for (auto iCal  = theCalibrations.begin();
            iCal != theCalibrations.end();
          ++iCal) {
    delete *iCal;
  }
  // TODO: Delete monitors as well?

  delete theAlignmentParameterStore;
  delete theTrackerAlignables;
  delete theMuonAlignables;
  delete theExtraAlignables;
  delete globalPositions_;
}



/******************************************************************************
 *** INTERFACE IMPLEMENTATION                                               ***
 ******************************************************************************/

//_____________________________________________________________________________
void PCLTrackerAlProducer
::beginJob()
{
  PRINT_INFO(CLASS_NAME, FUNC_NAME, "called");

  nevent_ = 0;

  for (auto iCal  = theCalibrations.begin();
            iCal != theCalibrations.end();
          ++iCal) {
    (*iCal)->beginOfJob(theTrackerAlignables,
                        theMuonAlignables,
                        theExtraAlignables);
  }

  for (auto monitor  = theMonitors.begin();
            monitor != theMonitors.end();
          ++monitor) {
     (*monitor)->beginOfJob(theTrackerAlignables,
                            theMuonAlignables,
                            theAlignmentParameterStore);
  }
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::endJob()
{
  PRINT_INFO(CLASS_NAME, FUNC_NAME, "called, events processed: ", nevent_);

  finish();

  for (auto monitor  = theMonitors.begin();
            monitor != theMonitors.end();
          ++monitor) {
    (*monitor)->endOfJob();
  }

  for (auto iCal  = theCalibrations.begin();
            iCal != theCalibrations.end();
          ++iCal) {
    (*iCal)->endOfJob();
  }
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::beginRun(const edm::Run& run, const edm::EventSetup& setup)
{
  PRINT_INFO(CLASS_NAME, FUNC_NAME, "called");
  // Do not forward edm::Run
  theAlignmentAlgo->beginRun(setup);
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::endRun(const edm::Run& run, const edm::EventSetup& setup)
{
  PRINT_INFO(CLASS_NAME, FUNC_NAME, "called");

  // TODO: Either MP nor HIP is implementing the endRun() method... so this
  //       seems to be useless?

  if (tkLasBeamTag_.encode().size()) {
    edm::Handle<TkFittedLasBeamCollection> lasBeams;
    edm::Handle<TsosVectorCollection> tsoses;
    run.getByLabel(tkLasBeamTag_, lasBeams);
    run.getByLabel(tkLasBeamTag_, tsoses);

    theAlignmentAlgo->endRun(EndRunInfo(run.id(), &(*lasBeams),
                                        &(*tsoses)), setup);
  } else {
    edm::LogInfo("Alignment") << "@SUB=PCLTrackerAlProducer::endRun"
                              << "No Tk LAS beams to forward to algorithm.";
    theAlignmentAlgo->endRun(EndRunInfo(run.id(), 0, 0), setup);
  }
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::beginLuminosityBlock(const edm::LuminosityBlock& lumiBlock,
                       const edm::EventSetup&      setup)
{
  PRINT_INFO(CLASS_NAME, FUNC_NAME, "called");
  // Do not forward edm::LuminosityBlock
  theAlignmentAlgo->beginLuminosityBlock(setup);
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::endLuminosityBlock(const edm::LuminosityBlock& lumiBlock,
                     const edm::EventSetup&      setup)
{
  PRINT_INFO(CLASS_NAME, FUNC_NAME, "called");
  // Do not forward edm::LuminosityBlock
  theAlignmentAlgo->endLuminosityBlock(setup);
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::analyze(const edm::Event& event, const edm::EventSetup& setup)
{
  ++nevent_;
  PRINT_INFO(CLASS_NAME, FUNC_NAME, "called, event number: ", nevent_);

  if (setupChanged(setup)) {
    initAlignmentAlgorithm(setup);
    initBeamSpot(event);
  }

  if (!theAlignmentAlgo->processesEvents()) {
    edm::LogWarning("BadConfig") << "@SUB=PCLTrackerAlProducer::analyze"
                                 << "Skipping event. The current configuration "
                                 << "of the alignment algorithm does not need "
                                 << "to process any events.";
    return;
  }

  // reading in survey records
  readInSurveyRcds(setup);

  // Retrieve trajectories and tracks from the event
  // -> merely skip if collection is empty
  edm::Handle<TrajTrackAssociationCollection> handleTrajTracksCollection;

  if (event.getByLabel(tjTkAssociationMapTag_, handleTrajTracksCollection)) {
    // Form pairs of trajectories and tracks
    ConstTrajTrackPairs trajTracks;
    for (auto iter  = handleTrajTracksCollection->begin();
              iter != handleTrajTracksCollection->end();
            ++iter) {
      trajTracks.push_back(ConstTrajTrackPair(&(*(*iter).key), &(*(*iter).val)));
    }

    //check that the input tag is not empty
    const AliClusterValueMap* clusterValueMapPtr = 0;
    if (clusterValueMapTag_.encode().size()) {
      edm::Handle<AliClusterValueMap> clusterValueMap;
      event.getByLabel(clusterValueMapTag_, clusterValueMap);
      clusterValueMapPtr = &(*clusterValueMap);
    }

    const EventInfo eventInfo(event.id(),
                              trajTracks,
                              *theBeamSpot,
                              clusterValueMapPtr);

    // Run the alignment algorithm with its input
    theAlignmentAlgo->run(setup, eventInfo);

    for (auto monitor  = theMonitors.begin();
              monitor != theMonitors.end();
            ++monitor) {
      (*monitor)->duringLoop(event, setup, trajTracks); // forward eventInfo?
    }

  } else {
    edm::LogError("Alignment") << "@SUB=PCLTrackerAlProducer::analyze"
                               << "No track collection found: skipping event";
  }
}



/******************************************************************************
 *** PRIVATE METHOD IMPLEMENTATION                                          ***
 ******************************************************************************/

/*** Code which is independent of Event & Setup
     Called from constructor ***/

//_____________________________________________________________________________
void PCLTrackerAlProducer
::createAlignmentAlgorithm(const edm::ParameterSet& config)
{
  edm::ParameterSet algoConfig    = config.getParameter<edm::ParameterSet>("algoConfig");
  edm::VParameterSet iovSelection = config.getParameter<edm::VParameterSet>("RunRangeSelection");
  algoConfig.addUntrackedParameter<edm::VParameterSet>("RunRangeSelection", iovSelection);

  std::string algoName = algoConfig.getParameter<std::string>("algoName");
  theAlignmentAlgo = AlignmentAlgorithmPluginFactory::get()->create(algoName, algoConfig);

  if (!theAlignmentAlgo) {
    throw cms::Exception("BadConfig")
      << "Couldn't find the called alignment algorithm: " << algoName;
  }
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::createMonitors(const edm::ParameterSet& config)
{
  edm::ParameterSet monitorConfig   = config.getParameter<edm::ParameterSet>("monitorConfig");
  std::vector<std::string> monitors = monitorConfig.getUntrackedParameter<std::vector<std::string>>("monitors");

  for (auto miter  = monitors.begin();
            miter != monitors.end();
          ++miter) {
    AlignmentMonitorBase* newMonitor = AlignmentMonitorPluginFactory::get()->create(
      *miter, monitorConfig.getUntrackedParameter<edm::ParameterSet>(*miter)
    );

    if (!newMonitor) {
      throw cms::Exception("BadConfig") << "Couldn't find monitor named "
                                        << *miter;
    }

    theMonitors.push_back(newMonitor);
  }
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::createCalibrations(const edm::ParameterSet& config)
{
  edm::VParameterSet calibrations = config.getParameter<edm::VParameterSet>("calibrations");

  for (auto iCalib  = calibrations.begin();
            iCalib != calibrations.end();
          ++iCalib) {
    theCalibrations.push_back(
      IntegratedCalibrationPluginFactory::get()->create(
        iCalib->getParameter<std::string>("calibrationName"), *iCalib
      )
    );
  }

  // Not all algorithms support calibrations - so do not pass empty vector
  // and throw if non-empty and not supported:
  if (!theCalibrations.empty()) {
    if (theAlignmentAlgo->supportsCalibrations()) {
      theAlignmentAlgo->addCalibrations(theCalibrations);

    } else {
      throw cms::Exception("BadConfig")
        << "[TrackerAlignmentProducerForPCL::init]\n"
        << "Configured " << theCalibrations.size() << " calibration(s) "
        << "for algorithm not supporting it.";
    }
  }
}



/*** Code which is dependent of Event & Setup
     Called and checked for each Event ***/

//_____________________________________________________________________________
bool PCLTrackerAlProducer
::setupChanged(const edm::EventSetup& setup)
{
  bool changed = false;

  if (doTracker_) {
    if (watchTrackerAlRcd.check(setup)) {
        PRINT_INFO(CLASS_NAME, FUNC_NAME, "TrackerAlignmentRcd has changed");
        changed = true;
      }

      if (watchTrackerAlErrorExtRcd.check(setup)) {
        PRINT_INFO(CLASS_NAME, FUNC_NAME, "TrackerAlignmentErrorExtendedRcd has changed");
        changed = true;
      }

      if (watchTrackerSurDeRcd.check(setup)) {
        PRINT_INFO(CLASS_NAME, FUNC_NAME, "TrackerSurfaceDeformationRcd has changed");
        changed = true;
      }
  }

  if (doMuon_) {
    if (watchDTAlRcd.check(setup)) {
      PRINT_INFO(CLASS_NAME, FUNC_NAME, "DTAlignmentRcd has changed");
      changed = true;
    }

    if (watchDTAlErrExtRcd.check(setup)) {
      PRINT_INFO(CLASS_NAME, FUNC_NAME, "DTAlignmentErrorExtendedRcd has changed");
      changed = true;
    }

    if (watchCSCAlRcd.check(setup)) {
      PRINT_INFO(CLASS_NAME, FUNC_NAME, "CSCAlignmentRcd has changed");
      changed = true;
    }

    if (watchCSCAlErrExtRcd.check(setup)) {
      PRINT_INFO(CLASS_NAME, FUNC_NAME, "CSCAlignmentErrorExtendedRcd has changed");
      changed = true;
    }
  }

  /* TODO: ExtraAlignables: Which record(s) to check?
   *
  if (useExtras_) {}
  */

  return changed;
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::initAlignmentAlgorithm(const edm::EventSetup& setup)
{
  PRINT_INFO(CLASS_NAME, FUNC_NAME, "called");

  // Retrieve tracker topology from geometry
  edm::ESHandle<TrackerTopology> tTopoHandle;
  setup.get<IdealGeometryRecord>().get(tTopoHandle);
  const TrackerTopology* const tTopo = tTopoHandle.product();

  // Create the geometries from the ideal geometries (first time only)
  //std::shared_ptr<TrackingGeometry> theTrackerGeometry;
  createGeometries(setup);

  applyAlignmentsToDB(setup);
  createAlignables(tTopo);
  buildParameterStore();
  applyMisalignment();

  // Initialize alignment algorithm and integrated calibration and pass the latter to algorithm
  theAlignmentAlgo->initialize(setup,
                               theTrackerAlignables,
                               theMuonAlignables,
                               theExtraAlignables,
                               theAlignmentParameterStore);

  applyAlignmentsToGeometry();
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::initBeamSpot(const edm::Event& event)
{
  event.getByLabel(beamSpotTag_, theBeamSpot);

  if (theExtraAlignables) {
    edm::LogInfo("Alignment") << "@SUB=TrackerAlignmentProducerForPCL::initBeamSpot"
                              << "Initializing AlignableBeamSpot";

    theExtraAlignables->initializeBeamSpot(theBeamSpot->x0(),
                                           theBeamSpot->y0(),
                                           theBeamSpot->z0(),
                                           theBeamSpot->dxdz(),
                                           theBeamSpot->dydz());
  }
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::createGeometries(const edm::EventSetup& setup)
{
  if (doTracker_) {
    edm::ESHandle<GeometricDet> geometricDet;
    setup.get<IdealGeometryRecord>().get(geometricDet);

    TrackerGeomBuilderFromGeometricDet trackerBuilder;
    theTrackerGeometry = boost::shared_ptr<TrackerGeometry>(
      trackerBuilder.build(&(*geometricDet), theParameterSet)
    );
  }

  if (doMuon_) {
    edm::ESTransientHandle<DDCompactView> cpv;
    edm::ESHandle<MuonDDDConstants> mdc;

    setup.get<IdealGeometryRecord>().get(cpv);
    setup.get<MuonNumberingRecord>().get(mdc);

    theMuonDTGeometry  = boost::shared_ptr<DTGeometry> (new DTGeometry);
    theMuonCSCGeometry = boost::shared_ptr<CSCGeometry>(new CSCGeometry);

    DTGeometryBuilderFromDDD  DTGeometryBuilder;
    CSCGeometryBuilderFromDDD CSCGeometryBuilder;
    DTGeometryBuilder.build (theMuonDTGeometry,  &(*cpv), *mdc);
    CSCGeometryBuilder.build(theMuonCSCGeometry, &(*cpv), *mdc);
  }
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::applyAlignmentsToDB(const edm::EventSetup& setup)
{
  // Retrieve and apply alignments, if requested (requires z setup)
  if (applyDbAlignment_) {
    // we need GlobalPositionRcd - and have to keep track for later removal
    // before writing again to DB...
    edm::ESHandle<Alignments> globalPositionRcd;
    setup.get<GlobalPositionRcd>().get(globalPositionRcd);
    globalPositions_ = new Alignments(*globalPositionRcd);

    if (doTracker_) {
      applyDB<TrackerGeometry,
              TrackerAlignmentRcd,
              TrackerAlignmentErrorExtendedRcd>(
        &(*theTrackerGeometry),
        setup,
        align::DetectorGlobalPosition(*globalPositions_, DetId(DetId::Tracker))
      );

      applyDB<TrackerGeometry,
              TrackerSurfaceDeformationRcd>(
          &(*theTrackerGeometry),
          setup
      );
    }

    if (doMuon_) {
      applyDB<DTGeometry,
              DTAlignmentRcd,
              DTAlignmentErrorExtendedRcd> (
        &(*theMuonDTGeometry),
        setup,
        align::DetectorGlobalPosition(*globalPositions_, DetId(DetId::Muon))
      );

      applyDB<CSCGeometry,
              CSCAlignmentRcd,
              CSCAlignmentErrorExtendedRcd> (
        &(*theMuonCSCGeometry),
        setup,
        align::DetectorGlobalPosition(*globalPositions_, DetId(DetId::Muon))
      );
    }
  }
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::createAlignables(const TrackerTopology* const tTopo)
{
  if (doTracker_) {
    theTrackerAlignables = new AlignableTracker(&(*theTrackerGeometry), tTopo);
  }

  if (doMuon_) {
     theMuonAlignables = new AlignableMuon(&(*theMuonDTGeometry), &(*theMuonCSCGeometry));
  }

  if (useExtras_) {
    theExtraAlignables = new AlignableExtras();
  }
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::buildParameterStore()
{
  // Create alignment parameter builder
  edm::LogInfo("Alignment") << "@SUB=PCLTrackerAlProducer::buildParameterStore"
                            << "Creating AlignmentParameterBuilder";

  edm::ParameterSet alParamBuildCfg = theParameterSet.getParameter<edm::ParameterSet>("ParameterBuilder");
  edm::ParameterSet alParamStoreCfg = theParameterSet.getParameter<edm::ParameterSet>("ParameterStore");

  AlignmentParameterBuilder alignmentParameterBuilder(theTrackerAlignables,
                                                      theMuonAlignables,
                                                      theExtraAlignables,
                                                      alParamBuildCfg);

  // Fix alignables if requested
  if (stNFixAlignables_ > 0) {
    alignmentParameterBuilder.fixAlignables(stNFixAlignables_);
  }

  // Get list of alignables
  Alignables theAlignables = alignmentParameterBuilder.alignables();
  edm::LogInfo("Alignment") << "@SUB=PCLTrackerAlProducer::buildParameterStore"
                            << "got " << theAlignables.size() << " alignables";

  // Create AlignmentParameterStore
  theAlignmentParameterStore = new AlignmentParameterStore(theAlignables, alParamStoreCfg);
  edm::LogInfo("Alignment") << "@SUB=PCLTrackerAlProducer::buildParameterStore"
                            << "AlignmentParameterStore created!";
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::applyMisalignment()
{
  // Apply misalignment scenario to alignable tracker and muon if requested
  // WARNING: this assumes scenarioConfig can be passed to both muon and tracker

  if (doMisalignmentScenario_ && (doTracker_ || doMuon_)) {
    edm::LogInfo("Alignment") << "@SUB=PCLTrackerAlProducer::beginOfJob"
                              << "Applying misalignment scenario to "
                              << (doTracker_ ? "tracker" : "")
                              << (doMuon_    ? (doTracker_ ? " and muon" : "muon") : ".");
    edm::ParameterSet scenarioConfig = theParameterSet.getParameter<edm::ParameterSet>("MisalignmentScenario");

    if (doTracker_) {
      TrackerScenarioBuilder scenarioBuilder(theTrackerAlignables);
      scenarioBuilder.applyScenario(scenarioConfig);
    }
    if (doMuon_) {
      MuonScenarioBuilder muonScenarioBuilder(theMuonAlignables);
      muonScenarioBuilder.applyScenario(scenarioConfig);
    }

  } else {
    edm::LogInfo("Alignment") << "@SUB=TrackerAlignmentProducerForPCL::beginOfJob"
                              << "NOT applying misalignment scenario!";
  }

  // Apply simple misalignment
  const std::string sParSel(theParameterSet.getParameter<std::string>("parameterSelectorSimple"));
  //simpleMisalignment(theAlignables, sParSel, stRandomShift_, stRandomRotation_, true);
  simpleMisalignment(theAlignmentParameterStore->alignables(), sParSel, stRandomShift_, stRandomRotation_, true);
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::simpleMisalignment(const Alignables& alivec, const std::string& selection,
                     float shift, float rot, bool local)
{
  std::ostringstream output; // collecting output

  if (shift > 0. || rot > 0.) {
    output << "Adding random flat shift of max size " << shift
           << " and adding random flat rotation of max size " << rot <<" to ";

    std::vector<bool> commSel(0);
    if (selection != "-1") {
      AlignmentParameterSelector aSelector(0,0); // no alignable needed here...
      const std::vector<char> cSel(aSelector.convertParamSel(selection));

      if (cSel.size() < RigidBodyAlignmentParameters::N_PARAM) {
        throw cms::Exception("BadConfig")
          << "[PCLTrackerAlProducer::simpleMisalignment_]\n"
          << "Expect selection string '" << selection << "' to be at least of length "
          << RigidBodyAlignmentParameters::N_PARAM << " or to be '-1'.\n"
          << "(Most probably you have to adjust the parameter 'parameterSelectorSimple'.)";
      }

      for (auto cIter  = cSel.begin();
                cIter != cSel.end();
              ++cIter) {
        commSel.push_back(*cIter == '0' ? false : true);
      }
      output << "parameters defined by (" << selection
             << "), representing (x,y,z,alpha,beta,gamma),";

    } else {
      output << "the active parameters of each alignable,";
    }
    output << " in " << (local ? "local" : "global") << " frame.";

    for (auto it  = alivec.begin();
              it != alivec.end();
            ++it) {
      Alignable* ali = (*it);
      std::vector<bool> mysel(commSel.empty() ? ali->alignmentParameters()->selector() : commSel);

      if (std::abs(shift)>0.00001) {
        double s0 = 0., s1 = 0., s2 = 0.;

        if (mysel[RigidBodyAlignmentParameters::dx]) s0 = shift * double(random()%1000-500)/500.;
        if (mysel[RigidBodyAlignmentParameters::dy]) s1 = shift * double(random()%1000-500)/500.;
        if (mysel[RigidBodyAlignmentParameters::dz]) s2 = shift * double(random()%1000-500)/500.;

        if (local) ali->move( ali->surface().toGlobal(align::LocalVector(s0,s1,s2)) );
        else       ali->move( align::GlobalVector(s0,s1,s2) );

        //AlignmentPositionError ape(dx,dy,dz);
        //ali->addAlignmentPositionError(ape);
      }

      if (std::abs(rot)>0.00001) {
        align::EulerAngles r(3);
        if (mysel[RigidBodyAlignmentParameters::dalpha]) r(1)=rot*double(random()%1000-500)/500.;
        if (mysel[RigidBodyAlignmentParameters::dbeta])  r(2)=rot*double(random()%1000-500)/500.;
        if (mysel[RigidBodyAlignmentParameters::dgamma]) r(3)=rot*double(random()%1000-500)/500.;

        const align::RotationType mrot = align::toMatrix(r);
        if (local) ali->rotateInLocalFrame(mrot);
        else       ali->rotateInGlobalFrame(mrot);

        //ali->addAlignmentPositionErrorFromRotation(mrot);
      }
    }

  } else {
    output << "No simple misalignment added!";
  }

  edm::LogInfo("Alignment")  << "@SUB=PCLTrackerAlProducer::simpleMisalignment_" << output.str();
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::applyAlignmentsToGeometry()
{
  edm::LogInfo("Alignment") << "@SUB=PCLTrackerAlProducer::startingNewLoop"
                            << "Now physically apply alignments to  geometry...";

  // Propagate changes to reconstruction geometry (from initialisation or iteration)
  GeometryAligner aligner;

  if (doTracker_) {
    std::auto_ptr<Alignments>                   alignments(     theTrackerAlignables->alignments());
    std::auto_ptr<AlignmentErrorsExtended>      alignmentErrExt(theTrackerAlignables->alignmentErrors());
    std::auto_ptr<AlignmentSurfaceDeformations> aliDeforms(     theTrackerAlignables->surfaceDeformations());

    aligner.applyAlignments<TrackerGeometry>(
      &(*theTrackerGeometry),
      &(*alignments),
      &(*alignmentErrExt),
      AlignTransform()
    ); // don't apply global a second time!

    aligner.attachSurfaceDeformations<TrackerGeometry>(
      &(*theTrackerGeometry),
      &(*aliDeforms)
    );
  }

  if (doMuon_) {
    std::auto_ptr<Alignments> dtAlignments( theMuonAlignables->dtAlignments());
    std::auto_ptr<Alignments> cscAlignments(theMuonAlignables->cscAlignments());

    std::auto_ptr<AlignmentErrorsExtended> dtAlignmentErrExt(
      theMuonAlignables->dtAlignmentErrorsExtended()
    );
    std::auto_ptr<AlignmentErrorsExtended> cscAlignmentErrExt(
      theMuonAlignables->cscAlignmentErrorsExtended()
    );

    aligner.applyAlignments<DTGeometry>(
      &(*theMuonDTGeometry),
      &(*dtAlignments),
      &(*dtAlignmentErrExt),
      AlignTransform()
    ); // don't apply global a second time!

    aligner.applyAlignments<CSCGeometry>(
      &(*theMuonCSCGeometry),
      &(*cscAlignments),
      &(*cscAlignmentErrExt),
      AlignTransform()
    ); // nope!
  }
}

//_____________________________________________________________________________
template<class G, class Rcd, class ErrRcd>
void PCLTrackerAlProducer
::applyDB(G* geometry, const edm::EventSetup& setup,
          const AlignTransform& globalCoordinates) const
{
  // 'G' is the geometry class for that DB should be applied,
  // 'Rcd' is the record class for its Alignments
  // 'ErrRcd' is the record class for its AlignmentErrorsExtended
  // 'globalCoordinates' are global transformation for this geometry

  const Rcd& record = setup.get<Rcd>();
  if (checkDbAlignmentValidity_) {
    const edm::ValidityInterval & validity = record.validityInterval();
    const edm::IOVSyncValue first = validity.first();
    const edm::IOVSyncValue last = validity.last();

    if (first != edm::IOVSyncValue::beginOfTime() ||
        last  != edm::IOVSyncValue::endOfTime()) {
      throw cms::Exception("DatabaseError")
        << "@SUB=PCLTrackerAlProducer::applyDB"
        << "\nTrying to apply " << record.key().name()
        << " with multiple IOVs in tag.\nValidity range is "
        << first.eventID().run() << " - " << last.eventID().run();
    }
  }

  edm::ESHandle<Alignments> alignments;
  record.get(alignments);

  edm::ESHandle<AlignmentErrorsExtended> alignmentErrExt;
  setup.get<ErrRcd>().get(alignmentErrExt);

  GeometryAligner aligner;
  aligner.applyAlignments<G>(geometry, &(*alignments), &(*alignmentErrExt),
                             globalCoordinates);
}

//_____________________________________________________________________________
template<class G, class DeformationRcd>
void PCLTrackerAlProducer
::applyDB(G* geometry, const edm::EventSetup& setup) const
{
  // 'G' is the geometry class for that DB should be applied,
  // 'DeformationRcd' is the record class for its surface deformations

  const DeformationRcd & record = setup.get<DeformationRcd>();
  if (checkDbAlignmentValidity_) {
    const edm::ValidityInterval & validity = record.validityInterval();
    const edm::IOVSyncValue first = validity.first();
    const edm::IOVSyncValue last = validity.last();

    if (first != edm::IOVSyncValue::beginOfTime() ||
        last  != edm::IOVSyncValue::endOfTime()) {
      throw cms::Exception("DatabaseError")
        << "@SUB=PCLTrackerAlProducer::applyDB"
        << "\nTrying to apply " << record.key().name()
        << " with multiple IOVs in tag.\nValidity range is "
        << first.eventID().run() << " - " << last.eventID().run();
    }
  }
  edm::ESHandle<AlignmentSurfaceDeformations> surfaceDeformations;
  record.get(surfaceDeformations);

  GeometryAligner aligner;
  aligner.attachSurfaceDeformations<G>(geometry, &(*surfaceDeformations));
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::readInSurveyRcds(const edm::EventSetup& setup)
{
/*
  // Get Survey Rcds and add Survey Info
  if (doTracker_ && useSurvey_) {
    bool tkSurveyBool    = watchTkSurveyRcd.check(setup);
    bool tkSurveyErrBool = watchTkSurveyErrExtRcd.check(setup);

    if (tkSurveyBool || tkSurveyErrBool) {
      edm::ESHandle<Alignments>   surveys;
      edm::ESHandle<SurveyErrors> surveyErrors;
      setup.get<TrackerSurveyRcd>().             get(surveys);
      setup.get<TrackerSurveyErrorExtendedRcd>().get(surveyErrors);

      theSurveyIndex  = 0;
      theSurveyValues = &(*surveys);
      theSurveyErrors = &(*surveyErrors);

      addSurveyInfo(theTrackerAlignables);
    }
  }

  if (doMuon_ && useSurvey_) {
    bool DTSurveyBool     = watchTkSurveyRcd.check(setup);
    bool DTSurveyErrBool  = watchTkSurveyErrExtRcd.check(setup);
    bool CSCSurveyBool    = watchTkSurveyRcd.check(setup);
    bool CSCSurveyErrBool = watchTkSurveyErrExtRcd.check(setup);

    if (DTSurveyBool || DTSurveyErrBool || CSCSurveyBool || CSCSurveyErrBool){
      edm::ESHandle<Alignments>   dtSurveys;
      edm::ESHandle<SurveyError>  dtSurveyErrors;
      edm::ESHandle<Alignments>   cscSurveys;
      edm::ESHandle<SurveyErrors> cscSurveyErrors;
      setup.get<DTSurveyRcd>().              get(dtSurveys);
      setup.get<DTSurveyErrorExtendedRcd>(). get(dtSurveyErrors);
      setup.get<CSCSurveyRcd>().             get(cscSurveys);
      setup.get<CSCSurveyErrorExtendedRcd>().get(cscSurveyErrors);

      theSurveyIndex  = 0;
      theSurveyValues = &(*dtSurveys);
      theSurveyErrors = &(*dtSurveyErrors);

      Alignables barrels = theMuonAlignables->DTBarrel();
      for (auto iter  = barrels.begin();
                iter != barrels.end();
              ++iter) {
        addSurveyInfo(*iter);
      }

      theSurveyIndex  = 0;
      theSurveyValues = &(*cscSurveys);
      theSurveyErrors = &(*cscSurveyErrors);

      Alignables endcaps = theMuonAlignables->CSCEndcaps();
      for (auto iter  = endcaps.begin();
                iter != endcaps.end();
              ++iter) {
        addSurveyInfo(*iter);
      }
    }
  }
*/
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::addSurveyInfo(Alignable* alignable)
{
  edm::LogInfo("Alignment") << "ADDING THE SURVEY INFORMATION";

  const std::vector<Alignable*>& comp = alignable->components();
  for (size_t i = 0; i < comp.size(); ++i) {
    addSurveyInfo(comp[i]);
  }

  const SurveyError& error = theSurveyErrors->m_surveyErrors[theSurveyIndex];

  if (alignable->id()                != error.rawId() ||
      alignable->alignableObjectId() != error.structureType()) {
    throw cms::Exception("DatabaseError")
      << "Error reading survey info from DB. Mismatched id!";
  }

  const CLHEP::Hep3Vector&  pos = theSurveyValues->m_align[theSurveyIndex].translation();
  const CLHEP::HepRotation& rot = theSurveyValues->m_align[theSurveyIndex].rotation();

  AlignableSurface surf(align::PositionType(pos.x(),  pos.y(),  pos.z()),
                        align::RotationType(rot.xx(), rot.xy(), rot.xz(),
                                            rot.yx(), rot.yy(), rot.yz(),
                                            rot.zx(), rot.zy(), rot.zz()));
  surf.setWidth (alignable->surface().width());
  surf.setLength(alignable->surface().length());

  alignable->setSurvey(new SurveyDet(surf, error.matrix()));

  ++theSurveyIndex;
}

/* TODO: One method for applying alignments to a given geometry?
 *
template<typename Geometry>
void PCLTrackerAlProducer::applyAlignmentsToGeometry2(Geometry* geometry,
                                                     Alignments* al,
                                                     AlignmentErrors* alErrs,
                                                     AlignTransform& globalCoordinates) {
  GeometryAligner aligner;

  aligner.applyAlignments<Geometry>(
      geometry,
      al,
      alErrs,
      globalCoordinates
  );
}
*/



/*** Code for writing results to database
     Called from endJob() ***/

//_____________________________________________________________________________
void PCLTrackerAlProducer
::finish()
{
  PRINT_INFO(CLASS_NAME, FUNC_NAME, "called");

  /* 1) Former: Status AlignmentProducer::endOfLoop(const edm::EventSetup& iSetup, unsigned int iLoop) */
  if (theAlignmentAlgo->processesEvents() && nevent_ == 0) {
    // beginOfJob is usually called by the framework in the first event of the first loop
    // (a hack: beginOfJob needs the EventSetup that is not well defined without an event)
    // and the algorithms rely on the initialisations done in beginOfJob. We cannot call
    // this->beginOfJob(iSetup); here either since that will access the EventSetup to get
    // some geometry information that is not defined either without having seen an event.
    edm::LogError("Alignment") << "@SUB=PCLTrackerAlProducer::finish"
                             << "Did not process any events, stop "
                             << "without terminating algorithm.";
    return;
  }

  /* !!! TODO: HACK for MillePede !!!
     Because the pede-part of MillePede needs at least 1 Event for initializing
     the geometry */
  if (nevent_ == 0) return;

  PRINT_INFO(CLASS_NAME, FUNC_NAME, "terminating algorithm");
  edm::LogInfo("Alignment") << "@SUB=PCLTrackerAlProducer::finish"
                            << "Terminating algorithm.";
  theAlignmentAlgo->terminate();

  /* 2) Former: void AlignmentProducer::endOfJob() */
  storeAlignmentsToDB();
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::storeAlignmentsToDB()
{
  if (theAlignmentAlgo->processesEvents() && nevent_ == 0) {
    // TODO: If this is the case, it would be already caught in finish()
    edm::LogError("Alignment") << "@SUB=PCLTrackerAlProducer::endOfJob"
                               << "Did not process any events in last loop, "
                               << "do not dare to store to DB.";
  } else {
    // Expand run ranges and make them unique
    edm::VParameterSet runRangeSelectionVPSet(theParameterSet.getParameter<edm::VParameterSet>("RunRangeSelection"));
    RunRanges uniqueRunRanges(makeNonOverlappingRunRanges(runRangeSelectionVPSet));

    // create dummy IOV
    if (uniqueRunRanges.empty()) {
      const RunRange runRange(cond::timeTypeSpecs[cond::runnumber].beginValue,
                              cond::timeTypeSpecs[cond::runnumber].endValue);
      uniqueRunRanges.push_back(runRange);
    }

    std::vector<AlgebraicVector> beamSpotParameters;

    for (auto iRunRange  = uniqueRunRanges.begin();
              iRunRange != uniqueRunRanges.end();
            ++iRunRange) {

      theAlignmentAlgo->setParametersForRunRange(*iRunRange);

      // Save alignments to database
      if (saveToDB_ || saveApeToDB_ || saveDeformationsToDB_) {
        PRINT_INFO(CLASS_NAME, FUNC_NAME, "Write alignments to db-file");
        writeForRunRange((*iRunRange).first);
      }

      // Deal with extra alignables, e.g. beam spot
      if (theExtraAlignables) {
        Alignables &alis = theExtraAlignables->beamSpot();
        if (!alis.empty()) {
          BeamSpotAlignmentParameters *beamSpotAliPars = dynamic_cast<BeamSpotAlignmentParameters*>(alis[0]->alignmentParameters());
          beamSpotParameters.push_back(beamSpotAliPars->parameters());
        }
      }
    }



    if (theExtraAlignables) {
      std::ostringstream bsOutput;

      auto itPar = beamSpotParameters.begin();
      for (auto iRunRange = uniqueRunRanges.begin();
                iRunRange != uniqueRunRanges.end();
              ++iRunRange, ++itPar) {
        bsOutput << "Run range: " << (*iRunRange).first << " - " << (*iRunRange).second << "\n";
        bsOutput << "  Displacement: x=" << (*itPar)[0] << ", y=" << (*itPar)[1] << "\n";
        bsOutput << "  Slope: dx/dz=" << (*itPar)[2] << ", dy/dz=" << (*itPar)[3] << "\n";
      }

      edm::LogInfo("Alignment") << "@SUB=PCLTrackerAlProducer::endOfJob"
                                << "Parameters for alignable beamspot:\n"
                                << bsOutput.str();
    }
  }
}

//_____________________________________________________________________________
RunRanges PCLTrackerAlProducer
::makeNonOverlappingRunRanges(const edm::VParameterSet& RunRangeSelectionVPSet)
{
  static bool oldRunRangeSelectionWarning = false;

  const RunNumber beginValue = cond::timeTypeSpecs[cond::runnumber].beginValue;
  const RunNumber endValue   = cond::timeTypeSpecs[cond::runnumber].endValue;

  RunRanges uniqueRunRanges;
  if (!RunRangeSelectionVPSet.empty()) {

    std::map<RunNumber,RunNumber> uniqueFirstRunNumbers;

    for (auto ipset  = RunRangeSelectionVPSet.begin();
              ipset != RunRangeSelectionVPSet.end();
            ++ipset) {
      const std::vector<std::string> RunRangeStrings = (*ipset).getParameter<std::vector<std::string> >("RunRanges");

      for (auto irange  = RunRangeStrings.begin();
                irange != RunRangeStrings.end();
              ++irange) {

        if ((*irange).find(':') == std::string::npos) {

          RunNumber first = beginValue;
          long int temp = strtol((*irange).c_str(), 0, 0);
          if (temp!=-1) first = temp;
          uniqueFirstRunNumbers[first] = first;

        } else {
          if (!oldRunRangeSelectionWarning) {
            edm::LogWarning("BadConfig") << "@SUB=PCLTrackerAlProducer::makeNonOverlappingRunRanges"
                         << "Config file contains old format for 'RunRangeSelection'. Only the start run\n"
                         << "number is used internally. The number of the last run is ignored and can be\n"
                         << "safely removed from the config file.\n";
            oldRunRangeSelectionWarning = true;
          }

          std::vector<std::string> tokens = edm::tokenize(*irange, ":");
          long int temp;
          RunNumber first = beginValue;
          temp = strtol(tokens[0].c_str(), 0, 0);
          if (temp!=-1) first = temp;
          uniqueFirstRunNumbers[first] = first;
        }
      }
    }

    for (auto iFirst  = uniqueFirstRunNumbers.begin();
              iFirst != uniqueFirstRunNumbers.end();
            ++iFirst) {
      uniqueRunRanges.push_back(std::pair<RunNumber,RunNumber>((*iFirst).first, endValue));
    }

    for (size_t i = 0; i < uniqueRunRanges.size()-1; ++i) {
      uniqueRunRanges[i].second = uniqueRunRanges[i+1].first - 1;
    }

  } else {
    uniqueRunRanges.push_back(std::pair<RunNumber,RunNumber>(beginValue, endValue));
  }

  return uniqueRunRanges;
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::writeForRunRange(cond::Time_t time)
{
  PRINT_INFO(CLASS_NAME, FUNC_NAME, "called");

  // first tracker
  if (doTracker_) {
    const AlignTransform* trackerGlobal = 0; // will be 'removed' from constants
    if (globalPositions_) { // i.e. applied before in applyDB
      trackerGlobal = &align::DetectorGlobalPosition(*globalPositions_,
                                                     DetId(DetId::Tracker));
    }

    // theTrackerAlignables->alignments calls new
    Alignments*              alignments      = theTrackerAlignables->alignments();
    AlignmentErrorsExtended* alignmentErrExt = theTrackerAlignables->alignmentErrors();

    writeDB(alignments, "TrackerAlignmentRcd",
            alignmentErrExt, "TrackerAlignmentErrorExtendedRcd",
            trackerGlobal,
            time);
  }

  // Save surface deformations to database
  if (doTracker_ && saveDeformationsToDB_) {
    AlignmentSurfaceDeformations* alignmentSurfaceDeformations = theTrackerAlignables->surfaceDeformations();
    writeDB(alignmentSurfaceDeformations, "TrackerSurfaceDeformationRcd", time);
  }

  // now muon
  if (doMuon_) {
    const AlignTransform* muonGlobal = 0; // will be 'removed' from constants
    if (globalPositions_) { // i.e. applied before in applyDB
      muonGlobal = &align::DetectorGlobalPosition(*globalPositions_,
                                                  DetId(DetId::Muon));
    }

    // Get alignments+errors, first DT - ownership taken over by writeDB(..), so no delete
    Alignments*              alignments      = theMuonAlignables->dtAlignments();
    AlignmentErrorsExtended* alignmentErrExt = theMuonAlignables->dtAlignmentErrorsExtended();

    writeDB(alignments, "DTAlignmentRcd",
            alignmentErrExt, "DTAlignmentErrorExtendedRcd",
            muonGlobal,
            time);

    // Get alignments+errors, now CSC - ownership taken over by writeDB(..), so no delete
    alignments      = theMuonAlignables->cscAlignments();
    alignmentErrExt = theMuonAlignables->cscAlignmentErrorsExtended();

    writeDB(alignments, "CSCAlignmentRcd",
            alignmentErrExt, "CSCAlignmentErrorExtendedRcd",
            muonGlobal,
            time);
  }
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::writeDB(Alignments* alignments, const std::string &alignRcd,
          AlignmentErrorsExtended* alignmentErrExt, const std::string &errRcd,
          const AlignTransform *globalCoordinates,
          cond::Time_t time) const
{
  PRINT_INFO(CLASS_NAME, FUNC_NAME, "called");

  Alignments*              tempAlignments      = alignments;
  AlignmentErrorsExtended* tempAlignmentErrExt = alignmentErrExt;

  // Call service
  edm::Service<cond::service::PoolDBOutputService> poolDb;
  if (!poolDb.isAvailable()) {  // Die if not available
    delete tempAlignments;      // promised to take over ownership...
    delete tempAlignmentErrExt; // dito

    throw cms::Exception("NotAvailable") << "PoolDBOutputService not available";
  }

  if (globalCoordinates && // happens only if (applyDbAlignment_ == true)
      globalCoordinates->transform() != AlignTransform::Transform::Identity) {

    tempAlignments      = new Alignments();               // temporary storage for
    tempAlignmentErrExt = new AlignmentErrorsExtended();  // final alignments and errors

    GeometryAligner aligner;
    aligner.removeGlobalTransform(alignments, alignmentErrExt,
                                  *globalCoordinates,
                                  tempAlignments, tempAlignmentErrExt);

    delete alignments;      // have to delete original alignments
    delete alignmentErrExt; // same thing for the errors

    edm::LogInfo("Alignment") << "@SUB=PCLTrackerAlProducer::writeDB"
                              << "globalCoordinates removed from alignments ("
                              << alignRcd << ") and errors (" << alignRcd << ").";
  }

  if (saveToDB_) {
    edm::LogInfo("Alignment") << "Writing Alignments for run " << time
                              << " to " << alignRcd << ".";
    poolDb->writeOne<Alignments>(tempAlignments, time, alignRcd);

  } else {
    // poolDb->writeOne(..) takes over 'alignments' ownership, ...
    delete tempAlignments; // ...otherwise we have to delete, as promised!
  }

  if (saveApeToDB_) {
    edm::LogInfo("Alignment") << "Writing AlignmentErrorsExtended for run "
                              << time << " to " << errRcd << ".";
    poolDb->writeOne<AlignmentErrorsExtended>(tempAlignmentErrExt, time, errRcd);

  } else {
    // poolDb->writeOne(..) takes over 'alignmentErrors' ownership, ...
    delete tempAlignmentErrExt; // ...otherwise we have to delete, as promised!
  }
}

//_____________________________________________________________________________
void PCLTrackerAlProducer
::writeDB(AlignmentSurfaceDeformations* alignmentSurfaceDeformations,
          const std::string &surfaceDeformationRcd,
          cond::Time_t time) const
{
  // Call service
  edm::Service<cond::service::PoolDBOutputService> poolDb;
  if (!poolDb.isAvailable()) { // Die if not available
    delete alignmentSurfaceDeformations; // promised to take over ownership...
    throw cms::Exception("NotAvailable") << "PoolDBOutputService not available";
  }

  if (saveDeformationsToDB_) {
    edm::LogInfo("Alignment") << "Writing AlignmentSurfaceDeformations for run "
                              << time << " to " << surfaceDeformationRcd  << ".";
    poolDb->writeOne<AlignmentSurfaceDeformations>(alignmentSurfaceDeformations, time,
                                                   surfaceDeformationRcd);

  } else {
    // poolDb->writeOne(..) takes over 'surfaceDeformation' ownership,...
    delete alignmentSurfaceDeformations; // ...otherwise we have to delete, as promised!
  }
}



DEFINE_FWK_MODULE(PCLTrackerAlProducer);
