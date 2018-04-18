#include <limits>
#include "FWCore/ParameterSet/interface/FileInPath.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "L1Trigger/L1THGCal/interface/be_algorithms/HGCalTriggerClusterIdentificationBase.h"
#include "L1Trigger/L1THGCal/interface/HGCalTriggerTools.h"
#include "CommonTools/Utils/interface/TMVAEvaluator.h"



class HGCalTriggerClusterIdentificationBDT : public HGCalTriggerClusterIdentificationBase
{

  public:
    class Category
    {
      public:
        Category(float pt_min, float pt_max, float eta_min, float eta_max):
          pt_min_(pt_min), pt_max_(pt_max), 
          eta_min_(eta_min), eta_max_(eta_max)
        {
        }
        ~Category(){}
        bool contains(float pt, float eta) const
        {
          bool output = true;
          if(pt<pt_min_ || pt>=pt_max_) output = false;
          if(std::abs(eta)<eta_min_ || std::abs(eta)>=eta_max_) output = false;
          return output;
        }

        float pt_min() const {return pt_min_;}
        float pt_max() const {return pt_max_;}
        float eta_min() const {return eta_min_;}
        float eta_max() const {return eta_max_;}

      private:
        float pt_min_ = 0.;
        float pt_max_ = std::numeric_limits<float>::max();
        float eta_min_ = 1.5;
        float eta_max_ = 3.;
    };

  public:

    HGCalTriggerClusterIdentificationBDT();
    ~HGCalTriggerClusterIdentificationBDT() override{};
    void initialize(const edm::ParameterSet& conf) final;
    float value(const l1t::HGCalMulticluster& cluster) const final;
    bool decision(const l1t::HGCalMulticluster& cluster) const final;

  private:
    std::vector<Category> categories_;
    std::vector<std::unique_ptr<TMVAEvaluator>> bdts_;
    std::vector<double> working_points_;
    std::vector<std::string> input_variables_;

    float clusterVariable(const std::string&, const l1t::HGCalMulticluster&) const;
    int category(float pt, float eta) const;


};

DEFINE_EDM_PLUGIN(HGCalTriggerClusterIdentificationFactory,
    HGCalTriggerClusterIdentificationBDT,
    "HGCalTriggerClusterIdentificationBDT" );


HGCalTriggerClusterIdentificationBDT::
HGCalTriggerClusterIdentificationBDT():HGCalTriggerClusterIdentificationBase()
{
}

void
HGCalTriggerClusterIdentificationBDT::
initialize(const edm::ParameterSet& conf)
{
  categories_.clear();
  bdts_.clear();
  working_points_.clear();
  input_variables_.clear();
  input_variables_ = conf.getParameter< std::vector<std::string> >("Inputs");
  std::vector<std::string> bdt_files = conf.getParameter< std::vector<std::string> >("Weights");
  std::vector<double> categories_etamin = conf.getParameter<std::vector<double>>("CategoriesEtaMin");
  std::vector<double> categories_etamax = conf.getParameter<std::vector<double>>("CategoriesEtaMax");
  std::vector<double> categories_ptmin = conf.getParameter<std::vector<double>>("CategoriesPtMin");
  std::vector<double> categories_ptmax = conf.getParameter<std::vector<double>>("CategoriesPtMax");
  working_points_ = conf.getParameter<std::vector<double>>("WorkingPoints");
  for(unsigned cat=0; cat<categories_etamin.size(); cat++)
  {
    categories_.emplace_back(
        categories_ptmin[cat],
        categories_ptmax[cat],
        categories_etamin[cat],
        categories_etamax[cat]);
  }
  std::vector<std::string> spectators = {};
  for (const auto& file : bdt_files)
  {
    // TMVAEvaluator bdt;
    bdts_.emplace_back(new TMVAEvaluator());
    bdts_.back()->initialize(
        "!Color:Silent:!Error",
        "BDT::BDT",
        edm::FileInPath(file).fullPath(),
        input_variables_,
        spectators,
        false, false);
    // bdts_.push_back(bdt);
  }
}

float
HGCalTriggerClusterIdentificationBDT::
value(const l1t::HGCalMulticluster& cluster) const
{
  std::map<std::string, float> inputs;
  for(const std::string& var : input_variables_)
  {
    inputs[var] = clusterVariable(var, cluster);
  }
  float pt = cluster.pt();
  float eta = cluster.eta();
  int cat = category(pt, eta);
  return (cat!=-1 ? bdts_.at(cat)->evaluate(inputs) : -999.);
}


bool
HGCalTriggerClusterIdentificationBDT::
decision(const l1t::HGCalMulticluster& cluster) const
{
  float bdt_output = value(cluster);
  float pt = cluster.pt();
  float eta = cluster.eta();
  int cat = category(pt, eta);
  return (cat!=-1 ? bdt_output>working_points_.at(cat) : true);
}

int 
HGCalTriggerClusterIdentificationBDT::
category(float pt, float eta) const
{
  for(unsigned cat=0; cat<categories_.size(); cat++)
  {
    if(categories_[cat].contains(pt, eta)) return static_cast<int>(cat);
  }
  edm::LogWarning("HGCalTriggerClusterIdentificationBDT|UnknownCategory")
    <<"pt="<<pt<<",eta="<<eta<<" doesn't match any defined category\n";
  return -1;
}




float
HGCalTriggerClusterIdentificationBDT::
clusterVariable(const std::string& variable, const l1t::HGCalMulticluster& cluster) const
{
  if(variable=="cl3d_showerlength") return cluster.showerLength();
  if(variable=="cl3d_coreshowerlength") return cluster.coreShowerLength();
  if(variable=="cl3d_firstlayer") return cluster.firstLayer();
  if(variable=="cl3d_maxlayer") return cluster.maxLayer();
  if(variable=="cl3d_seetot") return cluster.sigmaEtaEtaTot();
  if(variable=="cl3d_seemax") return cluster.sigmaEtaEtaMax();
  if(variable=="cl3d_spptot") return cluster.sigmaPhiPhiTot();
  if(variable=="cl3d_sppmax") return cluster.sigmaPhiPhiMax();
  if(variable=="cl3d_szz") return cluster.sigmaZZ();
  if(variable=="cl3d_srrtot") return cluster.sigmaRRTot();
  if(variable=="cl3d_srrmax") return cluster.sigmaRRMax();
  if(variable=="cl3d_srrmean") return cluster.sigmaRRMean();
  return 0.;
}
