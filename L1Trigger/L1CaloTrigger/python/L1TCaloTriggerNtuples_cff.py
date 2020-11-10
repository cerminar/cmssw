import FWCore.ParameterSet.Config as cms

from L1Trigger.L1THGCalUtilities.hgcalTriggerNtuples_cfi import *

l1CaloTriggerNtuplizer = hgcalTriggerNtuplizer.clone()

ntuple_multiclusters_hmvdr = ntuple_multiclusters.clone()
ntuple_multiclusters_hmvdr.Prefix = cms.untracked.string('HMvDR')

l1CaloTriggerNtuplizer.Ntuples = cms.VPSet(ntuple_event,
                                           ntuple_gen,
                                           ntuple_triggercells,
                                           ntuple_multiclusters_hmvdr)

# l1CaloTriggerNtuplizer.Ntuples.remove(ntuple_genjet)
# l1CaloTriggerNtuplizer.Ntuples.remove(ntuple_gentau)
# l1CaloTriggerNtuplizer.Ntuples.remove(ntuple_digis)
# l1CaloTriggerNtuplizer.Ntuples.remove(ntuple_multiclusters)
# l1CaloTriggerNtuplizer.Ntuples.remove(ntuple_towers)


from L1Trigger.L1CaloTrigger.ntuple_cfi import *

l1CaloTriggerNtuplizer.Ntuples.append(ntuple_egammaEE)
l1CaloTriggerNtuplizer.Ntuples.append(ntuple_egammaEB)
l1CaloTriggerNtuplizer.Ntuples.append(ntuple_TTTracks)
l1CaloTriggerNtuplizer.Ntuples.append(ntuple_tkEleEllEE)
l1CaloTriggerNtuplizer.Ntuples.append(ntuple_tkEleEllEB)

#
l1CaloTriggerNtuples = cms.Sequence(l1CaloTriggerNtuplizer)


l1CaloTriggerNtuplizer_egOnly = hgcalTriggerNtuplizer.clone()
l1CaloTriggerNtuplizer_egOnly.Ntuples = cms.VPSet(
    ntuple_event,
    ntuple_gen,
    ntuple_egammaEE,
    ntuple_egammaEB,
    ntuple_TTTracks,
    ntuple_tkEleEllEE,
    ntuple_tkEleEllEB,
    ntuple_PFegammaEE,
    ntuple_PFegammaEENoTk,
    ntuple_PFegammaEEHF,
    ntuple_PFtkEleEE,
    ntuple_PFtkEleEB,
    ntuple_PFtkEmEE,
    ntuple_PFtkEmEB,
    ntuple_tkEmEB,
    ntuple_tkEmEE
)
