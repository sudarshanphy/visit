// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ANARI_RENDERING_WIDGET_H
#define ANARI_RENDERING_WIDGET_H

#include <gui_exports.h>
#include <QWidget>

class RenderingAttributes;

class QvisRenderingWindow;
class QGroupBox;
class QComboBox;
class QSpinBox;
class QPushButton;
class QLineEdit;
class QCheckBox;
class QStackedLayout;

namespace anari
{
    enum class BackendType 
    {
        NONE,
        EXAMPLE,
        USD,
        VISRTX
    };

    enum class RendererType
    {
        UNKNOWN,
        SCIVIS,
        AO,
        RAYCAST,
        DEBUG
    };
}

using BackendType = anari::BackendType;

class GUI_API AnariRenderingWidget : public QWidget
{
    Q_OBJECT
public:
    AnariRenderingWidget(QvisRenderingWindow *, 
                         RenderingAttributes *,
                         QWidget *parent = nullptr);
    ~AnariRenderingWidget() = default;

    BackendType GetBackendType(const std::string);
    int GetRowCount() const { return m_totalRows; }
    void UpdateUI();

    // General
    void UpdateLibraryNames(const std::string);
    void UpdateLibrarySubtypes(const std::string);
    void UpdateRendererSubtypes(const std::string);
    void SetChecked(const bool);

    // Back-end
    void UpdateSamplesPerPixel(const int);
    void UpdateAOSamples(const int);
    void UpdateDenoiserSelection(const bool);

signals:
    void currentBackendChanged(int);

private slots:
    void renderingToggled(bool);
    void libraryChanged(const QString &);
    void librarySubtypeChanged(const QString &);
    void rendererSubtypeChanged(const QString &);

    // General
    void samplesPerPixelChanged(int);
    void aoSamplesChanged(int);
    void denoiserToggled(bool);
    void lightFalloffChanged();
    void ambientIntensityChanged();
    void maxDepthChanged(int);
    void rValueChanged();
    void debugMethodChanged(const QString &);

    // USD
    void outputLocationChanged();
    void selectButtonPressed();
    
private:
    QWidget *CreateGeneralWidget(int &);
    QWidget *CreateBackendWidget(int &);
    QWidget *CreateUSDWidget(int &);

    QvisRenderingWindow *m_renderingWindow;
    RenderingAttributes *m_renderingAttributes;
    QStackedLayout *m_backendStackedLayout;

    int m_totalRows;

    // General Widget Components
    QGroupBox   *m_renderingGroup;
    QComboBox   *m_libraryNames;
    QComboBox   *m_librarySubtypes;
    QComboBox   *m_rendererSubtypes;

    // Backend widget UI components
    QSpinBox    *m_samplesPerPixel;
    QSpinBox    *m_aoSamples;
    QLineEdit   *m_lightFalloff;
    QLineEdit   *m_ambientIntensity;
    QSpinBox    *m_maxDepth;
    QLineEdit   *m_rValue;
    QComboBox   *m_debugMethod;
    QCheckBox   *m_denoiserToggle;
    // QPushButton       *m_anariLibraryDetails;

    // USD widget UI components
    QString     *m_outputDir;
    QLineEdit   *m_dirLineEdit;   
    QCheckBox   *m_commitCheckBox;  
}; 

#endif