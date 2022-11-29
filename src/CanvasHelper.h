#ifndef CanvasHelper_HH_
#define CanvasHelper_HH_

#include <RtypesCore.h>
#include <Rtypes.h>
#include <TColor.h>
#include <TVirtualPad.h>
#include <THStack.h>
#include <TPaveStats.h>
#include <TLegend.h>
#include <TMap.h>
#include <TNamed.h>
#include <TLine.h>
#include <TQObject.h>

#include <utility>
#include <map>
#include <string>

/**
 * @namespace Round
 * Used for rounding parameter values in the ROOT statistics box to the first (or second) value of the corresponding error
 */
namespace Round {
  int getFirstDigit();
  std::pair<double, double> valueError(const double value, const double error);
  void paveTextValueErrors(TPaveText *pave);
}

/**
 * Enum is used to set alignment options (bits) for ROOT stats boxes and legends.
 * Horizontal and vertical emum variables can be combined.
 */
enum EPaveAlignBits {
  kPaveAlignLeft = BIT(14),    ///< align pave to the left frame border
  kPaveAlignRight = BIT(15),   ///< align pave to the right frame border
  kPaveAlignTop = BIT(16),     ///< align pave to the top frame border
  kPaveAlignBottom = BIT(17)   ///< align pave to the bottom frame border
};

/**
 * Enum is used to specify file formats to export canvas to. Formats can be passed as bit manipulation:
 *
 * @code{.cpp}
 * CavasHelper::saveCanvas(myCanvas, kCanvasPng | kCanvasPs);
 * @endcode*
 */
enum ECanvasFormatBits {
  kFormatPng = BIT(14),      ///< save canvas as .png
  kFormatPs = BIT(15),       ///< save canvas as .ps
  kFormatROOT = BIT(16),     ///< save canvas as .root
  kFormatC = BIT(17)         ///< save canvas as .c
};

/**
 * @class TNamedLine TNamedLine.h "TNamedLine.h"
 * ROOT TLine that has a name to access it on the pad
 */
class TNamedLine: public TLine {
  protected:
    TString fName;

  public:
    TNamedLine(const char *name, Double_t x1, Double_t y1, Double_t x2, Double_t y2);
    ~TNamedLine();

    const char *GetName() const override { return fName.Data(); }

  ClassDef(TNamedLine, 1)
};

/**
 * @class CanvasHelper CanvasHelper.h "CanvasHelper.h"
 * One and only library class represented by a singleton.
 */
class CanvasHelper: public TObject {
  public:
    /**
     * @brief Obtain an instance of the CanvasHelper class.
     *
     * @code{.cpp}
     * CanvasHelper* canvasHelper = CavasHelper::getInstance();
     * @endcode
     */
    static CanvasHelper* getInstance();

    /**
     * @brief Class destructor.
     */
    virtual ~CanvasHelper();

    /**
     * @brief Register your canvas for processing.
     * Canvas needs to be added after all the primitives are drawn on it.
     * @param canvas Canvas to be processed.
     *
     * @code{.cpp}
     * CavasHelper::getInstance()->addCanvas(myCanvas);
     * @endcode
     */
    void addCanvas(TCanvas *canvas);

    /**
     * @brief Add subtitle to the canvas.
     * lololo
     * @param pad Canvas object.
     * @param text Subtitle text. Can use TLatex syntax.
     *
     * @code{.cpp}
     * CavasHelper::addSubtitle(myCanvas, "Sample Subtitle Text");
     * @endcode
     */
    static void addSubtitle(TVirtualPad *pad, const char *text);

    /**
     * @brief Used to specify TPave's alignment on the ROOT canvas.
     * Emum variables can be combined:
     * @param pave Statistic box, legend or other TPave
     * @param align Binary combination of EPaveAlignBits
     *
     * @code{.cpp}
     * TPave *pave = CanvasHelper::getDefaultPaveStats(myCanvas);
     * CanvasHelper::setPaveAlignment(pave, kPaveAlignLeft | kPaveAlignTop);
     * @endcode
     *
     */
    static void setPaveAlignment(TPave *pave, UInt_t align);

    /**
     * @brief Used to obtain default ROOT statistics box from a canvas with histogram, graph etc.
     * Finds TPave in the histogram (graph) list of primitives. Detaches TPave and attaches to the canvas. Returns a pointer to the TPave.
     * @param pad Canvas or a sub-pad containing statistics box.
     *
     * @code{.cpp}
     * TPaveStats *pave = CanvasHelper::getDefaultPaveStats(myCanvas);
     * @endcode
     *
     */
    static TPaveStats* getDefaultPaveStats(TVirtualPad *pad);

    /**
     * @brief Used to obtain default ROOT legend from a canvas.
     * @param pad Canvas or a sub-pad containing the legend.
     *
     * @code{.cpp}
     * TLegend *pave = CanvasHelper::getDefaultLegend(myCanvas);
     * @endcode
     *
     */
    static TLegend* getDefaultLegend(TVirtualPad *pad);

    /**
     * @brief Used to add a line to the default canvas (pad) statistics box.
     * @param text String of text to be added. Use `=` as a spacer between left and right-aligned substrings. TLatex is also supported.
     * @param pad Canvas or a sub-pad containing the legend.
     *
     * @code{.cpp}
     * CanvasHelper::addTextToStats("Resolution, % = 10.1 #pm 0.2", myCanvas);
     * @endcode
     */
    static void addTextToStats(const char *text, TVirtualPad *pad);

    /**
     * @brief Used to add a line to a custom specific statistics box on a canvas (pad).
     * @param text String of text to be added.
     * @param stats Specific statistics box to add a line to.
     * @param pad Canvas or a sub-pad containing the legend.
     *
     * @code{.cpp}
     * CanvasHelper::addTextToStats("Resolution, % = 10.1 #pm 0.2", myStatBox, myCanvas);
     * @endcode
     */
    static void addTextToStats(const char *text, TPaveStats *stats, TVirtualPad *pad);

    /**
     * @brief Adding a multi-pad canvas title. Optionally supprts subtitle too.
     * @param canvas ROOT canvas divided into a number of sub-pads.
     * @param title String to be used as title.
     * @param subtitle String to be used as sub-title.
     */
    static void addMultiCanvasTitle(TCanvas *canvas, const char *title, const char *subtitle = "");

    /**
     * Function saves canvas to disk in certain format. Formats are manipulated as bits:
     *
     * @code{.cpp}
     * CavasHelper::saveCanvas(myCanvas, kCanvasPng | kCanvasPs | kCanvasRoot | kCanvasC);
     * @endcode*
     */
    static void saveCanvas(TCanvas *canvas, UInt_t format);

  protected:
    CanvasHelper();
    static CanvasHelper *fgInstance;

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

    static Style_t getFont(EFontFace fontFace = EFontFace::Helvetica);
    static UInt_t getPaveLines(TPave *pave);
    static UInt_t getPaveTextWidthPx(TPaveText *paveText);
    static UInt_t getLegendWidthPx(TLegend *paveText);

    // TMap *canvasesToBeExported;
    static std::pair<Double_t, Double_t> getSubtitleYNDCCoordinates(TVirtualPad *pad);

    std::map<TCanvas*, std::pair<unsigned int, unsigned int>> registeredCanvases;
//    std::map<std::string, double> defaultPadLeftMargins;
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

    static Bool_t isChildPad(TVirtualPad *pad);

    static void alignTitle(TVirtualPad *pad);
    static void alignSubtitle(TVirtualPad *pad);
    static void alignSubtitle(TPaveText *subtitle, TVirtualPad *pad);
    static void alignAllPaves(TVirtualPad *pad);
    // static void alignAxisTitles(TVirtualPad* pad);

    void processCanvas(TCanvas *canvas);
    void processPad(TVirtualPad *pad);
    static void setPadMargins(TVirtualPad *pad);

    static void setPadNDivisions(TVirtualPad *pad);
    void convertAxisToPxSize(TAxis *axis, const char type, TVirtualPad *pad);

    static void setPadCustomFrameBorder(TVirtualPad *pad);

    static Double_t getPadWidthPx(TVirtualPad *pad);
    static Double_t getPadHeightPx(TVirtualPad *pad);

    static std::pair<TAxis*, TAxis*> getPadXYAxis(TVirtualPad *pad);
    static Double_t getYAxisMaxLabelWidthPx(TVirtualPad *pad);
//    Double_t getLabelHeigthPx();

    static Double_t pxToNdcHorizontal(Int_t px, TVirtualPad *pad);
    static Double_t pxToNdcVertical(Int_t py, TVirtualPad *pad);

    static constexpr char subtitleObjectName[] = "subtitle";

    static TFrame* getPadFrame(TVirtualPad *pad);

  public:
    // Slot for canvas resizing (need to be public)
    void onCanvasResized();

  ClassDef(CanvasHelper, 0)
};

#endif
