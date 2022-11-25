#ifndef DataFormatsL1TCorrelator_TkElectron_h
#define DataFormatsL1TCorrelator_TkElectron_h

// -*- C++ -*-
//
// Package:     L1Trigger
// Class  :     TkEm
//

#include "DataFormats/Common/interface/Ref.h"
#include "DataFormats/Common/interface/Ptr.h"

#include "DataFormats/L1Trigger/interface/EGamma.h"

#include "DataFormats/L1TCorrelator/interface/TkEm.h"
#include "DataFormats/L1TCorrelator/interface/TkEmFwd.h"

#include "DataFormats/L1TrackTrigger/interface/TTTypes.h"

#include <vector>

namespace l1t {

  class TkElectron : public TkEm {
  public:
    typedef TTTrack<Ref_Phase2TrackerDigi_> L1TTTrackType;
    typedef std::vector<L1TTTrackType> L1TTTrackCollection;

    TkElectron();

    TkElectron(const LorentzVector& p4,
               const edm::Ref<EGammaBxCollection>& egRef,
               const edm::Ptr<L1TTTrackType>& trkPtr,
               float tkisol = -999.);

    // ---------- const member functions ---------------------

    const edm::Ptr<L1TTTrackType>& trkPtr() const { return trkPtr_; }

    float trkzVtx() const { return trkzVtx_; }
    double trackCurvature() const { return trackCurvature_; }
    float compositeBdtScore() const { return compositeBdtScore_; }
    float compositeHoE() const { return compositeHoE_; }
    float compositeSrrtot() const { return compositeSrrtot_; }
    float compositeDeta() const { return compositeDeta_; }
    float compositeDphi() const { return compositeDphi_; }
    float compositeDpt() const { return compositeDpt_; }
    float compositeMeanz() const { return compositeMeanz_; }
    float compositeNstubs() const { return compositeNstubs_; }
    float compositeChi2RPhi() const { return compositeChi2RPhi_; }
    float compositeChi2RZ() const { return compositeChi2RZ_; }
    float compositeChi2Bend() const { return compositeChi2Bend_; }

    // ---------- member functions ---------------------------

    void setTrkzVtx(float TrkzVtx) { trkzVtx_ = TrkzVtx; }
    void setTrackCurvature(double trackCurvature) { trackCurvature_ = trackCurvature; }
    void setCompositeBdtScore(float score) { compositeBdtScore_ = score; }
    void setCompositeHoE(float hoe) { compositeHoE_ = hoe; }
    void setCompositeSrrtot(float srrtot) { compositeSrrtot_ = srrtot; }
    void setCompositeDeta(float deta) { compositeDeta_ = deta; }
    void setCompositeDphi(float dphi) { compositeDphi_ = dphi; }
    void setCompositeDpt(float dpt) { compositeDpt_ = dpt; }
    void setCompositeMeanz(float meanz) { compositeMeanz_ = meanz; }
    void setCompositeNstubs(float nstubs) { compositeNstubs_ = nstubs; }
    void setCompositeChi2RPhi(float chi2rphi) { compositeChi2RPhi_ = chi2rphi; }
    void setCompositeChi2RZ(float chi2rz) { compositeChi2RZ_ = chi2rz; }
    void setCompositeChi2Bend(float chi2bend) { compositeChi2Bend_ = chi2bend; }

  private:
    edm::Ptr<L1TTTrackType> trkPtr_;
    float trkzVtx_;
    double trackCurvature_;
    float compositeBdtScore_;
    float compositeHoE_;
    float compositeSrrtot_;
    float compositeDeta_;
    float compositeDphi_;
    float compositeDpt_;
    float compositeMeanz_;
    float compositeNstubs_;
    float compositeChi2RPhi_;
    float compositeChi2RZ_;
    float compositeChi2Bend_;
  };
}  // namespace l1t
#endif
