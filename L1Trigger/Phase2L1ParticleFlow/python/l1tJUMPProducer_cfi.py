import FWCore.ParameterSet.Config as cms

l1tJUMPProducer = cms.EDProducer("L1JUMPProducer",
    RawMET = cms.InputTag("l1tMETNewPFProducer"),
    L1PFJets = cms.InputTag("l1tSC4PFL1PuppiCorrectedEmulator")
    # L1PFJets = cms.InputTag("l1tSC4PFL1PuppiEmulator")
)