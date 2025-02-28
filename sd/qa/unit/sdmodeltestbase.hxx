/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <string_view>

#include <test/bootstrapfixture.hxx>
#include <test/unoapi_test.hxx>
#include <test/xmldiff.hxx>
#include <test/xmltesttools.hxx>

#include <unotest/filters-test.hxx>
#include <unotest/macros_test.hxx>

#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <GraphicDocShell.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <tools/color.hxx>
#include <comphelper/fileformat.h>
#include <comphelper/processfactory.hxx>
#include <o3tl/safeint.hxx>
#include <rtl/strbuf.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svl/itemset.hxx>
#include <unomodel.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <drawinglayer/XShapeDumper.hxx>
#include <com/sun/star/text/XTextField.hpp>

using namespace ::com::sun::star;

class SdUnoApiTest : public UnoApiTest
{
public:
    SdUnoApiTest(OUString path)
        : UnoApiTest(path)
    {
    }

    uno::Reference<drawing::XDrawPage> getPage(int nPage)
    {
        uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xDoc.is());
        uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(nPage),
                                                 uno::UNO_QUERY_THROW);
        return xPage;
    }

    uno::Reference<beans::XPropertySet> getShapeFromPage(int nShape, int nPage)
    {
        uno::Reference<drawing::XDrawPage> xPage(getPage(nPage));
        uno::Reference<beans::XPropertySet> xShape(getShape(nShape, xPage));
        CPPUNIT_ASSERT_MESSAGE("Failed to load shape", xShape.is());

        return xShape;
    }

    // very confusing ... UNO index-based access to pages is 0-based. This one is 1-based
    const SdrPage* GetPage(int nPage)
    {
        SdXImpressDocument* pXImpressDocument
            = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pXImpressDocument);
        SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
        CPPUNIT_ASSERT_MESSAGE("no document", pDoc != nullptr);

        const SdrPage* pPage = pDoc->GetPage(nPage);
        CPPUNIT_ASSERT_MESSAGE("no page", pPage != nullptr);
        return pPage;
    }

    uno::Reference<beans::XPropertySet> getShape(int nShape,
                                                 uno::Reference<drawing::XDrawPage> const& xPage)
    {
        uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(nShape), uno::UNO_QUERY);
        CPPUNIT_ASSERT_MESSAGE("Failed to load shape", xShape.is());
        return xShape;
    }

    uno::Reference<text::XTextRange>
    getParagraphFromShape(int nPara, uno::Reference<beans::XPropertySet> const& xShape)
    {
        uno::Reference<text::XText> xText
            = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
        CPPUNIT_ASSERT_MESSAGE("Not a text shape", xText.is());

        uno::Reference<container::XEnumerationAccess> paraEnumAccess(xText, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> paraEnum(paraEnumAccess->createEnumeration());

        for (int i = 0; i < nPara; ++i)
            paraEnum->nextElement();

        uno::Reference<text::XTextRange> xParagraph(paraEnum->nextElement(), uno::UNO_QUERY_THROW);

        return xParagraph;
    }

    uno::Reference<text::XTextRange>
    getRunFromParagraph(int nRun, uno::Reference<text::XTextRange> const& xParagraph)
    {
        uno::Reference<container::XEnumerationAccess> runEnumAccess(xParagraph, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> runEnum = runEnumAccess->createEnumeration();

        for (int i = 0; i < nRun; ++i)
            runEnum->nextElement();

        uno::Reference<text::XTextRange> xRun(runEnum->nextElement(), uno::UNO_QUERY);

        return xRun;
    }

    uno::Reference<text::XTextField> getTextFieldFromPage(int nRun, int nPara, int nShape,
                                                          int nPage)
    {
        // get TextShape 1 from the first page
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(nShape, nPage));

        // Get first paragraph
        uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(nPara, xShape));

        // first chunk of text
        uno::Reference<text::XTextRange> xRun(getRunFromParagraph(nRun, xParagraph));

        uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

        uno::Reference<text::XTextField> xField;
        xPropSet->getPropertyValue("TextField") >>= xField;
        return xField;
    }
};

class SdUnoApiTestXml : public SdUnoApiTest, public XmlTestTools
{
public:
    SdUnoApiTestXml(OUString path)
        : SdUnoApiTest(path)
    {
    }

    xmlDocUniquePtr parseExport(utl::TempFileNamed const& rTempFile, OUString const& rStreamName)
    {
        std::unique_ptr<SvStream> const pStream(parseExportStream(rTempFile, rStreamName));
        xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
        OUString const url(rTempFile.GetURL());
        pXmlDoc->name = reinterpret_cast<char*>(xmlStrdup(reinterpret_cast<xmlChar const*>(
            OUStringToOString(url, RTL_TEXTENCODING_UTF8).getStr())));
        return pXmlDoc;
    }
};

CPPUNIT_NS_BEGIN

template <> struct assertion_traits<Color>
{
    static bool equal(const Color& c1, const Color& c2) { return c1 == c2; }

    static std::string toString(const Color& c)
    {
        OStringStream ost;
        ost << "Color: R:" << static_cast<int>(c.GetRed())
            << " G:" << static_cast<int>(c.GetGreen()) << " B:" << static_cast<int>(c.GetBlue())
            << " A:" << static_cast<int>(255 - c.GetAlpha());
        return ost.str();
    }
};

template <> struct assertion_traits<tools::Rectangle>
{
    static bool equal(const tools::Rectangle& r1, const tools::Rectangle& r2) { return r1 == r2; }

    static std::string toString(const tools::Rectangle& r)
    {
        OStringStream ost;
        ost << "Rect P: [" << r.Top() << ", " << r.Left()
            << "] "
               "S: ["
            << r.GetWidth() << ", " << r.GetHeight() << "]";
        return ost.str();
    }
};

CPPUNIT_NS_END

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
