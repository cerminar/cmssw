# Auto generated configuration file
# using:
# Revision: 1.19
# Source: /local/reps/CMSSW/CMSSW/Configuration/Applications/python/ConfigBuilder.py,v
# with command line options: step4 --data --filetype DQM --conditions auto:run2_data_relval -s HARVESTING:@standardDQM --scenario pp -n 100 --filein file:step3.root --fileout file:step4.root --no_exec
import FWCore.ParameterSet.Config as cms

process = cms.Process('HARVESTING')

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_AutoFromDBCurrent_cff')
process.load('Configuration.StandardSequences.DQMSaverAtRunEnd_cff')
#process.load('Configuration.StandardSequences.Harvesting_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(100)
)

# Input source
process.source = cms.Source("DQMRootSource",
    fileNames = cms.untracked.vstring('file:step3.root')
)

process.options = cms.untracked.PSet(
    Rethrow = cms.untracked.vstring('ProductNotFound'),
    fileMode = cms.untracked.string('FULLMERGE')
)

# Production Info
process.configurationMetadata = cms.untracked.PSet(
    annotation = cms.untracked.string('step4 nevts:100'),
    name = cms.untracked.string('Applications'),
    version = cms.untracked.string('$Revision: 1.19 $')
)

# Output definition

# Additional output definition

# Other statements
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_data_relval', '')

#here the customization for the DT DQM begins
process.load("DQM.DTMonitorModule.dt_dqm_sourceclient_common_cff")



# Path and EndPath definitions
# process.dqmHarvestingPOGMC = cms.Path(process.DQMOffline_SecondStep_PrePOGMC)
# process.validationHarvesting = cms.Path(process.postValidation+process.hltpostvalidation+process.postValidation_gen)
# process.validationprodHarvesting = cms.Path(process.hltpostvalidation_prod+process.postValidation_gen)
# process.dqmHarvestingFakeHLT = cms.Path(process.DQMOffline_SecondStep_FakeHLT+process.DQMOffline_Certification)
# process.validationpreprodHarvesting = cms.Path(process.postValidation_preprod+process.hltpostvalidation_preprod+process.postValidation_gen)
# process.validationHarvestingMiniAOD = cms.Path(process.JetPostProcessor+process.METPostProcessorHarvesting+process.postValidationMiniAOD)
# process.validationHarvestingHI = cms.Path(process.postValidationHI)
# process.genHarvesting = cms.Path(process.postValidation_gen)
# process.dqmHarvestingPOG = cms.Path(process.DQMOffline_SecondStep_PrePOG)
# process.alcaHarvesting = cms.Path()
# process.validationHarvestingFS = cms.Path(process.postValidation+process.hltpostvalidation+process.postValidation_gen)
process.dtHarversting = cms.Path(process.dtDQMTest)
process.dqmsave_step = cms.Path(process.DQMSaver)

# Schedule definition
process.schedule = cms.Schedule(process.dqmsave_step)
