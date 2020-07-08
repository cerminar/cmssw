import FWCore.ParameterSet.Config as cms

from L1Trigger.L1TTrackMatch.L1TkElectronTrackProducer_cfi import *

l1TkElectronTrackEllipticProducers = cms.Sequence(L1TkElectronsEllipticMatchCrystal+L1TkElectronsEllipticMatchHGC)
l1TkElectronTrackEllipticProducersEB = cms.Sequence(L1TkElectronsEllipticMatchCrystal)
l1TkElectronTrackEllipticProducersEE = cms.Sequence(L1TkElectronsEllipticMatchHGC)

# from L1Trigger.L1TTrackMatch.L1TkEmParticleProducer_cfi import *
