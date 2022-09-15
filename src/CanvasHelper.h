#ifndef CanvasHelper_HH_
#define CanvasHelper_HH_

#include <RtypesCore.h>
#include <Rtypes.h>
#include <TColor.h>
#include <TVirtualPad.h>
#include <TH1.h>
#include <THStack.h>
#include <TPaveStats.h>
#include <TLegend.h>
#include <TGraph.h>
#include <TMap.h>
#include <TQObject.h>
#include <utility>
#include <map>
#include <string>

struct Margin {
    Double_t left;
    Double_t right;
    Double_t bottom;
    Double_t top;
};

enum EFontFace {
  TimesItalic = 1,
  TimesBold = 2,
  TimesBoldItalic = 3,
  Helvetica = 4,
  HelveticaItalic = 5,
  HelveticaBold = 6,
  HelveticaBoldItalic = 7,
  Courier = 8,
  CourierItalic = 9,
  CourierBold = 10,
  CourierBoldItalic = 11,
  Symbol = 12,
  Times = 13,
  Wingdings = 14,
  SymbolItalic = 15
};

namespace Round {
int getFirstDigit();
std::pair<double, double> valueError(const double value, const double error);
void paveTextValueErrors(TPaveText *pave);
}

class CanvasHelper: public TObject {
  public:
    static CanvasHelper* getInstance();
    virtual ~CanvasHelper();

  protected:
    CanvasHelper();

    // Had to make public? or use this??
  public:
    static CanvasHelper *fgInstance;

    enum EPaveAlignBits {
      kPaveAlignLeft = BIT(14), kPaveAlignRight = BIT(15), kPaveAlignTop = BIT(16), kPaveAlignBottom = BIT(17)
    };

    // Font sizes in pixels
    static const Int_t FONT_SIZE_NORMAL;
    static const Int_t FONT_SIZE_SMALL;
    static const Int_t FONT_SIZE_SMALLER;
    static const Int_t FONT_SIZE_LARGE;
    static const Int_t FONT_SIZE_LARGER;
    static const Int_t FONT_SIZE_HUGE;

    static const Double_t LINE_HEIGHT;

//    static const Int_t AXIS_TITLE_OFFSET;
    static const Int_t AXIS_LABEL_OFFSET;
    static const Int_t AXIS_TICK_LENGTH;

    static const Int_t MARGIN_LEFT;
    static const Int_t MARGIN_TOP;
    static const Int_t MARGIN_RIGHT;
    static const Int_t MARGIN_BOTTOM;

    static const Int_t TITLE_VSPACE;
    static const Int_t SUBTITLE_VSPACE;
    static const Int_t PAVELINE_VSPACE;
    static const Int_t AXISTITLE_VSPACE;

    void addCanvas(TCanvas *canvas);

    static void addSubtitle(const char *text, TVirtualPad *pad);
    static void setPaveAlignment(TPave *pave, UInt_t align);

    static TPaveStats* getDefaultPaveStats(TVirtualPad *pad);
    static TLegend* getDefaultLegend(TVirtualPad *pad);

    static void addTextToStats(const char *text, TVirtualPad *pad);
    static void addTextToStats(const char *text, TPaveStats *stats, TVirtualPad *pad);

    static Style_t getFont(EFontFace fontFace = EFontFace::Helvetica);
    static UInt_t getPaveLines(TPave *pave);
    static UInt_t getPaveTextWidthPx(TPaveText *paveText);

    static void addMultiCanvasTitle(TCanvas *canvas, const char *title, const char *subtitle);

//    static UInt_t getTextWidthPx(TText* t);
//    static UInt_t getTextHeightPx(TText* t);
//    static std::pair<UInt_t, UInt_t> getTextSizePx(TText* t);

// Slot for canvas resizing
    void onCanvasResized();

//    void canvasReady(TCanvas*);      // *SIGNAL*
//    void exportCanvas(TCanvas* canvas, const char* filename);

  private:
    // TMap *canvasesToBeExported;
    static std::pair<Double_t, Double_t> getSubtitleYNDCCoordinates(TVirtualPad *pad);

    TList *registeredCanvases;
    TList *padsWithModifiedDivisions;
    std::map<std::string, std::pair<unsigned int, unsigned int>> registeredCanvasesSizes;

//    static TGraph* findTGraphOnPad(TVirtualPad* pad);

    static TObject* findObjectOnPad(TClass *c, TVirtualPad *pad);
    // static TObject* findObjectOnPad(const char* name, TVirtualPad* pad);

    static void alignChildPad(TVirtualPad *pad);

    static Int_t getFrameLeftMarginPx(TVirtualPad *pad);
    static Int_t getFrameRightMarginPx(/*TVirtualPad* pad*/);
    static Int_t getFrameTopMarginPx(TVirtualPad *pad);
    static Int_t getFrameBottomMarginPx(TVirtualPad *pad);

    static Bool_t hasXAxisTitle(TVirtualPad *pad);
    static Bool_t hasYAxisTitle(TVirtualPad *pad);

    static void alignTitle(TVirtualPad *pad);
    static void alignSubtitle(TVirtualPad *pad);
    static void alignSubtitle(TPaveText *subtitle, TVirtualPad *pad);
    static void alignAllPaves(TVirtualPad *pad);
    // static void alignAxisTitles(TVirtualPad* pad);

    void processCanvas(TCanvas *canvas);
    void processPad(TVirtualPad *pad);
    static void setPadMargins(TVirtualPad *pad);

    void convertAxisToPxSize(TAxis *axis, const char type, TVirtualPad *pad);

    static Double_t getPadWidthPx(TVirtualPad *pad);
    static Double_t getPadHeightPx(TVirtualPad *pad);

    static std::pair<TAxis*, TAxis*> getPadXYAxis(TVirtualPad *pad);
    static Double_t getYAxisMaxLabelWidthPx(TVirtualPad *pad);
//    Double_t getLabelHeigthPx();

    static Double_t pxToNdcHorizontal(Int_t px, TVirtualPad *pad);
    static Double_t pxToNdcVertical(Int_t py, TVirtualPad *pad);

    static constexpr char subtitleObjectName[] = "subtitle";

    static TFrame* getPadFrame(TVirtualPad *pad);

  public:ClassDef(CanvasHelper, 0)
};

#endif
