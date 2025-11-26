#include "CanvasHelper.h"

#include <TApplication.h>
#include <TCanvas.h>
#include <TVirtualPad.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TH1.h>
#include <TF1.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TRandom3.h>

void demo1() {

  // Instantiate and divide canvas
  TCanvas *c = new TCanvas();
  c->SetWindowSize(850, 650);

  // Divide canvas and leave no spacing between pads (looks better)
  c->Divide(2, 2, 1E-5, 1E-5);

  // PAD 1: draw histogram and fit it
  TVirtualPad *pad1 = c->cd(1);
  TH1 *h = new TH1D("hist", "Histogram from a Gaussian;Coordinate, m;Events", 100, -5, 15);
  h->FillRandom("gaus", 10000);
  h->Draw();
  gStyle->SetOptFit(111);
  h->Fit("gaus");

  // FEATURE 1: obtain default statistics box from the canvas and align it
  TPave *pave = CanvasHelper::getDefaultPaveStats(pad1);
  CanvasHelper::setPaveAlignment(pave, kPaveAlignRight | kPaveAlignTop);

  // PAD 2: draw a TMultiGraph from ROOt manual
  // https://root.cern.ch/doc/master/classTMultiGraph.html
  c->cd(2);
  /*
  TGraph *g = new TGraph(h);
  g->SetTitle("Graph from ROOT Manual; Array indexes; Array values");
  g->Draw();
  */
  {
      auto mg = new TMultiGraph("mg","mg");

      const Int_t size = 10;

      double px[size];
      double py1[size];
      double py2[size];
      double py3[size];

      for ( int i = 0; i <  size ; ++i ) {
         px[i] = i;
         py1[i] = size - i;
         py2[i] = size - 0.5 * i;
         py3[i] = size - 0.6 * i;
      }

      auto gr1 = new TGraph( size, px, py1 );
      gr1->SetName("gr1");
      gr1->SetTitle("graph 1");
      gr1->SetMarkerStyle(21);
      gr1->SetDrawOption("AP");
      gr1->SetLineColor(2);
      gr1->SetLineWidth(4);
      gr1->SetFillStyle(0);

      auto gr2 = new TGraph( size, px, py2 );
      gr2->SetName("gr2");
      gr2->SetTitle("graph 2");
      gr2->SetMarkerStyle(22);
      gr2->SetMarkerColor(2);
      gr2->SetDrawOption("P");
      gr2->SetLineColor(3);
      gr2->SetLineWidth(4);
      gr2->SetFillStyle(0);

      auto gr3 = new TGraph( size, px, py3 );
      gr3->SetName("gr3");
      gr3->SetTitle("graph 3");
      gr3->SetMarkerStyle(23);
      gr3->SetLineColor(4);
      gr3->SetLineWidth(4);
      gr3->SetFillStyle(0);

      mg->Add(gr1);
      mg->Add(gr2);
      // Line below is confusing - ROOT has issue with building legend for MultiGraph
      // Workaround is to Draw() last graph, instead of Add()
      gr3->Draw("ALP");
      mg->Draw("LP");

      TVirtualPad* p2 = c->cd(2);
      p2->BuildLegend();

      TLegend *legend = CanvasHelper::getDefaultLegend(p2);
      CanvasHelper::setPaveAlignment(legend, kPaveAlignRight | kPaveAlignTop);
  }
  // PAD 3: draw a function
  c->cd(3);
  TF1 *fa1 = new TF1("fa1", "sin(x)/x", 0, 10);
  fa1->GetXaxis()->SetTitle("X coordinate");
  fa1->GetYaxis()->SetTitle("Function value");
  fa1->Draw();

  // PAD 4: draw a function
  TVirtualPad *pad4 = c->cd(4);
  gStyle->SetOptStat(0);

  auto h1 = new TH1F("h1", "Histogram 1;X axis;Y axis", 100, -4, 4);
  auto h2 = new TH1F("h2", "Histogram 2", 100, -4, 4);

  TRandom3 rng;
  Double_t px, py;
  for (Int_t i = 0; i < 25000; i++) {
    rng.Rannor(px, py);
    h1->Fill(px, 10.);
    h2->Fill(px, 8.);
  }

  h1->SetMarkerStyle(kFullCircle);
  h2->SetMarkerStyle(kFullSquare);

  h1->Draw("PLC PMC");
  h2->Draw("SAME PLC PMC");

  pad4->BuildLegend();
  h1->SetTitle("Two Histograms");

  // FEATURE 2: align a legend
  TLegend *legend = CanvasHelper::getDefaultLegend(pad4);
  CanvasHelper::setPaveAlignment(legend, kPaveAlignLeft | kPaveAlignTop);

  // FEATURE 3: add title and/or subtitle to a multi-pad canvas
  CanvasHelper::addMultiCanvasTitle(c, "Multi-Pad Canvas Title");

  // FEATURE 4: add subtitle to a siongle canvas
  CanvasHelper::addSubtitle(c, "Subtitle can be added to a single canvas or multi-pad canvas");

  // Pass canvas for processing
  CanvasHelper::getInstance()->addCanvas(c);

  // Save Canvas to PNG and PS
  CanvasHelper::saveCanvas(c, kFormatPng | kFormatPs);
}

void demo2() {

  // Instantiate and divide canvas
  TCanvas *c = new TCanvas();
  c->SetWindowSize(850, 650);

  // Divide canvas and leave no spacing between pads (looks better)
  c->Divide(1, 2, 1E-5, 1E-5);

  // PAD 1: draw histogram and fit it
  TVirtualPad *pad1 = c->cd(1);
  TH1 *h1 = new TH1D("d2h1", "Histogram from a Gaussian;Coordinate, m;Events", 100, -5, 15);
  h1->FillRandom("gaus", 10000);
  h1->Draw();

  TPave *pave1 = CanvasHelper::getDefaultPaveStats(pad1);
  CanvasHelper::setPaveAlignment(pave1, kPaveAlignRight | kPaveAlignTop);

  // PAD 2: draw a graph
  c->cd(2);
  TH1 *h2 = new TH1D("d2h2", "Histogram from a Gaussian;Coordinate, m;Events", 100, 3, 15);
  h2->FillRandom("gaus", 10000);
  h2->Draw();

  TPave *pave2 = CanvasHelper::getDefaultPaveStats(pad1);
  CanvasHelper::setPaveAlignment(pave2, kPaveAlignRight | kPaveAlignTop);

  // Pass canvas for processing
  CanvasHelper::getInstance()->addCanvas(c);
}

void canvasHelperDemo() {
  #ifdef __CINT__
    // Load CanvasHelper library
    gSystem->Load("CanvasHelper");
  #endif

  demo1();
  demo2();
}

#ifndef __CINT__
  int main(int argc, char **argv) {
    // Instantiate TApplication
    TApplication app("demo", &argc, argv);

    // TODO: Call the ROOT macro entry function
    canvasHelperDemo();

    // Enter the event loop
    app.Run();
    return 0;
  }
#endif
