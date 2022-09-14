#include "CanvasHelper.h"
#include <Rtypes.h>
#include <TROOT.h>
#include <TPad.h>
#include <TCanvas.h>
#include <THistPainter.h>
#include <TObjString.h>
#include <TLatex.h>
#include <TFrame.h>
#include <TPaveText.h>
#include <TMath.h>
#include <TPRegexp.h>
#include <iostream>
#include <sstream>
#include <iterator>

ClassImp(CanvasHelper);

namespace Round {
    int getFirstDigit(double number){
        // We obtain the first digit of the error (always positive)
        // Like they teached in MEPhI
        number = std::abs(number);
        if (number == 1) return 1;
        if (number > 1){
            while(number >= 10){
                number = number/10;
            }
        }
        else if (number < 1){
            while(number < 1){
                number = number*10;
            }
        }
        return (int)number;
    }

    std::pair<double, double> valueError(const double value, const double error){
        // First we find the decimal point shift
        int decimalPointShift = int(log10(error)); // This will give "-0.6" for 0.0234 error, "3" for
        // For 0 < error < 1 we need to manually shift to the right
        if (error < 1) decimalPointShift--;
        // MEPhI - keep second digit if first is "1"
        if (getFirstDigit(error) == 1) decimalPointShift--;

        // Round error
        double errorRounded = round(error*pow(10, (double)-decimalPointShift));
        double errorReverted = errorRounded*pow(10, (double)decimalPointShift);

        // Do the same operation with value
        double valueRounded = int(value*pow(10, (double)-decimalPointShift));
        double valueReverted = valueRounded*pow(10, (double)decimalPointShift);

        return std::make_pair(valueReverted, errorReverted);
    }

    void paveTextValueErrors(TPaveText* pave){
        for(TObject* object: *(pave->GetListOfLines())){
            if (object->InheritsFrom(TText::Class())){
                TText* text = (TText*) object;
                TString s(text->GetTitle());

                TObjArray *subStrL = TPRegexp("^(.*=) (.*) #pm (.*)$").MatchS(s);
                const Int_t nrSubStr = subStrL->GetLast()+1;
                if (nrSubStr == 4) {
                  const TString before = ((TObjString *)subStrL->At(1))->GetString();
                  const TString value  = ((TObjString *)subStrL->At(2))->GetString();
                  const TString error  = ((TObjString *)subStrL->At(3))->GetString();
                  // const TString after  = ((TObjString *)subStrL->At(4))->GetString();
                  Double_t val = atof(value.Data());
                  Double_t err = atof(error.Data());
                  std::pair<Double_t, Double_t> pair = Round::valueError(val, err);

                  std::stringstream buffer;
                  buffer << before << " " << pair.first << " #pm " << pair.second;

                  TString newText = buffer.str().c_str();
                  text->SetTitle(newText.Data());
                }
            }
        }
    }
}

// Instance
CanvasHelper *CanvasHelper::fgInstance = nullptr;

// Constructor
CanvasHelper::CanvasHelper() {
    // canvasesToBeExported = new TMap();
    registeredCanvases = new TList();

    // Only accept resized signals from TCanvas. Child pads will also send these signals
    // However we want to omit them because they will duplicate the functinoality
    std::cout << TCanvas::Class_Name() << " " << this->Class_Name() << std::endl;
    TQObject::Connect(TCanvas::Class_Name(), "Resized()", this->Class_Name(), this, "onCanvasResized()");
}

// Destructor
CanvasHelper::~CanvasHelper() {
}

// Instance provider
CanvasHelper* CanvasHelper::getInstance() {
    static CanvasHelper instance;
    if (fgInstance == nullptr) {
        fgInstance = &instance;
    }
    return fgInstance;
}

// All sizes in pixels
const Int_t CanvasHelper::FONT_SIZE_NORMAL = 16;
const Int_t CanvasHelper::FONT_SIZE_SMALL = FONT_SIZE_NORMAL * 3. / 4.;
const Int_t CanvasHelper::FONT_SIZE_SMALLER = FONT_SIZE_SMALL * 3. / 4.;
const Int_t CanvasHelper::FONT_SIZE_LARGE = FONT_SIZE_NORMAL * 4. / 3.;
const Int_t CanvasHelper::FONT_SIZE_LARGER = FONT_SIZE_LARGE * 4. / 3.;
const Int_t CanvasHelper::FONT_SIZE_HUGE = FONT_SIZE_LARGER * 4. / 3.;

constexpr Double_t CanvasHelper::LINE_HEIGHT = 1.2;

// const Double_t CanvasHelper::AXIS_TITLE_OFFSET = 64;
const Int_t CanvasHelper::AXIS_LABEL_OFFSET = 6;
const Int_t CanvasHelper::AXIS_TICK_LENGTH = 10;

const Int_t CanvasHelper::MARGIN_LEFT = 25;
const Int_t CanvasHelper::MARGIN_TOP = 15;
const Int_t CanvasHelper::MARGIN_RIGHT = 30;
const Int_t CanvasHelper::MARGIN_BOTTOM = 25;

const Int_t CanvasHelper::TITLE_VSPACE = 25;
const Int_t CanvasHelper::SUBTITLE_VSPACE = 25;
const Int_t CanvasHelper::PAVELINE_VSPACE = 30;
const Int_t CanvasHelper::AXISTITLE_VSPACE = 30;

Style_t CanvasHelper::getFont(EFontFace fontFace) {
    // https://root.cern.ch/doc/master/classTAttText.html#autotoc_md31
    const Int_t precision = 3; // scalable and rotatable fonts (if "1" then problems rendering, "3" - non scalable fonts look bad on big screens)
    return (Int_t) fontFace * 10 + precision;
}

std::pair<TAxis*, TAxis*> CanvasHelper::getXYAxis(TVirtualPad *pad) {
    TList *primitives = pad->GetListOfPrimitives();
    TListIter next(primitives);
    TObject *object;
    while ((object = next())) {
        if (object->InheritsFrom(TH1::Class())) {
            TH1 *hist = (TH1*) object;
            return std::make_pair<TAxis*, TAxis*>(hist->GetXaxis(), hist->GetYaxis());
        }
        if (object->InheritsFrom(THStack::Class())) {
            THStack *stack = (THStack*) object;
            TAxis *yAxis = stack->GetYaxis();
            // Fix TStack not having Y axis limits
            yAxis->SetLimits(stack->GetMinimum(), stack->GetMaximum());
            return std::make_pair<TAxis*, TAxis*>(stack->GetXaxis(), stack->GetYaxis());
        }
        if (object->InheritsFrom(TGraph::Class())) {
            TGraph *graph = (TGraph*) object;
            return std::make_pair<TAxis*, TAxis*>(graph->GetXaxis(), graph->GetYaxis());
        }
        // TODO: for TMultiGraph
    }
    return std::make_pair(nullptr, nullptr);
}

Double_t CanvasHelper::getYAxisMaxLabelWidthPx(TVirtualPad *pad) {
    // Obtain axis from pad (could be histogram, Stack, Graph...)
    TAxis *axis = getXYAxis(pad).first;
    if (!axis)
        return 0;

//    TH1* hist = (TH1*)findObjectOnPad(TH1::Class(), pad);
//    if (hist != NULL){
//        hist->GetListOfFunctions()->Print();
//        pad->GetListOfPrimitives()->Print();
//    }
//
//    TFrame* frame = (TFrame*)findObjectOnPad(TFrame::Class(), pad);
//    if (frame != NULL){
//        frame->Get
//    }

    // Determine longest axis label
    Int_t axisMin = pad->GetLogy() ? 100 : axis->GetXmin();
    Int_t axisMax = pad->GetLogy() ? 100 : axis->GetXmax();
    std::stringstream bufferMin;
    bufferMin << axisMin;
    std::string axisMinLabel = bufferMin.str();
    std::stringstream bufferMax;
    bufferMax << axisMax;
    std::string axisMaxLabel = bufferMax.str();
    std::string axisLongestLabel =
            (Int_t) axisMinLabel.length() > (Int_t) axisMaxLabel.length() ? axisMinLabel : axisMaxLabel;

    // Determine longest label width in pixels
    TText *t = new TText(0.5, 0.5, axisLongestLabel.c_str());
    t->SetNDC();
    t->SetTextFont(getFont());
    t->SetTextSize(FONT_SIZE_NORMAL);
    UInt_t w = 0, h = 0;
    t->GetBoundingBox(w, h);
    // UInt_t w = getTextWidthPx(t);

    // Return longest label width
    return w;
}

// Todo - improve like done for
//Double_t CanvasHelper::getLabelHeigthPx() {
//    TText *t = new TText(0, 0, "1");
//    t->SetTextFont(getFont());
//    t->SetTextSize(FONT_SIZE_NORMAL);
////    UInt_t w = 0, h = 0;
////    t->GetBoundingBox(w, h);
//    UInt_t textHeight = getTextHeightPx(t);
//    // Return label height
//    return textHeight;
//}

Double_t CanvasHelper::getPadWidthPx(TVirtualPad *virtualPad) {
    if (virtualPad->InheritsFrom(TCanvas::Class())) {
        return virtualPad->GetWw();
    }
    if (virtualPad->InheritsFrom(TPad::Class())) {
        Double_t parentCanvasWidth = virtualPad->GetWw();
        Double_t widthRatio = virtualPad->GetAbsWNDC();
        return parentCanvasWidth * widthRatio;
    }
    return 0;
}

Double_t CanvasHelper::getPadHeightPx(TVirtualPad *virtualPad) {
    if (virtualPad->InheritsFrom(TCanvas::Class())) {
        return virtualPad->GetWh();
    }
    if (virtualPad->InheritsFrom(TPad::Class())) {
        Double_t parentCanvasHeight = virtualPad->GetWh();
        Double_t heightRatio = virtualPad->GetAbsHNDC();
        return parentCanvasHeight * heightRatio;
    }
    return 0;
}

void CanvasHelper::addCanvas(TCanvas *canvas) {
    registeredCanvases->Add(canvas);
    registeredCanvasesSizes.insert(std::make_pair(canvas->GetName(),std::make_pair(canvas->GetWw(), canvas->GetWh())));
    canvas->Modified(kTRUE);
    processCanvas(canvas);

    // Refresh canvas - copied code from the ROOT GUI Refresh button
    canvas->Paint();
    canvas->Update();
}

void CanvasHelper::alignTitle(TVirtualPad *pad) {
    TObject *object = pad->GetPrimitive("title");
    if (!object->InheritsFrom(TPaveText::Class()))
        return;

    TPaveText *title = (TPaveText*) object;
    title->SetTextFont(getFont());
    title->SetTextSize(FONT_SIZE_LARGE);
    title->SetFillStyle(kFEmpty);

    title->SetX1NDC(pxToNdcHorizontal(getFrameLeftMarginPx(pad), pad));
    title->SetX2NDC(1 - pxToNdcHorizontal(getFrameRightMarginPx(), pad));

    title->SetY2NDC(1 - pxToNdcVertical(MARGIN_TOP, pad));
    title->SetY1NDC(1 - pxToNdcVertical(MARGIN_TOP + TITLE_VSPACE, pad));
}

void CanvasHelper::alignSubtitle(TVirtualPad *pad) {
    TObject *object = pad->GetPrimitive("subtitle");
    if (!object->InheritsFrom(TPaveText::Class()))
        return;

    TPaveText *subtitle = (TPaveText*) object;

    subtitle->SetX1NDC(pxToNdcHorizontal(getFrameLeftMarginPx(pad), pad));
    subtitle->SetX2NDC(1 - pxToNdcHorizontal(getFrameRightMarginPx(), pad));

    Bool_t padHasTitle = pad->GetPrimitive("title") != nullptr;
    subtitle->SetY2NDC(1 - pxToNdcVertical(MARGIN_TOP + (padHasTitle ? TITLE_VSPACE : 0), pad));
    subtitle->SetY1NDC(1 - pxToNdcVertical(MARGIN_TOP + (padHasTitle ? TITLE_VSPACE : 0) + SUBTITLE_VSPACE, pad));
}

void CanvasHelper::alignAxisTitles(TVirtualPad *pad) {
    std::pair<TAxis*, TAxis*> axis = getXYAxis(pad);

    if (pad->GetPrimitive("xaxistitle") != NULL) {
        TText *xAxisTitle = (TText*) (pad->GetPrimitive("xaxistitle"));
        xAxisTitle->SetY(pxToNdcVertical(MARGIN_BOTTOM, pad));
        if (axis.first->GetCenterTitle()){
            // Centered titles
            xAxisTitle->SetTextAlign(ETextAlign::kHAlignCenter + ETextAlign::kVAlignCenter);
            Int_t xAxisXPx = getFrameLeftMarginPx(pad)
                    + (getPadWidthPx(pad) - getFrameRightMarginPx() - getFrameLeftMarginPx(pad)) / 2; // centered text
            xAxisTitle->SetX(pxToNdcHorizontal(xAxisXPx, pad));
        }
        else {
            // Right aligned titles
            xAxisTitle->SetTextAlign(ETextAlign::kHAlignRight + ETextAlign::kVAlignCenter);
            xAxisTitle->SetX(1 - pxToNdcHorizontal(getFrameRightMarginPx(), pad));
        }
    }
    if (pad->GetPrimitive("yaxistitle") != NULL) {
        TText *yAxisTitle = (TText*) (pad->GetPrimitive("yaxistitle"));
        yAxisTitle->SetX(pxToNdcHorizontal(MARGIN_LEFT, pad));
        if (axis.second->GetCenterTitle()){
            yAxisTitle->SetTextAlign(ETextAlign::kHAlignCenter + ETextAlign::kVAlignCenter);
            Int_t yAxisYPx = getFrameBottomMarginPx(pad)
                    + (getPadHeightPx(pad) - getFrameTopMarginPx(pad) - getFrameBottomMarginPx(pad)) / 2; // centered text
            yAxisTitle->SetY(pxToNdcVertical(yAxisYPx, pad));
        }
        else {
            yAxisTitle->SetTextAlign(ETextAlign::kHAlignRight + ETextAlign::kVAlignCenter);
            yAxisTitle->SetY(1-pxToNdcVertical(getFrameTopMarginPx(pad), pad));
        }
    }
}

void CanvasHelper::addSubtitle(const char *text, TVirtualPad *pad) {
    TPaveText *subtitle = new TPaveText(0, 1, 0, 1, "NBNDC"); // no border, ndc coordinates
    subtitle->SetTextAlign(kHAlignCenter + kVAlignCenter);
    subtitle->SetName("subtitle");
    pad->cd();
    subtitle->SetTextFont(getFont());
    // subtitle->SetTextFont(EFontFace::HelveticaBold);
    subtitle->SetTextSize(FONT_SIZE_NORMAL);
    subtitle->SetTextColor(kGray + 2);
    subtitle->SetFillStyle(kFEmpty);
    subtitle->SetLineWidth(0);
    subtitle->AddText(text);
    pad->GetListOfPrimitives()->Add(subtitle);        // attach subtitle to pad primitives otherwise alignment wont work
    alignSubtitle(pad); // just in case - sometimes showed up not centered

    subtitle->Draw();
}

void CanvasHelper::onCanvasResized() {
    // Every Pad will emit this signal. Supposedly child canvas pads as well.
    // We need to listen to only parent canvas signal to eliminate doing things multiple times

//  TSeqCollection* canvases = gROOT->GetListOfCanvases();
//  if (canvases == nullptr) return;

    TIter next(registeredCanvases);
    TObject *object;
    while ((object = next())) {
        if (object != nullptr && object->InheritsFrom(TCanvas::Class())) {
            TCanvas *canvas = (TCanvas*) object;

            // Only process canvas if its size changed. Otherwise we would process all canvases upon one resize...
            UInt_t newWidth = canvas->GetWw();
            UInt_t newHeight = canvas->GetWh();
            UInt_t oldWidth = registeredCanvasesSizes[canvas->GetName()].first;
            UInt_t oldHeight = registeredCanvasesSizes[canvas->GetName()].second;
            if (newWidth != oldWidth || newHeight != oldHeight){
                processCanvas(canvas);
                registeredCanvasesSizes[canvas->GetName()] = std::make_pair(newWidth, newHeight);
            }
        }
    }
}

void CanvasHelper::processCanvas(TCanvas *canvas) {
    // Process canvas itself
    // if (canvas->IsModified()) {
    std::cout << "Processing canvas \"" << canvas->GetName() << "\"" << std::endl;
    // }

    // canvas->Paint();
    // canvas->Update();

    // Find and process child pads
    UInt_t nChildPads = 0;
    for (Int_t i = 1;; i++) {
        TString childPadName = TString::Format("%s_%d", canvas->GetName(), i);
        TPad *childPad = (TPad*) gROOT->FindObject(childPadName);
        if (childPad) {
            std::cout << "  Found child pad \"" << childPadName << "\". Processing..." << std::endl;
            nChildPads++;
            processPad(childPad);

            childPad->GetListOfPrimitives()->Print();

            // Quicker redraw
            childPad->SetFillStyle(EFillStyle::kFEmpty);
            childPad->Modified();
            childPad->Paint();
            childPad->Update();
        } else {
            break;
        }
    }

    // Update pad itself only if it has no child pads - wrong
//    if (nChildPads == 0){
//        std::cout << "  No child pads. Processing Canvas itself..." << std::endl;
//        processPad(canvas);
//    } else {
//        std::cout << "  Found " << nChildPads << " child pads. Not processing main canvas." << std::endl;
//    }

    // Update Canvas itself because it may contain title and subtitle
    processPad(canvas);

    // Check if this particular canvas needed to be saved
//  TObjString* canvasFileName = (TObjString*)canvasesToBeExported->FindObject(canvas);
//  if (canvasFileName != nullptr){
//    // Save canvas
//    doExportCanvas(canvas, canvasFileName->GetString().Data());
//    // Pop canvas from saved
//    canvasesToBeExported->Remove(canvas);
//  }
}

void CanvasHelper::processPad(TVirtualPad *pad) {
    // At this point default ROOT components are already present on the canvas
    // We are simply tweaking the sizes, distances and objects.

    // Tweak axis and add custom axis titles that don't move around when scaling
    std::pair<TAxis*, TAxis*> axis = getXYAxis(pad);
    convertAxisToPxSize(axis.first, 'x', pad);
    convertAxisToPxSize(axis.second, 'y', pad);
    alignAxisTitles(pad);

    // Tweak title
    TObject *object = pad->GetPrimitive("title");
    if (object != nullptr && object->InheritsFrom(TPaveText::Class())) {
        TPaveText *title = (TPaveText*) object;
        alignTitle(pad);
    }

    // Align subtitle if exists
    object = pad->GetPrimitive("subtitle");
    if (object != nullptr && object->InheritsFrom(TPaveText::Class())) {
        TPaveText *subtitle = (TPaveText*) object;
        alignSubtitle(pad);
    }

    // Align stats and legends
    alignAllPaves(pad);


    setPadMargins(pad);

    // pad->Modified();
    //pad->Paint();
    // pad->Update();
    //pad->Update();
}

void CanvasHelper::alignAllPaves(TVirtualPad *pad) {
    TList *primitives = pad->GetListOfPrimitives();
    for (TObject *object : *primitives) {
        if (!object->InheritsFrom(TPave::Class()))
            continue;
        TPave *pave = (TPave*) object;

        // Do not process title, subtitle and axis titles
        TString paveName = pave->GetName();
        if (paveName.Contains("title")) continue;

        // Adjust font size
        if (pave->InheritsFrom(TPaveText::Class())) {
            TPaveText *paveText = (TPaveText*) pave;
            paveText->SetTextFont(getFont());
            paveText->SetTextSize(FONT_SIZE_NORMAL);
        }

        // Round stat value/errors
        if (pave->InheritsFrom(TPaveText::Class())) {
            TPaveText *paveText = (TPaveText*) pave;
            Round::paveTextValueErrors(paveText);
        }

        // Estimate pave width (TODO - for legend)
        Double_t paveWidthPx = 300;
        if (pave->InheritsFrom(TPaveText::Class())) {
            TPaveText *paveText = (TPaveText*) pave;
            paveWidthPx = (Int_t) getPaveTextWidthPx(paveText);
        }

        if (pave->TestBit(kPaveAlignLeft)) {
            Int_t leftMargin = getFrameLeftMarginPx(pad);
            pave->SetX1NDC(pxToNdcHorizontal(leftMargin, pad));
            pave->SetX2NDC(pxToNdcHorizontal(leftMargin + paveWidthPx, pad));
        }
        if (pave->TestBit(kPaveAlignRight)) {
            pave->SetX2NDC(1 - pxToNdcHorizontal(MARGIN_RIGHT, pad));
            pave->SetX1NDC(1 - pxToNdcHorizontal(MARGIN_RIGHT + paveWidthPx, pad));
        }
        Int_t paveHeightPx = getPaveLines(pave) * PAVELINE_VSPACE;
        if (pave->TestBit(kPaveAlignTop)) {
            Int_t topMargin = getFrameTopMarginPx(pad);
            pave->SetY2NDC(1 - pxToNdcVertical(topMargin, pad));
            pave->SetY1NDC(1 - pxToNdcVertical(topMargin + paveHeightPx, pad));
        }
        if (pave->TestBit(kPaveAlignBottom)) {
            Int_t bottomMargin = getFrameBottomMarginPx(pad);
            pave->SetY1NDC(pxToNdcVertical(bottomMargin, pad));
            pave->SetY2NDC(pxToNdcVertical(bottomMargin + paveHeightPx, pad));
        }
    }
}

Int_t CanvasHelper::getFrameLeftMarginPx(TVirtualPad *pad) {
    Int_t leftMargin = MARGIN_LEFT;
    // Add y axis title offset
    if (hasYAxisTitle(pad)) {
        leftMargin += SUBTITLE_VSPACE;
        leftMargin += TITLE_VSPACE / 2;
    }
    // Add y axis label width
    Double_t labelWidth = getYAxisMaxLabelWidthPx(pad);
    leftMargin += labelWidth;

    return leftMargin;
}

// TODO: To be deleted!
Bool_t CanvasHelper::hasXAxisTitle(TVirtualPad *pad) {
    if (pad->GetPrimitive("xaxistitle") != nullptr)
        return true;

    TAxis *xAxis = getXYAxis(pad).first;
    if (!xAxis) return false;
    if (strlen(xAxis->GetTitle()) > 0)
        return true;

    return false;
}

// TODO: To be deleted and tested
Bool_t CanvasHelper::hasYAxisTitle(TVirtualPad *pad) {
    if (pad->GetPrimitive("yaxistitle") != nullptr)
        return true;

    TAxis *yAxis = getXYAxis(pad).second;
    if (!yAxis) return false;
    if (strlen(yAxis->GetTitle()) > 0)
        return true;

    return false;
}

Int_t CanvasHelper::getFrameRightMarginPx(/*TVirtualPad *pad*/) {
    return MARGIN_RIGHT;
}

Int_t CanvasHelper::getFrameTopMarginPx(TVirtualPad *pad) {
    Int_t topMargin = MARGIN_TOP;
    if (pad->GetPrimitive("title") != nullptr) {
        topMargin += TITLE_VSPACE;
    }
    if (pad->GetPrimitive("subtitle") != nullptr) {
        topMargin += SUBTITLE_VSPACE;
    }
    if (pad->GetPrimitive("title") != nullptr || pad->GetPrimitive("subtitle") != nullptr) {
        topMargin += TITLE_VSPACE / 2;
    }
    return topMargin;
}

Int_t CanvasHelper::getFrameBottomMarginPx(TVirtualPad *pad) {
    Int_t bottomMargin = MARGIN_BOTTOM;
    if (hasXAxisTitle(pad)) {
        bottomMargin += SUBTITLE_VSPACE;
        bottomMargin += TITLE_VSPACE * 3. / 4.;
    }
    return bottomMargin;
}

void CanvasHelper::setPadMargins(TVirtualPad *pad) {
    // TFrame* frame = getPadFrame(pad);

    // Problem: child pads return Ww (width) and Wh (height) from parent's canvas object 🥵
    // Solution: wrote custom functions to get actual size
    // Double_t padWidth = getPadWidthPx(pad);
    // Double_t padHeight = getPadHeightPx(pad);

    Int_t leftMargin = getFrameLeftMarginPx(pad);
    pad->SetLeftMargin(pxToNdcHorizontal(leftMargin, pad));
    // if (frame) frame->SetX1(pxToNdcHorizontal(leftMargin, pad));

    Int_t topMargin = getFrameTopMarginPx(pad);
    pad->SetTopMargin(pxToNdcVertical(topMargin, pad));
    // if (frame) frame->SetY2(pxToNdcVertical(1-topMargin, pad));

    Int_t rightMargin = getFrameRightMarginPx();
    pad->SetRightMargin(pxToNdcHorizontal(rightMargin, pad));
    // if (frame) frame->SetX2(pxToNdcHorizontal(1-rightMargin, pad));

    Int_t bottomMargin = getFrameBottomMarginPx(pad);
    pad->SetBottomMargin(pxToNdcVertical(bottomMargin, pad));
    // if (frame) frame->SetY2(pxToNdcVertical(bottomMargin, pad));
}

void CanvasHelper::setPaveAlignment(TPave *pave, UInt_t align) {
    // Bitwise operands in C++
    // https://www.geeksforgeeks.org/bitwise-operators-in-c-cpp/
    // https://stackoverflow.com/questions/21257165/bitwise-how-can-i-check-if-a-binary-number-contains-another
    if ((align & kPaveAlignLeft) == kPaveAlignLeft) {
        pave->SetBit(EPaveAlignBits::kPaveAlignRight, kFALSE);
        pave->SetBit(align, kTRUE);
    }
    if ((align & kPaveAlignRight) == kPaveAlignRight) {
        pave->SetBit(EPaveAlignBits::kPaveAlignLeft, kFALSE);
        pave->SetBit(kPaveAlignRight, kTRUE);
    }
    if ((align & kPaveAlignTop) == kPaveAlignTop) {
        pave->SetBit(EPaveAlignBits::kPaveAlignBottom, kFALSE);
        pave->SetBit(kPaveAlignTop, kTRUE);
    }
    if ((align & kPaveAlignBottom) == kPaveAlignBottom) {
        pave->SetBit(EPaveAlignBits::kPaveAlignTop, kFALSE);
        pave->SetBit(kPaveAlignBottom, kTRUE);
    }
}

//Bool_t paveBelongsToHistogram(TPave* pave){
//    return pave->GetParent()->InheritsFrom("TH1")
//
//}

//TGraph* CanvasHelper::findTGraphOnPad(TVirtualPad* pad){
//    for (TObject* object: *(pad->GetListOfPrimitives())){
//        if (object->InheritsFrom(TGraph::Class())){
//            TGraph* graph = (TGraph*)object;
//            return graph;
//        }
//    }
//    return NULL;
//}

TObject* CanvasHelper::findObjectOnPad(TClass* c, TVirtualPad* pad){
    for (TObject* object: *(pad->GetListOfPrimitives())){
        if (object->InheritsFrom(c)) return object;
    }
    return NULL;
}

TPaveStats* CanvasHelper::getDefaultPaveStats(TVirtualPad *pad) {
    // Update Pad - in case the histogram was just drawn - need to update
    pad->Update();

    // SCENARIO 1: Statistics box is produced by histogram. See line THistPainter.cxx:8628 "stats->SetParent(fH);"
    // https://root.cern.ch/doc/master/classTPaveStats.html#autotoc_md223
    if (pad->GetPrimitive("stats") && ((TPaveStats*)pad->GetPrimitive("stats"))->GetParent()->InheritsFrom("TH1")){
        TPaveStats *pave = (TPaveStats*)(pad->GetPrimitive("stats"));
        pave->SetName("mystats");                     // rename to "mystats"
        ((TH1*) pave->GetParent())->SetStats(kFALSE); // disconnect from the histogram.
        pad->GetListOfPrimitives()->Add(pave);        // attach to pad primitives
        return pave;
    }

    // SCENARIO 2: TGraph case. Similarly, for TGraphPainter.cxx:4437
    // https://root-forum.cern.ch/t/cant-access-the-status-box-of-tgraph/18939/6
    if (findObjectOnPad(TGraph::Class(), pad) != NULL){
        TGraph* graph = (TGraph*)findObjectOnPad(TGraph::Class(), pad);
        if (pad->GetPrimitive("stats") && ((TPaveStats*)pad->GetPrimitive("stats"))->GetParent()==graph->GetListOfFunctions()){
            TPaveStats *pave = (TPaveStats*)(pad->GetPrimitive("stats"));
            pave->SetName("mystats");                     // rename to "mystats"
            graph->GetListOfFunctions()->Remove(pave);    // disconnect from the graph
            graph->SetStats(kFALSE);
            pad->GetListOfPrimitives()->Add(pave);        // attach to pad primitives
            return pave;
        }
    }

    // SCENARIO 3: Return stats box previously detached
    if (pad->GetPrimitive("mystats") != NULL){
        TPaveStats* pave = (TPaveStats*)pad->GetPrimitive("mystats");
        return pave;
    }

    // Return random stats found
    TPaveStats* pave = (TPaveStats*)findObjectOnPad(TPaveStats::Class(), pad);
    if (pave != NULL) return pave;

    return NULL;
}

TLegend* CanvasHelper::getDefaultLegend(TVirtualPad *pad) {
    // ROOT creates legend always with name "TPave" - thats how we find it on the pad
    TLegend *legend = (TLegend*) pad->FindObject("TPave");
    return legend;
}

void CanvasHelper::addTextToStats(const char *text, TVirtualPad *pad) {
    // Retrieve the stat box
    TPaveStats *stats = getDefaultPaveStats(pad);
    addTextToStats(text, stats, pad);
}

void CanvasHelper::addTextToStats(const char *text, TPaveStats *stats, TVirtualPad *pad) {
    if (!stats)
        return;

    TList* listOfLines = stats->GetListOfLines();

    // Note that "=" is a control character
    TText* newLine = new TLatex(0, 0, text);
    newLine->SetTextAlign(0);
    newLine->SetTextFont(0);
    newLine->SetTextSize(0);
    newLine->SetTextColor(0);
    listOfLines->Add(newLine);

    pad->Modified();
}

void CanvasHelper::convertAxisToPxSize(TAxis *axis, const char type, TVirtualPad *pad) {
    if (axis == nullptr)
        return;

    // Determine pad size in pixels
    Double_t padWidth = getPadWidthPx(pad);
    Double_t padHeight = getPadHeightPx(pad);

    // Style axis title - font, alignment, offset
//  axis->SetTitleFont(getFont());
//  axis->SetTitleSize(FONT_SIZE_NORMAL);
//  axis->CenterTitle(true);
//  if (type == 'x'){
//    // TODO: try to estimate the width of the TAxisPainter's asis from canvas primitive
//    Double_t offset = TITLE_OFFSET/padHeight*20;
//    axis->SetTitleOffset(offset);
//  }
//  else {
//    Double_t yAxisMaxLabelWidth = getYAxisMaxLabelWidthPx(pad);
//    Double_t offset = (TITLE_OFFSET+yAxisMaxLabelWidth)/padWidth*20;
//    axis->SetTitleOffset(offset);
//  }
    if (type == 'x') {
        if (strlen(axis->GetTitle()) > 0) {
            TText *xAxisText = new TText(0, 0, axis->GetTitle());
            xAxisText->SetName("xaxistitle");
            xAxisText->SetNDC();
            xAxisText->SetTextFont(getFont());
            xAxisText->SetTextSize(FONT_SIZE_NORMAL);
            pad->cd();
            xAxisText->Draw();
            axis->SetTitle("");
        }
    } else {
        if (strlen(axis->GetTitle()) > 0) {
            TText *yAxisText = new TText(0, 0, axis->GetTitle());
            yAxisText->SetName("yaxistitle");
            yAxisText->SetNDC();
            yAxisText->SetTextFont(getFont());
            yAxisText->SetTextSize(FONT_SIZE_NORMAL);
            yAxisText->SetTextAngle(90);
            pad->cd();
            yAxisText->Draw();
            axis->SetTitle("");
        }
    }

    // Style labels - font and offset
    axis->SetLabelFont(getFont());
    axis->SetLabelSize(FONT_SIZE_NORMAL);
    if (type == 'x') {
        Double_t offset = AXIS_LABEL_OFFSET / padHeight;
        axis->SetLabelOffset(offset);
    } else {
        Double_t offset = AXIS_LABEL_OFFSET / padWidth;
        axis->SetLabelOffset(offset);
    }

    // Set tick length
    if (type == 'x') {
        Double_t length = AXIS_TICK_LENGTH / padHeight * 2;
        axis->SetTickLength(length);
    } else {
        Double_t length = AXIS_TICK_LENGTH / padWidth * 2;
        axis->SetTickLength(length);
    }
}

Double_t CanvasHelper::pxToNdcHorizontal(Int_t px, TVirtualPad *pad) {
    Double_t padWidth = getPadWidthPx(pad);
    return px / padWidth;
}

Double_t CanvasHelper::pxToNdcVertical(Int_t py, TVirtualPad *pad) {
    Double_t padHeight = getPadHeightPx(pad);
    return py / padHeight;
}

TFrame* CanvasHelper::getPadFrame(TVirtualPad *pad) {
    for (TObject *object : *(pad->GetListOfPrimitives())) {
        if (object->InheritsFrom(TFrame::Class_Name())) {
            TFrame *frame = (TFrame*) object;
            return frame;
        }
    }
    return NULL;
}

UInt_t CanvasHelper::getPaveLines(TPave *pave) {
    if (TPaveText *paveText = dynamic_cast<TPaveText*>(pave)) {
        return paveText->GetListOfLines()->GetSize();
    } else if (TLegend *legend = dynamic_cast<TLegend*>(pave)) {
        return legend->GetNRows();
    }
    return 1;
}

UInt_t CanvasHelper::getPaveTextWidthPx(TPaveText *paveText) {
    UInt_t maxTextLengthPx = 0;
    for (TObject *obj : *(paveText->GetListOfLines())) {
        if (!obj->InheritsFrom(TLatex::Class()))
            continue;
        TLatex *latex = (TLatex*) obj;
        // TODO: try without cloning?
        TLatex *latexCopy = (TLatex*) latex->Clone(TString::Format("%s_copy", latex->GetName()));
        latexCopy->SetTextFont(getFont());
        latexCopy->SetTextSize(FONT_SIZE_NORMAL);

        UInt_t w = 0, h = 0;
        latexCopy->GetBoundingBox(w, h);
        maxTextLengthPx = TMath::Max(maxTextLengthPx, w);

        // UInt_t latexCopyWidthPx = getTextWidthPx(latexCopy);
        // maxTextLengthPx = TMath::Max(maxTextLengthPx, latexCopyWidthPx);
        latexCopy->Delete();
    }
    return maxTextLengthPx + 25;
}

// TODO: To be deleted! - not working well
// https://root-forum.cern.ch/t/width-of-a-tlatex-text/20961/16
//std::pair<UInt_t, UInt_t> CanvasHelper::getTextSizePx(TText *t) {
//	UInt_t w, h;
//	Int_t f = t->GetTextFont();
//
//	if (f % 10 <= 2) {
//		t->GetTextExtent(w, h, t->GetTitle());
//	} else {
//		w = 0;
//		TText t2 = *t;
//		t2.SetTextFont(f - 1);
//		TVirtualPad *pad = gROOT->GetSelectedPad();
//		if (!pad)
//			return std::make_pair(0, 0);
//		Float_t dy = pad->AbsPixeltoY(0)
//				- pad->AbsPixeltoY((Int_t) (t->GetTextSize()));
//		Float_t tsize = dy / (pad->GetY2() - pad->GetY1());
//		t2.SetTextSize(tsize);
//		t2.GetTextExtent(w, h, t2.GetTitle());
//	}
//	return std::make_pair(w, h);
//}
//
//UInt_t CanvasHelper::getTextWidthPx(TText *t) {
//	std::pair<UInt_t, UInt_t> textSize = CanvasHelper::getTextSizePx(t);
//	return textSize.first;
//}
//
//UInt_t CanvasHelper::getTextHeightPx(TText *t) {
//	std::pair<UInt_t, UInt_t> textSize = CanvasHelper::getTextSizePx(t);
//	return textSize.second;
//}

// Signals

//void CanvasHelper::canvasReady(TCanvas* canvas){
//  Emit("canvasReady(Int_t)", canvas);
//}

void CanvasHelper::addMultiCanvasTitle(TCanvas *canvas, const char *title, const char *subtitle) {
    // Define heights for title and subtitle (in pixels)
    const Int_t titleHeightPx = 40;
    const Int_t subtitleHeightPx = 25;

    Int_t titlePadHeight = titleHeightPx + (subtitle == 0 ? 0 : subtitleHeightPx);

    Int_t canvasHeightPx = canvas->GetWh();
    Double_t titlePadHeightNDC = (Double_t) titlePadHeight / (Double_t) canvasHeightPx;

    // If canvas has no child pads - create child pad and copy canvas into its child pad
    if (canvas->GetPad(1) == NULL) {
        canvas->Divide(1, 1);
        TVirtualPad *pad = canvas->cd(1);
        if (canvas->GetGridx()) pad->SetGridx();
        if (canvas->GetGridy()) pad->SetGridy();
        pad->SetPad(0, 0, 1, 1); // Remove subpad margin
        // canvas->DrawClonePad(); // Clone on its own subpad? - pad becomes not active

        // Delete all primitives but keep subpads
//        TList *primitives = canvas->GetListOfPrimitives();
//        if (primitives) {
//            TListIter next(primitives);
//            TObject *object;
//            while ((object = next())) {
//                if (!object->InheritsFrom(TPad::Class())) {
//                    // Not deleting primitives but removing them from the list - safer.
//                    // https://root.cern/root/roottalk/roottalk00/2082.html
//                    primitives->Remove(object);
//                }
//            }
//        }

        // Move all primitives keeping their objects - key to success
        TList *primitives = canvas->GetListOfPrimitives();
        if (primitives) {
            TListIter next(primitives);
            TObject *object;
            while ((object = next())) {
                if (!object->InheritsFrom(TPad::Class())) {
                    // Not deleting primitives but removing them from the list - safer.
                    // https://root.cern/root/roottalk/roottalk00/2082.html
                    primitives->Remove(object);
                    pad->GetListOfPrimitives()->Add(object);
                }
            }
        }


        // TODO: do we need this?
        canvas->Modified();
        canvas->Update();
    }

    // Resize child canvases to free space on to for title
    // If Canvas has child pads
    for (Int_t i = 1; canvas->GetPad(i) != NULL; i++) {
        TVirtualPad *childPad = canvas->GetPad(i);
        Double_t yLow = childPad->GetAbsYlowNDC();
        Double_t xLow = childPad->GetAbsXlowNDC();
        Double_t height = childPad->GetAbsHNDC();
        Double_t width = childPad->GetAbsWNDC();
        childPad->SetPad(xLow, yLow * (1 - titlePadHeightNDC), xLow + width, (yLow + height) * (1 - titlePadHeightNDC));
        canvas->Modified();
        canvas->Update();
    }

    // Add extra pad for the title on top
    canvas->cd();

    // Add title text (fixed size in px)
    TPaveText *t = new TPaveText(0, 0.9, 1, 1, "NBNDC");
    t->SetName("title");
    t->AddText(title);
    t->SetFillStyle(kFEmpty);
    t->SetLineWidth(0);
    t->Draw(); // Processor will align and style it later

    // Add subtitle text
    if (subtitle != 0) {
        addSubtitle(subtitle, canvas);
    }

    canvas->Modified();
    canvas->Update();
}