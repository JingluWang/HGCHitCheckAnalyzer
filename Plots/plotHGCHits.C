#include "lib.h"

// ------------------------------- Modification Zone - begin --------------------------------
TString TEXT1 = "HGCAL Sim-Digi-Rec:";
TString TEXT2;

// Cases:
int CASE = 4; // 1 - "011"
              // 2 - "111"
              // 3 - "100"
              // 4 - "101"

// 0-1-1
TString TEXT011 = "0-1-1 (Noise)";
TString hVar1_011 = "Layer_EE011";
TString hVar2_011 = "Layer_HSi011";
TString hVar3_011 = "Layer_HSc011";

// 1-1-1
TString TEXT111 = "1-1-1 (Signal)";
TString hVar1_111 = "Layer_EE111";
TString hVar2_111 = "Layer_HSi111";
TString hVar3_111 = "Layer_HSc111";

// 1-0-0
TString TEXT100 = "1-0-0 (Zero suppressed/Error)";
TString hVar1_100 = "Layer_EE100";
TString hVar2_100 = "Layer_HSi100";
TString hVar3_100 = "Layer_HSc100";

// 1-0-1
TString TEXT101 = "1-0-1 (Unexpected)";
TString hVar1_101 = "Layer_EE101";
TString hVar2_101 = "Layer_HSi101";
TString hVar3_101 = "Layer_HSc101";


// Input rootfiles & Legends
TString input1 = "../SimDigiRecMapping/test/outputRoot/test.root", LEGEND1 = "HGCalEE";
TString input2 = "../SimDigiRecMapping/test/outputRoot/test.root", LEGEND2 = "HGCalHSi";
TString input3 = "../SimDigiRecMapping/test/outputRoot/test.root", LEGEND3 = "HGCalHSc";

// Axis
float ScaleY = 1.2;
TString YAXIS = "Hits";
TString XAXIS = "Layer";

// Hists
TH1D *HIST1;
TH1D *HIST2;
TH1D *HIST3;

// Var
TString hVar1;
TString hVar2;
TString hVar3;

// Output label
TString casetag;
TString LABEL;
// ------------------------------- Modification Zone - end ----------------------------------



// --------------- Functions - begin -----------------
// QuickHist: draw one Hist 
TH1D *QuickHist(TString inputfileName, TString histName) {
    TFile *f = new TFile( inputfileName );
    TDirectory *folder = (TDirectory*) f->Get("ana");
    TH1D *HIST = (TH1D*) folder->Get( histName );
    HIST->SetDirectory(0);
    HIST->SetStats(0);
    HIST->SetTitle(0);
    HIST->SetLineWidth(2);

    return HIST;
} 
// --------------- Functions - end -----------------



///////////////
// --- Main ---
void plotHGCHits() {
    // Cases
    switch ( CASE ) {
        case 1: hVar1   = hVar1_011;// 1 - "011"
                hVar2   = hVar2_011; 
                hVar3   = hVar3_011; 
                ScaleY  = 1.2;
                TEXT2   = TEXT011;
                casetag = "011";
                break;
        case 2: hVar1   = hVar1_111;// 2 - "111"
                hVar2   = hVar2_111; 
                hVar3   = hVar3_111; 
                ScaleY  = 1.2;
                TEXT2   = TEXT111;
                casetag = "111";
                break;
        case 3: hVar1   = hVar1_100;// 3 - "100"
                hVar2   = hVar2_100; 
                hVar3   = hVar3_100; 
                ScaleY  = 2.8;
                TEXT2   = TEXT100;
                casetag = "100";
                break;
        case 4: hVar1   = hVar1_101;// 4 - "101"
                hVar2   = hVar2_101; 
                hVar3   = hVar3_101; 
                ScaleY  = 1.2;
                TEXT2   = TEXT101;
                casetag = "101";
                break;
    }

    LABEL = "figures/Case_" + casetag + ".png";

    HIST1 = QuickHist( input1, hVar1 );
    HIST1->GetXaxis()->SetTitle( XAXIS );// X-axis label
    HIST1->GetXaxis()->SetLabelSize(0.03);
    HIST1->GetXaxis()->SetLabelFont(62);
    HIST1->GetXaxis()->SetTitleSize(30);
    HIST1->GetXaxis()->SetTitleFont(63);
    HIST1->GetYaxis()->SetTitle( YAXIS );// Y-axis label
    HIST1->GetYaxis()->SetLabelFont(62);
    HIST1->GetYaxis()->SetLabelSize(0.03);
    HIST1->GetYaxis()->SetTitleSize(30);
    HIST1->GetYaxis()->SetTitleFont(63);

    HIST2 = QuickHist( input2, hVar2 );
    HIST3 = QuickHist( input3, hVar3 );


    // -------- Colors --------

    HIST1->SetLineColor(kRed);
    HIST2->SetLineColor(kViolet);
    HIST3->SetLineColor(kBlue);

    double mmax, t, MAX;
    mmax = HIST1->GetBinContent(1);
    for(int i = 2; i <= 50; i++)
    {
        t = HIST1->GetBinContent(i);
        if(mmax < t) {mmax = t;}
    }
    MAX = ScaleY*mmax;
    HIST1->SetMaximum(MAX);

    // drawing
    TCanvas *c = new TCanvas("c", "", 900, 800);
    c->SetMargin(0.17, 0.12, 0.23, 0.12);
    c->cd();

    HIST1->Draw("HIST");
    HIST2->Draw("HISTsame");
    HIST3->Draw("HISTsame");

    // Hists' legends
    TLegend *legend = new TLegend(0.65, 0.7, 0.85, 0.85);
    legend->AddEntry(HIST1, LEGEND1);// hist1 legend
    legend->AddEntry(HIST2, LEGEND2);// hist2 legend
    legend->AddEntry(HIST3, LEGEND3);// hist3 legend
    legend->SetFillColor(0);
    legend->SetFillStyle(0);
    legend->SetLineColor(0);
    legend->SetLineWidth(0);
    legend->SetTextSize(0.03);
    legend->SetTextFont(62);
    legend->Draw("same");

    TLatex *text1 = new TLatex(0.19, 0.83, TEXT1);// text for explanation
    text1->SetNDC();
    text1->SetTextSize(0.04);
    text1->Draw("same");
    TLatex *text2 = new TLatex(0.19, 0.79, TEXT2);
    text2->SetNDC();
    text2->SetTextSize(0.03);
    text2->Draw("same");
/*
    TLatex *text3 = new TLatex(0.19, 0.75, TEXT3);
    text3->SetNDC();
    text3->SetTextSize(0.03);
    text3->Draw("same");
*/
    c->Update();
    TGaxis *axis1 = new TGaxis(c->GetUxmax(), c->GetUymin(), c->GetUxmax(), c->GetUymax(), 0, MAX, 510, "+L");
    axis1->SetLineColor(kBlack);
    axis1->SetLabelSize(0);
    axis1->Draw();

    c->Update();
    TGaxis *axis2 = new TGaxis(c->GetUxmin(), c->GetUymax(), c->GetUxmax(), c->GetUymax(), 0, MAX, 510, "-");
    axis2->SetLineColor(kBlack);
    axis2->SetLabelSize(0);
    axis2->Draw();
/*
    TPad *pad = new TPad("pad", "pad", 0, 0, 1, 0.245);
    pad->SetTopMargin(1);
    pad->SetBottomMargin(0.38);
    pad->SetLeftMargin(0.13);
    pad->SetRightMargin(0.04);
    c->cd();
    pad->Draw("same");
    pad->cd();
*/
    c->SaveAs( LABEL );// output label
}
