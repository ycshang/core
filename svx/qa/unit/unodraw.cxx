/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>

#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/frame/XStorable.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <test/unoapi_test.hxx>
#include <unotools/tempfile.hxx>
#include <svx/unopage.hxx>
#include <vcl/virdev.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <drawinglayer/tools/primitive2dxmldump.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <test/xmltesttools.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/mediadescriptor.hxx>
#include <vcl/filter/PngImageReader.hxx>

#include <sdr/contact/objectcontactofobjlistpainter.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for svx/source/unodraw/ code.
class UnodrawTest : public UnoApiTest, public XmlTestTools
{
public:
    UnodrawTest()
        : UnoApiTest("svx/qa/unit/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(UnodrawTest, testWriterGraphicExport)
{
    // Load a document with a Writer picture in it.
    loadFromURL(u"unodraw-writer-image.odt");
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<lang::XComponent> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);

    // Export it as JPEG.
    uno::Reference<drawing::XGraphicExportFilter> xExportFilter
        = drawing::GraphicExportFilter::create(mxComponentContext);
    // This resulted in a css::lang::IllegalArgumentException for a Writer
    // picture.
    xExportFilter->setSourceDocument(xShape);

    utl::TempFileNamed aTempFile;
    aTempFile.EnableKillingFile();
    uno::Sequence<beans::PropertyValue> aProperties(
        comphelper::InitPropertySequence({ { "URL", uno::Any(aTempFile.GetURL()) },
                                           { "MediaType", uno::Any(OUString("image/jpeg")) } }));
    CPPUNIT_ASSERT(xExportFilter->filter(aProperties));
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testTdf93998)
{
    loadFromURL(u"tdf93998.odp");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDrawPagesSupplier.is());

    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDrawPage.is());

    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());

    uno::Reference<lang::XMultiServiceFactory> xFactory = comphelper::getProcessServiceFactory();
    uno::Reference<awt::XControlModel> xModel(
        xFactory->createInstance("com.sun.star.awt.UnoControlDialogModel"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xModel.is());

    uno::Reference<beans::XPropertySet> xModelProps(xModel, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xModelProps.is());

    // This resulted in a uno::RuntimeException, assigning a shape to a dialog model's image was
    // broken.
    xModelProps->setPropertyValue("ImageURL", xShape->getPropertyValue("GraphicURL"));
    uno::Reference<graphic::XGraphic> xGraphic;
    xModelProps->getPropertyValue("Graphic") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testTableShadowDirect)
{
    // Create an Impress document an insert a table shape.
    mxComponent = loadFromDesktop("private:factory/simpress",
                                  "com.sun.star.presentation.PresentationDocument");
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.TableShape"), uno::UNO_QUERY);
    xShape->setPosition(awt::Point(1000, 1000));
    xShape->setSize(awt::Size(10000, 10000));
    uno::Reference<drawing::XDrawPagesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPages> xDrawPages = xSupplier->getDrawPages();
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY);
    xDrawPage->add(xShape);

    // Create a red shadow on it without touching its style.
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with throwing a
    // beans.UnknownPropertyException, as shadow-as-direct-formatting on tables were not possible.
    xShapeProps->setPropertyValue("Shadow", uno::Any(true));
    sal_Int32 nRed = 0xff0000;
    xShapeProps->setPropertyValue("ShadowColor", uno::Any(nRed));
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue("ShadowColor") >>= nRed);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xff0000), nRed);

    // Add text.
    uno::Reference<table::XCellRange> xTable(xShapeProps->getPropertyValue("Model"),
                                             uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByPosition(0, 0), uno::UNO_QUERY);
    xCell->setString("A1");

    // Generates drawinglayer primitives for the shape.
    auto pDrawPage = dynamic_cast<SvxDrawPage*>(xDrawPage.get());
    CPPUNIT_ASSERT(pDrawPage);
    SdrPage* pSdrPage = pDrawPage->GetSdrPage();
    ScopedVclPtrInstance<VirtualDevice> aVirtualDevice;
    sdr::contact::ObjectContactOfObjListPainter aObjectContact(*aVirtualDevice,
                                                               { pSdrPage->GetObj(0) }, nullptr);
    const sdr::contact::ViewObjectContact& rDrawPageVOContact
        = pSdrPage->GetViewContact().GetViewObjectContact(aObjectContact);
    sdr::contact::DisplayInfo aDisplayInfo;
    drawinglayer::primitive2d::Primitive2DContainer xPrimitiveSequence;
    rDrawPageVOContact.getPrimitive2DSequenceHierarchy(aDisplayInfo, xPrimitiveSequence);

    // Check the primitives.
    drawinglayer::Primitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(xPrimitiveSequence);
    assertXPath(pDocument, "//shadow", /*nNumberOfNodes=*/1);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. there was shadow for the cell text, while here PowerPoint-compatible output is expected,
    // which has no shadow for cell text (only for cell borders and cell background).
    assertXPath(pDocument, "//shadow//sdrblocktext", /*nNumberOfNodes=*/0);
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testTitleShapeBullets)
{
    // Create a title shape with 2 paragraphs in it.
    mxComponent = loadFromDesktop("private:factory/simpress",
                                  "com.sun.star.presentation.PresentationDocument");
    uno::Reference<drawing::XDrawPagesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPages> xDrawPages = xSupplier->getDrawPages();
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY);
    // A default document contains a title shape and a text shape on the first slide.
    uno::Reference<drawing::XShape> xTitleShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xTitleShapeInfo(xTitleShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTitleShapeInfo->supportsService("com.sun.star.presentation.TitleTextShape"));
    uno::Reference<text::XTextRange> xTitleShapeText(xTitleShape, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTitleShapeText->getText();
    uno::Reference<text::XTextRange> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "foo", /*bAbsorb=*/false);
    xText->insertControlCharacter(xCursor, text::ControlCharacter::APPEND_PARAGRAPH,
                                  /*bAbsorb=*/false);
    xText->insertString(xCursor, "bar", /*bAbsorb=*/false);

    // Check that the title shape has 2 paragraphs.
    uno::Reference<container::XEnumerationAccess> xTextEA(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xTextE = xTextEA->createEnumeration();
    // Has a first paragraph.
    CPPUNIT_ASSERT(xTextE->hasMoreElements());
    xTextE->nextElement();
    // Has a second paragraph.
    // Without the accompanying fix in place, this test would have failed, because the 2 paragraphs
    // were merged together (e.g. 1 bullet instead of 2 bullets for bulleted paragraphs).
    CPPUNIT_ASSERT(xTextE->hasMoreElements());
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testPngExport)
{
    // Given an empty Impress document:
    mxComponent = loadFromDesktop("private:factory/simpress",
                                  "com.sun.star.presentation.PresentationDocument");

    // When exporting that document to PNG with a JSON size:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY_THROW);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOut = new utl::OOutputStreamWrapper(aStream);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("impress_png_Export");
    aMediaDescriptor["FilterOptions"]
        <<= OUString("{\"PixelHeight\":{\"type\":\"long\",\"value\":\"192\"},"
                     "\"PixelWidth\":{\"type\":\"long\",\"value\":\"192\"}}");
    aMediaDescriptor["OutputStream"] <<= xOut;
    xStorable->storeToURL("private:stream", aMediaDescriptor.getAsConstPropertyValueList());

    // Then make sure that the size request is handled:
    aStream.Seek(STREAM_SEEK_TO_BEGIN);
    vcl::PngImageReader aPngReader(aStream);
    BitmapEx aBitmapEx;
    aPngReader.read(aBitmapEx);
    Size aSize = aBitmapEx.GetSizePixel();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 192
    // - Actual  : 595
    // i.e. it was not possible to influence the size from the cmdline.
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(192), aSize.getHeight());
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(192), aSize.getWidth());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
