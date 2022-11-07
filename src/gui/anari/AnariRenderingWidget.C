// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "AnariRenderingWidget.h"
#include "QvisRenderingWindow.h"

#include <RenderingAttributes.h>
#include <DebugStream.h>

#include <QGroupBox>
#include <QComboBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QString>
#include <QSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QCheckBox>
#include <QSpacerItem>
#include <QDir>
#include <QPushButton>
#include <QFileDialog>

#include <anari/anari_cpp.hpp>


// ****************************************************************************
// Method: AnariRenderingWidget::AnariRenderingWidget
//
// Purpose:
//   Constructor for the AnariRenderingWidget class.
//
// Arguments:
//   renderingWindow        Window that displays rendering settings 
//   renderingAttributes    Contains ANARI rendering attributes
//   parent                 If parent is another widget, this widget becomes a 
//                          child window inside parent. The new widget is deleted 
//                          when its parent is deleted. 
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

AnariRenderingWidget::AnariRenderingWidget(QvisRenderingWindow *renderingWindow, 
                                           RenderingAttributes *renderingAttributes,
                                           QWidget *parent) :
    QWidget(parent),
    m_renderingWindow(renderingWindow),
    m_renderingAttributes(renderingAttributes),    
    m_backendStackedLayout(nullptr)
{
    // row, col, rowspan, colspan
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    // Rendering Group
    m_renderingGroup = new QGroupBox(tr("ANARI Rendering"));
    m_renderingGroup->setCheckable(true);
    m_renderingGroup->setChecked(false);
    connect(m_renderingGroup, &QGroupBox::toggled, 
            this, &AnariRenderingWidget::renderingToggled);
    
    QVBoxLayout *renderingGroupVBoxLayout = new QVBoxLayout(m_renderingGroup);
    m_totalRows = 0;
    int rows = m_totalRows;

    renderingGroupVBoxLayout->addWidget(CreateGeneralWidget(rows));
    m_totalRows += rows;   

    // Create and add the back-end specific widgets
    m_backendStackedLayout = new QStackedLayout();

// TODO: Create HAVE_ANARI_BACKEND
#if defined(HAVE_ANARI_EXAMPLE) || defined(HAVE_ANARI_VISRTX)
    rows = 0;
    m_backendStackedLayout->addWidget(CreateBackendWidget(rows));
    m_totalRows += rows;
#endif

#if defined(HAVE_ANARI_USD)
    rows = 0;
    m_backendStackedLayout->addWidget(CreateUSDWidget(rows));
    m_totalRows += rows;
#endif

    renderingGroupVBoxLayout->addLayout(m_backendStackedLayout);
    mainLayout->addWidget(m_renderingGroup);

    connect(this, &AnariRenderingWidget::currentBackendChanged,
            m_backendStackedLayout, &QStackedLayout::setCurrentIndex);
}

// ****************************************************************************
// Method: AnariRenderingWidget::CreateGeneralWidget
//
// Purpose:
//   Creates the UI components for selecting back-end options used by all
//   ANARI libraries.
//
// Arguments:
//   rows keeps track of the total rows used to create this widget
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

QWidget *
AnariRenderingWidget::CreateGeneralWidget(int &rows)
{
    QWidget *generalOptionsWidget = new QWidget(this);

    QGridLayout *gridLayout = new QGridLayout(generalOptionsWidget);
    gridLayout->setSpacing(10);
    gridLayout->setMargin(10);

    gridLayout->setColumnStretch(1, 2);
    gridLayout->setColumnStretch(3, 2);
    gridLayout->setColumnStretch(4, 5);

    m_libraryNames = new QComboBox();
    m_libraryNames->setInsertPolicy(QComboBox::InsertPolicy::InsertAlphabetically);

    #ifdef HAVE_ANARI_EXAMPLE
    m_libraryNames->addItem("example");
    #endif
    #ifdef HAVE_ANARI_VISRTX
    m_libraryNames->addItem("visrtx");
    #endif
    #ifdef HAVE_ANARI_USD
    m_libraryNames->addItem("usd");
    #endif

    connect(m_libraryNames, QOverload<const QString&>::of(&QComboBox::currentIndexChanged),
            this, &AnariRenderingWidget::libraryChanged);

    // Back-end and subtype
    QLabel *backendLabel = new QLabel(tr("Back-end")); 
    backendLabel->setToolTip(tr("ANARI back-end device"));

    gridLayout->addWidget(backendLabel, rows, 0, 1, 1);    
    gridLayout->addWidget(m_libraryNames, rows, 1, 1, 1);
    
    // Back-end subtype
    m_librarySubtypes = new QComboBox();    
    m_librarySubtypes->setInsertPolicy(QComboBox::InsertPolicy::InsertAlphabetically);
    connect(m_librarySubtypes, QOverload<const QString&>::of(&QComboBox::currentIndexChanged),
            this, &AnariRenderingWidget::librarySubtypeChanged);

    QLabel *subtypeLabel = new QLabel(tr("Back-end Subtype"));

    gridLayout->addWidget(subtypeLabel, rows, 2, 1, 1);
    gridLayout->addWidget(m_librarySubtypes, rows, 3, 1, 1);

    gridLayout->addItem(new QSpacerItem(10, 10), rows++, 4, 1, 1);

    // Renderer
    m_rendererSubtypes = new QComboBox();    
    m_rendererSubtypes->setInsertPolicy(QComboBox::InsertPolicy::InsertAlphabetically);
    connect(m_rendererSubtypes, QOverload<const QString&>::of(&QComboBox::currentIndexChanged),
            this, &AnariRenderingWidget::rendererSubtypeChanged);

    QLabel *rendererLabel = new QLabel(tr("Renderer"));
    rendererLabel->setToolTip(tr("Renderer subtype"));

    gridLayout->addWidget(rendererLabel, rows, 0, 1, 1);    
    gridLayout->addWidget(m_rendererSubtypes, rows, 1, 1, 1);

    gridLayout->addItem(new QSpacerItem(10, 10), rows++, 3, 1, 3);

    // Initialize UI components
    if(m_libraryNames->count() > 0)
    {
        libraryChanged(m_libraryNames->itemText(0));
    }

    return generalOptionsWidget;
}

// ****************************************************************************
// Method: AnariRenderingWidget::CreateBackendWidget
//
// Purpose:
//   Creates the UI components for selecting rendering options used by ANARI 
//   back-ends (not including the USD back-end).
//
// Arguments:
//   rows keeps track of the total rows used to create this widget
//
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

QWidget *
AnariRenderingWidget::CreateBackendWidget(int &rows)
{ 
    QWidget *widget = new QWidget(this);

    QGridLayout *gridLayout = new QGridLayout(widget);
    gridLayout->setSpacing(10);
    gridLayout->setMargin(10);

    // Row 1
    // pixelSamples (ANARI_INT32) - all
    m_samplesPerPixel = new QSpinBox();
    m_samplesPerPixel->setMinimum(1);
    connect(m_samplesPerPixel, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &AnariRenderingWidget::samplesPerPixelChanged);

    QLabel *anariSPPLabel = new QLabel("SPP");
    anariSPPLabel->setToolTip(tr("Samples Per Pixel"));

    gridLayout->addWidget(anariSPPLabel, rows, 0, 1, 1);
    gridLayout->addWidget(m_samplesPerPixel, rows, 1, 1, 1);

    // ambientSamples ANARI_INT32 - scivis, ao
    m_aoSamples = new QSpinBox();
    m_aoSamples->setMinimum(0);
    connect(m_aoSamples, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &AnariRenderingWidget::aoSamplesChanged);

    QLabel *aoLabel = new QLabel(tr("AO Samples"));
    aoLabel->setToolTip(tr("Ambient Occlusion Samples"));

    gridLayout->addWidget(aoLabel, rows, 2, 1, 1);
    gridLayout->addWidget(m_aoSamples, rows++, 3, 1, 1);

    // Row 2
    // lightFalloff ANARI_FLOAT32 - scivis    
    m_lightFalloff = new QLineEdit("0.0", widget);
    QDoubleValidator *dv0 = new QDoubleValidator();
    dv0->setDecimals(4);
    m_lightFalloff->setValidator(dv0);
    connect(m_lightFalloff, &QLineEdit::editingFinished, 
            this, &AnariRenderingWidget::lightFalloffChanged);

    QLabel *lfoLabel = new QLabel(tr("Falloff"));
    lfoLabel->setToolTip(tr("Light Falloff"));

    gridLayout->addWidget(lfoLabel, rows, 0, 1, 1);
    gridLayout->addWidget(m_lightFalloff, rows, 1, 1, 1);

    // ambientIntensity ANARI_FLOAT32 - scivis
    m_ambientIntensity = new QLineEdit("0.0", widget);
    QDoubleValidator *dv1 = new QDoubleValidator(0.0, 1.0, 4);
    m_ambientIntensity->setValidator(dv1);
    connect(m_ambientIntensity, &QLineEdit::editingFinished, 
            this, &AnariRenderingWidget::ambientIntensityChanged);

    QLabel *intensityLabel = new QLabel(tr("Ambient Intensity"));
    intensityLabel->setToolTip(tr("Ambient Light Intensity"));

    gridLayout->addWidget(intensityLabel, rows, 2, 1, 1);
    gridLayout->addWidget(m_ambientIntensity, rows++, 3, 1, 1);

    // Row 3
    // maxDepth ANRI_INT32 - dpt
    m_maxDepth = new QSpinBox();
    m_maxDepth->setMinimum(1);
    connect(m_maxDepth, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &AnariRenderingWidget::maxDepthChanged);

    QLabel *maxDepthLabel = new QLabel(tr("Max Depth"));
    maxDepthLabel->setToolTip(tr("Max depth for tracing rays"));

    gridLayout->addWidget(maxDepthLabel, rows, 0, 1, 1);
    gridLayout->addWidget(m_maxDepth, rows, 1, 1, 1);

    // R  ANARI_FLOAT32 - dpt
    m_rValue = new QLineEdit("0.0", widget);
    QDoubleValidator *dv2 = new QDoubleValidator(0.0, 1.0, 4);
    m_rValue->setValidator(dv2);
    connect(m_rValue, &QLineEdit::editingFinished, 
            this, &AnariRenderingWidget::rValueChanged);

    QLabel *rLabel = new QLabel(tr("R"));
    rLabel->setToolTip(tr("R"));

    gridLayout->addWidget(rLabel, rows, 2, 1, 1);
    gridLayout->setAlignment(rLabel, Qt::AlignRight);

    gridLayout->addWidget(m_rValue, rows++, 3, 1, 1);

    // Row 4
    // debug method ANARI_STRING - debug
    m_debugMethod = new QComboBox();    
    m_debugMethod->setInsertPolicy(QComboBox::InsertPolicy::InsertAlphabetically);
    m_debugMethod->addItem("backface");
    m_debugMethod->addItem("primID");
    m_debugMethod->addItem("geomID");
    m_debugMethod->addItem("instID");
    m_debugMethod->addItem("Ng");
    m_debugMethod->addItem("uvw");
    m_debugMethod->addItem("istri");
    m_debugMethod->addItem("isvol");
    connect(m_debugMethod, QOverload<const QString&>::of(&QComboBox::currentIndexChanged),
            this, &AnariRenderingWidget::debugMethodChanged);

    QLabel *debugMethodLabel = new QLabel(tr("Debug Method"));
    debugMethodLabel->setToolTip(tr("Controls which debugging views of the scene is used"));

    gridLayout->addWidget(debugMethodLabel, rows, 0, 1, 1);
    gridLayout->addWidget(m_debugMethod, rows, 1, 1, 1);

    m_denoiserToggle = new QCheckBox(tr("Denoiser"));
    m_denoiserToggle->setChecked(m_renderingAttributes->GetUseAnariDenoiser());
    m_denoiserToggle->setToolTip(tr("Enable the OptiX denoiser"));

    connect(m_denoiserToggle, &QCheckBox::toggled, 
            this, &AnariRenderingWidget::denoiserToggled);
    gridLayout->addWidget(m_denoiserToggle, rows++, 2, 1, 2);

    return widget;
}

// ****************************************************************************
// Method: AnariRenderingWidget::CreateUSDWidget
//
// Purpose:
//   Creates the UI components used by ANARI back-ends.
//
// Arguments:
//   rows keeps track of the total rows of UI components
//
//
// Programmer: Kevin Griffin
// Creation:  
//
// Modifications:
//
// ****************************************************************************

QWidget *
AnariRenderingWidget::CreateUSDWidget(int &rows)
{
    // vtkAnariRendererNode::GetUSDOutputLocation(this->Owner->GetRenderer());
//   const char *location = "/home/kgriffin/Desktop";  
//   // vtkAnariRendererNode::GetUSDOutputBinary()
//   bool outputBinary = false;   
//   // vtkAnariRendererNode::GetUSDOutputMaterial()
//   bool outputMaterial = true;
//   bool outputPreviewSurface = true;
//   bool outputMdl = true;
//   bool outputDisplayColors = true;
//   bool outputMdlColors = true;
//   bool writeAtCommit = false;

    QWidget *widget = new QWidget(this);
    QGridLayout *gridLayout = new QGridLayout(widget);
    gridLayout->setSpacing(10);
    gridLayout->setMargin(10);

    gridLayout->setColumnStretch(1, 3);

    // row, col, rowspan, colspan
    // Output location for the USD files
    m_outputDir = new QString(QDir::homePath());

    QLabel *locationLabel = new QLabel("Directory");
    locationLabel->setToolTip(tr("Output location for saving the USD files"));

    m_dirLineEdit = new QLineEdit(*m_outputDir, widget);
    connect(m_dirLineEdit, &QLineEdit::editingFinished, 
            this, &AnariRenderingWidget::outputLocationChanged);    

    QPushButton *dirSelectButton = new QPushButton("Select");
    connect(dirSelectButton, &QPushButton::pressed,
            this, &AnariRenderingWidget::selectButtonPressed);

    m_commitCheckBox = new QCheckBox(tr("commit"));
    // m_commitCheckBox->setChecked(m_renderingAttributes->GetUseAnariDenoiser());
    m_commitCheckBox->setToolTip(tr("Write USD at ANARI commit call"));

    gridLayout->addWidget(locationLabel, rows, 0, 1, 1);
    gridLayout->addWidget(m_dirLineEdit, rows, 1, 1, 2);
    gridLayout->addWidget(dirSelectButton, rows, 3, 1, 1);
    gridLayout->addWidget(m_commitCheckBox, rows++, 4, 1, 1);

    return widget;
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateLibrarySubtypes
//
// Purpose:
//   Creates the UI components used by ANARI back-ends.
//
// Arguments:
//   rows keeps track of the total rows of UI components
//
//
// Programmer: Kevin Griffin
// Creation:  
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateLibrarySubtypes(const std::string subtype)
{
    m_librarySubtypes->blockSignals(true);
    QString textItem = QString::fromStdString(subtype);
    int index = m_librarySubtypes->findText(textItem);

    if(index == -1)
    {
        m_librarySubtypes->addItem(textItem);                
    }

    m_librarySubtypes->blockSignals(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateLibraryNames
//
// Purpose:
//   Creates the UI components used by ANARI back-ends.
//
// Arguments:
//   rows keeps track of the total rows of UI components
//
//
// Programmer: Kevin Griffin
// Creation:  
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateLibraryNames(const std::string libname)
{
    m_libraryNames->blockSignals(true);
    QString textItem = QString::fromStdString(libname);
    int index = m_libraryNames->findText(textItem);

    if(index == -1)
    {                
        m_libraryNames->addItem(textItem);
    }

    m_libraryNames->blockSignals(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateRendererSubtypes
//
// Purpose:
//   Creates the UI components used by ANARI back-ends.
//
// Arguments:
//   rows keeps track of the total rows of UI components
//
//
// Programmer: Kevin Griffin
// Creation:  
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateRendererSubtypes(const std::string subtype)
{
    m_rendererSubtypes->blockSignals(true);
    QString textItem = QString::fromStdString(subtype);
    int index = m_rendererSubtypes->findText(textItem);
    
    if(index == -1)
    {
        m_rendererSubtypes->addItem(textItem);
    }
    m_rendererSubtypes->blockSignals(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::SetChecked
//
// Purpose:
//   Creates the UI components used by ANARI back-ends.
//
// Arguments:
//   rows keeps track of the total rows of UI components
//
//
// Programmer: Kevin Griffin
// Creation:  
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::SetChecked(const bool val)
{
    m_renderingGroup->setChecked(val);
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateSamplesPerPixel
//
// Purpose:
//   Creates the UI components used by ANARI back-ends.
//
// Arguments:
//   rows keeps track of the total rows of UI components
//
//
// Programmer: Kevin Griffin
// Creation:  
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateSamplesPerPixel(const int val)
{
    m_samplesPerPixel->blockSignals(true);
    m_samplesPerPixel->setValue(val);
    m_samplesPerPixel->blockSignals(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateAOSamples
//
// Purpose:
//   Creates the UI components used by ANARI back-ends.
//
// Arguments:
//   rows keeps track of the total rows of UI components
//
//
// Programmer: Kevin Griffin
// Creation:  
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateAOSamples(const int val)
{
    m_aoSamples->blockSignals(true);
    m_aoSamples->setValue(val);
    m_aoSamples->blockSignals(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateDenoiserSelection
//
// Purpose:
//   Creates the UI components used by ANARI back-ends.
//
// Arguments:
//   rows keeps track of the total rows of UI components
//
//
// Programmer: Kevin Griffin
// Creation:  
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateDenoiserSelection(const bool val)
{
    m_denoiserToggle->blockSignals(true);
    m_denoiserToggle->setChecked(val);
    m_denoiserToggle->blockSignals(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::GetBackendType
//
// Purpose:
//   Creates the UI components used by ANARI back-ends.
//
// Arguments:
//   rows keeps track of the total rows of UI components
//
//
// Programmer: Kevin Griffin
// Creation:  
//
// Modifications:
//
// ****************************************************************************

BackendType 
AnariRenderingWidget::GetBackendType(const std::string libname)
{
    if(libname.compare("example") == 0)
    {
        return BackendType::EXAMPLE;
    }
    else if(libname.compare("usd") == 0)
    {
        return BackendType::USD;
    }
    else if(libname.compare("visrtx") == 0)
    {
        return BackendType::VISRTX;
    }
    
    return BackendType::NONE;
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateUI
//
// Purpose:
//   Creates the UI components used by ANARI back-ends.
//
// Arguments:
//   rows keeps track of the total rows of UI components
//
//
// Programmer: Kevin Griffin
// Creation:  
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateUI()
{
    // TODO: enable/disable UI components based on back-end and renderer

    // Back-end 
    // if(m_backendWidget != nullptr)
    // {
    //     m_backendWidget->setVisible(m_backendType != BackendType::USD);

    //     if(m_backendType == BackendType::EXAMPLE)
    //     {
    //         // TODO: Enable example back-end UI components
    //     }
    //     else if(m_backendType == BackendType::VISRTX)
    //     {
    //         // TODO: Enable visrtx back-end UI components
    //     }
    // }

    // USD
    // if(m_usdWidget != nullptr)
    // {
    //     m_usdWidget->setVisible(m_backendType == BackendType::USD);
    // }
}

// SLOTS
//----------------------------------------------------------------------------

// ****************************************************************************
// Method: AnariRenderingWidget::renderingToggled
//
// Purpose: 
//      Triggered when ANARI rendering is toggled.
//
// Arguments:
//      val when true use ANARI for rendering
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

void
AnariRenderingWidget::renderingToggled(bool val)
{
    m_renderingAttributes->SetAnariRendering(val);
    // m_renderingWindow->SetUpdate(false);
    // m_renderingWindow->Apply();
    m_renderingWindow->ApplyAnariChanges(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::libraryChanged
//
// Purpose: 
//      Triggered when ANARI Back-end rendering library has changed.
//
// Argument:
//      name    ANARI back-end library name
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

void
AnariRenderingWidget::libraryChanged(const QString &name)
{
    auto libname = name.toStdString();
    auto anariLibrary = anari::loadLibrary(libname.c_str());
    
    if(anariLibrary)
    {
        m_renderingAttributes->SetAnariLibrary(libname);

        // Update back-end subtypes
        m_librarySubtypes->blockSignals(true);
        m_librarySubtypes->clear();
        const char **devices = anariGetDeviceSubtypes(anariLibrary);

        if(devices)
        {
            for(const char **d = devices; *d != NULL; d++)
            {
                m_librarySubtypes->addItem(*d);
            }
        }
        else
        {
            m_librarySubtypes->addItem("default");
        }
        m_librarySubtypes->blockSignals(false);

        auto libSubtype = m_librarySubtypes->itemText(0).toStdString();
        m_renderingAttributes->SetAnariLibrarySubtype(libSubtype);

        // Update renderers
        m_rendererSubtypes->blockSignals(true);
        m_rendererSubtypes->clear();
        const char **renderers = anariGetObjectSubtypes(anariLibrary, libSubtype.c_str(), ANARI_RENDERER);

        if(renderers)
        {
            for(const char **d = renderers; *d != NULL; d++)
            {
                m_rendererSubtypes->addItem(*d);
            }
        }
        else
        {
            m_rendererSubtypes->addItem("default");
        }
        m_rendererSubtypes->blockSignals(false);

        auto rendererSubtype = m_rendererSubtypes->itemText(0).toStdString();
        m_renderingAttributes->SetAnariRendererSubtype(rendererSubtype);

        anariUnloadLibrary(anariLibrary);

        auto backendType = GetBackendType(libname);

        if(backendType != BackendType::USD)
        {
            emit currentBackendChanged(0);
        }
        else
        {
            emit currentBackendChanged(1);
        }

        UpdateUI();
        m_renderingWindow->ApplyAnariChanges(false);
    }
    else 
    {
        debug1 << "Could not load the ANARI library (" << libname.c_str() << ") to update the Rendering UI." << endl;
    }
}

// ****************************************************************************
// Method: AnariRenderingWidget::librarySubtypeChanged
//
// Purpose: 
//      Triggered when ANARI Library subtype has changed.
//
// Arguments:
//      subtype the new library subtype
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

void 
AnariRenderingWidget::librarySubtypeChanged(const QString &subtype)
{
    auto libSubtype = subtype.toStdString();
   
    auto libname = m_libraryNames->itemText(0).toStdString();
    auto anariLibrary = anari::loadLibrary(libname.c_str());

    if(anariLibrary)
    {
        // Update renderers
        m_rendererSubtypes->blockSignals(true);
        m_rendererSubtypes->clear();
        const char **renderers = anariGetObjectSubtypes(anariLibrary, libSubtype.c_str(), ANARI_RENDERER);

        if(renderers)
        {
            for(const char **d = renderers; *d != NULL; d++)
            {
                m_rendererSubtypes->addItem(*d);
            }
        }
        else
        {
            m_rendererSubtypes->addItem("default");
        }

        auto rendererSubtype = m_rendererSubtypes->itemText(0).toStdString();
        m_renderingAttributes->SetAnariRendererSubtype(rendererSubtype);
        m_rendererSubtypes->blockSignals(false);

        anariUnloadLibrary(anariLibrary);

        m_renderingAttributes->SetAnariLibrarySubtype(libSubtype);
        m_renderingWindow->ApplyAnariChanges(false);
    }
    else
    {
        debug1 << "Could not load the ANARI library (" << libname.c_str() << ") to update the Rendering UI." << endl;
    }
}

// ****************************************************************************
// Method: AnariRenderingWidget::rendererSubtypeChanged
//
// Purpose: 
//      Triggered when ANARI renderer subtype has changed.
//
// Arguments:
//      subtype the new renderer subtype
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

void 
AnariRenderingWidget::rendererSubtypeChanged(const QString &subtype)
{
    m_renderingAttributes->SetAnariRendererSubtype(subtype.toStdString());
    m_renderingWindow->ApplyAnariChanges(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::anariSPPChanged
//
// Purpose: 
//      Triggered when ANARI samples per pixel has changed.
//
// Arguments:
//      val     new samples per pixel value
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

void 
AnariRenderingWidget::samplesPerPixelChanged(int val)
{
    m_renderingAttributes->SetAnariSPP(val);
    m_renderingWindow->ApplyAnariChanges(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::aoSamplesChanged
//
// Purpose: 
//      Triggered when ANARI ambient occlusion samples has changed.
//
// Arguments:
//      val     new ANARI ambient occlusion sample count
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

void 
AnariRenderingWidget::aoSamplesChanged(int val)
{
    m_renderingAttributes->SetAnariAO(val);
    m_renderingWindow->ApplyAnariChanges(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::denoiserToggled
//
// Purpose: 
//      Triggered when ANARI denoiser is toggled.
//
// Arguments:
//      val when true use the OptiX denoiser when rendering
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

void
AnariRenderingWidget::denoiserToggled(bool val)
{
    m_renderingAttributes->SetUseAnariDenoiser(val);
    m_renderingWindow->ApplyAnariChanges(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::lightFalloffChanged
//
// Purpose: 
//      Triggered when the light falloff value changes
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

void
AnariRenderingWidget::lightFalloffChanged()
{
    bool ok;
    QString text = m_lightFalloff->text();
    double val = text.toDouble(&ok);

    if(ok)
    {
        std::cout << "lightFalloff = " << val << std::endl;
        // TODO: implement
        // m_renderingAttributes->SetAnariLightFalloff(val);
        // m_renderingWindow->ApplyAnariChanges(false);
    }
    else
    {
        debug5 << "Failed to convert Light Falloff input text (" << text.toStdString() << ") to a double" << std::endl;
    }
}

// ****************************************************************************
// Method: AnariRenderingWidget::ambientIntensityChanged
//
// Purpose: 
//      Triggered when the ambient intensity value changes
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

void
AnariRenderingWidget::ambientIntensityChanged()
{
    bool ok;
    QString text = m_ambientIntensity->text();
    double val = text.toDouble(&ok);

    if(ok)
    {
        std::cout << "ambient intensity = " << val << std::endl;
        // TODO: implement
        // m_renderingAttributes->SetAnariAmbientIntensity(val);
        // m_renderingWindow->ApplyAnariChanges(false);
    }
    else
    {
        debug5 << "Failed to convert Ambient Intensity input text (" << text.toStdString() << ") to a double" << std::endl;
    }
}

// ****************************************************************************
// Method: AnariRenderingWidget::maxDepthChanged
//
// Purpose: 
//      Triggered when max depth has changed.
//
// Arguments:
//      val     new max depth value
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

void 
AnariRenderingWidget::maxDepthChanged(int val)
{
    // m_renderingAttributes->SetAnariMaxDepth(val);
    // m_renderingWindow->ApplyAnariChanges(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::rValueChanged
//
// Purpose: 
//      Triggered when the R value changes
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

void
AnariRenderingWidget::rValueChanged()
{
    bool ok;
    QString text = m_rValue->text();
    double val = text.toDouble(&ok);

    if(ok)
    {
        std::cout << "R = " << val << std::endl;
        // TODO: implement
        // m_renderingAttributes->SetAnariR(val);
        // m_renderingWindow->ApplyAnariChanges(false);
    }
    else
    {
        debug5 << "Failed to convert R value input text (" << text.toStdString() << ") to a double" << std::endl;
    }
}

// ****************************************************************************
// Method: AnariRenderingWidget::debugMethodChanged
//
// Purpose: 
//      Triggered when the rendering debug method has changed.
//
// Arguments:
//      method the selected debug method
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

void 
AnariRenderingWidget::debugMethodChanged(const QString &method)
{
    // m_renderingAttributes->SetAnariDebugMethod(method.toStdString());
    // m_renderingWindow->ApplyAnariChanges(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::outputLocationChanged
//
// Purpose: 
//      Triggered when the Output Directory changes
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

void
AnariRenderingWidget::outputLocationChanged()
{
    auto dir = m_dirLineEdit->text().toStdString();

    std::cout << "USD Output Directory: " << dir << std::endl;
    // TODO: implement
    // m_renderingAttributes->SetAnariUSDDirectory(text.toStdString());
    // m_renderingWindow->ApplyAnariChanges(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::selectButtonPressed
//
// Purpose: 
//      Triggered when the rendering debug method has changed.
//
// Arguments:
//      method the selected debug method
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

void 
AnariRenderingWidget::selectButtonPressed()
{
    auto dir = QFileDialog::getExistingDirectory(this, 
                                                 tr("Open Directory"), 
                                                 *m_outputDir,
                                                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if(!dir.isEmpty())
    {
        m_dirLineEdit->setText(dir);
        outputLocationChanged();
    }
}