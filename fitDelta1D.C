// trial fitting of delta vs. one amplitude

void fitDelta1D(int w=0) {
  TGraphErrors * g = new TGraphErrors();
  int i=0;

  TF1 * f;

  switch(w) {
    case 0: // A=0.04, vary B, f=sinPhiH
      g->SetPoint(i++,-0.8,0.0087);
      g->SetPoint(i++,-0.4,0.0052);
      g->SetPoint(i++,-0.2,0.0027);
      g->SetPoint(i++,-0.1,0.0014);
      g->SetPoint(i++,0,0);
      g->SetPoint(i++,0.1,-0.0016);
      g->SetPoint(i++,0.2,-0.0033);
      g->SetPoint(i++,0.4,-0.0072);
      g->SetPoint(i++,0.8,-0.02);
      f = new TF1("f","[0]*0.04*x/(1+[0]*x)",-1,1);
      g->SetTitle("#delta vs. B;B;#delta");
      break;
    case 1: // A=0.16, vary B, f=sinPhiH
      g->SetPoint(i++,-0.8,0.0367);
      g->SetPoint(i++,-0.4,0.0213);
      g->SetPoint(i++,-0.2,0.0114);
      g->SetPoint(i++,-0.1,0.0060);
      g->SetPoint(i++,0,0);
      g->SetPoint(i++,0.1,-0.0066);
      g->SetPoint(i++,0.2,-0.0139);
      g->SetPoint(i++,0.4,-0.0307);
      g->SetPoint(i++,0.8,-0.0776);
      f = new TF1("f","[0]*0.16*x/(1+[0]*x)",-1,1);
      g->SetTitle("#delta vs. B;B;#delta");
      break;
    case 2: // B=0.2, vary A, f=sinPhiH
      g->SetPoint(i++,-0.32,0.0273);
      g->SetPoint(i++,-0.16,0.0142);
      g->SetPoint(i++,-0.08,0.0068);
      g->SetPoint(i++,-0.04,0.0033);
      g->SetPoint(i++,0,0);
      g->SetPoint(i++,0.04,-0.0033);
      g->SetPoint(i++,0.08,-0.0068);
      g->SetPoint(i++,0.16,-0.0140);
      g->SetPoint(i++,0.32,-0.0273);
      f = new TF1("f","[0]*x",-1,1);
      g->SetTitle("#delta vs. A;A;#delta");
      break;
    case 3: // B=0.2, vary A, f=sin(PhiH-PhiR)
      g->SetPoint(i++,-0.32,0.0281);
      g->SetPoint(i++,-0.16,0.0136);
      g->SetPoint(i++,-0.08,0.0069);
      g->SetPoint(i++,-0.04,0.0035);
      g->SetPoint(i++,0,0);
      g->SetPoint(i++,0.04,-0.0031);
      g->SetPoint(i++,0.08,-0.0070);
      g->SetPoint(i++,0.16,-0.0135);
      g->SetPoint(i++,0.32,-0.0282);
      f = new TF1("f","[0]*x",-1,1);
      g->SetTitle("#delta vs. A;A;#delta");
      break;
    case 4: // A=0.04, vary B, f=sin(PhiH-PhiR)
      g->SetPoint(i++,-0.8,0.0095);
      g->SetPoint(i++,-0.4,0.0055);
      g->SetPoint(i++,-0.2,0.0029);
      g->SetPoint(i++,-0.1,0.0015);
      g->SetPoint(i++,0,0);
      g->SetPoint(i++,0.1,-0.00054);
      g->SetPoint(i++,0.2,-0.0031);
      g->SetPoint(i++,0.4,-0.0071);
      g->SetPoint(i++,0.8,-0.0191);
      f = new TF1("f","[0]*0.04*x/(1+[0]*x)",-1,1);
      g->SetTitle("#delta vs. B;B;#delta");
      break;
    case 5: // A=0.16, vary B, f=sin(PhiH-PhiR)
      g->SetPoint(i++,-0.8,0.0374);
      g->SetPoint(i++,-0.4,0.0212);
      g->SetPoint(i++,-0.2,0.0114);
      g->SetPoint(i++,-0.1,0.0061);
      g->SetPoint(i++,0,0);
      g->SetPoint(i++,0.1,-0.0063);
      g->SetPoint(i++,0.2,-0.0135);
      g->SetPoint(i++,0.4,-0.0304);
      g->SetPoint(i++,0.8,-0.0788);
      f = new TF1("f","[0]*0.16*x/(1+[0]*x)",-1,1);
      g->SetTitle("#delta vs. B;B;#delta");
      break;
    case 6: // B=0.2, vary A, g=sinTheta
      g->SetPoint(i++,-0.32,-0.0497);
      g->SetPoint(i++,-0.16,-0.0247);
      g->SetPoint(i++,-0.08,-0.0124);
      g->SetPoint(i++,-0.04,-0.0059);
      g->SetPoint(i++,0,0);
      g->SetPoint(i++,0.04,0.0060);
      g->SetPoint(i++,0.08,0.0125);
      g->SetPoint(i++,0.16,0.0252);
      g->SetPoint(i++,0.32,0.0487);
      f = new TF1("f","[0]*x",-1,1);
      g->SetTitle("#delta vs. A;A;#delta");
      break;
    case 7: // A=0.04, vary B, g=sinTheta
      g->SetPoint(i++,-0.8,-0.1348);
      g->SetPoint(i++,-0.4,-0.0238);
      g->SetPoint(i++,-0.2,-0.0089);
      g->SetPoint(i++,-0.1,-0.0043);
      g->SetPoint(i++,0,0);
      g->SetPoint(i++,0.1,0.0031);
      g->SetPoint(i++,0.2,0.0060);
      g->SetPoint(i++,0.4,0.0106);
      g->SetPoint(i++,0.8,0.0171);
      f = new TF1("f","[0]*0.04*x/(1+[0]*x)",-1,1);
      g->SetTitle("#delta vs. B;B;#delta");
      break;
    case 8: // A=0.16, vary B, g=sinTheta
      g->SetPoint(i++,-0.8,-0.5368);
      g->SetPoint(i++,-0.4,-0.0962);
      g->SetPoint(i++,-0.2,-0.0369);
      g->SetPoint(i++,-0.1,-0.0166);
      g->SetPoint(i++,0,0);
      g->SetPoint(i++,0.1,0.0135);
      g->SetPoint(i++,0.2,0.0252);
      g->SetPoint(i++,0.4,0.0430);
      g->SetPoint(i++,0.8,0.0670);
      f = new TF1("f","[0]*0.16*x/(1+[0]*x)",-1,1);
      g->SetTitle("#delta vs. B;B;#delta");
      break;
    case 9: // A=0.04, vary B, g=P_{2,0}, fix C=0.2, h=sinTheta
      g->SetPoint(i,-0.8,0.012512); g->SetPointError(i++,0,0.000024);
      g->SetPoint(i,-0.4,0.009799); g->SetPointError(i++,0,0.000026);
      g->SetPoint(i,-0.2,0.008087); g->SetPointError(i++,0,0.000017);
      g->SetPoint(i,-0.1,0.007002); g->SetPointError(i++,0,0.000011);
      //g->SetPoint(i,0,0); // not measured...
      g->SetPoint(i,0.1,0.004811); g->SetPointError(i++,0,0.000019);
      g->SetPoint(i,0.2,0.003323); g->SetPointError(i++,0,0.000003);
      g->SetPoint(i,0.4,0.000970); g->SetPointError(i++,0,0.000009);
      g->SetPoint(i,0.8,-0.006047); g->SetPointError(i++,0,0.000018);
      f = new TF1("f","0.04*(x*[0]+0.2*[1])/(1+x*[0]+0.2*[1])",-1,1);
      //f->FixParameter(0,-0.41);
      //f->FixParameter(1,0.97);
      g->SetTitle("#delta vs. B;B;#delta");
      break;
    case 10: // A=0.16, vary B, g=P_{2,0}, fix C=0.2, h=sinTheta
      g->SetPoint(i,-0.8,0.051578); g->SetPointError(i++,0,0.000133);
      g->SetPoint(i,-0.4,0.040438); g->SetPointError(i++,0,0.000121);
      g->SetPoint(i,-0.2,0.033321); g->SetPointError(i++,0,0.000112);
      g->SetPoint(i,-0.1,0.029368); g->SetPointError(i++,0,0.000105);
      //g->SetPoint(i,0,0); // not measured...
      g->SetPoint(i,0.1,0.020388); g->SetPointError(i++,0,0.000089);
      g->SetPoint(i,0.2,0.015404); g->SetPointError(i++,0,0.000079);
      g->SetPoint(i,0.4,0.003577); g->SetPointError(i++,0,0.000032);
      g->SetPoint(i,0.8,-0.025195); g->SetPointError(i++,0,0.000106);
      f = new TF1("f","0.16*(x*[0]+0.2*[1])/(1+x*[0]+0.2*[1])",-1,1);
      f->FixParameter(0,-0.41);
      f->FixParameter(1,0.97);
      g->SetTitle("#delta vs. B;B;#delta");
      break;
    case 11: // numerator: 6%Ah + 8%Ahr; denominator B*P_{2,0}; fix B=0.2
             // plot deltaAh, varying Ah
      g->SetPoint(i,-8*0.06,0.035570); g->SetPointError(i++,0,0.000413);
      g->SetPoint(i,-4*0.06,0.019987); g->SetPointError(i++,0,0.000176);
      g->SetPoint(i,-2*0.06,0.010003); g->SetPointError(i++,0,0.000087);
      g->SetPoint(i,-1*0.06,0.004982); g->SetPointError(i++,0,0.000043);
      g->SetPoint(i,0,0); g->SetPointError(i++,0,0);
      g->SetPoint(i,1*0.06,-0.005117); g->SetPointError(i++,0,0.000045);
      g->SetPoint(i,2*0.06,-0.010016); g->SetPointError(i++,0,0.000087);
      g->SetPoint(i,4*0.06,-0.019910); g->SetPointError(i++,0,0.000178);
      g->SetPoint(i,8*0.06,-0.037102); g->SetPointError(i++,0,0.000419);
      f = new TF1("f","[0]*x",-1,1);
      g->SetTitle("#delta vs. A_{h};A_{h};#delta");
      break;
    case 12: // numerator: 6%Ah + 8%Ahr; denominator B*P_{2,0}; fix B=0.2
             // plot deltaAhr, varying Ahr
      g->SetPoint(i,-8*0.08,0.053356); g->SetPointError(i++,0,0.000410);
      g->SetPoint(i,-4*0.08,0.027515); g->SetPointError(i++,0,0.000182);
      g->SetPoint(i,-2*0.08,0.013924); g->SetPointError(i++,0,0.000090);
      g->SetPoint(i,-1*0.08,0.006750); g->SetPointError(i++,0,0.000045);
      g->SetPoint(i,0,0); g->SetPointError(i++,0,0);
      g->SetPoint(i,1*0.08,-0.007078); g->SetPointError(i++,0,0.000046);
      g->SetPoint(i,2*0.08,-0.013519); g->SetPointError(i++,0,0.000089);
      g->SetPoint(i,4*0.08,-0.027645); g->SetPointError(i++,0,0.000184);
      g->SetPoint(i,8*0.08,-0.052697); g->SetPointError(i++,0,0.000406);
      f = new TF1("f","[0]*x",-1,1);
      g->SetTitle("#delta vs. A_{hr};A_{hr};#delta");
      break;
    case 13: // numerator: 6%Ah + 8%Ahr; denominator B*P_{2,0};
             // fix Ah=6% and Ahr=8%; vary B, plot deltaAh
      g->SetPoint(i,-0.8,0.013768); g->SetPointError(i++,0,0.000067);
      g->SetPoint(i,-0.4,0.007980); g->SetPointError(i++,0,0.000053);
      g->SetPoint(i,-0.2,0.004130); g->SetPointError(i++,0,0.000039);
      g->SetPoint(i,-0.1,0.002084); g->SetPointError(i++,0,0.000027);
      g->SetPoint(i,0,0); g->SetPointError(i++,0,0);
      g->SetPoint(i,0.1,-0.002351); g->SetPointError(i++,0,0.000030);
      g->SetPoint(i,0.2,-0.005117); g->SetPointError(i++,0,0.000045);
      g->SetPoint(i,0.4,-0.011000); g->SetPointError(i++,0,0.000067);
      g->SetPoint(i,0.8,-0.028745); g->SetPointError(i++,0,0.000114);
      f = new TF1("f","0.06*x*[0]/(1+x*[0])",-1,1);
      g->SetTitle("#delta_{h} vs. B;B;#delta_{h}");
      break;
    case 14: // numerator: 6%Ah + 8%Ahr; denominator B*P_{2,0};
             // fix Ah=6% and Ahr=8%; vary B, plot deltaAhr
      g->SetPoint(i,-0.8,0.018216); g->SetPointError(i++,0,0.000067);
      g->SetPoint(i,-0.4,0.010583); g->SetPointError(i++,0,0.000053);
      g->SetPoint(i,-0.2,0.005813); g->SetPointError(i++,0,0.000040);
      g->SetPoint(i,-0.1,0.003018); g->SetPointError(i++,0,0.000028);
      g->SetPoint(i,0,0); g->SetPointError(i++,0,0);
      g->SetPoint(i,0.1,-0.003313); g->SetPointError(i++,0,0.000031);
      g->SetPoint(i,0.2,-0.007078); g->SetPointError(i++,0,0.000046);
      g->SetPoint(i,0.4,-0.015701); g->SetPointError(i++,0,0.000069);
      g->SetPoint(i,0.8,-0.040176); g->SetPointError(i++,0,0.000118);
      f = new TF1("f","0.08*x*[0]/(1+x*[0])",-1,1);
      g->SetTitle("#delta_{hr} vs. B;B;#delta_{hr}");
      break;
  };

  TCanvas * c = new TCanvas();
  c->SetGrid(1,1);
  g->SetMarkerStyle(kFullCircle);
  g->Draw("APE");
  g->Fit(f,"","",-1,1);
  //f->Draw("same");
};
