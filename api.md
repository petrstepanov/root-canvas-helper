# Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`namespace `[`Round`](#namespaceRound) | Used for rounding parameter values in the ROOT statistics box to the first (or second) value of the corresponding error
`class `[`CanvasHelper`](#classCanvasHelper) | One and only library class represented by a singleton.
`struct `[`CanvasHelper::Margin`](#structCanvasHelper_1_1Margin) | 

# namespace `Round` 

Used for rounding parameter values in the ROOT statistics box to the first (or second) value of the corresponding error

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public int `[`getFirstDigit`](#namespaceRound_1ab2d6343e7efc0ccbefa551fe79d079e0)`(double number)`            | 
`public std::pair< double, double > `[`valueError`](#namespaceRound_1a7cc0fb2fdf1a71c197f521bfe58c3897)`(const double value,const double error)`            | 
`public void `[`paveTextValueErrors`](#namespaceRound_1a80a15730c6bc82808f54b67ef57bd7b3)`(TPaveText * pave)`            | 
`public int `[`getFirstDigit`](#namespaceRound_1a8d224aaaa4e2ff8a5263d122db01e1af)`()`            | 

## Members

#### `public int `[`getFirstDigit`](#namespaceRound_1ab2d6343e7efc0ccbefa551fe79d079e0)`(double number)` 

#### `public std::pair< double, double > `[`valueError`](#namespaceRound_1a7cc0fb2fdf1a71c197f521bfe58c3897)`(const double value,const double error)` 

#### `public void `[`paveTextValueErrors`](#namespaceRound_1a80a15730c6bc82808f54b67ef57bd7b3)`(TPaveText * pave)` 

#### `public int `[`getFirstDigit`](#namespaceRound_1a8d224aaaa4e2ff8a5263d122db01e1af)`()` 

# class `CanvasHelper` 

```
class CanvasHelper
  : public TObject
```  

One and only library class represented by a singleton.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public virtual  `[`~CanvasHelper`](#classCanvasHelper_1a3622f03eea23e8b740f12f8177c74cc3)`()` | Class destructor.
`public void `[`addCanvas`](#classCanvasHelper_1ade239b7879bfff344d7da95aba9f133a)`(TCanvas * canvas)` | Register your canvas for processing. Canvas needs to be added after all the primitives are drawn on it.
`protected TList * `[`registeredCanvases`](#classCanvasHelper_1adb839ce36e92c083085c98a6281d5c0f) | 
`protected TList * `[`padsWithModifiedDivisions`](#classCanvasHelper_1a8caf2468fddc852c4788002a31d48ba7) | 
`protected std::map< std::string, std::pair< unsigned int, unsigned int > > `[`registeredCanvasesSizes`](#classCanvasHelper_1aa72d6e76dece731169383d2028c00d34) | 
`protected  `[`CanvasHelper`](#classCanvasHelper_1a356c54078dad9e58aca74100b841ff64)`()` | 
`protected void `[`onCanvasResized`](#classCanvasHelper_1a605d386b4c62cf6cd53d22283697c371)`()` | 
`protected void `[`processCanvas`](#classCanvasHelper_1ac440d59386e589466077251c808f2fd1)`(TCanvas * canvas)` | 
`protected void `[`processPad`](#classCanvasHelper_1a753bf2e1951bf20a0728a99968d2cb06)`(TVirtualPad * pad)` | 
`protected void `[`convertAxisToPxSize`](#classCanvasHelper_1a9e6b6a6b35ef73e1e40ffa6d0d1853bb)`(TAxis * axis,const char type,TVirtualPad * pad)` | 
`enum `[`EPaveAlignBits`](#classCanvasHelper_1aed359e291be7f893286cbef53e08b574) | Used to set alignment options (bits) for ROOT stats boxes and legends. Horizontal and vertical emum variables can be combined.

## Members

#### `public virtual  `[`~CanvasHelper`](#classCanvasHelper_1a3622f03eea23e8b740f12f8177c74cc3)`()` 

Class destructor.

#### `public void `[`addCanvas`](#classCanvasHelper_1ade239b7879bfff344d7da95aba9f133a)`(TCanvas * canvas)` 

Register your canvas for processing. Canvas needs to be added after all the primitives are drawn on it.

#### Parameters
* `canvas` Canvas to be processed.

```cpp
CavasHelper::getInstance()->addCanvas(myCanvas);
```

#### `protected TList * `[`registeredCanvases`](#classCanvasHelper_1adb839ce36e92c083085c98a6281d5c0f) 

#### `protected TList * `[`padsWithModifiedDivisions`](#classCanvasHelper_1a8caf2468fddc852c4788002a31d48ba7) 

#### `protected std::map< std::string, std::pair< unsigned int, unsigned int > > `[`registeredCanvasesSizes`](#classCanvasHelper_1aa72d6e76dece731169383d2028c00d34) 

#### `protected  `[`CanvasHelper`](#classCanvasHelper_1a356c54078dad9e58aca74100b841ff64)`()` 

#### `protected void `[`onCanvasResized`](#classCanvasHelper_1a605d386b4c62cf6cd53d22283697c371)`()` 

#### `protected void `[`processCanvas`](#classCanvasHelper_1ac440d59386e589466077251c808f2fd1)`(TCanvas * canvas)` 

#### `protected void `[`processPad`](#classCanvasHelper_1a753bf2e1951bf20a0728a99968d2cb06)`(TVirtualPad * pad)` 

#### `protected void `[`convertAxisToPxSize`](#classCanvasHelper_1a9e6b6a6b35ef73e1e40ffa6d0d1853bb)`(TAxis * axis,const char type,TVirtualPad * pad)` 

#### `enum `[`EPaveAlignBits`](#classCanvasHelper_1aed359e291be7f893286cbef53e08b574) 

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
kPaveAlignLeft            | align pave to the left frame border
kPaveAlignRight            | align pave to the right frame border
kPaveAlignTop            | align pave to the top frame border
kPaveAlignBottom            | align pave to the bottom frame border

Used to set alignment options (bits) for ROOT stats boxes and legends. Horizontal and vertical emum variables can be combined.

# struct `CanvasHelper::Margin` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public Double_t `[`left`](#structCanvasHelper_1_1Margin_1a27a79755df77f329a6ec0c2ea2ef354e) | 
`public Double_t `[`right`](#structCanvasHelper_1_1Margin_1a98d4e36151559805d6a1e9b1bcaf96fb) | 
`public Double_t `[`bottom`](#structCanvasHelper_1_1Margin_1a832025f022798776deee13d8be11ccf2) | 
`public Double_t `[`top`](#structCanvasHelper_1_1Margin_1adb884d4a1ba334d32b984268cae10566) | 

## Members

#### `public Double_t `[`left`](#structCanvasHelper_1_1Margin_1a27a79755df77f329a6ec0c2ea2ef354e) 

#### `public Double_t `[`right`](#structCanvasHelper_1_1Margin_1a98d4e36151559805d6a1e9b1bcaf96fb) 

#### `public Double_t `[`bottom`](#structCanvasHelper_1_1Margin_1a832025f022798776deee13d8be11ccf2) 

#### `public Double_t `[`top`](#structCanvasHelper_1_1Margin_1adb884d4a1ba334d32b984268cae10566) 

Generated by [Moxygen](https://sourcey.com/moxygen)