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
#include <TStyle.h>
#include <TGaxis.h>
#include <TPad.h>
#include <TSystem.h>
#include <TPRegexp.h>
#include <TLegendEntry.h>

#include <TH1.h>
#include <THStack.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TF1.h>

#include <string>
#include <iostream>
#include <sstream>
#include <limits>

#include <chrono>
#include <thread>

ClassImp(TNamedLine);

TNamedLine::TNamedLine(const char* name, Double_t x1, Double_t y1, Double_t x2, Double_t y2) : TLine(x1, y1, x2, y2){
  fName  = name;
};

TNamedLine::~TNamedLine(){};

namespace Round {
  int getFirstDigit(double number) {
    // We obtain the first digit of the error (always positive)
    // Like they teached in MEPhI
    number = std::abs(number);
    if (number == 1)
      return 1;
    if (number > 1) {
      while (number >= 10) {
        number = number / 10;
      }
    } else if (number < 1) {
      while (number < 1) {
        number = number * 10;
      }
    }
    return (int) number;
  }

  std::pair<double, double> valueError(const double value, const double error) {
    // If error is zero - return as is (avoid infinite loop)
    if (error == 0){
      return std::make_pair(value, error);
    }
    // First we find the decimal point shift
    int decimalPointShift = int(log10(error)); // This will give "-0.6" for 0.0234 error, "3" for
    // For 0 < error < 1 we need to manually shift to the right
    if (error < 1)
      decimalPointShift--;
    // MEPhI - keep second digit if first is "1"
    if (getFirstDigit(error) == 1)
      decimalPointShift--;

    // Round error
    double errorRounded = round(error * pow(10, (double) -decimalPointShift));
    double errorReverted = errorRounded * pow(10, (double) decimalPointShift);

    // Do the same operation with value
    double valueRounded = int(value * pow(10, (double) -decimalPointShift));
    double valueReverted = valueRounded * pow(10, (double) decimalPointShift);

    return std::make_pair(valueReverted, errorReverted);
  }

  void paveTextValueErrors(TPaveText *pave) {
    for (TObject *object : *(pave->GetListOfLines())) {
      if (object->InheritsFrom(TText::Class())) {
        TText *text = (TText*) object;
        TString s(text->GetTitle());

        TObjArray *subStrL = TPRegexp("^(.*=) (.*) #pm (.*)$").MatchS(s);
        const Int_t nrSubStr = subStrL->GetLast() + 1;
        if (nrSubStr == 4) {
          const TString before = ((TObjString*) subStrL->At(1))->GetString();
          const TString value = ((TObjString*) subStrL->At(2))->GetString();
          const TString error = ((TObjString*) subStrL->At(3))->GetString();
          // const TString after  = ((TObjString *)subStrL->At(4))->GetString();
          Double_t val = atof(value.Data());
          Double_t err = atof(error.Data());
          std::pair<Double_t, Double_t> pair = Round::valueError(val, err);

          std::stringstream buffer;
          buffer << before << " " << pair.first << " #pm " << pair.second;

          TString newText((buffer.str()).c_str());
          text->SetTitle(newText.Data());
        }
      }
    }
  }
}

ClassImp(CanvasHelper);

// Instance
CanvasHelper *CanvasHelper::fgInstance = nullptr;

// Constructor
CanvasHelper::CanvasHelper() {
  // Only accept resized signals from TCanvas. Child pads will also send these signals. However we want to omit them
  TQObject::Connect(TCanvas::Class_Name(), "Resized()", this->Class_Name(), this, "onCanvasResized()");
}

// Destructor
CanvasHelper::~CanvasHelper() {
  delete fgInstance;
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
// const Int_t CanvasHelper::AXIS_TITLE_OFFSET = 25;
const Int_t CanvasHelper::AXIS_TICK_LENGTH = 20;

const Int_t CanvasHelper::MARGIN_LEFT = 25;
const Int_t CanvasHelper::MARGIN_TOP = 30;       // In particular this is for TGaxis::PaintAxis() x10^3 label
const Int_t CanvasHelper::MARGIN_RIGHT = 40;     // In particular this is for TGaxis::PaintAxis() x10^3 label
const Int_t CanvasHelper::MARGIN_BOTTOM = 10;

const Int_t CanvasHelper::TITLE_VSPACE = 25;
const Int_t CanvasHelper::SUBTITLE_VSPACE = 20;
const Int_t CanvasHelper::PAVELINE_VSPACE = 22;
const Int_t CanvasHelper::AXISTITLE_VSPACE = 20;

Style_t CanvasHelper::getFont(EFontFace fontFace) {
  // Default font face is 4
  // https://root.cern.ch/doc/master/classTAttText.html#autotoc_md31
  const Int_t precision = 3; // scalable and rotatable fonts (if "1" then problems rendering, "3" - non scalable fonts look bad on big screens)
  return (Int_t) fontFace * 10 + precision;
}

std::pair<TAxis*, TAxis*> CanvasHelper::getPadXYAxis(TVirtualPad *pad) {
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
    if (object->InheritsFrom(TMultiGraph::Class())) {
      TMultiGraph *mgraph = (TMultiGraph*) object;
      return std::make_pair<TAxis*, TAxis*>(mgraph->GetXaxis(), mgraph->GetYaxis());
    }
    if (object->InheritsFrom(TF1::Class())) {
      TF1 *func = (TF1*) object;
      return std::make_pair<TAxis*, TAxis*>(func->GetXaxis(), func->GetYaxis());
    }
  }
  return std::make_pair(nullptr, nullptr);
}

Double_t CanvasHelper::getYAxisMaxLabelWidthPx(TVirtualPad *pad) {
  // Obtain axis from pad (could be histogram, Stack, Graph...)
  TAxis *xaxis = getPadXYAxis(pad).first;
  TAxis *yaxis = getPadXYAxis(pad).second;
  if (!yaxis)
    return 0;

  Double_t axisLabelMax = 100;
  Double_t axisLabelMin = 100;
  if (!pad->GetLogy()) {
    Double_t pMax = pad->GetUymax();
    Double_t pMin = pad->GetUymin();
    Int_t nDiv = yaxis->GetNdivisions();
    Int_t N2 = nDiv / 100;
    Int_t N1 = nDiv % 100;

    Double_t majorDivisionSize = std::abs(pMax - pMin);
    // We round axis maximum value to the major division step and get pretty much what the maximum label is
    std::pair<double, double> roundedMaximum = Round::valueError(pMax, majorDivisionSize);
    // Same with minimum value - this way we remove insignificant digits
    std::pair<double, double> roundedMinimum = Round::valueError(pMin, majorDivisionSize);
    axisLabelMax = roundedMaximum.first;
    axisLabelMin = roundedMinimum.first;
  }

  // Determine longest axis label
  std::stringstream bufferMin;
  bufferMin << axisLabelMin;
  std::string axisMinLabel = bufferMin.str();
  std::stringstream bufferMax;
  bufferMax << axisLabelMax;
  std::string axisMaxLabel = bufferMax.str();
  std::string axisLongestLabel =
      (Int_t) axisMinLabel.length() > (Int_t) axisMaxLabel.length() ? axisMinLabel : axisMaxLabel;

//  // Label cannot be longer than axis -> getmaxdigits()
//  if (yaxis->GetMaxDigits() != 0 && axisLongestLabel.length() > yaxis->GetMaxDigits()) {
//    axisLongestLabel = "";
//    for (int i = 0; i < yaxis->GetMaxDigits(); i++)
//      axisLongestLabel += "#";
//  }

  // Determine longest label width in pixels
  TText *t = new TText(0.5, 0.5, axisLongestLabel.c_str());
  t->SetNDC();
  t->SetTextFont(getFont());
  t->SetTextSize(FONT_SIZE_NORMAL);
  // t->SetTextSizePixels(FONT_SIZE_NORMAL);
  // t->Draw();
  UInt_t w = 0, h = 0;
  t->GetBoundingBox(w, h);
  // UInt_t w = getTextWidthPx(t);

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
  if (canvas == nullptr) return;

  // Update Pad - in case the histogram was just drawn - need to update otherwise no primitives
  // Force unconditionally paint canvas
  canvas->cd()->Paint();

  // Weird but this makes TTF::GetTextExtent() return correct value
  TText *t = new TText(1.5, 0.5, "Hi!");
  t->SetNDC();
  t->SetTextFont(getFont());
  t->SetTextSize(FONT_SIZE_NORMAL);
  canvas->GetListOfPrimitives()->Add(t);
  canvas->Paint();

  registeredCanvases.insert( { canvas, { canvas->GetWw(), canvas->GetWh() } });
  processCanvas(canvas);
}

Bool_t CanvasHelper::isChildPad(TVirtualPad *pad) {
  // If global title was added to a multi-pad canvas
  TPMERegexp name_re("_\\d+");
  if (name_re.Match(pad->GetName())) {
    return kTRUE;
  }
  // If global title was added to a multi-pad canvas
  TString motherName = pad->GetMother()->GetName();
  if (motherName.Contains("_child")) {
    return kTRUE;
  }
  return kFALSE;
}

void CanvasHelper::alignTitle(TVirtualPad *pad) {
  TObject *object = pad->GetPrimitive("title");
  if (!object || !object->InheritsFrom(TPaveText::Class()))
    return;

  TPaveText *title = (TPaveText*) object;
  title->SetTextFont(getFont());

  title->SetTextSize(isChildPad(pad) ? FONT_SIZE_NORMAL : FONT_SIZE_LARGE);

  title->SetFillStyle(kFEmpty);
  title->SetTextAlign(ETextAlign::kHAlignCenter + ETextAlign::kVAlignCenter);

//    title->SetX1NDC(pxToNdcHorizontal(getFrameLeftMarginPx(pad), pad));
//    title->SetX2NDC(1 - pxToNdcHorizontal(getFrameRightMarginPx(), pad));

  title->SetX1NDC(0);
  title->SetX2NDC(1);

  title->SetY2NDC(1 - pxToNdcVertical(MARGIN_TOP / 2 + 0, pad));

  Double_t topTitlePadding = MARGIN_TOP / 2 + TITLE_VSPACE;
  if (isChildPad(pad))
    topTitlePadding /= 2;
  title->SetY1NDC(1 - pxToNdcVertical(topTitlePadding, pad));
}

void CanvasHelper::alignSubtitle(TPaveText *subtitle, TVirtualPad *pad) {
  // subtitle->SetX1NDC(pxToNdcHorizontal(getFrameLeftMarginPx(pad), pad));
  // subtitle->SetX2NDC(1 - pxToNdcHorizontal(getFrameRightMarginPx(), pad));
  subtitle->SetX1NDC(0);
  subtitle->SetX2NDC(1);

  std::pair<Double_t, Double_t> subtitleYCoords = getSubtitleYNDCCoordinates(pad);
  subtitle->SetY1NDC(subtitleYCoords.first);
  subtitle->SetY2NDC(subtitleYCoords.second);
}

void CanvasHelper::alignSubtitle(TVirtualPad *pad) {
  TObject *object = pad->GetPrimitive("subtitle");
  if (!object || !object->InheritsFrom(TPave::Class()))
    return;

  alignSubtitle((TPaveText*) object, pad);
}

//void CanvasHelper::alignAxisTitles(TVirtualPad *pad) {
//    std::pair<TAxis*, TAxis*> axis = getPadXYAxis(pad);
//
//    if (pad->GetPrimitive("xaxistitle") != NULL) {
//        TText *xAxisTitle = (TText*) (pad->GetPrimitive("xaxistitle"));
//        xAxisTitle->SetY(pxToNdcVertical(MARGIN_BOTTOM, pad));
//        if (axis.first->GetCenterTitle()) {
//            // Centered titles
//            xAxisTitle->SetTextAlign(ETextAlign::kHAlignCenter + ETextAlign::kVAlignCenter);
//            Int_t xAxisXPx = getFrameLeftMarginPx(pad)
//                    + (getPadWidthPx(pad) - getFrameRightMarginPx() - getFrameLeftMarginPx(pad)) / 2; // centered text
//            xAxisTitle->SetX(pxToNdcHorizontal(xAxisXPx, pad));
//        } else {
//            // Right aligned titles
//            xAxisTitle->SetTextAlign(ETextAlign::kHAlignRight + ETextAlign::kVAlignCenter);
//            xAxisTitle->SetX(1 - pxToNdcHorizontal(getFrameRightMarginPx(), pad));
//        }
//    }
//    if (pad->GetPrimitive("yaxistitle") != NULL) {
//        TText *yAxisTitle = (TText*) (pad->GetPrimitive("yaxistitle"));
//        yAxisTitle->SetX(pxToNdcHorizontal(MARGIN_LEFT, pad));
//        if (axis.second->GetCenterTitle()) {
//            yAxisTitle->SetTextAlign(ETextAlign::kHAlignCenter + ETextAlign::kVAlignCenter);
//            Int_t yAxisYPx = getFrameBottomMarginPx(pad)
//                    + (getPadHeightPx(pad) - getFrameTopMarginPx(pad) - getFrameBottomMarginPx(pad)) / 2; // centered text
//            yAxisTitle->SetY(pxToNdcVertical(yAxisYPx, pad));
//        } else {
//            yAxisTitle->SetTextAlign(ETextAlign::kHAlignRight + ETextAlign::kVAlignCenter);
//            yAxisTitle->SetY(1 - pxToNdcVertical(getFrameTopMarginPx(pad), pad));
//        }
//    }
//}

std::pair<Double_t, Double_t> CanvasHelper::getSubtitleYNDCCoordinates(TVirtualPad *pad) {
  Bool_t padHasTitle = pad->GetPrimitive("title") != nullptr;

  Double_t y2 = 1 - pxToNdcVertical(MARGIN_TOP / 2 + (padHasTitle ? TITLE_VSPACE : 0), pad);
  Double_t y1 = 1 - pxToNdcVertical(MARGIN_TOP / 2 + (padHasTitle ? TITLE_VSPACE : 0) + SUBTITLE_VSPACE, pad);
  return std::make_pair(y1, y2);
}

void CanvasHelper::addSubtitle(TVirtualPad *pad, const char *text) {
  std::pair<Double_t, Double_t> subtitleYCoords = getSubtitleYNDCCoordinates(pad);
  TPaveText *subtitle = new TPaveText(0, subtitleYCoords.first, 1, subtitleYCoords.second, "NBNDC"); // ndc coordinates
  subtitle->SetTextAlign(kHAlignCenter + kVAlignCenter);
  subtitle->SetName("subtitle");
  subtitle->SetFillStyle(kFEmpty);
  subtitle->SetLineWidth(0);
  subtitle->SetTextFont(getFont());
  subtitle->SetTextSize(FONT_SIZE_NORMAL);
  subtitle->SetTextColor(kGray + 2);
  subtitle->SetFillStyle(kFEmpty);
  subtitle->SetLineWidth(0);
  subtitle->AddText(text);

  pad->cd();
  subtitle->Draw();

  // pad->GetListOfPrimitives()->Add(subtitle);        // attach subtitle to pad primitives otherwise alignment wont work
  // pad->Update();

  // subtitle->Print();

  // Tweak top margin
  setPadMargins(pad);
}

void CanvasHelper::onCanvasResized() {
  // Every Pad will emit this signal. Supposedly child canvas pads as well.
  // We need to listen to only parent canvas signal to eliminate doing things multiple times
  for (auto const &entry : registeredCanvases) {
    // Pop canvas if not existing anymore
    // if (entry.first == nullptr){
    //   registeredCanvases.erase( nullptr );
    //   continue;
    // }
    UInt_t currentWidth = entry.first->GetWw();
    UInt_t currentHeight = entry.first->GetWh();
    UInt_t oldWidth = entry.second.first;
    UInt_t oldHeight = entry.second.second;
    if (currentWidth != oldWidth || currentHeight != oldHeight) {
      processCanvas(entry.first);
      registeredCanvases[entry.first] = { currentWidth, currentHeight };
    }
  }
}

void CanvasHelper::processCanvas(TCanvas *canvas) {
  // Process canvas itself
  // std::cout << "Processing canvas \"" << canvas->GetName() << "\"" << std::endl;

  // If canvas has multi-title added, align child canvas with sub-pads
  alignChildPad(canvas);

  // Get number of child pads
//  UInt_t nSubPads = 0;
//  for (Int_t i = 1;; i++) {
//    TVirtualPad *subPad = canvas->GetPad(i);
//    // We are looking up pads via name not getPad() because getPad() may also give us "_child" pad produces via AddMultiTitle()
//    // TVirtualPad *childPad = canvas->GetPad(1);
//    if (subPad) {
//      // std::cout << "  Found child pad \"" << childPad->GetName() << "\". Processing..." << std::endl;
//      nSubPads = i;
//    } else {
//      break;
//    }
//  }

  // Find and process child pads
  for (Int_t i = 1;; i++) {
    // Check if canvas has multui title and account on it
    TPad* c = (canvas->GetPad(-1) != nullptr) ? (TPad*)canvas->GetPad(-1) : canvas;

    // If canvas has multi title then his sub pads dont belong to it, but to the "child" pad
    TVirtualPad *subPad = c->GetPad(i);
    if (subPad) {
      subPad->SetFillStyle(EFillStyle::kFEmpty);
      processPad(subPad);
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
  // gSystem->ProcessEvents();
  // gStyle->Reset();
}

void CanvasHelper::processPad(TVirtualPad *pad) {
  // Remember default left margin - related to the fact that we cannot get TGaxis from canvas
//  std::string padName = pad->GetName();
//  if (defaultPadLeftMargins.find(padName) == defaultPadLeftMargins.end()){
//    Double_t padLeftPaddingPx = pad->GetLeftMargin()*getPadWidthPx(pad);
//    Double_t padYAxisFontSize = getPadXYAxis(pad).second->GetLabel
//    defaultPadLeftMargins.insert( { padName, pad->GetLeftMargin() });
//  }
  // At this point default ROOT components are already present on the canvas
  // We are simply tweaking the sizes, distances and objects.

  // Tweak axis and add custom axis titles that don't move around when scaling
  std::pair<TAxis*, TAxis*> axis = getPadXYAxis(pad);
  convertAxisToPxSize(axis.first, 'x', pad);
  convertAxisToPxSize(axis.second, 'y', pad);
  // alignAxisTitles(pad);

  alignTitle(pad);
  alignSubtitle(pad);
  alignAllPaves(pad);
  setPadMargins(pad);
  setPadCustomFrameBorder(pad); // should go after setPadMargins();
  setPadNDivisions(pad);

  pad->Modified();
  pad->Update();
}

void CanvasHelper::alignAllPaves(TVirtualPad *pad) {
  TList *primitives = pad->GetListOfPrimitives();
  for (TObject *object : *primitives) {
    if (!object->InheritsFrom(TPave::Class()))
      continue;
    TPave *pave = (TPave*) object;

    // Do not process title, subtitle and axis titles
    TString paveName = pave->GetName();
    if (paveName.Contains("title"))
      continue;

    // Adjust font size
    if (pave->InheritsFrom(TPaveText::Class())) {
      TPaveText *paveText = (TPaveText*) pave;
      paveText->SetTextFont(getFont());
      paveText->SetTextSize(FONT_SIZE_NORMAL);
    } else if (pave->InheritsFrom(TLegend::Class())) {
      TLegend *legend = (TLegend*) pave;
//      legend->SetTextFont(getFont());
//      legend->SetTextSize(FONT_SIZE_NORMAL);
      for (TObject *obj : *(legend->GetListOfPrimitives())) {
        if (!obj->InheritsFrom(TLegendEntry::Class()))
          continue;
        TLegendEntry *entry = (TLegendEntry*) obj;
        entry->SetTextFont(getFont());
        entry->SetTextSize(FONT_SIZE_NORMAL);
      }
    }

    // Round stat value/errors
    if (pave->InheritsFrom(TPaveText::Class())) {
      TPaveText *paveText = (TPaveText*) pave;
      Round::paveTextValueErrors(paveText);
    }

    // Estimate pave width
    Double_t paveWidthPx = 300;
    if (pave->InheritsFrom(TPaveText::Class())) {
      TPaveText *paveText = (TPaveText*) pave;
      paveWidthPx = (Int_t) getPaveTextWidthPx(paveText);
    } else if (pave->InheritsFrom(TLegend::Class())) {
      TLegend *legend = (TLegend*) pave;
      paveWidthPx = (Int_t) getLegendWidthPx(legend);
      paveWidthPx = paveWidthPx * legend->GetNColumns();
    }

    if (pave->TestBit(kPaveAlignLeft)) {
      Int_t leftMargin = getFrameLeftMarginPx(pad);
      pave->SetX1NDC(pxToNdcHorizontal(leftMargin - 1, pad));  // -1 accounts on pixel perfect alignment (replace with border?)
      pave->SetX2NDC(pxToNdcHorizontal(leftMargin + paveWidthPx, pad));
    }
    if (pave->TestBit(kPaveAlignRight)) {
      Int_t rightMargin = getFrameRightMarginPx();
      pave->SetX2NDC(1 - pxToNdcHorizontal(rightMargin, pad));
      pave->SetX1NDC(1 - pxToNdcHorizontal(rightMargin + paveWidthPx, pad));
    }

    // Calculate and align height
    Int_t paveHeightPx = 100;
    if (pave->InheritsFrom(TPaveText::Class())) {
      TPaveText *paveText = (TPaveText*) pave;
      paveHeightPx = getPaveLines(pave) * PAVELINE_VSPACE;
    } else if (pave->InheritsFrom(TLegend::Class())) {
      TLegend *legend = (TLegend*) pave;
      paveHeightPx = legend->GetNRows() * PAVELINE_VSPACE;
    }

    if (pave->TestBit(kPaveAlignTop)) {
      Int_t topMargin = getFrameTopMarginPx(pad);
      pave->SetY2NDC(1 - pxToNdcVertical(topMargin - 1, pad));
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
    leftMargin += AXISTITLE_VSPACE;
  }

  // Add y axis label width
  Double_t labelWidth = getYAxisMaxLabelWidthPx(pad);
  leftMargin += labelWidth;

  // Add y axis label offset
  leftMargin += AXIS_LABEL_OFFSET;
  return leftMargin;
}

Bool_t CanvasHelper::hasXAxisTitle(TVirtualPad *pad) {
//    if (pad->GetPrimitive("xaxistitle") != nullptr)
//        return true;

  TAxis *xAxis = getPadXYAxis(pad).first;
  if (!xAxis)
    return false;
  if (strlen(xAxis->GetTitle()) > 0)
    return true;

  return false;
}

// TODO: To be deleted and tested
Bool_t CanvasHelper::hasYAxisTitle(TVirtualPad *pad) {
//    if (pad->GetPrimitive("yaxistitle") != nullptr)
//        return true;

  TAxis *yAxis = getPadXYAxis(pad).second;
  if (!yAxis)
    return false;
  if (strlen(yAxis->GetTitle()) > 0)
    return true;

  return false;
}

Int_t CanvasHelper::getFrameRightMarginPx(/*TVirtualPad *pad*/) {
  return MARGIN_RIGHT;
}

Int_t CanvasHelper::getFrameTopMarginPx(TVirtualPad *pad) {
  Int_t topMargin = isChildPad(pad) ? MARGIN_TOP / 2 : MARGIN_TOP;
  if (pad->GetPrimitive("title") != nullptr) {
    topMargin += isChildPad(pad) ? TITLE_VSPACE * 3. / 4. : TITLE_VSPACE;
  }
  if (pad->GetPrimitive("subtitle") != nullptr) {
    topMargin += SUBTITLE_VSPACE;
  }
//    if (pad->GetPrimitive("title") != nullptr || pad->GetPrimitive("subtitle") != nullptr) {
//        topMargin += TITLE_VSPACE / 2;
//    }

  return topMargin;
}

Int_t CanvasHelper::getFrameBottomMarginPx(TVirtualPad *pad) {
  Int_t bottomMargin = MARGIN_BOTTOM;
  if (hasXAxisTitle(pad)) {
    bottomMargin += AXISTITLE_VSPACE;
  }

  // Add x axis label height
  bottomMargin += FONT_SIZE_NORMAL;

  // Add x axis label offset
  bottomMargin += AXIS_LABEL_OFFSET;

  return bottomMargin;
}

void CanvasHelper::setPadMargins(TVirtualPad *pad) {
  // TFrame* frame = getPadFrame(pad);

  // Problem: child pads return Ww (width) and Wh (height) from parent's canvas object 🥵
  // Solution: wrote custom functions to get actual size
  // Double_t padWidth = getPadWidthPx(pad);
  // Double_t padHeight = getPadHeightPx(pad);

  Int_t leftMargin = getFrameLeftMarginPx(pad);
  // gStyle->SetPadLeftMargin(pxToNdcHorizontal(leftMargin, pad));
  pad->SetLeftMargin(pxToNdcHorizontal(leftMargin, pad));
  // if (frame) frame->SetX1(pxToNdcHorizontal(leftMargin, pad));

  Int_t topMargin = getFrameTopMarginPx(pad);
  // gStyle->SetPadTopMargin(pxToNdcVertical(topMargin, pad));
  pad->SetTopMargin(pxToNdcVertical(topMargin, pad));
  // if (frame) frame->SetY2(pxToNdcVertical(1-topMargin, pad));

  Int_t rightMargin = getFrameRightMarginPx();
  // gStyle->SetPadRightMargin(pxToNdcHorizontal(rightMargin, pad));
  pad->SetRightMargin(pxToNdcHorizontal(rightMargin, pad));
  // if (frame) frame->SetX2(pxToNdcHorizontal(1-rightMargin, pad));

  Int_t bottomMargin = getFrameBottomMarginPx(pad);
  // gStyle->SetPadBottomMargin(pxToNdcVertical(bottomMargin, pad));
  pad->SetBottomMargin(pxToNdcVertical(bottomMargin, pad));
  // if (frame) frame->SetY2(pxToNdcVertical(bottomMargin, pad));
}

// Function prevents double border with left (and potentially bottom axis)
// TODO: account on existing axis, add maybe left line if needed - rear case
void CanvasHelper::setPadCustomFrameBorder(TVirtualPad *pad) {
  TFrame* frame = (TFrame*)(pad->GetListOfPrimitives()->FindObject("TFrame"));
  if (!frame) return;

  // Remove pad frame background border
  if (frame->GetFillStyle() != EFillStyle::kFEmpty) frame->SetFillStyle(EFillStyle::kFEmpty);
  if (frame->GetLineWidth() != 0) frame->SetLineWidth(0);

  // Draw or update custom frame made from two lines - top and right
  // Top line
  if (pad->GetListOfPrimitives()->FindObject("frameTopLine") == nullptr){
    TNamedLine* l = new TNamedLine("frameTopLine", pad->GetLeftMargin(), 1-pad->GetTopMargin(), 1-pad->GetRightMargin(), 1-pad->GetTopMargin());
    l->SetNDC();
    pad->GetListOfPrimitives()->AddAfter(frame, l);
  } else {
    TLine* l = (TLine*)(pad->GetListOfPrimitives()->FindObject("frameTopLine"));
    l->SetX1(pad->GetLeftMargin());
    l->SetY1(1-pad->GetTopMargin());
    l->SetX2(1-pad->GetRightMargin());
    l->SetY2(1 - pad->GetTopMargin());
  }
  // Right Line
  if (pad->GetListOfPrimitives()->FindObject("frameRightLine") == nullptr){
    TNamedLine* l = new TNamedLine("frameRightLine", 1-pad->GetRightMargin(), 1-pad->GetTopMargin(), 1-pad->GetRightMargin(), pad->GetBottomMargin());
    l->SetNDC();
    pad->GetListOfPrimitives()->AddAfter(frame, l);
  } else {
    TLine* l = (TLine*)(pad->GetListOfPrimitives()->FindObject("frameRightLine"));
    l->SetX1(1-pad->GetRightMargin());
    l->SetY1(1-pad->GetTopMargin());
    l->SetX2(1-pad->GetRightMargin());
    l->SetY2(pad->GetBottomMargin());
  }

  pad->GetListOfPrimitives()->Print();
}

void CanvasHelper::setPadNDivisions(TVirtualPad *pad) {
  std::pair<TAxis*, TAxis*> axis = getPadXYAxis(pad);

  // For x axis
  if (axis.first) {
    // Get whatever minor divisions were originally (refer to documentation)
    Int_t nDivX = axis.first->GetNdivisions();
    Int_t nDivXMinor = nDivX / 100;
    // Set one major division per 75 px
    Int_t width = getPadWidthPx(pad);
    Int_t nDivXMajor = width / 75;
    axis.first->SetNdivisions(nDivXMajor, nDivXMinor, 0, kTRUE);
  }

  // For y axis
  if (axis.second) {
    // Get whatever minor divisions were originally (refer to documentation)
    Int_t nDivY = axis.second->GetNdivisions();
    Int_t nDivYMinor = nDivY / 100;
    // Set one major division per 50 px
    Int_t height = getPadHeightPx(pad);
    Int_t nDivYMajor = height / 50;
    axis.second->SetNdivisions(nDivYMajor, nDivYMinor, 0, kTRUE);
  }
}

void CanvasHelper::setPaveAlignment(TPave *pave, UInt_t align) {
  if (!pave) return;
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

void CanvasHelper::saveCanvas(TCanvas *canvas, UInt_t format) {
  // Workaround for the thick lines on the multi-pad
  // https://root-forum.cern.ch/t/lines-in-the-pdf-file-are-way-too-thick/16510
  // Line scale should be proportional to the size of the default canvas
  Double_t canvasWidth = canvas->GetWw();
  Double_t defaultCanvasWidth = gStyle->GetCanvasDefW();
  Double_t ratio = canvasWidth/defaultCanvasWidth;
  gStyle->SetLineScalePS(3./ratio);

  TString fileName = canvas->GetName();
  if ((format & kFormatC) == kFormatC) {
    canvas->SaveAs(fileName + ".c");
  }
  if ((format & kFormatPng) == kFormatPng) {
    canvas->SaveAs(fileName + ".png");
  }
  if ((format & kFormatPs) == kFormatPs) {
    canvas->SaveAs(fileName + ".ps");
  }
  if ((format & kFormatROOT) == kFormatROOT) {
    canvas->SaveAs(fileName + ".root");
  }
  if ((format & kFormatPdf) == kFormatPdf) {
    canvas->SaveAs(fileName + ".pdf");
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

TObject* CanvasHelper::findObjectOnPad(TClass *c, TVirtualPad *pad) {
  for (TObject *object : *(pad->GetListOfPrimitives())) {
    if (object->InheritsFrom(c))
      return object;
  }
  return NULL;
}

TPaveStats* CanvasHelper::getDefaultPaveStats(TVirtualPad *pad) {
  // Update Pad - in case the histogram was just drawn - need to update otherwise no primitives
  if (pad->IsModified()) {
    pad->Update();
  }

  // SCENARIO 1: Statistics box is produced by histogram. See line THistPainter.cxx:8628 "stats->SetParent(fH);"
  // https://root.cern.ch/doc/master/classTPaveStats.html#autotoc_md223
  // TObject* o = pad->GetPrimitive("stats");
  // TPaveStats* ps = (TPaveStats*)o;
  // TObject* par = ps->GetParent();
  if (pad->GetPrimitive("stats") && ((TPaveStats*)(pad->GetPrimitive("stats")))->GetParent() && ((TPaveStats*)(pad->GetPrimitive("stats")))->GetParent()->InheritsFrom("TH1")) {
    TH1 *hist = (TH1*) findObjectOnPad(TH1::Class(), pad);
    TPaveStats *pave = (TPaveStats*) (pad->GetPrimitive("stats"));
    pave->SetName("mystats");                     // rename to "mystats"
    hist->GetListOfFunctions()->Remove(pave);    // disconnect from the graph
    hist->SetStats(kFALSE); // disconnect from the histogram.
    pad->GetListOfPrimitives()->Add(pave);        // attach to pad primitives
    pave->SetParent(pad);
    return pave;
  }

  // SCENARIO 2: TGraph case. Similarly, for TGraphPainter.cxx:4437
  // https://root-forum.cern.ch/t/cant-access-the-status-box-of-tgraph/18939/6
  if (findObjectOnPad(TGraph::Class(), pad) != NULL) {
    TGraph *graph = (TGraph*) findObjectOnPad(TGraph::Class(), pad);
    if (pad->GetPrimitive("stats") && ((TPaveStats*) pad->GetPrimitive("stats"))->GetParent()
        == graph->GetListOfFunctions()) {
      TPaveStats *pave = (TPaveStats*) (pad->GetPrimitive("stats"));
      pave->SetName("mystats");                     // rename to "mystats"
      graph->GetListOfFunctions()->Remove(pave);    // disconnect from the graph
      graph->SetStats(kFALSE);
      pad->GetListOfPrimitives()->Add(pave);        // attach to pad primitives
      pave->SetParent(pad);
      return pave;
    }
  }

  // SCENARIO 3: Return stats box previously detached
  if (pad->GetPrimitive("mystats") != NULL) {
    TPaveStats *pave = (TPaveStats*) pad->GetPrimitive("mystats");
    return pave;
  }

  // Return random stats found
  TPaveStats *pave = (TPaveStats*) findObjectOnPad(TPaveStats::Class(), pad);
  if (pave != NULL)
    return pave;

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

  TList *listOfLines = stats->GetListOfLines();

  // Note that "=" is a control character
  TText *newLine = new TLatex(0, 0, text);
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
//    if (type == 'x') {
//        if (strlen(axis->GetTitle()) > 0) {
//            TText *xAxisText = new TText(0, 0, axis->GetTitle());
//            xAxisText->SetName("xaxistitle");
//            xAxisText->SetNDC();
//            xAxisText->SetTextFont(getFont());
//            xAxisText->SetTextSize(FONT_SIZE_NORMAL);
//            pad->cd();
//            xAxisText->Draw();
//            axis->SetTitle("");
//        }
//    } else {
//        if (strlen(axis->GetTitle()) > 0) {
//            TText *yAxisText = new TText(0, 0, axis->GetTitle());
//            yAxisText->SetName("yaxistitle");
//            yAxisText->SetNDC();
//            yAxisText->SetTextFont(getFont());
//            yAxisText->SetTextSize(FONT_SIZE_NORMAL);
//            yAxisText->SetTextAngle(90);
//            pad->cd();
//            yAxisText->Draw();
//            axis->SetTitle("");
//        }
//    }

  // Set tiles
  axis->SetTitleFont(getFont());
  axis->SetTitleSize(FONT_SIZE_NORMAL);
  // TODO: figure how to adjust Y axis offsset - maybe not set it at all??
  if (type == 'x')
    axis->SetTitleOffset(1.4);
  if (type == 'y') {
    Double_t titleOffsetPx = (Double_t)getFrameLeftMarginPx(pad) - (Double_t)MARGIN_LEFT + 8;
    Double_t coefficient = 30.; // Guestimated
    // Extra ratio seems to be needed - guestimated to be ratio of the frame width to pad width ???
    axis->SetTitleOffset(titleOffsetPx / coefficient);
  }

  // Style labels
  axis->SetLabelFont(getFont());
  axis->SetLabelSize(FONT_SIZE_NORMAL);
  Double_t labelOffset = AXIS_LABEL_OFFSET / (type == 'x' ? getPadHeightPx(pad) : getPadWidthPx(pad));
  axis->SetLabelOffset(labelOffset);

  // Tick lengths
  Double_t tickLength = AXIS_TICK_LENGTH / (type == 'x' ? getPadHeightPx(pad) : getPadWidthPx(pad));
  axis->SetTickLength(tickLength);
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

UInt_t CanvasHelper::getLegendWidthPx(TLegend *legend) {
  UInt_t maxTextLengthPx = 0;
  for (TObject *obj : *(legend->GetListOfPrimitives())) {
    if (!obj->InheritsFrom(TLegendEntry::Class()))
      continue;
    TLegendEntry *entry = (TLegendEntry*) obj;

    TLatex *latexCopy = new TLatex(0, 0, entry->GetLabel());
    latexCopy->SetTextFont(getFont());
    latexCopy->SetTextSize(FONT_SIZE_NORMAL);

    UInt_t w = 0, h = 0;
    latexCopy->GetBoundingBox(w, h);
    maxTextLengthPx = TMath::Max(maxTextLengthPx, w);

    // UInt_t latexCopyWidthPx = getTextWidthPx(latexCopy);
    // maxTextLengthPx = TMath::Max(maxTextLengthPx, latexCopyWidthPx);
    latexCopy->Delete();
  }
  return maxTextLengthPx + 45;
}

// TODO: To be deleted! - not working well
// https://root-forum.cern.ch/t/width-of-a-tlatex-text/20961/16
//std::pair<UInt_t, UInt_t> CanvasHelper::getTextSizePx(TText *t) {
//  UInt_t w, h;
//  Int_t f = t->GetTextFont();
//
//  if (f % 10 <= 2) {
//    t->GetTextExtent(w, h, t->GetTitle());
//  } else {
//    w = 0;
//    TText t2 = *t;
//    t2.SetTextFont(f - 1);
//    TVirtualPad *pad = gROOT->GetSelectedPad();
//    if (!pad)
//      return std::make_pair(0, 0);
//    Float_t dy = pad->AbsPixeltoY(0)
//        - pad->AbsPixeltoY((Int_t) (t->GetTextSize()));
//    Float_t tsize = dy / (pad->GetY2() - pad->GetY1());
//    t2.SetTextSize(tsize);
//    t2.GetTextExtent(w, h, t2.GetTitle());
//  }
//  return std::make_pair(w, h);
//}
//
//UInt_t CanvasHelper::getTextWidthPx(TText *t) {
//  std::pair<UInt_t, UInt_t> textSize = CanvasHelper::getTextSizePx(t);
//  return textSize.first;
//}
//
//UInt_t CanvasHelper::getTextHeightPx(TText *t) {
//  std::pair<UInt_t, UInt_t> textSize = CanvasHelper::getTextSizePx(t);
//  return textSize.second;
//}

// Signals

//void CanvasHelper::canvasReady(TCanvas* canvas){
//  Emit("canvasReady(Int_t)", canvas);
//}

void CanvasHelper::addMultiCanvasTitle(TCanvas *canvas, const char *title, const char *subtitle) {

  // If canvas has no child pads - create child pad and copy canvas into its child pad
//    if (canvas->GetPad(1) == NULL) {
  TVirtualPad *childPad = new TPad();
  TString childPadName = TString::Format("%s_child", canvas->GetName());
  childPad->SetName(childPadName.Data());
  childPad->SetFillStyle(EFillStyle::kFEmpty);
  childPad->SetCanvas(canvas);
//        canvas->cd(1);
//        if (canvas->GetGridx())
//            pad->SetGridx();
//        if (canvas->GetGridy())
//            pad->SetGridy();
  // Remove subpad margin
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
      // object->Dump();
      childPad->cd();
      if (object->InheritsFrom(TPad::Class())) {
        TPad *subPad = (TPad*) object;
        // Not deleting primitives but removing them from the list - safer.
        // https://root.cern/root/roottalk/roottalk00/2082.html
        primitives->Remove(subPad);
        subPad->Draw();
        //childPad->GetListOfPrimitives()->Add(subPad);
        // subPad->SetCanvas(childPad);
      }
    }
  }
  // canvas->AppendPad(childPad);
  // canvas->GetListOfPrimitives()->Add(childPad);
  // pad->SetPad(0, 0, 1, 1);

  // TODO: do we need this?
//    }

  canvas->cd();
  childPad->Draw();
  ((TPad*)childPad)->SetNumber(-1); // Hack - child pad will have number -1. This way we can access it without name
  // Add title text (fixed size in px)
  TPaveText *t = new TPaveText(0, 0.9, 1, 1, "NBNDC");
  t->SetName("title");
  t->AddText(title);
  t->SetFillStyle(kFEmpty);
  t->SetLineWidth(0);
  t->Draw(); // Processor will align and style it later

  // Add subtitle text
  if (strlen(subtitle) > 0) {
    addSubtitle(canvas, subtitle);
  }

  alignChildPad(canvas);

  alignTitle(canvas);
  alignSubtitle(canvas);

  canvas->Modified();
  canvas->Update();
}

void CanvasHelper::alignChildPad(TVirtualPad *canvas) {

  TVirtualPad *childPad = (TVirtualPad*) canvas->GetPad(-1);
  if (childPad == nullptr)
    return;

  Double_t childPadHeightNDC = 1 - pxToNdcVertical(getFrameTopMarginPx(canvas), canvas);
  childPad->SetPad(0, 0, 1, childPadHeightNDC);
  childPad->Modified();
  childPad->Update();
}
