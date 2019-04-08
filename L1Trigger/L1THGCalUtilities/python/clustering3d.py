import FWCore.ParameterSet.Config as cms

from L1Trigger.L1THGCal.customClustering import binSums, dr_layerbylayer
from L1Trigger.L1THGCal.egammaIdentification import *


def create_distance(process, inputs,
        distance=0.01,
        working_points=tight_wp
        ):
    producer = process.hgcalBackEndLayer2Producer.clone()
    producer.ProcessorParameters.C3d_parameters.dR_multicluster = cms.double(distance)
    producer.ProcessorParameters.C3d_parameters.dist_dbscan_multicluster=cms.double(0.)
    producer.ProcessorParameters.C3d_parameters.minN_dbscan_multicluster=cms.uint32(0)
    producer.ProcessorParameters.C3d_parameters.type_multicluster = cms.string('dRC3d')
    producer.InputCluster = cms.InputTag('{}:HGCalBackendLayer1Processor2DClustering'.format(inputs))

    if len(working_points)!=len(working_points_drnn_cone):
        raise RuntimeError('HGC TPG ID: Number of working points ({0}) not compatible with number of categories ({1})'.format(
                    len(working_points), len(working_points_drnn_cone)))
    for wp,cat in zip(working_points,working_points_drnn_cone):
        if not wp in cat:
            raise KeyError('HGC TPG ID: Cannot find a cut corresponding to the working point {}'.format(wp))

    identification = egamma_identification_drnn_cone.clone()
    identification.WorkingPoints = cms.vdouble(
            [wps[eff] for wps,eff in zip(working_points_drnn_cone,working_points)]
            )
    producer.ProcessorParameters.C3d_parameters.EGIdentification = identification
    return producer


def create_dbscan(process, inputs,
        distance=0.005,
        min_points=3,
        working_points=tight_wp
        ):
    producer = process.hgcalBackEndLayer2Producer.clone()
    producer.ProcessorParameters.C3d_parameters.dR_multicluster = cms.double(0.)
    producer.ProcessorParameters.C3d_parameters.dist_dbscan_multicluster = cms.double(distance)
    producer.ProcessorParameters.C3d_parameters.minN_dbscan_multicluster = cms.uint32(min_points)
    producer.ProcessorParameters.C3d_parameters.type_multicluster = cms.string('DBSCANC3d')
    producer.InputCluster = cms.InputTag('{}:HGCalBackendLayer1Processor2DClustering'.format(inputs))

    if len(working_points)!=len(working_points_drnn_dbscan):
        raise RuntimeError('HGC TPG ID: Number of working points ({0}) not compatible with number of categories ({1})'.format(
                    len(working_points), len(working_points_drnn_dbscan)))
    for wp,cat in zip(working_points,working_points_drnn_dbscan):
        if not wp in cat:
            raise KeyError('HGC TPG ID: Cannot find a cut corresponding to the working point {}'.format(wp))
    identification = egamma_identification_drnn_dbscan.clone()
    identification.WorkingPoints = cms.vdouble(
            [wps[eff] for wps,eff in zip(working_points_drnn_dbscan,working_points)]
            )
    producer.ProcessorParameters.C3d_parameters.EGIdentification = identification

    return producer


def create_histoMax(process, inputs,
        distance = 0.03,
        nBins_R = 36,
        nBins_Phi = 216,
        binSumsHisto = binSums,
        seed_threshold = 0,
        working_points=tight_wp
        ):
    producer = process.hgcalBackEndLayer2Producer.clone()
    producer.ProcessorParameters.C3d_parameters.dR_multicluster = cms.double(distance)
    producer.ProcessorParameters.C3d_parameters.nBins_R_histo_multicluster = cms.uint32(nBins_R)
    producer.ProcessorParameters.C3d_parameters.nBins_Phi_histo_multicluster = cms.uint32(nBins_Phi)
    producer.ProcessorParameters.C3d_parameters.binSumsHisto = binSumsHisto
    producer.ProcessorParameters.C3d_parameters.threshold_histo_multicluster = seed_threshold
    producer.ProcessorParameters.C3d_parameters.type_multicluster = cms.string('HistoMaxC3d')
    producer.InputCluster = cms.InputTag('{}:HGCalBackendLayer1Processor2DClustering'.format(inputs))
    if len(working_points)!=len(working_points_histomax):
        raise RuntimeError('HGC TPG ID: Number of working points ({0}) not compatible with number of categories ({1})'.format(
                    len(working_points), len(working_points_histomax)))
    for wp,cat in zip(working_points,working_points_drnn_dbscan):
        if not wp in cat:
            raise KeyError('HGC TPG ID: Cannot find a cut corresponding to the working point {}'.format(wp))
    parameters_c3d = process.hgcalBackEndLayer2Producer.ProcessorParameters.C3d_parameters
    identification = egamma_identification_histomax.clone()
    identification.WorkingPoints = cms.vdouble(
            [wps[eff] for wps,eff in zip(working_points_histomax,working_points)]
            )
    producer.ProcessorParameters.C3d_parameters.EGIdentification = identification
    return producer


def create_histoMax_variableDr(process, inputs,
        distances = dr_layerbylayer,
        nBins_R = 36,
        nBins_Phi = 216,
        binSumsHisto = binSums,
        seed_threshold = 0,
        working_points=tight_wp
        ):
    producer = create_histoMax(process, inputs, 0, nBins_R, nBins_Phi, binSumsHisto, seed_threshold, working_points)
    producer.ProcessorParameters.C3d_parameters.dR_multicluster_byLayer = cms.vdouble(distances)
    return producer


def create_histoInterpolatedMax(process, inputs,
        distance = 0.03,
        nBins_R = 36,
        nBins_Phi = 216,
        binSumsHisto = binSums,
        ):
    producer = create_histoMax( process, inputs, distance, nBins_R, nBins_Phi, binSumsHisto )
    producer.ProcessorParameters.C3d_parameters.type_multicluster = cms.string('HistoInterpolatedMaxC3d')
    return producer

def create_histoInterpolatedMax1stOrder(process, inputs):
    producer = create_histoInterpolatedMax( process, inputs )
    producer.ProcessorParameters.C3d_parameters.neighbour_weights=cms.vdouble(  0    , 0.25, 0   ,
                                                   0.25 , 0   , 0.25,
                                                   0    , 0.25, 0
                                                )
    return producer



def create_histoInterpolatedMax2ndOrder(process, inputs):
    producer = create_histoInterpolatedMax( process,inputs )
    producer.ProcessorParameters.C3d_parameters.neighbour_weights=cms.vdouble( -0.25, 0.5, -0.25,
                                                   0.5 , 0  ,  0.5 ,
                                                  -0.25, 0.5, -0.25
                                                )
    return producer



def create_histoThreshold(process, inputs,
        threshold = 20.,
        distance = 0.03,
        nBins_R = 36,
        nBins_Phi = 216,
        binSumsHisto = binSums,
        ):
    producer = process.hgcalBackEndLayer2Producer.clone()
    producer.ProcessorParameters.C3d_parameters.threshold_histo_multicluster = cms.double(threshold)
    producer.ProcessorParameters.C3d_parameters.dR_multicluster = cms.double(distance)
    producer.ProcessorParameters.C3d_parameters.nBins_R_histo_multicluster = cms.uint32(nBins_R)
    producer.ProcessorParameters.C3d_parameters.nBins_Phi_histo_multicluster = cms.uint32(nBins_Phi)
    producer.ProcessorParameters.C3d_parameters.binSumsHisto = binSumsHisto
    producer.ProcessorParameters.C3d_parameters.type_multicluster = cms.string('HistoThresholdC3d')
    producer.InputCluster = cms.InputTag('{}:HGCalBackendLayer1Processor2DClustering'.format(inputs))
    return producer
