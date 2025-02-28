/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
// MyEDITENG, due to exported EditEng
#ifndef INCLUDED_EDITENG_EDITENG_HXX
#define INCLUDED_EDITENG_EDITENG_HXX

#include <memory>
#include <vector>

#include <optional>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>

#include <o3tl/span.hxx>
#include <svl/typedwhich.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editstat.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editengdllapi.h>
#include <i18nlangtag/lang.h>

#include <tools/lineend.hxx>
#include <tools/degree.hxx>
#include <tools/long.hxx>

#include <editeng/eedata.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <svl/languageoptions.hxx>
#include <comphelper/errcode.hxx>
#include <functional>

template <typename Arg, typename Ret> class Link;

namespace com::sun::star {
  namespace linguistic2 {
    class XSpellChecker1;
    class XHyphenator;
  }
  namespace datatransfer {
    class XTransferable;
  }
  namespace lang {
    struct Locale;
  }
}

namespace svx {
struct SpellPortion;
typedef std::vector<SpellPortion> SpellPortions;
}

class SfxUndoManager;
namespace basegfx { class B2DPolyPolygon; }
namespace editeng {
    struct MisspellRanges;
}

class ImpEditEngine;
class EditView;
class OutputDevice;
class SvxFont;
class SfxItemPool;
class SfxStyleSheet;
class SfxStyleSheetPool;
class SvxSearchItem;
class SvxFieldItem;
class MapMode;
class Color;
namespace vcl { class Font; }
class KeyEvent;
class Size;
class Point;
namespace tools { class Rectangle; }
class SvStream;
namespace vcl { class Window; }
class SvKeyValueIterator;
class SvxForbiddenCharactersTable;
class SvxNumberFormat;
class SvxFieldData;
class ContentNode;
class ParaPortion;
class EditSelection;
class EditPaM;
class EditLine;
class InternalEditStatus;
class EditSelectionEngine;
class EditDoc;
class Range;
struct EPaM;
class DeletedNodeInfo;
class ParaPortionList;
enum class CharCompressType;
enum class TransliterationFlags;
class LinkParamNone;

/** values for:
       SfxItemSet GetAttribs( const ESelection& rSel, EditEngineAttribs nOnlyHardAttrib = EditEngineAttribs::All );
*/
enum class EditEngineAttribs {
    All,          /// returns all attributes even when they are not set
    OnlyHard      /// returns only attributes hard set on portions
};

/** values for:
       SfxItemSet  GetAttribs( sal_Int32 nPara, sal_Int32 nStart, sal_Int32 nEnd, sal_uInt8 nFlags = 0xFF ) const;
*/
enum class GetAttribsFlags
{
    NONE         = 0x00,
    STYLESHEET   = 0x01,
    PARAATTRIBS  = 0x02,
    CHARATTRIBS  = 0x04,
    ALL          = 0x07,
};
namespace o3tl
{
    template<> struct typed_flags<GetAttribsFlags> : is_typed_flags<GetAttribsFlags, 0x07> {};
}

enum class SetAttribsMode {
    NONE, WholeWord, Edge
};

class EDITENG_DLLPUBLIC EditEngine
{
    friend class EditView;
    friend class ImpEditView;
    friend class Outliner;
    friend class TextChainingUtils;


public:
    typedef std::vector<EditView*> ViewsType;

    EditSelection InsertText(
        css::uno::Reference<css::datatransfer::XTransferable > const & rxDataObj,
        const OUString& rBaseURL, const EditPaM& rPaM, bool bUseSpecial);

private:
    std::unique_ptr<ImpEditEngine>  pImpEditEngine;

                                       EditEngine( const EditEngine& ) = delete;
                       EditEngine&     operator=( const EditEngine& ) = delete;
    EDITENG_DLLPRIVATE bool            PostKeyEvent( const KeyEvent& rKeyEvent, EditView* pView, vcl::Window const * pFrameWin );

    EDITENG_DLLPRIVATE void CursorMoved(const ContentNode* pPrevNode);
    EDITENG_DLLPRIVATE void CheckIdleFormatter();
    EDITENG_DLLPRIVATE bool IsIdleFormatterActive() const;
    EDITENG_DLLPRIVATE ParaPortion* FindParaPortion(ContentNode const * pNode);
    EDITENG_DLLPRIVATE const ParaPortion* FindParaPortion(ContentNode const * pNode) const;
    EDITENG_DLLPRIVATE const ParaPortion* GetPrevVisPortion(const ParaPortion* pCurPortion) const;

    EDITENG_DLLPRIVATE css::uno::Reference<
        css::datatransfer::XTransferable>
            CreateTransferable(const EditSelection& rSelection);

    EDITENG_DLLPRIVATE EditPaM EndOfWord(const EditPaM& rPaM);

    EDITENG_DLLPRIVATE EditPaM GetPaM(const Point& aDocPos, bool bSmart = true);

    EDITENG_DLLPRIVATE EditSelection SelectWord(
        const EditSelection& rCurSelection,
        sal_Int16 nWordType = css::i18n::WordType::ANYWORD_IGNOREWHITESPACES);

    EDITENG_DLLPRIVATE tools::Long GetXPos(
        const ParaPortion* pParaPortion, const EditLine* pLine, sal_Int32 nIndex, bool bPreferPortionStart = false) const;

    EDITENG_DLLPRIVATE Range GetLineXPosStartEnd(
        const ParaPortion* pParaPortion, const EditLine* pLine) const;

    EDITENG_DLLPRIVATE InternalEditStatus& GetInternalEditStatus();

    EDITENG_DLLPRIVATE void HandleBeginPasteOrDrop(PasteOrDropInfos& rInfos);
    EDITENG_DLLPRIVATE void HandleEndPasteOrDrop(PasteOrDropInfos& rInfos);
    EDITENG_DLLPRIVATE bool HasText() const;
    EDITENG_DLLPRIVATE const EditSelectionEngine& GetSelectionEngine() const;
    EDITENG_DLLPRIVATE void SetInSelectionMode(bool b);

protected:


public:
                    EditEngine( SfxItemPool* pItemPool );
    virtual         ~EditEngine();

    const SfxItemSet& GetEmptyItemSet() const;

    void            SetDefTab( sal_uInt16 nDefTab );

    void            SetRefDevice( OutputDevice* pRefDef );
    OutputDevice*   GetRefDevice() const;

    void            SetRefMapMode( const MapMode& rMapMode );
    MapMode const & GetRefMapMode() const;

    /// Change the update mode per bUpdate and potentially trigger FormatAndUpdate.
    /// bRestoring is used for LOK to update cursor visibility, specifically,
    /// when true, it means we are restoring the update mode after internally
    /// disabling it (f.e. during SetText to set/delete default text in Impress).
    /// @return previous value of update
    bool            SetUpdateLayout(bool bUpdate, bool bRestoring = false);
    bool            IsUpdateLayout() const;

    void            SetBackgroundColor( const Color& rColor );
    Color const &   GetBackgroundColor() const;
    Color           GetAutoColor() const;
    void            EnableAutoColor( bool b );
    void            ForceAutoColor( bool b );
    bool            IsForceAutoColor() const;

    void            InsertView(EditView* pEditView, size_t nIndex = EE_APPEND);
    EditView*       RemoveView( EditView* pEditView );
    void            RemoveView(size_t nIndex);
    EditView*       GetView(size_t nIndex = 0) const;
    size_t          GetViewCount() const;
    bool            HasView( EditView* pView ) const;
    EditView*       GetActiveView() const;
    void SetActiveView(EditView* pView);

    void            SetPaperSize( const Size& rSize );
    const Size&     GetPaperSize() const;

    void            SetVertical( bool bVertical );
    bool            IsEffectivelyVertical() const;
    bool            IsTopToBottom() const;
    bool            GetVertical() const;
    void            SetRotation(TextRotation nRotation);
    TextRotation    GetRotation() const;

    void SetTextColumns(sal_Int16 nColumns, sal_Int32 nSpacing);

    void            SetFixedCellHeight( bool bUseFixedCellHeight );

    void                        SetDefaultHorizontalTextDirection( EEHorizontalTextDirection eHTextDir );
    EEHorizontalTextDirection   GetDefaultHorizontalTextDirection() const;

    SvtScriptType   GetScriptType( const ESelection& rSelection ) const;
    editeng::LanguageSpan GetLanguage(const EditPaM& rPaM) const;
    editeng::LanguageSpan GetLanguage( sal_Int32 nPara, sal_Int32 nPos ) const;

    void            TransliterateText( const ESelection& rSelection, TransliterationFlags nTransliterationMode );
    EditSelection   TransliterateText( const EditSelection& rSelection, TransliterationFlags nTransliterationMode );

    void            SetAsianCompressionMode( CharCompressType nCompression );

    void            SetKernAsianPunctuation( bool bEnabled );

    void            SetAddExtLeading( bool b );

    void            SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon );
    void            SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::B2DPolyPolygon* pLinePolyPolygon);
    void            ClearPolygon();

    const Size&     GetMinAutoPaperSize() const;
    void            SetMinAutoPaperSize( const Size& rSz );

    const Size&     GetMaxAutoPaperSize() const;
    void            SetMaxAutoPaperSize( const Size& rSz );

    void SetMinColumnWrapHeight(tools::Long nVal);

    OUString        GetText( LineEnd eEnd = LINEEND_LF ) const;
    OUString        GetText( const ESelection& rSelection ) const;
    sal_Int32       GetTextLen() const;
    sal_uInt32      GetTextHeight() const;
    sal_uInt32      GetTextHeightNTP() const;
    sal_uInt32      CalcTextWidth();

    OUString        GetText( sal_Int32 nParagraph ) const;
    sal_Int32       GetTextLen( sal_Int32 nParagraph ) const;
    sal_uInt32      GetTextHeight( sal_Int32 nParagraph ) const;

    sal_Int32       GetParagraphCount() const;

    sal_Int32       GetLineCount( sal_Int32 nParagraph ) const;
    sal_Int32       GetLineLen( sal_Int32 nParagraph, sal_Int32 nLine ) const;
    void            GetLineBoundaries( /*out*/sal_Int32& rStart, /*out*/sal_Int32& rEnd, sal_Int32 nParagraph, sal_Int32 nLine ) const;
    sal_Int32       GetLineNumberAtIndex( sal_Int32 nPara, sal_Int32 nIndex ) const;
    sal_uInt32      GetLineHeight( sal_Int32 nParagraph );
    tools::Rectangle GetParaBounds( sal_Int32 nPara );
    ParagraphInfos  GetParagraphInfos( sal_Int32 nPara );
    sal_Int32       FindParagraph( tools::Long nDocPosY );
    EPosition       FindDocPosition( const Point& rDocPos ) const;
    tools::Rectangle       GetCharacterBounds( const EPosition& rPos ) const;

    OUString        GetWord(sal_Int32 nPara, sal_Int32 nIndex);

    ESelection      GetWord( const ESelection& rSelection, sal_uInt16 nWordType ) const;

    void            Clear();
    void            SetText( const OUString& rStr );

    std::unique_ptr<EditTextObject> CreateTextObject();
    std::unique_ptr<EditTextObject> GetEmptyTextObject() const;
    std::unique_ptr<EditTextObject> CreateTextObject( sal_Int32 nPara, sal_Int32 nParas = 1 );
    std::unique_ptr<EditTextObject> CreateTextObject( const ESelection& rESelection );
    void            SetText( const EditTextObject& rTextObject );

    void            RemoveParagraph(sal_Int32 nPara);
    void            InsertParagraph(sal_Int32 nPara, const EditTextObject& rTxtObj, const bool bAppend = false);
    void            InsertParagraph(sal_Int32 nPara, const OUString& rText);

    void            SetText(sal_Int32 nPara, const OUString& rText);

    virtual void        SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet );
    const SfxItemSet&   GetParaAttribs( sal_Int32 nPara ) const;

    /// Set attributes from rSet an all characters of nPara.
    void SetCharAttribs(sal_Int32 nPara, const SfxItemSet& rSet);
    void            GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const;

    SfxItemSet      GetAttribs( sal_Int32 nPara, sal_Int32 nStart, sal_Int32 nEnd, GetAttribsFlags nFlags = GetAttribsFlags::ALL ) const;
    SfxItemSet      GetAttribs( const ESelection& rSel, EditEngineAttribs nOnlyHardAttrib = EditEngineAttribs::All );

    bool            HasParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const;
    const SfxPoolItem&  GetParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const;
    template<class T>
    const T&            GetParaAttrib( sal_Int32 nPara, TypedWhichId<T> nWhich ) const
    {
        return static_cast<const T&>(GetParaAttrib(nPara, sal_uInt16(nWhich)));
    }

    vcl::Font       GetStandardFont( sal_Int32 nPara );
    SvxFont         GetStandardSvxFont( sal_Int32 nPara );

    void            RemoveAttribs( const ESelection& rSelection, bool bRemoveParaAttribs, sal_uInt16 nWhich );

    void            ShowParagraph( sal_Int32 nParagraph, bool bShow );

    SfxUndoManager& GetUndoManager();
    SfxUndoManager* SetUndoManager(SfxUndoManager* pNew);
    void            UndoActionStart( sal_uInt16 nId );
    void            UndoActionStart(sal_uInt16 nId, const ESelection& rSel);
    void            UndoActionEnd();
    bool            IsInUndo() const;

    void            EnableUndo( bool bEnable );
    bool            IsUndoEnabled() const;

    /** returns the value last used for bTryMerge while calling ImpEditEngine::InsertUndo
        This is currently used in a bad but needed hack to get undo actions merged in the
        OutlineView in impress. Do not use it unless you want to sell your soul too! */
    bool            HasTriedMergeOnLastAddUndo() const;

    void            ClearModifyFlag();
    void            SetModified();
    bool            IsModified() const;

    void            SetModifyHdl( const Link<LinkParamNone*,void>& rLink );
    Link<LinkParamNone*,void> const & GetModifyHdl() const;

    bool            IsInSelectionMode() const;

    void            StripPortions();
    void            GetPortions( sal_Int32 nPara, std::vector<sal_Int32>& rList );

    tools::Long            GetFirstLineStartX( sal_Int32 nParagraph );
    Point           GetDocPosTopLeft( sal_Int32 nParagraph );
    Point           GetDocPos( const Point& rPaperPos ) const;
    bool            IsTextPos( const Point& rPaperPos, sal_uInt16 nBorder );

    // StartDocPos corresponds to VisArea.TopLeft().
    void            Draw( OutputDevice& rOutDev, const tools::Rectangle& rOutRect );
    void            Draw( OutputDevice& rOutDev, const tools::Rectangle& rOutRect, const Point& rStartDocPos );
    void            Draw( OutputDevice& rOutDev, const tools::Rectangle& rOutRect, const Point& rStartDocPos, bool bClip );
    void            Draw( OutputDevice& rOutDev, const Point& rStartPos, Degree10 nOrientation = 0_deg10 );

    ErrCode         Read( SvStream& rInput, const OUString& rBaseURL, EETextFormat, SvKeyValueIterator* pHTTPHeaderAttrs = nullptr );
    void            Write( SvStream& rOutput, EETextFormat );

    void            SetStatusEventHdl( const Link<EditStatus&,void>& rLink );
    Link<EditStatus&,void> const & GetStatusEventHdl() const;

    void            SetNotifyHdl( const Link<EENotify&,void>& rLink );
    Link<EENotify&,void> const & GetNotifyHdl() const;

    void            SetRtfImportHdl( const Link<RtfImportInfo&,void>& rLink );
    const Link<RtfImportInfo&,void>& GetRtfImportHdl() const;

    void            SetHtmlImportHdl( const Link<HtmlImportInfo&,void>& rLink );
    const Link<HtmlImportInfo&,void>& GetHtmlImportHdl() const;

    // Do not evaluate font formatting => For Outliner
    bool            IsFlatMode() const;
    void            SetFlatMode( bool bFlat );

    void            SetControlWord( EEControlBits nWord );
    EEControlBits   GetControlWord() const;

    void            QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel );
    void            QuickMarkInvalid( const ESelection& rSel );
    void            QuickFormatDoc( bool bFull = false );
    void            QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel );
    void            QuickInsertLineBreak( const ESelection& rSel );
    void            QuickInsertText(const OUString& rText, const ESelection& rSel);
    void            QuickDelete( const ESelection& rSel );
    void            QuickMarkToBeRepainted( sal_Int32 nPara );

    void            SetGlobalCharStretching( sal_uInt16 nX, sal_uInt16 nY );
    void            GetGlobalCharStretching( sal_uInt16& rX, sal_uInt16& rY ) const;

    void            SetEditTextObjectPool( SfxItemPool* pPool );
    SfxItemPool*    GetEditTextObjectPool() const;

    void                SetStyleSheetPool( SfxStyleSheetPool* pSPool );
    SfxStyleSheetPool*  GetStyleSheetPool();

    void SetStyleSheet(const EditSelection& aSel, SfxStyleSheet* pStyle);
    void                 SetStyleSheet( sal_Int32 nPara, SfxStyleSheet* pStyle );
    const SfxStyleSheet* GetStyleSheet( sal_Int32 nPara ) const;
    SfxStyleSheet* GetStyleSheet( sal_Int32 nPara );

    void            SetWordDelimiters( const OUString& rDelimiters );
    const OUString& GetWordDelimiters() const;

    void            EraseVirtualDevice();

    void            SetSpeller( css::uno::Reference<
                            css::linguistic2::XSpellChecker1 > const &xSpeller );
    css::uno::Reference<
        css::linguistic2::XSpellChecker1 > const &
                    GetSpeller();
    void            SetHyphenator( css::uno::Reference<
                            css::linguistic2::XHyphenator > const & xHyph );

    void GetAllMisspellRanges( std::vector<editeng::MisspellRanges>& rRanges ) const;
    void SetAllMisspellRanges( const std::vector<editeng::MisspellRanges>& rRanges );

    static void     SetForbiddenCharsTable(const std::shared_ptr<SvxForbiddenCharactersTable>& xForbiddenChars);

    void            SetDefaultLanguage( LanguageType eLang );
    LanguageType    GetDefaultLanguage() const;

    bool            HasOnlineSpellErrors() const;
    void            CompleteOnlineSpelling();

    bool            ShouldCreateBigTextObject() const;

    // For fast Pre-Test without view:
    EESpellState    HasSpellErrors();
    void ClearSpellErrors();
    bool            HasText( const SvxSearchItem& rSearchItem );

    //spell and return a sentence
    bool            SpellSentence(EditView const & rEditView, svx::SpellPortions& rToFill );
    // put spell position to start of current sentence
    void            PutSpellingToSentenceStart( EditView const & rEditView );
    //applies a changed sentence
    void            ApplyChangedSentence(EditView const & rEditView, const svx::SpellPortions& rNewPortions, bool bRecheck );

    // for text conversion (see also HasSpellErrors)
    bool            HasConvertibleTextPortion( LanguageType nLang );
    virtual bool    ConvertNextDocument();

    bool            UpdateFields();
    bool            UpdateFieldsOnly();
    void            RemoveFields( const std::function<bool ( const SvxFieldData* )>& isFieldData = [] (const SvxFieldData* ){return true;} );

    sal_uInt16      GetFieldCount( sal_Int32 nPara ) const;
    EFieldInfo      GetFieldInfo( sal_Int32 nPara, sal_uInt16 nField ) const;

    bool            IsRightToLeft( sal_Int32 nPara ) const;

    css::uno::Reference< css::datatransfer::XTransferable >
                    CreateTransferable( const ESelection& rSelection ) const;

    // MT: Can't create new virtual functions like for ParagraphInserted/Deleted, must be compatible in SRC638, change later...
    void            SetBeginMovingParagraphsHdl( const Link<MoveParagraphsInfo&,void>& rLink );
    void            SetEndMovingParagraphsHdl( const Link<MoveParagraphsInfo&,void>& rLink );
    void            SetBeginPasteOrDropHdl( const Link<PasteOrDropInfos&,void>& rLink );
    void            SetEndPasteOrDropHdl( const Link<PasteOrDropInfos&,void>& rLink );

    virtual void    PaintingFirstLine(sal_Int32 nPara, const Point& rStartPos, const Point& rOrigin, Degree10 nOrientation, OutputDevice& rOutDev);
    virtual void    ParagraphInserted( sal_Int32 nNewParagraph );
    virtual void    ParagraphDeleted( sal_Int32 nDeletedParagraph );
    virtual void    ParagraphConnected( sal_Int32 nLeftParagraph, sal_Int32 nRightParagraph );
    virtual void    ParaAttribsChanged( sal_Int32 nParagraph );
    virtual void    StyleSheetChanged( SfxStyleSheet* pStyle );
    void            ParagraphHeightChanged( sal_Int32 nPara );

    virtual void DrawingText( const Point& rStartPos, const OUString& rText,
                              sal_Int32 nTextStart, sal_Int32 nTextLen,
                              o3tl::span<const sal_Int32> pDXArray,
                              o3tl::span<const sal_Bool> pKashidaArray,
                              const SvxFont& rFont,
                              sal_Int32 nPara, sal_uInt8 nRightToLeft,
                              const EEngineData::WrongSpellVector* pWrongSpellVector,
                              const SvxFieldData* pFieldData,
                              bool bEndOfLine,
                              bool bEndOfParagraph,
                              const css::lang::Locale* pLocale,
                              const Color& rOverlineColor,
                              const Color& rTextLineColor);

    virtual void DrawingTab( const Point& rStartPos, tools::Long nWidth, const OUString& rChar,
                             const SvxFont& rFont, sal_Int32 nPara, sal_uInt8 nRightToLeft,
                             bool bEndOfLine,
                             bool bEndOfParagraph,
                             const Color& rOverlineColor,
                             const Color& rTextLineColor);
    virtual OUString  GetUndoComment( sal_uInt16 nUndoId ) const;
    virtual bool    SpellNextDocument();
    /** @return true, when click was consumed. false otherwise. */
    virtual bool    FieldClicked( const SvxFieldItem& rField );
    virtual OUString CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, std::optional<Color>& rTxtColor, std::optional<Color>& rFldColor );

    // override this if access to bullet information needs to be provided
    virtual const SvxNumberFormat * GetNumberFormat( sal_Int32 nPara ) const;

    virtual tools::Rectangle GetBulletArea( sal_Int32 nPara );

    static rtl::Reference<SfxItemPool> CreatePool();
    static SfxItemPool& GetGlobalItemPool();
    static bool     DoesKeyChangeText( const KeyEvent& rKeyEvent );
    static bool     DoesKeyMoveCursor( const KeyEvent& rKeyEvent );
    static bool     IsSimpleCharInput( const KeyEvent& rKeyEvent );
    static void     SetFontInfoInItemSet( SfxItemSet& rItemSet, const vcl::Font& rFont );
    static void     SetFontInfoInItemSet( SfxItemSet& rItemSet, const SvxFont& rFont );
    static vcl::Font CreateFontFromItemSet( const SfxItemSet& rItemSet, SvtScriptType nScriptType );
    static SvxFont  CreateSvxFontFromItemSet( const SfxItemSet& rItemSet );
    static bool     IsPrintable( sal_Unicode c ) { return ( ( c >= 32 ) && ( c != 127 ) ); }
    static bool     HasValidData( const css::uno::Reference< css::datatransfer::XTransferable >& rTransferable );
    /** sets a link that is called at the beginning of a drag operation at an edit view */
    void            SetBeginDropHdl( const Link<EditView*,void>& rLink );
    Link<EditView*,void> const & GetBeginDropHdl() const;

    /** sets a link that is called at the end of a drag operation at an edit view */
    void            SetEndDropHdl( const Link<EditView*,void>& rLink );
    Link<EditView*,void> const & GetEndDropHdl() const;

    /// specifies if auto-correction should capitalize the first word or not (default is on)
    void            SetFirstWordCapitalization( bool bCapitalize );

    /** specifies if auto-correction should replace a leading single quotation
        mark (apostrophe) or not (default is on) */
    void            SetReplaceLeadingSingleQuotationMark( bool bReplace );

    EditDoc& GetEditDoc();
    const EditDoc& GetEditDoc() const;
    void dumpAsXmlEditDoc(xmlTextWriterPtr pWriter) const;

    ParaPortionList& GetParaPortions();
    const ParaPortionList& GetParaPortions() const;

    bool IsFormatted() const;
    bool IsHtmlImportHandlerSet() const;
    bool IsRtfImportHandlerSet() const;
    bool IsImportRTFStyleSheetsSet() const;

    void CallRtfImportHandler(RtfImportInfo& rInfo);
    void CallHtmlImportHandler(HtmlImportInfo& rInfo);

    void ParaAttribsToCharAttribs(ContentNode* pNode);

    EditPaM CreateEditPaM(const EPaM& rEPaM);
    EditPaM ConnectParagraphs(
        ContentNode* pLeft, ContentNode* pRight, bool bBackward);

    EditPaM InsertField(const EditSelection& rEditSelection, const SvxFieldItem& rFld);
    EditPaM InsertText(const EditSelection& aCurEditSelection, const OUString& rStr);
    EditSelection InsertText(const EditTextObject& rTextObject, const EditSelection& rSel);
    EditPaM InsertParaBreak(const EditSelection& rEditSelection);
    EditPaM InsertLineBreak(const EditSelection& rEditSelection);

    EditPaM CursorLeft(
        const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode = css::i18n::CharacterIteratorMode::SKIPCELL);
    EditPaM CursorRight(
        const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode = css::i18n::CharacterIteratorMode::SKIPCELL);

    void SeekCursor(ContentNode* pNode, sal_Int32 nPos, SvxFont& rFont);

    EditPaM DeleteSelection(const EditSelection& rSel);

    ESelection CreateESelection(const EditSelection& rSel) const;
    EditSelection CreateSelection(const ESelection& rSel);

    const SfxItemSet& GetBaseParaAttribs(sal_Int32 nPara) const;
    void SetParaAttribsOnly(sal_Int32 nPara, const SfxItemSet& rSet);
    void SetAttribs(const EditSelection& rSel, const SfxItemSet& rSet, SetAttribsMode nSpecial = SetAttribsMode::NONE);

    OUString GetSelected(const EditSelection& rSel) const;
    EditPaM DeleteSelected(const EditSelection& rSel);

    SvtScriptType GetScriptType(const EditSelection& rSel) const;

    void RemoveParaPortion(sal_Int32 nNode);

    void SetCallParaInsertedOrDeleted(bool b);
    bool IsCallParaInsertedOrDeleted() const;

    void AppendDeletedNodeInfo(DeletedNodeInfo* pInfo);
    void UpdateSelections();

    void InsertContent(ContentNode* pNode, sal_Int32 nPos);
    EditPaM SplitContent(sal_Int32 nNode, sal_Int32 nSepPos);
    EditPaM ConnectContents(sal_Int32 nLeftNode, bool bBackward);

    void InsertFeature(const EditSelection& rEditSelection, const SfxPoolItem& rItem);

    EditSelection MoveParagraphs(const Range& rParagraphs, sal_Int32 nNewPos);

    void RemoveCharAttribs(sal_Int32 nPara, sal_uInt16 nWhich = 0, bool bRemoveFeatures = false);
    void RemoveCharAttribs(const EditSelection& rSel, bool bRemoveParaAttribs, sal_uInt16 nWhich);
    void RemoveCharAttribs(const EditSelection& rSel, EERemoveParaAttribsMode eMode, sal_uInt16 nWhich);

    ViewsType& GetEditViews();
    const ViewsType& GetEditViews() const;

    void SetUndoMode(bool b);
    void FormatAndLayout(EditView* pCurView, bool bCalledFromUndo = false);

    void Undo(EditView* pView);
    void Redo(EditView* pView);

    sal_Int32 GetOverflowingParaNum() const;
    sal_Int32 GetOverflowingLineNum() const;
    void ClearOverflowingParaNum();
    bool IsPageOverflow();

    // tdf#132288  By default inserting an attribute beside another that is of
    // the same type expands the original instead of inserting another. But the
    // spell check dialog doesn't want that behaviour
    void DisableAttributeExpanding();

    // Optimization, if set, formatting will be done only for text lines that fit
    // in given paper size and exceeding lines will be ignored.
    void EnableSkipOutsideFormat(bool set);

    void SetLOKSpecialPaperSize(const Size& rSize);
    const Size& GetLOKSpecialPaperSize() const;

#ifdef DBG_UTIL
    static void DumpData(const EditEngine* pEE, bool bInfoBox);
#endif
};

#endif // INCLUDED_EDITENG_EDITENG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
