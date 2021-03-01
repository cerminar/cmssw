import FWCore.ParameterSet.Config as cms

ntuple_egammaEE = cms.PSet(
    NtupleName = cms.string('L1TriggerNtupleEgamma'),
    Egamma = cms.InputTag('l1EGammaEEProducer:L1EGammaCollectionBXVWithCuts'),
    BranchNamePrefix = cms.untracked.string("egammaEE")
)

ntuple_egammaEB = cms.PSet(
    NtupleName = cms.string('L1TriggerNtupleEgamma'),
    Egamma = cms.InputTag("L1EGammaClusterEmuProducer"),
    BranchNamePrefix = cms.untracked.string("egammaEB")
)


ntuple_PFegammaEE = cms.PSet(
    NtupleName = cms.string('L1TriggerNtupleEgamma'),
    Egamma = cms.InputTag('l1pfProducerHGCal:L1Eg'),
    BranchNamePrefix = cms.untracked.string("PFegammaEE")
)

ntuple_PFegammaEENoTk = cms.PSet(
    NtupleName = cms.string('L1TriggerNtupleEgamma'),
    Egamma = cms.InputTag('l1pfProducerHGCalNoTK:L1Eg'),
    BranchNamePrefix = cms.untracked.string("PFegammaEENoTk")
)

ntuple_PFegammaEEHF = cms.PSet(
    NtupleName = cms.string('L1TriggerNtupleEgamma'),
    Egamma = cms.InputTag('l1pfProducerHF:L1Eg'),
    BranchNamePrefix = cms.untracked.string("PFegammaEEHF")
)


ntuple_TTTracks = cms.PSet(
    NtupleName = cms.string('L1TriggerNtupleTrackTrigger'),
    TTTracks = cms.InputTag("TTTracksFromTrackletEmulation", "Level1TTTracks"),
    BranchNamePrefix = cms.untracked.string("l1Trk")
)

# ntuple_tkEleEE = cms.PSet(
#     NtupleName = cms.string('L1TriggerNtupleTkElectrons'),
#     TkElectrons = cms.InputTag("L1TkElectronsHGC","EG"),
#     BranchNamePrefix = cms.untracked.string("tkEleEE")
# )
#
# ntuple_tkEleEB = cms.PSet(
#     NtupleName = cms.string('L1TriggerNtupleTkElectrons'),
#     TkElectrons = cms.InputTag("L1TkElectronsCrystal","EG"),
#     BranchNamePrefix = cms.untracked.string("tkEleEB")
# )

ntuple_PFtkEleEE = cms.PSet(
    NtupleName = cms.string('L1TriggerNtupleTkElectrons'),
    TkElectrons = cms.InputTag("l1pfProducerHGCal","L1TkEle"),
    BranchNamePrefix = cms.untracked.string("PFtkEleEE")
)

ntuple_PFtkEleEB = cms.PSet(
    NtupleName = cms.string('L1TriggerNtupleTkElectrons'),
    TkElectrons = cms.InputTag("l1pfProducerBarrel","L1TkEle"),
    BranchNamePrefix = cms.untracked.string("PFtkEleEB")
)

ntuple_PFtkEmEE = cms.PSet(
    NtupleName = cms.string('L1TriggerNtupleTkEm'),
    TkEms = cms.InputTag("l1pfProducerHGCal", "L1TkEm"),
    BranchNamePrefix = cms.untracked.string("PFtkEmEE")
)

ntuple_PFtkEmEENoTk = cms.PSet(
    NtupleName = cms.string('L1TriggerNtupleTkEm'),
    TkEms = cms.InputTag("l1pfProducerHGCalNoTK", "L1TkEm"),
    BranchNamePrefix = cms.untracked.string("PFtkEmEENoTk")
)

ntuple_PFtkEmEB = cms.PSet(
    NtupleName = cms.string('L1TriggerNtupleTkEm'),
    TkEms = cms.InputTag("l1pfProducerBarrel", "L1TkEm"),
    BranchNamePrefix = cms.untracked.string("PFtkEmEB")
)

ntuple_tkEleEllEE = cms.PSet(
    NtupleName = cms.string('L1TriggerNtupleTkElectrons'),
    TkElectrons = cms.InputTag("L1TkElectronsEllipticMatchHGC","EG"),
    BranchNamePrefix = cms.untracked.string("tkEleEE")
)

ntuple_tkEmEB = cms.PSet(
    NtupleName = cms.string('L1TriggerNtupleTkEm'),
    TkEms = cms.InputTag("L1TkPhotonsCrystal","EG"),
    BranchNamePrefix = cms.untracked.string("tkEmEB")
)

ntuple_tkEmEE = cms.PSet(
    NtupleName = cms.string('L1TriggerNtupleTkEm'),
    TkEms = cms.InputTag("L1TkPhotonsHGC","EG"),
    BranchNamePrefix = cms.untracked.string("tkEmEE")
)

ntuple_tkEleEllEB = cms.PSet(
    NtupleName = cms.string('L1TriggerNtupleTkElectrons'),
    TkElectrons = cms.InputTag("L1TkElectronsEllipticMatchCrystal","EG"),
    BranchNamePrefix = cms.untracked.string("tkEleEB")
)


ntuple_tkIsoEleEE = cms.PSet(
    NtupleName = cms.string('L1TriggerNtupleTkElectrons'),
    TkElectrons = cms.InputTag("L1TkIsoElectronsHGC","EG"),
    BranchNamePrefix = cms.untracked.string("tkIsoEleEE")
)

ntuple_tkIsoEleEB = cms.PSet(
    NtupleName = cms.string('L1TriggerNtupleTkElectrons'),
    TkElectrons = cms.InputTag("L1TkIsoElectronsCrystal","EG"),
    BranchNamePrefix = cms.untracked.string("tkIsoEleEB")
)
