#include <CanvasHelper.h>

void drawCanvas(){
    // Create canvas
    TCanvas* c = new TCanvas();

    TH1* h = new TH1D("h1","Histogram from a Gaussian",100,-10,10);
    h->FillRandom("gaus",10000);
    gStyle->SetOptFit(111); 
    h->Fit("gaus"); 

    // FEATURE 1: obtain default stat box from the canvas and align it to the left top corner
    TPave *pave = CanvasHelper::getDefaultPaveStats(c);
    CanvasHelper::setPaveAlignment(pave, CanvasHelper::kPaveAlignLeft | CanvasHelper::kPaveAlignTop);
    
    // FEATURE 2: adding a subtitle to the canvas
    CanvasHelper::addSubtitle("10k events #bullet chi-squared fit", c);

    // Draw histogram
    h->Draw();
    
    // Pass canvas for processing
    CanvasHelper::getInstance()->addCanvas(c);

    c->Modified();
    c->Update();
}

void drawCanvasWithManyPads(){
    // Instantiate and divide canvas
    TCanvas* c = new TCanvas();
    c->SetWindowSize(800, 400);
    c->Divide(2,1);

    // Draw histogram on sub pad 1
    TVirtualPad* pad1 = c->cd(1);
    TH1* h = new TH1D("h2","Histogram from a Gaussian;Coordinate, m;Events",100,-10,10);
    h->FillRandom("gaus",10000);
    h->Draw();
    TPave *pave = CanvasHelper::getDefaultPaveStats(pad1);
    CanvasHelper::setPaveAlignment(pave, CanvasHelper::kPaveAlignLeft | CanvasHelper::kPaveAlignTop);

    // Draw graph on sub pad 2
    c->cd(2);
    TGraph* g = new TGraph(h);
    g->SetTitle("Graph from ROOT Manual; Array indexes; Array values");
    g->Draw();
    
    // FEATURE 3: add title and/or subtitle to a multi-pad canvas
    CanvasHelper::addMultiCanvasTitle(c, "Multi-Pad Canvas Title", "Canvas features a ROOT histogram and a graph.");

    // Pass canvas for processing
    CanvasHelper::getInstance()->addCanvas(c);

    c->Modified();
    c->Update();
}

void test() {
    // Load CanvasHelper library
    gSystem->Load("CanvasHelper_cpp.so");

    // Canvas with a single pad
    drawCanvas();
    
    // Canvas with multiple pads
    drawCanvasWithManyPads();
    
    // Process events
}
