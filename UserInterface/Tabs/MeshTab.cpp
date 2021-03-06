#include "MeshTab.h"

#include <QFileDialog>

#include <CellarWorkbench/Image/Image.h>
#include <CellarWorkbench/GL/GlToolkit.h>

#include "GpuMeshCharacter.h"
#include "ui_MainWindow.h"

using namespace std;
using namespace cellar;


MeshTab::MeshTab(Ui::MainWindow* ui,
                 const std::shared_ptr<GpuMeshCharacter>& character) :
    _ui(ui),
    _character(character)
{
    deployTechniques();
    connect(_ui->geometryTechniqueMenu, &QComboBox::currentTextChanged,
            this, &MeshTab::techniqueChanged);

    deployModels();

    connect(_ui->generateMeshButton,
            static_cast<void(QPushButton::*)(bool)>(&QPushButton::clicked),
            this, &MeshTab::generateMesh);

    connect(_ui->clearMeshButton,
            static_cast<void(QPushButton::*)(bool)>(&QPushButton::clicked),
            this, &MeshTab::clearMesh);

    connect(_ui->saveMeshButton,
            static_cast<void(QPushButton::*)(bool)>(&QPushButton::clicked),
            this, &MeshTab::saveMesh);

    connect(_ui->loadMeshButton,
            static_cast<void(QPushButton::*)(bool)>(&QPushButton::clicked),
            this, &MeshTab::loadMesh);

    connect(_ui->screenshotButton,
            static_cast<void(QPushButton::*)(bool)>(&QPushButton::clicked),
            this, &MeshTab::screenshot);
}

MeshTab::~MeshTab()
{

}

void MeshTab::techniqueChanged(const QString&)
{
    deployModels();
}

void MeshTab::generateMesh()
{
    _character->generateMesh(
        _ui->geometryTechniqueMenu->currentText().toStdString(),
        _ui->geometryModelMenu->currentText().toStdString(),
        _ui->meshVertexCountSpin->value());
}

void MeshTab::clearMesh()
{
    _character->clearMesh();
}

void MeshTab::saveMesh()
{
    QString fileName = QFileDialog::getSaveFileName(
            nullptr, "Save Mesh", "resources/data/");
    if(!fileName.isNull())
    {
        if(QFileInfo(fileName).suffix().isEmpty())
            fileName = fileName.split(".").at(0) + ".json";

        _character->saveMesh(fileName.toStdString());
    }
}

void MeshTab::loadMesh()
{
    QString fileName = QFileDialog::getOpenFileName(
        nullptr, "Load Mesh", "resources/reports/mesh/");

    if(!fileName.isNull())
    {
        _character->loadMesh(fileName.toStdString());

        emit metricLoaded();
    }
}

void MeshTab::screenshot()
{
    Image screenshotImage;
    GlToolkit::takeFramebufferShot(screenshotImage);

    QString fileName = QFileDialog::getSaveFileName(
            nullptr, "Screenshot", "resources/screenshots");
    if(!fileName.isNull())
    {
        if(QFileInfo(fileName).suffix().isEmpty())
            fileName = fileName.split(".").at(0) + ".png";
        screenshotImage.save(fileName.toStdString());
    }
}

void MeshTab::deployTechniques()
{
    OptionMapDetails techniques = _character->availableMeshers();

    _ui->geometryTechniqueMenu->clear();
    for(const auto& name : techniques.options)
        _ui->geometryTechniqueMenu->addItem(QString(name.c_str()));
    _ui->geometryTechniqueMenu->setCurrentText(techniques.defaultOption.c_str());
}

void MeshTab::deployModels()
{
    OptionMapDetails models = _character->availableMeshModels(
        _ui->geometryTechniqueMenu->currentText().toStdString());

    _ui->geometryModelMenu->clear();
    for(const auto& name : models.options)
        _ui->geometryModelMenu->addItem(QString(name.c_str()));
    _ui->geometryModelMenu->setCurrentText(models.defaultOption.c_str());
}
