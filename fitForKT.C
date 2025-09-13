R__LOAD_LIBRARY(DiSpin)
#include "src/Tools.h"

// attempt to fit PhPerp^2 vs. z^2 distribution, to get some leverage on kT;
// this assumes that 
//   <p_T^2>(x,z) = <p_perp^2>(z) + z^2 <k_T^2>(x), where this p_T is our PhPerp; see e.g., arXiv:1111.1740
//
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// IMPORTANT:in hindsight, this is NOT A GOOD IDEA since the <p_perp^2>(z) has a non-trivial z-dependence, and
// may be dominant at low z^2, where most of our data are...
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//

void fitForKT(TString plots_file_name="plots.root" /*output from diagnostics.exe*/)
{
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(1);

  auto plots_file = new TFile(plots_file_name, "READ");
  std::vector<TH2D*> plots;
  plots.push_back((TH2D*)plots_file->Get("hadPperp2vsHadZ2_piPlus"));
  plots.push_back((TH2D*)plots_file->Get("hadPperp2vsHadZ2_piMinus"));
  auto* canv = new TCanvas("canv", "canv", 1000, 1000);
  canv->Divide(1,2);

  int pad_num=0;
  std::vector<TProfile*> profs;
  for(auto plot : plots) {
    auto pad = canv->GetPad(++pad_num);
    pad->cd();
    pad->SetLogz();
    pad->SetGrid(1,1);
    if(plot==nullptr)
      throw std::runtime_error("plot not found");
    plot->Draw("colz");
    profs.push_back(plot->ProfileX());
    profs.back()->SetLineColor(kBlack);
    profs.back()->SetLineWidth(3);
    profs.back()->Draw("same");
    auto fit = profs.back()->Fit("pol1", "S", "", 0.02, 0.1);
    auto kT2 = fit->GetParams()[1];
    std::cout << "sqrt(<kT2>) = " << TMath::Sqrt(kT2) << std::endl;
  }

  // plots_file->Close();
}
