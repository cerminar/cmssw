from __future__ import absolute_import
import FWCore.ParameterSet.Config as cms
from . import hgcalLayersCalibrationCoefficients_cfi as layercalibparam


def custom_cluster_calibration_global(process,
        factor=1.084
        ):
    parameters_c2d = process.hgcalBackEndLayer1Producer.ProcessorParameters.C2d_parameters
    parameters_c2d.calibSF_cluster = cms.double(factor)
    parameters_c2d.applyLayerCalibration = cms.bool(False)
    return process


def custom_cluster_calibration_layers(process,
        weights=layercalibparam.TrgLayer_weights
        ):
    parameters_c2d = process.hgcalBackEndLayer1Producer.ProcessorParameters.C2d_parameters
    parameters_c2d.layerWeights = weights
    parameters_c2d.applyLayerCalibration = cms.bool(True)
    return process


def custom_fe_thresholds(process, fe_threshold=1.5, cl3d_seed_threshold=7.5):
    process.hgcalConcentratorProducer.ProcessorParameters.triggercell_threshold_scintillator = cms.double(fe_threshold)
    process.hgcalConcentratorProducer.ProcessorParameters.triggercell_threshold_silicon = cms.double(fe_threshold)
    process.hgcalBackEndLayer2Producer.ProcessorParameters.C3d_parameters.threshold_histo_multicluster = cl3d_seed_threshold
    return process
