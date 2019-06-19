#ifndef FPGAHISTIMP_H
#define FPGAHISTIMP_H

#include <TFile.h>
#include <TH1F.h>
#include <TEfficiency.h>

#include "FPGAHistBase.hh"

using namespace std;

class FPGAHistImp:public FPGAHistBase{

public:

  FPGAHistImp() {
    h_file_ = 0;
    h_layerresid_phi_L3_L1L2_=0;
    h_layerresid_phi_L3_L1L2_match_=0;
    h_layerresid_phif_L3_L1L2_=0;
    h_layerresid_phif_L3_L1L2_match_=0;
    h_layerresid_z_L3_L1L2_=0;
    h_layerresid_z_L3_L1L2_match_=0;
    h_layerresid_zf_L3_L1L2_=0;
    h_layerresid_zf_L3_L1L2_match_=0;

    h_diskresid_phi_D1_L1L2_=0;
    h_diskresid_phi_D1_L1L2_match_=0;
    h_diskresid_phif_D1_L1L2_=0;
    h_diskresid_phif_D1_L1L2_match_=0;
    h_diskresid_r_D1_L1L2_=0;
    h_diskresid_r_D1_L1L2_match_=0;
    h_diskresid_rf_D1_L1L2_=0;
    h_diskresid_rf_D1_L1L2_match_=0;

    h_rinv_L1L2_=0;
    h_irinv_L1L2_=0;
    h_rinvres_L1L2_=0;
    h_irinvres_L1L2_=0;
    
  }

  virtual ~FPGAHistImp() {}

  void init() {
    h_file_ = new TFile("fpgahist.root","RECREATE");
  }

  void close() {
    if(h_file_) {
      //h_file_->ls();
      h_file_->Write();
      h_file_->Close();
      h_file_ = 0;
    }
  }
  
  void bookLayerResidual(){

    TH1::AddDirectory(kTRUE); 

    assert(h_file_!=0);
    h_file_->cd();
    
    h_layerresid_phi_L3_L1L2_ = new TH1F("L3 phiresid L1L2","L3 phiresid L1L2",100,-0.5,0.5);
    h_layerresid_phi_L3_L1L2_match_ = new TH1F("L3 phiresid L1L2 Match","L3 phiresid L1L2 Match",100,-0.5,0.5);
    h_layerresid_phif_L3_L1L2_ = new TH1F("L3 phiresid float L1L2","L3 phiresid float L1L2",100,-0.5,0.5);
    h_layerresid_phif_L3_L1L2_match_ = new TH1F("L3 phiresid float L1L2 Match","L3 phiresid float L1L2 Match",100,-0.5,0.5);

    h_layerresid_z_L3_L1L2_ = new TH1F("L3 zresid L1L2","L3 zresid L1L2",100,-5.0,5.0);
    h_layerresid_z_L3_L1L2_match_ = new TH1F("L3 zresid L1L2 Match","L3 zresid L1L2 Match",100,-5.0,5.0);
    h_layerresid_zf_L3_L1L2_ = new TH1F("L3 zresid float L1L2","L3 zresid float L1L2",100,-5.0,5.0);
    h_layerresid_zf_L3_L1L2_match_ = new TH1F("L3 zresid float L1L2 Match","L3 zresid float L1L2 Match",100,-5.0,5.0);

  };

  void bookDiskResidual(){

    TH1::AddDirectory(kTRUE); 

    assert(h_file_!=0);
    h_file_->cd();

    h_diskresid_phi_D1_L1L2_ = new TH1F("D1 phiresid L1L2","D1 phiresid L1L2",100,-0.5,0.5);
    h_diskresid_phi_D1_L1L2_match_ = new TH1F("D1 phiresid L1L2 Match","D1 phiresid L1L2 Match",100,-0.5,0.5);
    h_diskresid_phif_D1_L1L2_ = new TH1F("D1 phiresid float L1L2","D1 phiresid float L1L2",100,-0.5,0.5);
    h_diskresid_phif_D1_L1L2_match_ = new TH1F("D1 phiresid float L1L2 Match","D1 phiresid float L1L2 Match",100,-0.5,0.5);

    h_diskresid_r_D1_L1L2_ = new TH1F("D1 rresid L1L2","D1 rresid L1L2",100,-5.0,5.0);
    h_diskresid_r_D1_L1L2_match_ = new TH1F("D1 rresid L1L2 Match","D1 rresid L1L2 Match",100,-5.0,5.0);
    h_diskresid_rf_D1_L1L2_ = new TH1F("D1 rresid float L1L2","D1 rresid float L1L2",100,-5.0,5.0);
    h_diskresid_rf_D1_L1L2_match_ = new TH1F("D1 rresid float L1L2 Match","D1 rresid float L1L2 Match",100,-5.0,5.0);

  };


  void bookTrackletParams(){

    TH1::AddDirectory(kTRUE); 

    assert(h_file_!=0);
    h_file_->cd();

    h_rinv_L1L2_ = new TH1F("Tracklet rinv in L1L2","Tracklet rinv in L1L2",140,-0.007,0.007);
    h_irinv_L1L2_ = new TH1F("Tracklet irinv in L1L2","Tracklet irinv in L1L2",140,-0.007,0.007);
    h_rinv_matched_L1L2_ = new TH1F("Tracklet rinv in matched L1L2","Tracklet rinv in matched L1L2",140,-0.007,0.007);
    h_irinv_matched_L1L2_ = new TH1F("Tracklet irinv in matched L1L2","Tracklet irinv in matched L1L2",140,-0.007,0.007);
    h_rinvres_L1L2_= new TH1F("Tracklet rinv res in L1L2","Tracklet rinv res in L1L2",100,-0.0005,0.0005);
    h_irinvres_L1L2_ = new TH1F("Tracklet irinv res in L1L2","Tracklet irinv res in L1L2",100,-0.0005,0.0005);

    h_phi0_L1L2_ = new TH1F("Tracklet phi0 in L1L2","Tracklet phi0 in L1L2",100,0.0,1.0);
    h_iphi0_L1L2_ = new TH1F("Tracklet iphi0 in L1L2","Tracklet iphi0 in L1L2",100,0.0,1.0);
    h_phi0_matched_L1L2_ = new TH1F("Tracklet phi0 in matched L1L2","Tracklet phi0 in matched L1L2",100,0.0,1.0);
    h_iphi0_matched_L1L2_ = new TH1F("Tracklet iphi0 in matched L1L2","Tracklet iphi0 in matched L1L2",100,0.0,1.0);
    h_phi0global_L1L2_ = new TH1F("Tracklet phi0 global in L1L2","Tracklet phi0 global in L1L2",99,-M_PI,M_PI);
    h_iphi0global_L1L2_ = new TH1F("Tracklet iphi0 global in L1L2","Tracklet iphi0 global in L1L2",99,-M_PI,M_PI);
    h_phi0global_matched_L1L2_ = new TH1F("Tracklet phi0 global in matched L1L2","Tracklet phi0 global in matched L1L2",99,-M_PI,M_PI);
    h_iphi0global_matched_L1L2_ = new TH1F("Tracklet iphi0 global in matched L1L2","Tracklet iphi0 global in matched L1L2",99,-M_PI,M_PI);
    h_phi0res_L1L2_= new TH1F("Tracklet phi0 res in L1L2","Tracklet phi0 res in L1L2",100,-0.5,0.5);
    h_iphi0res_L1L2_ = new TH1F("Tracklet iphi0 res in L1L2","Tracklet iphi0 res in L1L2",100,-0.5,0.5);

    h_eta_L1L2_ = new TH1F("Tracklet eta in L1L2","Tracklet eta in L1L2",100,-2.5,2.5);
    h_ieta_L1L2_ = new TH1F("Tracklet ieta in L1L2","Tracklet ieta in L1L2",100,-2.5,2.5);
    h_eta_matched_L1L2_ = new TH1F("Tracklet eta in matched L1L2","Tracklet eta in matched L1L2",100,-2.5,2.5);
    h_ieta_matched_L1L2_ = new TH1F("Tracklet ieta in matched L1L2","Tracklet ieta in matched L1L2",100,-2.5,2.5);
    h_etares_L1L2_= new TH1F("Tracklet eta res in L1L2","Tracklet eta res in L1L2",100,-0.05,0.05);
    h_ietares_L1L2_ = new TH1F("Tracklet ieta res in L1L2","Tracklet ieta res in L1L2",100,-0.05,0.05);

    h_z0_L1L2_ = new TH1F("Tracklet z0 in L1L2","Tracklet z0 in L1L2",100,-25.0,25.0);
    h_iz0_L1L2_ = new TH1F("Tracklet iz0 in L1L2","Tracklet iz0 in L1L2",100,-25.0,25.0);
    h_z0_matched_L1L2_ = new TH1F("Tracklet z0 in matched L1L2","Tracklet z0 in matched L1L2",100,-25.0,25.0);
    h_iz0_matched_L1L2_ = new TH1F("Tracklet iz0 in matched L1L2","Tracklet iz0 in matched L1L2",100,-25.0,25.0);
    h_z0res_L1L2_= new TH1F("Tracklet z0 res in L1L2","Tracklet z0 res in L1L2",100,-5.0,5.0);
    h_iz0res_L1L2_ = new TH1F("Tracklet iz0 res in L1L2","Tracklet iz0 res in L1L2",100,-5.0,5.0);
    
  }
  
  //arguments are
  // int seedIndex
  // double irinv, rinv
  // double iphi0, phi0
  // double ieta, eta
  // double iz0, z0
  // int tp
  virtual void fillTrackletParams(int seedIndex, int iSector,
				  double rinv, double irinv,
				  double phi0, double iphi0,
				  double eta, double ieta,
				  double z0, double iz0,
				  int tp) {

    if (seedIndex==0) {
      h_rinv_L1L2_->Fill(rinv);
      h_irinv_L1L2_->Fill(irinv);
      h_phi0_L1L2_->Fill(phi0);
      h_iphi0_L1L2_->Fill(iphi0);
      double phi0global=phi0+iSector*2*M_PI/NSector;
      if (phi0global>M_PI) phi0global-=2*M_PI;
      if (phi0global<-M_PI) phi0global+=2*M_PI;
      double iphi0global=iphi0+iSector*2*M_PI/NSector;
      if (iphi0global>M_PI) iphi0global-=2*M_PI;
      if (iphi0global<-M_PI) iphi0global+=2*M_PI;
      h_phi0global_L1L2_->Fill(phi0global);
      h_iphi0global_L1L2_->Fill(iphi0global);
      h_eta_L1L2_->Fill(eta);
      h_ieta_L1L2_->Fill(ieta);
      h_z0_L1L2_->Fill(z0);
      h_iz0_L1L2_->Fill(iz0);
      if (tp!=0) {
	h_rinv_matched_L1L2_->Fill(rinv);
	h_irinv_matched_L1L2_->Fill(irinv);
	h_phi0_matched_L1L2_->Fill(phi0);
	h_iphi0_matched_L1L2_->Fill(iphi0);
	h_phi0global_matched_L1L2_->Fill(phi0global);
	h_iphi0global_matched_L1L2_->Fill(iphi0global);
	h_eta_matched_L1L2_->Fill(eta);
	h_ieta_matched_L1L2_->Fill(ieta);
	h_z0_matched_L1L2_->Fill(z0);
	h_iz0_matched_L1L2_->Fill(iz0);
	L1SimTrack simtrk=FPGAGlobal::event()->simtrack(tp-1);
	h_rinvres_L1L2_->Fill(rinv-simtrk.rinv());
	h_irinvres_L1L2_->Fill(irinv-simtrk.rinv());
	double simtrkphi0=simtrk.phi();
	double dphiHG=0.5*dphisectorHG-M_PI/NSector;
	double phimin=+dphiHG-M_PI/NSector;
	double phioffset=phimin;
	while (iphi0-phioffset-simtrkphi0>M_PI/NSector) simtrkphi0+=2*M_PI/NSector;
	while (iphi0-phioffset-simtrkphi0<-M_PI/NSector) simtrkphi0-=2*M_PI/NSector;
	h_phi0res_L1L2_->Fill(phi0-phioffset-simtrkphi0);
	h_iphi0res_L1L2_->Fill(iphi0-phioffset-simtrkphi0);
	h_etares_L1L2_->Fill(eta-simtrk.eta());
	h_ietares_L1L2_->Fill(ieta-simtrk.eta());
	h_z0res_L1L2_->Fill(z0-simtrk.vz());
	h_iz0res_L1L2_->Fill(iz0-simtrk.vz());
      }
    }
    

    return; 
  }
  

  
  void FillLayerResidual(int layer, int seed, double phiresid, double iphiresid, 
			 double zresid, double izresid, bool match){
    if (layer==3) {
      if (seed==0) {
	h_layerresid_phi_L3_L1L2_->Fill(iphiresid);
	h_layerresid_phif_L3_L1L2_->Fill(phiresid);
	h_layerresid_z_L3_L1L2_->Fill(izresid);
	h_layerresid_zf_L3_L1L2_->Fill(zresid);
	if (match) {
	  h_layerresid_phi_L3_L1L2_match_->Fill(iphiresid);
	  h_layerresid_phif_L3_L1L2_match_->Fill(phiresid);
	  h_layerresid_z_L3_L1L2_match_->Fill(izresid);
	  h_layerresid_zf_L3_L1L2_match_->Fill(zresid);
	}
      }
    }
    return; 
  }
  
  virtual void FillDiskResidual(int disk, int seed, double phiresid, double iphiresid, 
				double rresid, double irresid, bool match){


    if (disk==1) {
      if (seed==0) {
	h_diskresid_phi_D1_L1L2_->Fill(iphiresid);
	h_diskresid_phif_D1_L1L2_->Fill(phiresid);
	h_diskresid_r_D1_L1L2_->Fill(irresid);
	h_diskresid_rf_D1_L1L2_->Fill(rresid);
	if (match) {
	  h_diskresid_phi_D1_L1L2_match_->Fill(iphiresid);
	  h_diskresid_phif_D1_L1L2_match_->Fill(phiresid);
	  h_diskresid_r_D1_L1L2_match_->Fill(irresid);
	  h_diskresid_rf_D1_L1L2_match_->Fill(rresid);
	}
      }
    }

    return; 
  }

  void bookSeedEff(){

    TH1::AddDirectory(kTRUE); 

    assert(h_file_!=0);
    h_file_->cd();

    h_eff_eta_L1L2seed_ = new TEfficiency("Efficincy for L1L2 seeding vs eta","Efficiency for L1L2 seeding vs eta",50,-2.5,2.5);
    h_eff_eta_L2L3seed_ = new TEfficiency("Efficincy for L2L3 seeding vs eta","Efficiency for L2L3 seeding vs eta",50,-2.5,2.5);
    h_eff_eta_L3L4seed_ = new TEfficiency("Efficincy for L3L4 seeding vs eta","Efficiency for L3L4 seeding vs eta",50,-2.5,2.5);
    h_eff_eta_L5L6seed_ = new TEfficiency("Efficincy for L5L6 seeding vs eta","Efficiency for L5L6 seeding vs eta",50,-2.5,2.5);
    h_eff_eta_D1D2seed_ = new TEfficiency("Efficincy for D1D2 seeding vs eta","Efficiency for D1D2 seeding vs eta",50,-2.5,2.5);
    h_eff_eta_D3D4seed_ = new TEfficiency("Efficincy for D3D4 seeding vs eta","Efficiency for D3D4 seeding vs eta",50,-2.5,2.5);
    h_eff_eta_D1L1seed_ = new TEfficiency("Efficincy for D1L1 seeding vs eta","Efficiency for D1L1 seeding vs eta",50,-2.5,2.5);
    h_eff_eta_D1L2seed_ = new TEfficiency("Efficincy for D1L2 seeding vs eta","Efficiency for D1L2 seeding vs eta",50,-2.5,2.5);

  }
    
  //Efficiency for finding seed
  virtual void fillSeedEff(int seedIndex, double etaTP, bool eff) {
    if (seedIndex==0) {
      h_eff_eta_L1L2seed_->Fill(eff,etaTP);
    }
    if (seedIndex==1) {
      h_eff_eta_L2L3seed_->Fill(eff,etaTP);
    }
    if (seedIndex==2) {
      h_eff_eta_L3L4seed_->Fill(eff,etaTP);
    }
    if (seedIndex==3) {
      h_eff_eta_L5L6seed_->Fill(eff,etaTP);
    }
    if (seedIndex==4) {
      h_eff_eta_D1D2seed_->Fill(eff,etaTP);
    }
    if (seedIndex==5) {
      h_eff_eta_D3D4seed_->Fill(eff,etaTP);
    }
    if (seedIndex==6) {
      h_eff_eta_D1L1seed_->Fill(eff,etaTP);
    }
    if (seedIndex==7) {
      h_eff_eta_D1L2seed_->Fill(eff,etaTP);
    }
    
    return;
  }

  
private:

  TFile* h_file_;

  //Layer residuales
  TH1F* h_layerresid_phi_L3_L1L2_;
  TH1F* h_layerresid_phi_L3_L1L2_match_;
  TH1F* h_layerresid_phif_L3_L1L2_;
  TH1F* h_layerresid_phif_L3_L1L2_match_;
  TH1F* h_layerresid_z_L3_L1L2_;
  TH1F* h_layerresid_z_L3_L1L2_match_;
  TH1F* h_layerresid_zf_L3_L1L2_;
  TH1F* h_layerresid_zf_L3_L1L2_match_;

  //Disk residuals
  TH1F* h_diskresid_phi_D1_L1L2_;
  TH1F* h_diskresid_phi_D1_L1L2_match_;
  TH1F* h_diskresid_phif_D1_L1L2_;
  TH1F* h_diskresid_phif_D1_L1L2_match_;
  TH1F* h_diskresid_r_D1_L1L2_;
  TH1F* h_diskresid_r_D1_L1L2_match_;
  TH1F* h_diskresid_rf_D1_L1L2_;
  TH1F* h_diskresid_rf_D1_L1L2_match_;

  //Tracklet parameters
  TH1F *h_rinv_L1L2_;
  TH1F *h_irinv_L1L2_;
  TH1F *h_rinv_matched_L1L2_;
  TH1F *h_irinv_matched_L1L2_;
  TH1F *h_rinvres_L1L2_;
  TH1F *h_irinvres_L1L2_;
  TH1F *h_phi0_L1L2_;
  TH1F *h_iphi0_L1L2_;
  TH1F *h_phi0_matched_L1L2_;
  TH1F *h_iphi0_matched_L1L2_;
  TH1F *h_phi0global_L1L2_;
  TH1F *h_iphi0global_L1L2_;
  TH1F *h_phi0global_matched_L1L2_;
  TH1F *h_iphi0global_matched_L1L2_;
  TH1F *h_phi0res_L1L2_;
  TH1F *h_iphi0res_L1L2_;
  TH1F *h_eta_L1L2_;
  TH1F *h_ieta_L1L2_;
  TH1F *h_eta_matched_L1L2_;
  TH1F *h_ieta_matched_L1L2_;
  TH1F *h_etares_L1L2_;
  TH1F *h_ietares_L1L2_;
  TH1F *h_z0_L1L2_;
  TH1F *h_iz0_L1L2_;
  TH1F *h_z0_matched_L1L2_;
  TH1F *h_iz0_matched_L1L2_;
  TH1F *h_z0res_L1L2_;
  TH1F *h_iz0res_L1L2_;

  //seeding efficiency
  TEfficiency *h_eff_eta_L1L2seed_;
  TEfficiency *h_eff_eta_L2L3seed_;
  TEfficiency *h_eff_eta_L3L4seed_;
  TEfficiency *h_eff_eta_L5L6seed_;
  TEfficiency *h_eff_eta_D1D2seed_;
  TEfficiency *h_eff_eta_D3D4seed_;
  TEfficiency *h_eff_eta_D1L1seed_;
  TEfficiency *h_eff_eta_D1L2seed_;

};



#endif



