import FWCore.ParameterSet.Config as cms

l1tMETCorrPFProducer = cms.EDProducer("L1MetCorrPfProducer",
    RawMET = cms.InputTag("l1tMETNewPFProducer"),
    L1PFJets = cms.InputTag("l1tSC4PFL1PuppiCorrectedEmulator")
    # L1PFJets = cms.InputTag("l1tSC4PFL1PuppiEmulator")
)