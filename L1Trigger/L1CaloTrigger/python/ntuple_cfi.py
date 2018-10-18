import FWCore.ParameterSet.Config as cms

ntuple_egammaEE = cms.PSet(
    NtupleName = cms.string('L1TriggerNtupleEgammaEE'),
    EgammaEE = cms.InputTag('l1EGammaEEProducer:L1EGammaCollectionBXVWithCuts')
)
