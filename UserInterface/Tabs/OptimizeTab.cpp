#include "OptimizeTab.h"

#include <QCheckBox>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QWheelEvent>
#include <QTextDocument>
#include <QFont>

#include <CellarWorkbench/Image/Image.h>
#include <CellarWorkbench/GL/GlToolkit.h>

#include "../Dialogs/ConfigComparator.h"
#include "../SmoothingReport.h"
#include "GpuMeshCharacter.h"
#include "ui_MainWindow.h"

using namespace std;
using namespace cellar;


OptimizeTab::OptimizeTab(Ui::MainWindow* ui,
        const std::shared_ptr<GpuMeshCharacter>& character) :
    _ui(ui),
    _character(character),
    _reportWidget(nullptr)
{
    // Scehduling
    autoPilotToggled(_ui->scheduleAutoPilotRadio->isChecked());
    connect(_ui->scheduleAutoPilotRadio, &QRadioButton::toggled,
            this, &OptimizeTab::autoPilotToggled);

    minQualThresholdChanged(_ui->scheduleMinThresholdSpin->value());
    connect(_ui->scheduleMinThresholdSpin,
            static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &OptimizeTab::minQualThresholdChanged);

    qualMeanThresholdChanged(_ui->scheduleMeanThresholdSpin->value());
    connect(_ui->scheduleMeanThresholdSpin,
            static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &OptimizeTab::qualMeanThresholdChanged);


    fixedIterationsToggled(_ui->scheduleFixedIterationsRadio->isChecked());
    connect(_ui->scheduleFixedIterationsRadio, &QRadioButton::toggled,
            this, &OptimizeTab::fixedIterationsToggled);

    globalPassCount(_ui->scheduleGlobalPassCountSpin->value());
    connect(_ui->scheduleGlobalPassCountSpin,
            static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &OptimizeTab::globalPassCount);

    nodeRelocationPassCount(_ui->scheduleRelocPassCountSpin->value());
    connect(_ui->scheduleRelocPassCountSpin,
            static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &OptimizeTab::nodeRelocationPassCount);


    // Topology Modifications
    enableTopology(_ui->topoEnabledCheck->isChecked());
    connect(_ui->topoEnabledCheck, &QRadioButton::toggled,
            this, &OptimizeTab::enableTopology);

    topologyPassCount(_ui->topoPassCountSpin->value());
    connect(_ui->topoPassCountSpin,
            static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &OptimizeTab::topologyPassCount);

    connect(_ui->restructureMeshButton,
            static_cast<void(QPushButton::*)(bool)>(&QPushButton::clicked),
            this, &OptimizeTab::restructureMesh);


    // Node Relocation
    deployTechniques();
    connect(_ui->smoothingTechniqueMenu,
            static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged),
            this, &OptimizeTab::techniqueChanged);

    deployImplementations();
    connect(_ui->smoothingImplementationMenu,
            static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged),
            this, &OptimizeTab::implementationChanged);

    connect(_ui->smoothMeshButton,
            static_cast<void(QPushButton::*)(bool)>(&QPushButton::clicked),
            this, &OptimizeTab::smoothMesh);


    // Becnhmarking
    connect(_ui->smoothBenchmarkImplButton,
            static_cast<void(QPushButton::*)(bool)>(&QPushButton::clicked),
            this, &OptimizeTab::benchmarkImplementations);
}

OptimizeTab::~OptimizeTab()
{

}

void OptimizeTab::autoPilotToggled(bool checked)
{
    _schedule.autoPilotEnabled = checked;
    _ui->scheduleAutoPiltoWidget->setEnabled(checked);
}

void OptimizeTab::minQualThresholdChanged(double threshold)
{
    _schedule.minQualThreshold = threshold;
}

void OptimizeTab::qualMeanThresholdChanged(double threshold)
{
    _schedule.qualMeanThreshold = threshold;
}

void OptimizeTab::fixedIterationsToggled(bool checked)
{
    _ui->scheduleFixedIterationsWidget->setEnabled(checked);
}

void OptimizeTab::globalPassCount(int passCount)
{
    _schedule.globalPassCount = passCount;
}

void OptimizeTab::nodeRelocationPassCount(int passCount)
{
    _schedule.nodeRelocationsPassCount = passCount;
}

void OptimizeTab::enableTopology(bool checked)
{
    _schedule.topoOperationEnabled = checked;
}

void OptimizeTab::topologyPassCount(int count)
{
    _schedule.topoOperationPassCount = count;
}

void OptimizeTab::restructureMesh()
{
    _character->restructureMesh(
        _ui->topoPassCountSpin->value());
}

void OptimizeTab::techniqueChanged(const QString&)
{
    deployImplementations();
}

void OptimizeTab::implementationChanged(const QString& implName)
{
    _lastImpl = implName.toStdString();
}

void OptimizeTab::smoothMesh()
{
    _character->smoothMesh(
        _ui->smoothingTechniqueMenu->currentText().toStdString(),
        _ui->smoothingImplementationMenu->currentText().toStdString(),
        _schedule);
}

void OptimizeTab::benchmarkImplementations()
{
    ConfigComparator comparator(_character);

    comparator.show();
    if(comparator.exec() == QDialog::Accepted)
    {
        const string REPORT_PATH = "Reports/Report.pdf";
        const QString preShootPath = "Reports/PreSmoothingShot.png";
        const QString postShootPath = "Reports/PostSmoothingShot.png";

        Image preSmoothingShot;
        GlToolkit::takeFramebufferShot(preSmoothingShot);
        preSmoothingShot.save(preShootPath.toStdString());

        OptimizationPlot plot;
        _character->benchmarkSmoothers(
            plot, _schedule,
            comparator.configurations());

        QApplication::processEvents();

        Image postSmoothingShot;
        GlToolkit::takeFramebufferShot(postSmoothingShot);
        postSmoothingShot.save(postShootPath.toStdString());

        SmoothingReport report;
        report.setPreSmoothingShot(QImage(preShootPath));
        report.setPostSmoothingShot(QImage(postShootPath));
        report.setOptimizationPlot(plot);
        report.save(REPORT_PATH);

        delete _reportWidget;
        _reportWidget = new QTextEdit();
        _reportWidget->resize(1000, 800);
        report.display(*_reportWidget);
        _reportWidget->show();
    }
}

void OptimizeTab::deployTechniques()
{
    OptionMapDetails techniques = _character->availableSmoothers();

    _ui->smoothingTechniqueMenu->clear();
    for(const auto& name : techniques.options)
        _ui->smoothingTechniqueMenu->addItem(QString(name.c_str()));
    _ui->smoothingTechniqueMenu->setCurrentText(techniques.defaultOption.c_str());
}

void OptimizeTab::deployImplementations()
{
    OptionMapDetails implementations = _character->availableSmootherImplementations(
        _ui->smoothingTechniqueMenu->currentText().toStdString());


    // Fill implementation combo box
    bool lastImplFound = false;
    string lastImplCopy = _lastImpl;
    _ui->smoothingImplementationMenu->clear();
    for(const auto& name : implementations.options)
    {
        _ui->smoothingImplementationMenu->addItem(QString(name.c_str()));

        if(name == _lastImpl)
            lastImplFound = true;
    }

    if(lastImplFound)
    {
        _ui->smoothingImplementationMenu->setCurrentText(
                    lastImplCopy.c_str());
    }
    else
    {
        _lastImpl = implementations.defaultOption.c_str();
        _ui->smoothingImplementationMenu->setCurrentText(
                    implementations.defaultOption.c_str());
    }
}
