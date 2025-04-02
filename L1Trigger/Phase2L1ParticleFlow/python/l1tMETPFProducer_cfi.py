import FWCore.ParameterSet.Config as cms

l1tMETPFProducer = cms.EDProducer("L1MetPfProducer",
                                 L1PFObjects = cms.InputTag("l1tLayer1","Puppi"),
                                 maxCands = cms.int32(128),
                                 modelVersion = cms.string(""),
                                 OptMETHLS = cms.bool(False),
)

l1tMETMLProducer = cms.EDProducer("L1MetPfProducer",
                                 L1PFObjects = cms.InputTag("l1tLayer1","Puppi"),
                                 maxCands = cms.int32(100),
                                 modelVersion = cms.string("L1METML_v1"),
                                 OptMETHLS = cms.bool(False),
)

l1tMETNewPFProducer = cms.EDProducer("L1MetPfProducer",
                                 L1PFObjects = cms.InputTag("l1tLayer1","Puppi"),
                                 maxCands = cms.int32(128),
                                 modelVersion = cms.string(""),
                                 OptMETHLS = cms.bool(True),
)
