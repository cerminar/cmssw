/**
 * @package   Alignment/MillePedeAlignmentAlgorithm
 * @file      MillePedeDQMModule.cc
 *
 * @author    Max Stark (max.stark@cern.ch)
 * @date      Oct 26, 2015
 */



/*** Header file ***/
#include "MillePedeDQMModule.h"

/*** ROOT objects ***/
#include "TCanvas.h"
#include "TH1F.h"
#include "TPaveText.h"



MillePedeDQMModule
::MillePedeDQMModule(const edm::ParameterSet& config) :
  millePedeLogFile_(config.getUntrackedParameter<std::string>("millePedeLogFile")),
  millePedeResFile_(config.getUntrackedParameter<std::string>("millePedeResFile"))
{
}

MillePedeDQMModule
::~MillePedeDQMModule()
{
}

void MillePedeDQMModule
::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;

  desc.add<std::string>("millePedeLogFile", "")->setComment(
    "The .log-file of the MillePede alignment algorithm (pede-mode)"
  );
  desc.add<std::string>("millePedeResFile", "")->setComment(
    "The .res-file of the MillePede alignment algorithm (pede-mode)"
  );

  descriptions.add("MillePedeDQMModule", desc);
  descriptions.setComment("Generic .cfi-file for the MillePedeDQMModule");
}



//=============================================================================
//===   INTERFACE IMPLEMENTATION                                            ===
//=============================================================================

void MillePedeDQMModule
::bookHistograms(DQMStore::IBooker& booker,
                 edm::Run const& /* run */,
                 edm::EventSetup const& /* setup */)
{
  edm::LogInfo("MillePedeDQMModule") << "Booking histograms";

  //FIXME: this needs a new folder in the DQM structure
  h_xPos[0] = booker.book1D("Xpos",   "#Delta X;;#mu m", 6, 0, 6.);
  h_xPos[1] = booker.book1D("Xpos_1", "Xpos_1",          6, 0, 6.);
  h_xPos[2] = booker.book1D("Xpos_2", "Xpos_2",          6, 0, 6.);
  h_xPos[3] = booker.book1D("Xpos_3", "Xpos_3",          6, 0, 6.);

  h_xRot[0] = booker.book1D("Xrot",   "#Delta #theta_{X};;#mu rad", 6, 0, 6.);
  h_xRot[1] = booker.book1D("Xrot_1", "Xrot_1",                     6, 0, 6.);
  h_xRot[2] = booker.book1D("Xrot_2", "Xrot_2",                     6, 0, 6.);
  h_xRot[3] = booker.book1D("Xrot_3", "Xrot_3",                     6, 0, 6.);

  h_yPos[0] = booker.book1D("Ypos",   "#Delta Y;;#mu m", 6, 0., 6.);
  h_yPos[1] = booker.book1D("Ypos_1", "Ypos_1",          6, 0., 6.);
  h_yPos[2] = booker.book1D("Ypos_2", "Ypos_2",          6, 0., 6.);
  h_yPos[3] = booker.book1D("Ypos_3", "Ypos_3",          6, 0., 6.);

  h_yRot[0] = booker.book1D("Yrot",   "#Delta #theta_{Y};;#mu rad", 6, 0, 6.);
  h_yRot[1] = booker.book1D("Yrot_1", "Yrot_1",                     6, 0, 6.);
  h_yRot[2] = booker.book1D("Yrot_2", "Yrot_2",                     6, 0, 6.);
  h_yRot[3] = booker.book1D("Yrot_3", "Yrot_3",                     6, 0, 6.);

  h_zPos[0] = booker.book1D("Zpos",   "#Delta Z;;#mu m", 6, 0., 6.);
  h_zPos[1] = booker.book1D("Zpos_1", "Zpos_1",          6, 0., 6.);
  h_zPos[2] = booker.book1D("Zpos_2", "Zpos_2",          6, 0., 6.);
  h_zPos[3] = booker.book1D("Zpos_3", "Zpos_3",          6, 0., 6.);

  h_zRot[0] = booker.book1D("Zrot",   "#Delta #theta_{Z};;#mu rad", 6, 0, 6.);
  h_zRot[1] = booker.book1D("Zrot_1", "Zrot_1",                     6, 0, 6.);
  h_zRot[2] = booker.book1D("Zrot_2", "Zrot_2",                     6, 0, 6.);
  h_zRot[3] = booker.book1D("Zrot_3", "Zrot_3",                     6, 0, 6.);
}

void MillePedeDQMModule
::endRun(edm::Run const&, edm::EventSetup const&)
{
  readMillePedeLogFile();
  readMillePedeResultFile();

  fillExpertHistos();
  drawSummary();
}

bool MillePedeDQMModule
::updateDB()
{
  return DBupdated;
}



//=============================================================================
//===   PRIVATE METHOD IMPLEMENTATION                                       ===
//=============================================================================

void MillePedeDQMModule
::readMillePedeLogFile()
{
  std::ifstream logFile;
  logFile.open(millePedeLogFile_.c_str());

  if (logFile.is_open()) {
    edm::LogInfo("MillePedeDQMModule") << "Reading millepede log-file";
    std::string line;

    while (getline(logFile, line)) {
      std::string Nrec_string = "NREC =";

      if (line.find(Nrec_string) != std::string::npos) {
        std::istringstream iss(line);
        std::string trash;
        iss >> trash >> trash >> Nrec;

        if (Nrec < 25000) {
          PedeSuccess = false;
          Movements   = false;
          Error       = false;
          Significant = false;
          DBupdated   = false;
        }
      }
    }

  } else {
    edm::LogError("MillePedeDQMModule") << "Could not read millepede log-file.";

    PedeSuccess = false;
    Movements   = false;
    Error       = false;
    Significant = false;
    DBupdated   = false;
    Nrec = 0;
  }
}

void MillePedeDQMModule
::readMillePedeResultFile()
{
  std::ifstream resFile;
  resFile.open(millePedeResFile_.c_str());

  if (resFile.is_open()) {
    edm::LogInfo("MillePedeDQMModule") << "Reading millepede result-file";
    float Multiplier[6] = {10000.,10000.,10000.,1000000.,1000000.,1000000.};

    std::string line;
    getline(resFile, line); // drop first line

    while (getline(resFile, line)) {
      std::istringstream iss(line);

      std::vector<std::string> tokens;
      std::string token;
      while (iss >> token) {
        tokens.push_back(token);
      }

      if (tokens.size() > 4 /*3*/) {
        PedeSuccess = true;

        int alignable      = std::stoi(tokens[0]);
        int alignableIndex = alignable % 10 - 1;

        float ObsMove = std::stof(tokens[3]) * Multiplier[alignableIndex];
        float ObsErr  = std::stof(tokens[4]) * Multiplier[alignableIndex];

        int det = -1;

        if (alignable >= 60 && alignable <= 69) {
          det = 2; // TPBHalfBarrel (x+)
        } else if (alignable >= 8780 && alignable <= 8789) {
          det = 3; // TPBHalfBarrel (x-)
        } else if (alignable >= 17520 && alignable <= 17529) {
          det = 4; // TPEHalfCylinder (x+,z+)
        } else if (alignable >= 22380 && alignable <= 22389) {
          det = 5; // TPEHalfCylinder (x-,z+)
        } else if (alignable >= 27260 && alignable <= 27269) {
          det = 0; // TPEHalfCylinder (x+,z-)
        } else if (alignable >= 32120 && alignable <= 32129) {
          det = 1; //TPEHalfCylinder (x-,z-)
        } else {
          continue;
        }

        if (alignableIndex == 0 && det >= 0 && det <= 5) {
          Xobs[det] = ObsMove;
          XobsErr[det] = ObsErr;
        } else if (alignableIndex == 1 && det >= 0 && det <= 5) {
          Yobs[det] = ObsMove;
          YobsErr[det] = ObsErr;
        } else if (alignableIndex == 2 && det >= 0 && det <= 5) {
          Zobs[det] = ObsMove;
          ZobsErr[det] = ObsErr;
        } else if (alignableIndex == 3 && det >= 0 && det <= 5) {
          tXobs[det] = ObsMove;
          tXobsErr[det] = ObsErr;
        } else if (alignableIndex == 4 && det >= 0 && det <= 5) {
          tYobs[det] = ObsMove;
          tYobsErr[det] = ObsErr;
        } else if (alignableIndex == 5 && det >= 0 && det <= 5) {
          tZobs[det] = ObsMove;
          tZobsErr[det] = ObsErr;
        }

        if (abs(ObsMove) > MaxMoveCut) {
          Movements   = false;
          Error       = false;
          Significant = false;
          DBupdated   = false;
          HitMax      = false;
          continue;

        } else if (abs(ObsMove) > Cutoffs[alignableIndex]) {
          Movements = true;

          if (abs(ObsErr) > MaxErrorCut) {
            Error       = false;
            Significant = false;
            DBupdated   = false;
            HitErrorMax = true;
            continue;
          } else {
            Error = true;
            if (abs(ObsMove/ObsErr) > SigCut) {
              Significant = true;
            }
          }
        }
        DBupdated = true;
      }
    }
  } else {
    edm::LogError("MillePedeDQMModule") << "Could not read millepede result-file.";

    PedeSuccess = false;
    Movements   = false;
    Error       = false;
    Significant = false;
    DBupdated   = false;
    Nrec = 0;
  }
}

void MillePedeDQMModule
::fillExpertHistos()
{
  TCanvas c("PCL_SiPixAl_Expert", "PCL_SiPixAl_Expert", 1500, 800);
  c.Divide(3, 2);

  c.cd(1); fillExpertHisto(h_xPos,  Xobs,  XobsErr,  Xcut);
  c.cd(4); fillExpertHisto(h_xRot, tXobs, tXobsErr, tXcut);

  c.cd(2); fillExpertHisto(h_yPos,  Yobs,  YobsErr,  Ycut);
  c.cd(5); fillExpertHisto(h_yRot, tYobs, tYobsErr, tYcut);

  c.cd(3); fillExpertHisto(h_zPos,  Zobs,  ZobsErr,  Zcut);
  c.cd(6); fillExpertHisto(h_zRot, tZobs, tZobsErr, tZcut);

  c.Write();
}

void MillePedeDQMModule
::fillExpertHisto(MonitorElement* histos[],
                  float obs[6], float obsErr[6], float cut)
{
  TH1F* histo_0 = histos[0]->getTH1F();
  TH1F* histo_1 = histos[1]->getTH1F();
  TH1F* histo_2 = histos[2]->getTH1F();
  TH1F* histo_3 = histos[3]->getTH1F();

  histo_0->SetMinimum(-MaxMoveCut);
  histo_0->SetMaximum( MaxMoveCut);
  histo_0->SetLabelSize(0.06);

  for (size_t i = 0; i < Bins.size(); ++i) {
    histo_0->GetXaxis()->SetBinLabel(i+1, Bins[i]);

    histo_0->SetBinContent(i+1, obs[i]);
    histo_1->SetBinContent(i+1, obs[i]);
    histo_2->SetBinContent(i+1,  cut);
    histo_3->SetBinContent(i+1, -cut);

    histo_0->SetBinError(i+1, obsErr[i]);
    histo_1->SetBinError(i+1, (histo_1->GetBinContent(i+1) / SigCut));
  }

  histo_0->SetLineWidth(2);
  histo_1->SetLineWidth(2);
  histo_2->SetLineWidth(2);
  histo_3->SetLineWidth(2);

  histo_0->SetLineColor(kBlack);
  histo_1->SetLineColor(kRed+2);
  histo_2->SetLineColor(kRed+2);
  histo_3->SetLineColor(kRed+2);

  histo_0->SetFillColor(kGreen+3);

  histo_0->Draw("HIST");
  histo_0->Draw("AE1 SAME");
  histo_1->Draw("E1 SAME");
  histo_2->Draw("HIST same");
  histo_3->Draw("HIST same");
}

void MillePedeDQMModule
::drawSummary()
{
  TCanvas c("PCL_SiPixAl","PCL_SiPixAl", 1000, 600);
  c.Divide(1, 3);



  TPaveText Text_DB(.0, .0, 1., 1.);
  Text_DB.SetBorderSize(1);
  Text_DB.AddText("DB updated");
  Text_DB.SetTextSize(0.15);
  Text_DB.SetFillColor(kGray);

  if (HitMax || HitErrorMax) {
    Text_DB.SetFillColor(kRed);
    //command = "echo \"New Alignment to be Updated "+os.getcwd()+" Nrec={0}\" |
    //          mail -s \"New Prompt Unreasonable Alignment Update\" chmartin@cern.ch".format(Nrec)
    //os.system(command)
    //os.system("rm -f TkAlignment.db")

  } else if (DBupdated) {
    Text_DB.SetFillColor(kOrange);
    // command = "echo \"New Alignment to be Updated "+os.getcwd()+" Nrec={0}\" |
    //           mail -s \"New Prompt Reasonable Alignment Update\" chmartin@cern.ch".format(Nrec)
    // os.system(command)
  }

  c.cd(1);
  Text_DB.Draw();



  TVirtualPad* c1_2 = c.cd(2);
  c1_2->Divide(4,1);

  c1_2->cd(1);
  TPaveText Text_Move(.0, .0, 1., 1.);
  Text_Move.SetBorderSize(1);
  Text_Move.AddText("Movement Seen");
  Text_Move.SetTextSize(0.15);
  if (Movements) {
    Text_Move.SetFillColor(kOrange);
  } else {
    Text_Move.SetFillColor(kGray);
    // os.system("rm -f TkAlignment.db")
  }
  Text_Move.Draw();

  c1_2->cd(2);
  TPaveText Text_Move2(.0, .0, 1., 1.);
  Text_Move2.SetBorderSize(1);
  Text_Move2.AddText("#splitline{Movement}{Reasonable}");
  Text_Move2.SetTextSize(0.15);
  Text_Move2.SetFillColor(kGray);
  if (HitMax) {
    Text_Move2.SetFillColor(kRed);
    // os.system("rm -f TkAlignment.db")
  } else {
    Text_Move2.SetFillColor(kGreen);
  }
  Text_Move2.Draw();

  c1_2->cd(3);
  TPaveText Text_Error(.0, .0, 1., 1.);
  Text_Error.SetBorderSize(1);
  Text_Error.AddText("#splitline{Uncertainty}{Reasonable}");
  Text_Error.SetTextSize(0.15);
  Text_Error.SetFillColor(kGray);
  if (HitErrorMax) {
    Text_Error.SetFillColor(kRed);
    // os.system("rm -f TkAlignment.db")
  } else if (Error) {
      Text_Error.SetFillColor(kGreen);
  }
  Text_Error.Draw();

  c1_2->cd(4);
  TPaveText Text_Signif(.0, .0, 1., 1.);
  Text_Signif.SetBorderSize(1);
  Text_Signif.AddText("#splitline{Movement}{Significant}");
  Text_Signif.SetTextSize(0.15);
  if (Significant) {
    Text_Signif.SetFillColor(kGreen);
  } else {
    Text_Signif.SetFillColor(kGray);
    // os.system("rm -f TkAlignment.db")
  }
  Text_Signif.Draw();



  TPaveText Text_Pede(.0, .0, 1., 1.);
  Text_Pede.SetBorderSize(1);
  std::string text("Alignment Performed Nrec = " + std::to_string(Nrec));
  Text_Pede.AddText(text.c_str());

  Text_Pede.SetTextSize(0.15);
  if (PedeSuccess) {
    Text_Pede.SetFillColor(kGreen);
  } else {
    Text_Pede.SetFillColor(kRed);
    // os.system("rm -f TkAlignment.db")
  }
  c.cd(3);
  Text_Pede.Draw();



  c.Write();
}



//define this as a plug-in
DEFINE_FWK_MODULE(MillePedeDQMModule);
