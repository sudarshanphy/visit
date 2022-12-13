// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <AnariRenderingWidget.h>
#include <QvisRenderingWindow.h>
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
#include <QMessageBox>

#include <algorithm>


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
    m_backendStackedLayout(nullptr),
    m_rendererParams(),
    m_totalRows(0),
    m_renderingGroup(nullptr),
    m_libraryNames(nullptr),
    m_librarySubtypes(nullptr),
    m_rendererSubtypes(nullptr),
    m_samplesPerPixel(nullptr),
    m_aoSamples(nullptr),
    m_lightFalloff(nullptr),
    m_ambientIntensity(nullptr),
    m_maxDepth(nullptr),
    m_rValue(nullptr),
    m_debugMethod(nullptr),
    m_denoiserToggle(nullptr),
    m_outputDir()
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
        libraryChanged(m_libraryNames->currentText());
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
    m_samplesPerPixel->setValue(m_renderingAttributes->GetAnariSPP());

    connect(m_samplesPerPixel, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &AnariRenderingWidget::samplesPerPixelChanged);

    QLabel *anariSPPLabel = new QLabel("SPP");
    anariSPPLabel->setToolTip(tr("Samples Per Pixel"));

    gridLayout->addWidget(anariSPPLabel, rows, 0, 1, 1);
    gridLayout->addWidget(m_samplesPerPixel, rows, 1, 1, 1);

    // ambientSamples ANARI_INT32 - scivis, ao
    m_aoSamples = new QSpinBox();
    m_aoSamples->setMinimum(0);
    m_aoSamples->setValue(m_renderingAttributes->GetAnariAO());

    connect(m_aoSamples, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &AnariRenderingWidget::aoSamplesChanged);

    QLabel *aoLabel = new QLabel(tr("AO Samples"));
    aoLabel->setToolTip(tr("Ambient Occlusion Samples"));

    gridLayout->addWidget(aoLabel, rows, 2, 1, 1);
    gridLayout->addWidget(m_aoSamples, rows++, 3, 1, 1);

    // Row 2
    // lightFalloff ANARI_FLOAT32 - scivis    
    m_lightFalloff = new QLineEdit(QString::number(m_renderingAttributes->GetAnariLightFalloff()),
                                   widget);
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
    m_ambientIntensity = new QLineEdit(QString::number(m_renderingAttributes->GetAnariAmbientIntensity()),
                                       widget);
    QDoubleValidator *dv1 = new QDoubleValidator(0.0, 1.0, 4);
    m_ambientIntensity->setValidator(dv1);

    connect(m_ambientIntensity, &QLineEdit::editingFinished, 
            this, &AnariRenderingWidget::ambientIntensityChanged);

    QLabel *intensityLabel = new QLabel(tr("Ambient Intensity"));
    intensityLabel->setToolTip(tr("0.0 <= Ambient Light Intensity <= 1.0"));

    gridLayout->addWidget(intensityLabel, rows, 2, 1, 1);
    gridLayout->addWidget(m_ambientIntensity, rows++, 3, 1, 1);

    // Row 3
    // maxDepth ANRI_INT32 - dpt
    m_maxDepth = new QSpinBox();
    m_maxDepth->setMinimum(0);
    m_maxDepth->setValue(m_renderingAttributes->GetAnariMaxDepth());

    connect(m_maxDepth, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &AnariRenderingWidget::maxDepthChanged);

    QLabel *maxDepthLabel = new QLabel(tr("Max Depth"));
    maxDepthLabel->setToolTip(tr("Max depth for tracing rays"));

    gridLayout->addWidget(maxDepthLabel, rows, 0, 1, 1);
    gridLayout->addWidget(m_maxDepth, rows, 1, 1, 1);

    // R  ANARI_FLOAT32 - dpt
    m_rValue = new QLineEdit(QString::number(m_renderingAttributes->GetAnariRValue()),
                             widget);
    QDoubleValidator *dv2 = new QDoubleValidator(0.0, 1.0, 4);
    m_rValue->setValidator(dv2);

    connect(m_rValue, &QLineEdit::editingFinished, 
            this, &AnariRenderingWidget::rValueChanged);

    QLabel *rLabel = new QLabel(tr("R"));
    rLabel->setToolTip(tr("0.0 <= R <= 1.0"));

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

    debugMethodChanged(m_debugMethod->currentText());
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

    UpdateUI();
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
    QWidget *widget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setSpacing(10);
    gridLayout->setMargin(10);

    gridLayout->setColumnStretch(1, 3);

    // row, col, rowspan, colspan
    // Output location for the USD files
    m_outputDir.reset(new QString(QDir::homePath()));

    // Row 1
    QLabel *locationLabel = new QLabel("Directory");
    locationLabel->setToolTip(tr("Output location for saving the USD files"));

    m_dirLineEdit = new QLineEdit(*m_outputDir);
    connect(m_dirLineEdit, &QLineEdit::editingFinished, this, &AnariRenderingWidget::outputLocationChanged);    
    outputLocationChanged();

    QPushButton *dirSelectButton = new QPushButton("Select");
    connect(dirSelectButton, &QPushButton::pressed, this, &AnariRenderingWidget::selectButtonPressed);

    m_commitCheckBox = new QCheckBox(tr("commit"));
    m_commitCheckBox->setToolTip(tr("Write USD at ANARI commit call"));
    m_commitCheckBox->setChecked(m_renderingAttributes->GetUsdAtCommit());

    connect(m_commitCheckBox, &QCheckBox::toggled, this, &AnariRenderingWidget::commitToggled);

    gridLayout->addWidget(locationLabel, 0, 0, 1, 1);
    gridLayout->addWidget(m_dirLineEdit, 0, 1, 1, 2);
    gridLayout->addWidget(dirSelectButton, 0, 3, 1, 1);
    gridLayout->addWidget(m_commitCheckBox, 0, 4, 1, 1);

    mainLayout->addLayout(gridLayout);

    // Row 2
    rows++;
    QGroupBox *outputGroup = new QGroupBox(tr("Output"));

    QGridLayout *gridLayout2 = new QGridLayout(outputGroup);
    gridLayout2->setSpacing(10);
    gridLayout2->setMargin(10);

    m_binaryCheckBox = new QCheckBox(tr("Binary"));
    m_binaryCheckBox->setToolTip(tr("Binary or text output"));
    m_binaryCheckBox->setChecked(m_renderingAttributes->GetUsdOutputBinary());

    connect(m_binaryCheckBox, &QCheckBox::toggled, this, &AnariRenderingWidget::binaryToggled);
    gridLayout2->addWidget(m_binaryCheckBox, 0, 0, 1, 1);

    m_materialCheckBox = new QCheckBox(tr("Material"));
    m_materialCheckBox->setToolTip(tr("Include material objects in the output"));
    m_materialCheckBox->setChecked(m_renderingAttributes->GetUsdOutputMaterial());

    connect(m_materialCheckBox, &QCheckBox::toggled, this, &AnariRenderingWidget::materialToggled);
    gridLayout2->addWidget(m_materialCheckBox, 0, 1, 1, 1);

    m_previewCheckBox = new QCheckBox(tr("Preview Surface"));
    m_previewCheckBox->setToolTip(tr("Include preview surface shader prims in the output for material objects"));
    m_previewCheckBox->setChecked(m_renderingAttributes->GetUsdOutputPreviewSurface());

    connect(m_previewCheckBox, &QCheckBox::toggled, this, &AnariRenderingWidget::previewSurfaceToggled);
    gridLayout2->addWidget(m_previewCheckBox, 0, 2, 1, 1);

    // Row 3
    rows++;

    m_mdlCheckBox = new QCheckBox(tr("MDL"));
    m_mdlCheckBox->setToolTip(tr("Include MDL shader prims in the output for material objects"));
    m_mdlCheckBox->setChecked(m_renderingAttributes->GetUsdOutputMDL());

    connect(m_mdlCheckBox, &QCheckBox::toggled, this, &AnariRenderingWidget::mdlToggled);
    gridLayout2->addWidget(m_mdlCheckBox, 1, 0, 1, 1);

    m_mdlColorCheckBox = new QCheckBox(tr("MDL Colors"));
    m_mdlColorCheckBox->setToolTip(tr("Include MDL colors in the output for material objects"));
    m_mdlColorCheckBox->setChecked(m_renderingAttributes->GetUsdOutputMDLColors());

    connect(m_mdlColorCheckBox, &QCheckBox::toggled, this, &AnariRenderingWidget::mdlColorsToggled);
    gridLayout2->addWidget(m_mdlColorCheckBox, 1, 1, 1, 1);

    m_displayColorCheckBox = new QCheckBox(tr("Display Colors"));
    m_displayColorCheckBox->setToolTip(tr("Include display colors in the output"));
    m_displayColorCheckBox->setChecked(m_renderingAttributes->GetUsdOutputDisplayColors());

    connect(m_displayColorCheckBox, &QCheckBox::toggled, this, &AnariRenderingWidget::displayColorsToggled);
    gridLayout2->addWidget(m_displayColorCheckBox, 1, 2, 1, 1);

    rows++;
    mainLayout->addWidget(outputGroup);

    return widget;
}

// ****************************************************************************
// Method: AnariRenderingWidget::GetBackendType
//
// Purpose:
//   Gets the back-end type represented by libname.
//
// Arguments:
//   libname the name of the back-end to load
//
//
// Programmer: Kevin Griffin
// Creation:  
//
// Modifications:
//
// ****************************************************************************

BackendType 
AnariRenderingWidget::GetBackendType(const std::string &libname) const
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
//   Update the state of the UI components
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
    auto start = m_rendererParams->begin();
    auto stop = m_rendererParams->end();
    auto result = std::find(start, stop, "pixelSamples");

    if(m_samplesPerPixel != nullptr)
    {        
        m_samplesPerPixel->setEnabled(result != stop);
    }

    if(m_aoSamples != nullptr)
    {
        result = std::find(start, stop, "aoSamples");
        auto result2 = std::find(start, stop, "ambientSamples");
        m_aoSamples->setEnabled((result != stop) || (result2 != stop));
    }

    if(m_lightFalloff != nullptr)
    {
        result = std::find(start, stop, "lightFalloff");
        m_lightFalloff->setEnabled(result != stop);
    }

    if(m_ambientIntensity != nullptr)
    {
        result = std::find(start, stop, "ambientIntensity");
        m_ambientIntensity->setEnabled(result != stop);
    }

    if(m_maxDepth != nullptr)
    {
        result = std::find(start, stop, "maxDepth");
        m_maxDepth->setEnabled(result != stop);
    }
    
    if(m_rValue != nullptr)
    {
        result = std::find(start, stop, "R");
        m_rValue->setEnabled(result != stop);
    }
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateRendererParams
//
// Purpose:
//   Update the list of supported parameters for a specific renderer subtype.
//
// Arguments:
//   rendererSubtype the renderer subtype (e.g., scivis)
//   library the ANARI library (e.g., visrtx)
//
//
// Programmer: Kevin Griffin
// Creation:  
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateRendererParams(const std::string &rendererSubtype, anari::Library library)
{
    bool unloadLibrary = false;    

    if(library == nullptr)
    {
        unloadLibrary = true;
        auto libname = m_libraryNames->currentText().toStdString();
        library = anari::loadLibrary(libname.c_str());
    }    
    
    if(library)
    {
        auto libsubtype = m_librarySubtypes->currentText().toStdString();
        const anari::Parameter *rendererParams = 
            static_cast<const anari::Parameter *>(anariGetObjectInfo(library,
                                                                     libsubtype.c_str(),
                                                                     rendererSubtype.c_str(),
                                                                     ANARI_RENDERER,
                                                                     "parameter",
                                                                     ANARI_PARAMETER_LIST));
        // Clear old renderer parameters
        m_rendererParams.reset(new std::vector<std::string>());

        // Add new renderer parameters
        if(rendererParams)
        {
            for(auto p = rendererParams; p->name != NULL; p++)
            {
                std::string param(p->name);
                m_rendererParams->emplace_back(param);
            }
        }

        if(unloadLibrary)
        {
            anariUnloadLibrary(library);
        }
    }
    else 
    {
        debug1 << "Could not load the ANARI library to update the renderer parameters list." << std::endl;
    }
}

// External Updates
//----------------------------------------------------------------------------

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateLibrarySubtypes
//
// Purpose:
//   Adds subtype to the library subtypes combo box. If subtype is already in
//   the list, it will be ignored.
//
// Arguments:
//   subtype the library subtype to add to the combo box
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
//   Updates the list of available ANARI back-ends.
//
// Arguments:
//   libname the name of the ANARI back-end
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
//   Updates the list of available renderers. If subtype is already in the list
//   it will not be added again.
//
// Arguments:
//   subtype the renderer subtype to add
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
//   Sets the check state of the ANARI rendering group box.
//
// Arguments:
//   val    If true, surface rendering will be done by an ANARI back-end
//          renderer, otherwise, the default rendering is used.
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
//   Updates the samples per pixel that will be used by the currently selected 
//   renderer.
//
// Arguments:
//   val the new samples per pixel value
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
//   Updates the ambient occulsion value that will be used by the currently
//   selected renderer.
//
// Arguments:
//   val the new ambient occlusion value
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
// Method: AnariRenderingWidget::UpdateLightFalloff
//
// Purpose:
//   Update the light falloff value that will be used by the currently
//   selected renderer.
//
// Arguments:
//   val the new light falloff value
//
// Programmer: Kevin Griffin
// Creation:  
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateLightFalloff(const float val)
{
    m_lightFalloff->blockSignals(true);
    m_lightFalloff->setText(QString::number(val));
    m_lightFalloff->blockSignals(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateAmbientIntensity
//
// Purpose:
//   Update the ambient intensity value that will be used by the currently
//   selected renderer.
//
// Arguments:
//   val the new ambient intensity value
//
// Programmer: Kevin Griffin
// Creation:  
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateAmbientIntensity(const float val)
{
    m_ambientIntensity->blockSignals(true);
    m_ambientIntensity->setText(QString::number(val));
    m_ambientIntensity->blockSignals(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateMaxDepth
//
// Purpose:
//   Updates the max depth value that will be used by the currently
//   selected renderer.
//
// Arguments:
//   val the new max depth value
//
// Programmer: Kevin Griffin
// Creation:  
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateMaxDepth(const int val)
{
    m_maxDepth->blockSignals(true);
    m_maxDepth->setValue(val);
    m_maxDepth->blockSignals(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateRValue
//
// Purpose:
//   Update the R value that will be used by the currently selected renderer.
//
// Arguments:
//   val the new R value
//
// Programmer: Kevin Griffin
// Creation:  
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateRValue(const float val)
{
    m_rValue->blockSignals(true);
    m_rValue->setText(QString::number(val));
    m_rValue->blockSignals(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateDebugMethod
//
// Purpose:
//   Updates the debug method that will be used by the currently selected 
//   renderer.
//
// Arguments:
//   method the new debug method
//
// Programmer: Kevin Griffin
// Creation:  
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateDebugMethod(const std::string method)
{
    QString textItem = QString::fromStdString(method);

    if(!textItem.isEmpty())
    {
        m_debugMethod->blockSignals(true);
        int index = m_debugMethod->findText(textItem);

        if(index == -1)
        {                
            m_debugMethod->addItem(textItem);
        }
         m_debugMethod->blockSignals(false);
    }
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateDenoiserSelection
//
// Purpose:
//   Updates the checked state of the denoiser option.
//
// Arguments:
//   val    if true, and the currently selected renderer supports it, a denoiser
//   will be used to reduce visible noise in the rendered image.
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
// Method: AnariRenderingWidget::UpdateUSDOutputLocation
//
// Purpose:
//   Updates the USD output location path.
//
// Arguments:
//   path output location for saving the USD files
//
//
// Programmer: Kevin Griffin
// Creation:  
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateUSDOutputLocation(const std::string path)
{
    QString directoryQStr = QString::fromStdString(path);

    if(!directoryQStr.isEmpty())
    {
        QDir directory(directoryQStr);

        if(directory.exists())
        {
            m_dirLineEdit->blockSignals(true);
            m_dirLineEdit->setText(directoryQStr);
            m_dirLineEdit->blockSignals(false);
        }
        else
        {
            debug5 << "AnariRenderingWidget::UpdateUSDOutputLocation: " << path << " does not exist" << std::endl;
        }
    }
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateUSDParameter
//
// Purpose:
//   Sets the checked state of the USD Ooutput parameter check boxes.
//
// Arguments:
//   param  the USD output parameter to update
//   bool   if true then param is selected
//
// Programmer: Kevin Griffin
// Creation:  
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateUSDParameter(const USDParameter param, const bool val)
{
    switch(param)
    {
        case USDParameter::COMMIT:
            m_commitCheckBox->blockSignals(true);
            m_commitCheckBox->setChecked(val);
            m_commitCheckBox->blockSignals(false);
            break;
        case USDParameter::BINARY:
            m_binaryCheckBox->blockSignals(true);
            m_binaryCheckBox->setChecked(val);
            m_binaryCheckBox->blockSignals(false);
            break;
        case USDParameter::MATERIAL:
            m_materialCheckBox->blockSignals(true);
            m_materialCheckBox->setChecked(val);
            m_materialCheckBox->blockSignals(false);
            break;
        case USDParameter::PREVIEW:
            m_previewCheckBox->blockSignals(true);
            m_previewCheckBox->setChecked(val);
            m_previewCheckBox->blockSignals(false);
            break;
        case USDParameter::MDL:
            m_mdlCheckBox->blockSignals(true);
            m_mdlCheckBox->setChecked(val);
            m_mdlCheckBox->blockSignals(false);
            break;
        case USDParameter::MDLCOLORS:
            m_mdlColorCheckBox->blockSignals(true);
            m_mdlColorCheckBox->setChecked(val);
            m_mdlColorCheckBox->blockSignals(false);
            break;
        case USDParameter::DISPLAY:
            m_displayColorCheckBox->blockSignals(true);
            m_displayColorCheckBox->setChecked(val);
            m_displayColorCheckBox->blockSignals(false);
            break;
    }
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

        auto libSubtype = m_librarySubtypes->currentText().toStdString();
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

        auto rendererSubtype = m_rendererSubtypes->currentText().toStdString();
        UpdateRendererParams(rendererSubtype, anariLibrary);

        m_renderingAttributes->SetAnariRendererSubtype(rendererSubtype);
        m_rendererSubtypes->blockSignals(false);

        anariUnloadLibrary(anariLibrary);
        auto backendType = GetBackendType(libname);

        if(backendType != BackendType::USD)
        {
            emit currentBackendChanged(0);
            UpdateUI();
        }
        else
        {
            emit currentBackendChanged(1);
        }

        m_renderingWindow->ApplyAnariChanges(false);
    }
    else 
    {
        debug1 << "Could not load the ANARI library (" << libname.c_str() << ") to update the Rendering UI." << std::endl;
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
   
    auto libname = m_libraryNames->currentText().toStdString();
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

        auto rendererSubtype = m_rendererSubtypes->currentText().toStdString();
        UpdateRendererParams(rendererSubtype, anariLibrary); 

        m_renderingAttributes->SetAnariRendererSubtype(rendererSubtype);
        m_rendererSubtypes->blockSignals(false);

        anariUnloadLibrary(anariLibrary);
        UpdateUI();
        
        m_renderingAttributes->SetAnariLibrarySubtype(libSubtype);
        m_renderingWindow->ApplyAnariChanges(false);
    }
    else
    {
        debug1 << "Could not load the ANARI library (" << libname.c_str() << ") to update the Rendering UI." << std::endl;
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
    auto rendererSubtype = subtype.toStdString();

    UpdateRendererParams(rendererSubtype);    
    UpdateUI();

    m_renderingAttributes->SetAnariRendererSubtype(rendererSubtype);
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
    float val = text.toFloat(&ok);

    if(ok)
    {
        m_renderingAttributes->SetAnariLightFalloff(val);
        m_renderingWindow->ApplyAnariChanges(false);
    }
    else
    {
        debug5 << "Failed to convert Light Falloff input text (" << text.toStdString() << ") to a float" << std::endl;
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
    float val = text.toFloat(&ok);

    if(ok)
    {
        m_renderingAttributes->SetAnariAmbientIntensity(val);
        m_renderingWindow->ApplyAnariChanges(false);
    }
    else
    {
        debug5 << "Failed to convert Ambient Intensity input text (" << text.toStdString() << ") to a float" << std::endl;
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
    m_renderingAttributes->SetAnariMaxDepth(val);
    m_renderingWindow->ApplyAnariChanges(false);
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
    float val = text.toFloat(&ok);

    if(ok)
    {
        m_renderingAttributes->SetAnariRValue(val);
        m_renderingWindow->ApplyAnariChanges(false);
    }
    else
    {
        debug5 << "Failed to convert R value input text (" << text.toStdString() << ") to a float" << std::endl;
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
    m_renderingAttributes->SetAnariDebugMethod(method.toStdString());
    m_renderingWindow->ApplyAnariChanges(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::outputLocationChanged
//
// Purpose: 
//      Triggered when the USD output directory changes
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
    QDir directory(m_dirLineEdit->text());
    *m_outputDir = directory.absolutePath();

    if(directory.exists())
    {
        m_renderingAttributes->SetUsdDir(m_outputDir->toStdString());
        m_renderingWindow->ApplyAnariChanges(false);
    }
    else
    {
        QString message = tr("%1 doesn't exist").arg(*m_outputDir);
        QMessageBox::critical(this, tr("USD Output Directory"), message);
    }    
}

// ****************************************************************************
// Method: AnariRenderingWidget::selectButtonPressed
//
// Purpose: 
//      Triggered when the USD output directory select button is pressed.
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

// ****************************************************************************
// Method: AnariRenderingWidget::commitToggled
//
// Purpose: 
//      Triggered when USD commit is toggled.
//
// Arguments:
//      val when true writing to USD will happen immediately at the anariCommit
//          call, otherwise it will happen at anariRenderFrame.
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

void
AnariRenderingWidget::commitToggled(bool val)
{
    m_renderingAttributes->SetUsdAtCommit(val);
    m_renderingWindow->ApplyAnariChanges(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::binaryToggled
//
// Purpose: 
//      Triggered when output type is toggled (binary or text).
//
// Arguments:
//      val if true USD output is binary, otherwise text
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

void
AnariRenderingWidget::binaryToggled(bool val)
{
    m_renderingAttributes->SetUsdOutputBinary(val);
    m_renderingWindow->ApplyAnariChanges(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::materialToggled
//
// Purpose: 
//      Triggered when material checkbox is toggled to determine if material 
//      objects are included in the USD output.
//
// Arguments:
//      val if true material objects are included in the USD output
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

void
AnariRenderingWidget::materialToggled(bool val)
{
    m_renderingAttributes->SetUsdOutputMaterial(val);
    m_renderingWindow->ApplyAnariChanges(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::previewSurfaceToggled
//
// Purpose: 
//      Triggered when preview surface checkbox is toggled to determine if  
//      preview surface shader prims are included in the output for material 
//      objects.
//
// Arguments:
//      val if true preview surface shader prims are included in output for 
//          material objects
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

void
AnariRenderingWidget::previewSurfaceToggled(bool val)
{
    m_renderingAttributes->SetUsdOutputPreviewSurface(val);
    m_renderingWindow->ApplyAnariChanges(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::mdlToggled
//
// Purpose: 
//      Triggered when the mdl checkbox is toggled to determine if mdl shader 
//      prims are included in the output for material objects.
//
// Arguments:
//      val if true mdl shader prims are included in output for material objects
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

void
AnariRenderingWidget::mdlToggled(bool val)
{
    m_renderingAttributes->SetUsdOutputMDL(val);
    m_renderingWindow->ApplyAnariChanges(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::mdlColorsToggled
//
// Purpose: 
//      Triggered when the mdl colors checkbox is toggled to determine if mdl 
//      colors are included in the output for material objects.
//
// Arguments:
//      val if true mdl colors are included in output for material objects
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

void
AnariRenderingWidget::mdlColorsToggled(bool val)
{
    m_renderingAttributes->SetUsdOutputMDLColors(val);
    m_renderingWindow->ApplyAnariChanges(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::displayColorsToggled
//
// Purpose: 
//      Triggered when the display colors checkbox is toggled to determine if 
//      display colors are included in the output.
//
// Arguments:
//      val if true include display colors in the output
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

void
AnariRenderingWidget::displayColorsToggled(bool val)
{
    m_renderingAttributes->SetUsdOutputDisplayColors(val);
    m_renderingWindow->ApplyAnariChanges(false);
}