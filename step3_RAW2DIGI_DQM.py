# Auto generated configuration file
# using:
# Revision: 1.19
# Source: /local/reps/CMSSW/CMSSW/Configuration/Applications/python/ConfigBuilder.py,v
# with command line options: step3 --conditions auto:run2_data_relval -s RAW2DIGI,DQM --process RECO --data --era Run2_2016 --eventcontent DQM --scenario pp --datatier DQMIO -n 100 --filein /store/data/Run2016B/DoubleMuon/RAW/v2/000/274/199/00000/00B82081-6526-E611-9A62-02163E012B9E.root --fileout file:step3.root --no_exec
import FWCore.ParameterSet.Config as cms

from Configuration.StandardSequences.Eras import eras

process = cms.Process('RECO',eras.Run2_2016)

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_AutoFromDBCurrent_cff')
#process.load('Configuration.StandardSequences.RawToDigi_Data_cff')
#process.load('DQMOffline.Configuration.DQMOffline_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(100)
)

# Input source
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring('/store/data/Run2016B/DoubleMuon/RAW/v2/000/274/199/00000/00B82081-6526-E611-9A62-02163E012B9E.root'),
    secondaryFileNames = cms.untracked.vstring()
)

process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True)
)

# Production Info
process.configurationMetadata = cms.untracked.PSet(
    annotation = cms.untracked.string('step3 nevts:100'),
    name = cms.untracked.string('Applications'),
    version = cms.untracked.string('$Revision: 1.19 $')
)

# Output definition

process.DQMoutput = cms.OutputModule("DQMRootOutputModule",
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('DQMIO'),
        filterName = cms.untracked.string('')
    ),
    fileName = cms.untracked.string('file:step3.root'),
    outputCommands = process.DQMEventContent.outputCommands,
    splitLevel = cms.untracked.int32(0)
)

# Additional output definition

# Other statements
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_data_relval', '')


#here the customization for the DT DQM begins
process.load("DQM.DTMonitorModule.dt_dqm_sourceclient_common_cff")

#process.dtDQMPathPhys = cms.Path(process.unpackers + process.dqmmodules + process.physicsEventsFilter *  process.dtDQMPhysSequence)
process.dtDQM_sequence =   cms.Sequence(process.dtScalerInfoMonitor + process.gtDigis + process.reco + process.dtDQMTask)

# Path and EndPath definitions
process.raw2digi_step = cms.Path(process.unpackers)
process.dqmoffline_step = cms.Path(process.physicsEventsFilter *  process.dtDQM_sequence)
process.DQMoutput_step = cms.EndPath(process.DQMoutput)

# Schedule definition
process.schedule = cms.Schedule(process.raw2digi_step,process.dqmoffline_step,process.DQMoutput_step)
