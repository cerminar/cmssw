#ifndef Alignment_MillePedeAlignmentAlgorithm_MillePedeDQMModule_h
#define Alignment_MillePedeAlignmentAlgorithm_MillePedeDQMModule_h

/**
 * @package   Alignment/MillePedeAlignmentAlgorithm
 * @file      MillePedeDQMModule.h
 *
 * @author    Max Stark (max.stark@cern.ch)
 * @date      Oct 26, 2015
 *
 * @brief     DQM Plotter for PCL-Alignment
 */



/*** System includes ***/
#include <memory>
#include <fstream>
#include <string>

/*** Core framework functionality ***/
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"

/*** DQM ***/
#include "DQMServices/Core/interface/DQMEDAnalyzer.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"



class MillePedeDQMModule : public DQMEDAnalyzer {

  //========================== PUBLIC METHODS ==================================
  public: //====================================================================

    // Constructor
    explicit MillePedeDQMModule(const edm::ParameterSet&);
    // Destructor
    ~MillePedeDQMModule();

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

    virtual void bookHistograms(DQMStore::IBooker&, edm::Run const&,
                                edm::EventSetup const&) override;

    virtual void analyze(edm::Event const&, edm::EventSetup const&) {};

    virtual void endRun(edm::Run const&, edm::EventSetup const&) override;

    virtual bool updateDB();



  //========================= PRIVATE METHODS ==================================
  private: //===================================================================

    void readMillePedeLogFile();
    void readMillePedeResultFile();

    void fillExpertHistos();
    void fillExpertHisto(MonitorElement* histos[], float[], float[], float);

    void drawSummary();


  //========================== PRIVATE DATA ====================================
  //============================================================================

    // Config-file parameter
    std::string millePedeLogFile_;
    std::string millePedeResFile_;

    // Histograms
    MonitorElement* h_xPos[4];
    MonitorElement* h_xRot[4];
    MonitorElement* h_yPos[4];
    MonitorElement* h_yRot[4];
    MonitorElement* h_zPos[4];
    MonitorElement* h_zRot[4];


    bool PedeSuccess = false;
    bool Movements   = false;
    bool Error       = false;
    bool Significant = false;
    bool DBupdated   = false;
    bool HitMax      = false;
    bool HitErrorMax = false;

    int Nrec = 0;

    // Signifiance of movement must be above
    float SigCut = 2.5;

    // Cutoff in micro-meter & micro-rad
    float Xcut  =  5.0;
    float tXcut = 30.0; //thetaX
    float Ycut  = 10.0;
    float tYcut = 30.0; //thetaY
    float Zcut  = 15.0;
    float tZcut = 30.0; //thetaZ
    float Cutoffs[6] = {Xcut, Ycut, Zcut, tXcut, tYcut, tZcut};

    // maximum movement in micro-meter/rad
    float MaxMoveCut  = 200;
    float MaxErrorCut = 10;

    // Order same as "Bins" List
    float Xobs[6]     = {0.,0.,0.,0.,0.,0.};
    float XobsErr[6]  = {0.,0.,0.,0.,0.,0.};
    float tXobs[6]    = {0.,0.,0.,0.,0.,0.};
    float tXobsErr[6] = {0.,0.,0.,0.,0.,0.};
    float Yobs[6]     = {0.,0.,0.,0.,0.,0.};
    float YobsErr[6]  = {0.,0.,0.,0.,0.,0.};
    float tYobs[6]    = {0.,0.,0.,0.,0.,0.};
    float tYobsErr[6] = {0.,0.,0.,0.,0.,0.};
    float Zobs[6]     = {0.,0.,0.,0.,0.,0.};
    float ZobsErr[6]  = {0.,0.,0.,0.,0.,0.};
    float tZobs[6]    = {0.,0.,0.,0.,0.,0.};
    float tZobsErr[6] = {0.,0.,0.,0.,0.,0.};

    std::vector<const char*> Bins {
      "FPIX(x+,z-)",
      "FPIX(x-,z-)",
      "BPIX(x+)",
      "BPIX(x-)",
      "FPIX(x+,z+)",
      "FPIX(x-,z+)"
    };
};

// define this as a plug-in
DEFINE_FWK_MODULE(MillePedeDQMModule);


#endif
