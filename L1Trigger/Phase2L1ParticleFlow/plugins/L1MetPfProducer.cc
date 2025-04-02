#include <vector>
#include <string>
#include <ap_int.h>
#include <ap_fixed.h>
#include <TVector2.h>

#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/L1TParticleFlow/interface/PFCandidate.h"
// For HLS MET Data Formats
#include "DataFormats/L1TParticleFlow/interface/puppi.h"
#include "DataFormats/L1TParticleFlow/interface/sums.h"

#include "DataFormats/L1Trigger/interface/EtSum.h"
#include "DataFormats/Math/interface/LorentzVector.h"

#include "hls4ml/emulator.h"

using namespace l1t;

class L1MetPfProducer : public edm::global::EDProducer<> {
public:
  explicit L1MetPfProducer(const edm::ParameterSet&);
  ~L1MetPfProducer() override;
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void produce(edm::StreamID, edm::Event& iEvent, const edm::EventSetup& iSetup) const override;
  edm::EDGetTokenT<vector<l1t::PFCandidate>> _l1PFToken;

  int maxCands_ = 128;

  // quantization controllers
  typedef ap_ufixed<14, 12, AP_RND, AP_WRAP> pt_t;  // LSB is 0.25 and max is 4 TeV
  typedef ap_int<12> phi_t;                         // LSB is pi/720 ~ 0.0044 and max is +/-8.9
  static constexpr float ptLSB_ = 0.25;             // GeV
  static constexpr float phiLSB_ = M_PI / 720;      // rad

  // derived, helper types
  typedef ap_fixed<pt_t::width + 1, pt_t::iwidth + 1, AP_RND, AP_SAT> pxy_t;
  typedef ap_fixed<2 * pt_t::width, 2 * pt_t::iwidth, AP_RND, AP_SAT> pt2_t;
  // derived, helper constants
  static constexpr float maxPt_ = ((1 << pt_t::width) - 1) * ptLSB_;
  const phi_t hwPi_ = round(M_PI / phiLSB_);
  const phi_t hwPiOverTwo_ = round(M_PI / (2 * phiLSB_));

  typedef ap_ufixed<pt_t::width, 0> inv_t;  // can't easily use the MAXPT/pt trick with ap_fixed

  // to make configurable...
  static constexpr int dropBits_ = 2;
  static constexpr int dropFactor_ = (1 << dropBits_);
  static constexpr int invTableBits_ = 10;
  static constexpr int invTableSize_ = (1 << invTableBits_);

  // hls4ml emulator objects
  bool useMlModel_;
  std::shared_ptr<hls4mlEmulator::Model> model;
  std::string modelVersion_;
  typedef ap_fixed<32, 16> input_t;
  typedef ap_fixed<32, 16> result_t;
  static constexpr int numContInputs_ = 4;
  static constexpr int numPxPyInputs_ = 2;
  static constexpr int numCatInputs_ = 2;
  static constexpr int numInputs_ = numContInputs_ + numPxPyInputs_ + numCatInputs_;

  // HLS MET emulator objects
  bool OptMETHLS;
  typedef ap_fixed<22, 12> proj_t;
  typedef ap_fixed<32, 22> proj2_t;
  typedef ap_fixed<32, 2> poly_t;
  typedef ap_fixed<32, 2> poly2_t;
  typedef l1ct::Sum Met;

  void Project(pt_t pt, phi_t phi, pxy_t& pxy, bool isX, bool debug = false) const;
  void PhiFromXY(pxy_t px, pxy_t py, phi_t& phi, bool debug = false) const;

  void Get_xy(pt_t pt, phi_t phi, proj_t& px, proj_t& py) const;
  void pxpy_to_ptphi(proj_t met_x, proj_t met_y, Met& hls_met) const;


  int EncodePdgId(int pdgId) const;

  void CalcMetNewHLS(const std::vector<float>& pt,
                     const std::vector<float>& phi,
                     reco::Candidate::PolarLorentzVector& metVector) const;

  void CalcMetHLS(const std::vector<float>& pt,
                  const std::vector<float>& phi,
                  reco::Candidate::PolarLorentzVector& metVector) const;  

  void CalcMlMet(const std::vector<float>& pt,
                 const std::vector<float>& eta,
                 const std::vector<float>& phi,
                 const std::vector<float>& puppiWeight,
                 const std::vector<int>& pdgId,
                 const std::vector<int>& charge,
                 reco::Candidate::PolarLorentzVector& metVector) const;
};

L1MetPfProducer::L1MetPfProducer(const edm::ParameterSet& cfg)
    : _l1PFToken(consumes<std::vector<l1t::PFCandidate>>(cfg.getParameter<edm::InputTag>("L1PFObjects"))),
      maxCands_(cfg.getParameter<int>("maxCands")),
      modelVersion_(cfg.getParameter<std::string>("modelVersion")),
      OptMETHLS(cfg.getParameter<bool>("OptMETHLS"))
      {
  produces<std::vector<l1t::EtSum>>();
  useMlModel_ = (modelVersion_.length() > 0);
  if (useMlModel_) {
    hls4mlEmulator::ModelLoader loader(modelVersion_);
    model = loader.load_model();
  }
}

void L1MetPfProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("L1PFObjects", edm::InputTag("L1PFProducer", "l1pfCandidates"));
  desc.add<int>("maxCands", 128);
  desc.add<std::string>("modelVersion", "");
  desc.add<bool>("OptMETHLS", false);
  descriptions.add("L1MetPfProducer", desc);
}

void L1MetPfProducer::produce(edm::StreamID, edm::Event& iEvent, const edm::EventSetup& iSetup) const {
  edm::Handle<l1t::PFCandidateCollection> l1PFCandidates;
  iEvent.getByToken(_l1PFToken, l1PFCandidates);

  std::vector<float> pt;
  std::vector<float> eta;
  std::vector<float> phi;
  std::vector<float> puppiWeight;
  std::vector<int> pdgId;
  std::vector<int> charge;

  for (int i = 0; i < int(l1PFCandidates->size()) && (i < maxCands_ || maxCands_ < 0); i++) {
    const auto& l1PFCand = l1PFCandidates->at(i);
    pt.push_back(l1PFCand.pt());
    eta.push_back(l1PFCand.eta());
    phi.push_back(l1PFCand.phi());
    puppiWeight.push_back(l1PFCand.puppiWeight());
    pdgId.push_back(l1PFCand.pdgId());
    charge.push_back(l1PFCand.charge());
  }

  reco::Candidate::PolarLorentzVector metVector;

  if (useMlModel_) {
    CalcMlMet(pt, eta, phi, puppiWeight, pdgId, charge, metVector);
  } else if (OptMETHLS) {
    CalcMetNewHLS(pt, phi, metVector);
  } else {
    CalcMetHLS(pt, phi, metVector);
  }

  l1t::EtSum theMET(metVector, l1t::EtSum::EtSumType::kTotalHt, 0, 0, 0, 0);

  auto metCollection = std::make_unique<std::vector<l1t::EtSum>>(0);
  metCollection->push_back(theMET);
  iEvent.put(std::move(metCollection));
}

int L1MetPfProducer::EncodePdgId(int pdgId) const {
  switch (abs(pdgId)) {
    case 211:  // charged hadron (pion)
      return 1;
    case 130:  // neutral hadron (kaon)
      return 2;
    case 22:  // photon
      return 3;
    case 13:  // muon
      return 4;
    case 11:  // electron
      return 5;
    default:
      return 0;
  }
}

void L1MetPfProducer::CalcMlMet(const std::vector<float>& pt,
                                const std::vector<float>& eta,
                                const std::vector<float>& phi,
                                const std::vector<float>& puppiWeight,
                                const std::vector<int>& pdgId,
                                const std::vector<int>& charge,
                                reco::Candidate::PolarLorentzVector& metVector) const {
  const int inputSize = maxCands_ * numInputs_;

  input_t input[800];
  result_t result[2];

  // initialize with zeros (for padding)
  for (int i = 0; i < inputSize; i++) {
    input[i] = 0;
  }

  for (uint i = 0; i < pt.size(); i++) {
    // input_cont
    input[i * numContInputs_] = pt[i];
    input[i * numContInputs_ + 1] = eta[i];
    input[i * numContInputs_ + 2] = phi[i];
    input[i * numContInputs_ + 3] = puppiWeight[i];
    // input_pxpy
    input[(maxCands_ * numContInputs_) + (i * numPxPyInputs_)] = pt[i] * cos(phi[i]);
    input[(maxCands_ * numContInputs_) + (i * numPxPyInputs_) + 1] = pt[i] * sin(phi[i]);
    // input_cat0
    input[maxCands_ * (numContInputs_ + numPxPyInputs_) + i] = EncodePdgId(pdgId[i]);
    // input_cat1
    input[maxCands_ * (numContInputs_ + numPxPyInputs_ + 1) + i] = (abs(charge[i]) <= 1) ? (charge[i] + 2) : 0;
  }

  model->prepare_input(input);
  model->predict();
  model->read_result(result);

  double met_px = -result[0].to_double();
  double met_py = -result[1].to_double();
  metVector.SetPt(hypot(met_px, met_py));
  metVector.SetPhi(atan2(met_py, met_px));
  metVector.SetEta(0);
}

void L1MetPfProducer::CalcMetNewHLS(const std::vector<float>& pt,
                                 const std::vector<float>& phi,
                                 reco::Candidate::PolarLorentzVector& metVector) const {

  proj_t hw_px = 0;
  proj_t hw_py = 0;
  proj_t hw_met_px = 0;
  proj_t hw_met_py = 0;

  for (uint i = 0; i < pt.size(); i++) {
    pt_t hw_pt = min(pt[i], maxPt_);
    phi_t hw_phi = float(TVector2::Phi_mpi_pi(phi[i]) / phiLSB_);

    Get_xy(hw_pt, hw_phi, hw_px, hw_py);

    hw_met_px -= hw_px;
    hw_met_py -= hw_py;
  }

  Met hw_met;

  pxpy_to_ptphi(hw_met_px, hw_met_py, hw_met);

  metVector.SetPt(hw_met.hwPt.to_double());
  metVector.SetPhi(hw_met.hwPhi.to_double() * phiLSB_);
  metVector.SetEta(0);
}



void L1MetPfProducer::Get_xy(pt_t pt, phi_t phi, proj_t& px, proj_t& py) const {
  /*
      Convert pt, phi to px, py
      Use 2nd order Polynomial interpolation for cos, sin with 16 points
    */

    poly2_t cos2_par0[16] = {-1.00007,-0.924181,-0.707596,-0.382902,-0.000618262,0.382137,0.707056,0.923708,1.00007,0.924181,0.707594,0.383285,0.000188727,-0.382139,-0.706719,-0.923708};
    poly2_t cos2_par1[16] = {9.164680268990924e-06, 0.0017064607695524156, 0.0031441321076514446, 0.004079929656016374, 0.004437063290882583, 0.004095969231842202, 0.0031107221424451436, 0.001689531075808071, -9.161756842493832e-06, -0.001706456406229286, -0.003143961938049376, -0.004103015998697129, -0.004411145151490469, -0.0040958165155326525, -0.0031310072316764474, -0.001689531075808071};
    poly2_t cos2_par2[16] = {9.319674765430664e-06, 7.871694899063284e-06, 5.222989318251642e-06, 2.0256106486379287e-06, -1.9299417402361656e-06, -5.35167113952279e-06, -7.740062096537953e-06, -9.348822844786505e-06, -9.319674765430664e-06, -7.871694899063284e-06, -5.225331064666252e-06, -1.780776301343235e-06, 1.6556927733433181e-06, 5.3495197789955455e-06, 7.954684107366423e-06, 9.348822844786505e-06};

    poly2_t sin2_par0[16] = {0.000524872,-0.382229,-0.706791,-0.923959,-1.00008,-0.924156,-0.707264,-0.383199,-0.000525527,0.382228,0.706792,0.923752,1.00013,0.924155,0.707535,0.3832};
    poly2_t sin2_par1[16] = {-0.004431478237276202, -0.00409041472149773, -0.0031267268116859314, -0.00167440343451641, 9.741773386162849e-06, 0.0017049641497188307, 0.00312406082125351, 0.0040978672774037465, 0.004431478237276202, 0.00409041472149773, 0.0031266351819002015, 0.0016868781753450394, -1.249302315254411e-05, -0.001704846339994321, -0.003140405829698437, -0.0040978672774037465};
    poly2_t sin2_par2[16] = {1.870674613498914e-06, 5.292404012785538e-06, 7.909829192302831e-06, 9.188746390688592e-06, 9.313525301268721e-06, 7.887020962996302e-06, 5.435897856093815e-06, 1.8358587462761668e-06, -1.870668901922293e-06, -5.292404012785538e-06, -7.908420336736317e-06, -9.320836119343602e-06, -9.284396260501616e-06, -7.88869635880513e-06, -5.262894200243701e-06, -1.835864457852788e-06};

    phi_t phi2_edges[16] = {-720, -630, -540, -450, -360, -270, -180, -90, 0, 90, 180, 270, 360, 450, 540, 630};


    int phibin = 0;
    if      (phi < phi2_edges[1]) phibin = 0;
    else if (phi < phi2_edges[2]) phibin = 1;
    else if (phi < phi2_edges[3]) phibin = 2;
    else if (phi < phi2_edges[4]) phibin = 3;
    else if (phi < phi2_edges[5]) phibin = 4;
    else if (phi < phi2_edges[6]) phibin = 5;
    else if (phi < phi2_edges[7]) phibin = 6;
    else if (phi < phi2_edges[8]) phibin = 7;
    else if (phi < phi2_edges[9]) phibin = 8;
    else if (phi < phi2_edges[10]) phibin = 9;
    else if (phi < phi2_edges[11]) phibin = 10;
    else if (phi < phi2_edges[12]) phibin = 11;
    else if (phi < phi2_edges[13]) phibin = 12;
    else if (phi < phi2_edges[14]) phibin = 13;
    else if (phi < phi2_edges[15]) phibin = 14;
    else if (phi >= phi2_edges[15]) phibin = 15;
    
    poly_t cos_var = cos2_par0[phibin] + cos2_par1[phibin] * (phi - phi2_edges[phibin]) + cos2_par2[phibin] * (phi - phi2_edges[phibin]) * (phi - phi2_edges[phibin]);
    poly_t sin_var = sin2_par0[phibin] + sin2_par1[phibin] * (phi - phi2_edges[phibin]) + sin2_par2[phibin] * (phi - phi2_edges[phibin]) * (phi - phi2_edges[phibin]);
    px = pt * cos_var;
    py = pt * sin_var;

}

void L1MetPfProducer::pxpy_to_ptphi(proj_t met_x, proj_t met_y, Met& hls_met) const {
    // convert x, y coordinate to pt, phi coordinate using math library

    hls_met.clear();
    hls_met.hwPt = hypot(met_x.to_float(), met_y.to_float());

    // Reduce Latency by not-using division.
    hls_met.hwPhi = phi_t(ap_fixed<26, 11>(atan2(met_y.to_float(), met_x.to_float())) * ap_fixed<26, 11>(229.29936)); // 720/pi
    // out_metphi = l1ct::Scales::makeGlbPhi(hls::atan2(mety, metx));

    return;

}

void L1MetPfProducer::CalcMetHLS(const std::vector<float>& pt,
  const std::vector<float>& phi,
  reco::Candidate::PolarLorentzVector& metVector) const {
pxy_t hw_px = 0;
pxy_t hw_py = 0;
pxy_t hw_sumx = 0;
pxy_t hw_sumy = 0;

for (uint i = 0; i < pt.size(); i++) {
pt_t hw_pt = min(pt[i], maxPt_);
phi_t hw_phi = float(TVector2::Phi_mpi_pi(phi[i]) / phiLSB_);

Project(hw_pt, hw_phi, hw_px, true);
Project(hw_pt, hw_phi, hw_py, false);

hw_sumx = hw_sumx - hw_px;
hw_sumy = hw_sumy - hw_py;
}

pt2_t hw_met = pt2_t(hw_sumx) * pt2_t(hw_sumx) + pt2_t(hw_sumy) * pt2_t(hw_sumy);
hw_met = sqrt(int(hw_met));  // stand-in for HLS::sqrt

phi_t hw_met_phi = 0;
PhiFromXY(hw_sumx, hw_sumy, hw_met_phi);

metVector.SetPt(hw_met.to_double());
metVector.SetPhi(hw_met_phi.to_double() * phiLSB_);
metVector.SetEta(0);
}

void L1MetPfProducer::Project(pt_t pt, phi_t phi, pxy_t& pxy, bool isX, bool debug) const {
/*
Convert pt and phi to px (py)
1) Map phi to the first quadrant to reduce LUT size
2) Lookup sin(phiQ1), where the result is in [0,maxPt]
which is used to encode [0,1].
3) Multiply pt by sin(phiQ1) to get px. Result will be px*maxPt, but
wrapping multiplication is 'mod maxPt' so the correct value is returned.
4) Check px=-|px|.
*/

// set phi to first quadrant
phi_t phiQ1 = (phi > 0) ? phi : phi_t(-phi);  // Q1/Q4
if (phiQ1 >= hwPiOverTwo_)
phiQ1 = hwPi_ - phiQ1;

if (phiQ1 > hwPiOverTwo_) {
edm::LogWarning("L1MetPfProducer") << "unexpected phi (high)";
phiQ1 = hwPiOverTwo_;
} else if (phiQ1 < 0) {
edm::LogWarning("L1MetPfProducer") << "unexpected phi (low)";
phiQ1 = 0;
}
if (isX) {
typedef ap_ufixed<14, 12, AP_RND, AP_WRAP> pt_t;  // LSB is 0.25 and max is 4 TeV
ap_ufixed<pt_t::width, 0> cosPhi = cos(phiQ1.to_double() / hwPiOverTwo_.to_double() * M_PI / 2);
pxy = pt * cosPhi;
if (phi > hwPiOverTwo_ || phi < -hwPiOverTwo_)
pxy = -pxy;
} else {
ap_ufixed<pt_t::width, 0> sinPhi = sin(phiQ1.to_double() / hwPiOverTwo_.to_double() * M_PI / 2);
pxy = pt * sinPhi;
if (phi < 0)
pxy = -pxy;
}
}

void L1MetPfProducer::PhiFromXY(pxy_t px, pxy_t py, phi_t& phi, bool debug) const {
if (px == 0 && py == 0) {
phi = 0;
return;
}
if (px == 0) {
phi = py > 0 ? hwPiOverTwo_ : phi_t(-hwPiOverTwo_);
return;
}
if (py == 0) {
phi = px > 0 ? phi_t(0) : phi_t(-hwPi_);
return;
}

// get q1 coordinates
pt_t x = px > 0 ? pt_t(px) : pt_t(-px);  //px>=0 ? px : -px;
pt_t y = py > 0 ? pt_t(py) : pt_t(-py);  //px>=0 ? px : -px;
// transform so a<b
pt_t a = x < y ? x : y;
pt_t b = x < y ? y : x;

if (b.to_double() > maxPt_ / dropFactor_)
b = maxPt_ / dropFactor_;
// map [0,max/4) to inv table size
int index = round((b.to_double() / (maxPt_ / dropFactor_)) * invTableSize_);
float bcheck = (float(index) / invTableSize_) * (maxPt_ / dropFactor_);
inv_t inv_b = 1. / ((float(index) / invTableSize_) * (maxPt_ / dropFactor_));

inv_t a_over_b = a * inv_b;

if (debug) {
printf("  a, b = %f, %f;   index, inv = %d, %f; ratio = %f \n",
a.to_double(),
b.to_double(),
index,
inv_b.to_double(),
a_over_b.to_double());
printf("    bcheck, 1/bc = %f, %f  -- %d  %f  %d  \n", bcheck, 1. / bcheck, invTableSize_, maxPt_, dropFactor_);
}

int atanTableBits_ = 7;
int atanTableSize_ = (1 << atanTableBits_);
index = round(a_over_b.to_double() * atanTableSize_);
phi = atan(float(index) / atanTableSize_) / phiLSB_;

if (debug) {
printf("    atan index, phi = %d, %f (%f rad)  real atan(a/b)= %f  \n",
index,
phi.to_double(),
phi.to_double() * (M_PI / hwPi_.to_double()),
atan(a.to_double() / b.to_double()));
}

// rotate from (0,pi/4) to full quad1
if (y > x)
phi = hwPiOverTwo_ - phi;  //phi = pi/2 - phi
// other quadrants
if (px < 0 && py > 0)
phi = hwPi_ - phi;  // Q2 phi = pi - phi
if (px > 0 && py < 0)
phi = -phi;  // Q4 phi = -phi
if (px < 0 && py < 0)
phi = -(hwPi_ - phi);  // Q3 composition of both

if (debug) {
printf("    phi hw, float, real = %f, %f    (%f rad from x,y = %f, %f) \n",
phi.to_double(),
phi.to_double() * (M_PI / hwPi_.to_double()),
atan2(py.to_double(), px.to_double()),
px.to_double(),
py.to_double());
}
}

L1MetPfProducer::~L1MetPfProducer() {}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(L1MetPfProducer);

