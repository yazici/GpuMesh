#include "MastersTestSuite.h"

#include <set>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <QDir>
#include <QString>
#include <QMargins>
#include <QTextTable>
#include <QTextCursor>
#include <QTextDocument>
#include <QCoreApplication>

#include <CellarWorkbench/DateAndTime/Calendar.h>
#include <CellarWorkbench/DataStructure/Grid2D.h>

#include "DataStructures/OptimizationPlot.h"
#include "DataStructures/Schedule.h"

#include "GpuMeshCharacter.h"

using namespace std;
using namespace cellar;


const string RESULTS_PATH = "resources/reports/";
const string RESULT_MESH_PATH = RESULTS_PATH + "mesh/";
const string RESULT_LATEX_PATH = RESULTS_PATH + "latex/";
const string RESULT_CSV_PATH = RESULTS_PATH + "csv/";
const string DATA_MESH_PATH = "resources/data/";

const string MESH_NODE_ORDER  = RESULT_MESH_PATH + "Sphere (node order).json";
const string MESH_TETCUBE_10K  = RESULT_MESH_PATH + "TetCube (N=10K).json";
const string MESH_TETCUBE_175K = RESULT_MESH_PATH + "TetCube (N=175K).json";
const string MESH_TETCUBE_500K = RESULT_MESH_PATH + "TetCube (N=500K).json";
const string MESH_HEXCUBE_10K  = RESULT_MESH_PATH + "HexCube (N=10K).json";
const string MESH_HEXCUBE_175K = RESULT_MESH_PATH + "HexCube (N=175K).json";
const string MESH_HEXCUBE_500K = RESULT_MESH_PATH + "HexCube (N=500K).json";
const string MESH_TURBINE_500K = RESULT_MESH_PATH + "Turbine (N=500K).cgns";
const string MESH_CAVITY_32K = RESULT_MESH_PATH + "Cavity/ALL.pie";

const string MESH_PRECISION_BASE = RESULT_MESH_PATH + "Precision (A=%1).json";
const string MESH_SCALING_BASE = RESULT_MESH_PATH + "Scaling (Scale=%1).json";


const int BS = 1e4;
const vector<int> SPHERE_TARGET_SIZES = {
    1*BS,  2*BS,  4*BS,  8*BS,
    16*BS, 32*BS, 64*BS, 128*BS};

double spherePower = 2.897029276;
double sphereCoeff = 3.1359873691;
double sphereSizeToScale(int size) {
    return glm::pow(size, 1/spherePower) / sphereCoeff;}

double cubePower = 2.917814;
double cubeCoeff = 3.141715;
double cubeSizeToScale(int size) {
    return glm::pow(size, 1/cubePower) / cubeCoeff;}


const double PRECISION_METRIC_K = 16.0;
vector<double> PRECISION_METRIC_As = {1, 2, 4, 8, 16};

const double NODE_ORDER_METRIC_K = cubeSizeToScale(100e3);

const double ADAPTATION_METRIC_K_10K  = cubeSizeToScale(10e3);
const double ADAPTATION_METRIC_K_175K = cubeSizeToScale(175e3);
const double ADAPTATION_METRIC_K_500K = cubeSizeToScale(500e3);
const double ADAPTATION_METRIC_A = 8;

const int ADAPTATION_TOPO_PASS = 5;
const int ADAPTATION_REFINEMENT_SWEEPS = 5;
const int ADAPTATION_RELOC_PASS = 10;


const int EVALUATION_THREAD_COUNT_GLSL = 16;
const int EVALUATION_THREAD_COUNT_CUDA = 32;

const int SMOOTHING_THREAD_COUNT_GLSL = 16;
const int SMOOTHING_THREAD_COUNT_CUDA = 32;

const int QUAL_MIN_PREC = 3;
const int QUAL_MEAN_PREC = 3;
const int TIME_SEC_PREC = 2;
const int TIME_MS_PREC = 0;
const int TIME_ACC_PREC = 1;


string testNumber(int n)
{
    return QString("%1").arg(n, 2, 10, QChar('0')).toStdString();
}

MastersTestSuite::MastersTestSuite(
        GpuMeshCharacter& character) :
    _character(character),
    _availableMastersTests("Master's tests")
{
    using namespace std::placeholders;

    int tId = 0;
    _availableMastersTests.setDefault("N/A");

    _availableMastersTests.setContent({

        {testNumber(++tId) + ". Metric Precision",
        MastersTestFunc(bind(&MastersTestSuite::metricPrecision,            this, _1))},

        {testNumber(++tId) + ". Texture Precision",
        MastersTestFunc(bind(&MastersTestSuite::texturePrecision,           this, _1))},

        {testNumber(++tId) + ". Evaluator Block Size",
        MastersTestFunc(bind(&MastersTestSuite::evaluatorBlockSize,         this, _1))},

        {testNumber(++tId) + ". Metric Cost (TetCube)",
        MastersTestFunc(bind(&MastersTestSuite::metricCostTetCube,          this, _1))},

        {testNumber(++tId) + ". Metric Cost (HexCube)",
        MastersTestFunc(bind(&MastersTestSuite::metricCostHexCube,          this, _1))},

        {testNumber(++tId) + ". Node Order",
        MastersTestFunc(bind(&MastersTestSuite::nodeOrder,                  this, _1))},

        {testNumber(++tId) + ". Smoothers Efficacity",
        MastersTestFunc(bind(&MastersTestSuite::smootherEfficacity,         this, _1))},

        {testNumber(++tId) + ". Smoothers Block Size (TetCube)",
        MastersTestFunc(bind(&MastersTestSuite::smootherBlockSizeTetCube,   this, _1))},

        {testNumber(++tId) + ". Smoothers Block Size (HexCube)",
        MastersTestFunc(bind(&MastersTestSuite::smootherBlockSizeHexCube,   this, _1))},

        {testNumber(++tId) + ". Smoothers Speed (TetCube)",
        MastersTestFunc(bind(&MastersTestSuite::smootherSpeedTetCube,       this, _1))},

        {testNumber(++tId) + ". Smoothers Speed (HexCube)",
        MastersTestFunc(bind(&MastersTestSuite::smootherSpeedHexCube,       this, _1))},

        {testNumber(++tId) + ". Relocation Scaling",
        MastersTestFunc(bind(&MastersTestSuite::relocationScaling,          this, _1))},

        {testNumber(++tId) + ". Smoothing Gain Speed",
        MastersTestFunc(bind(&MastersTestSuite::smoothingGainSpeed,         this, _1))},

        {testNumber(++tId) + ". Cavity Test Case",
        MastersTestFunc(bind(&MastersTestSuite::cavityTestCase,             this, _1))},
    });

    _translateSamplingTechniques = {
        {"Analytic",    "Analytique"},
        {"Local",       "Rech. loc."},
        {"Texture",     "Texture"},
        {"Kd-Tree",     "kD-Tree"},
    };

    _translateImplementations = {
        {"Serial",      "Séquentiel"},
        {"Thread",      "Parallèle"},
        {"GLSL",        "GLSL"},
        {"CUDA",        "CUDA"},
    };

    _translateSmoothers = {
        {"Spring Laplace",      "Laplace à ressorts"},
        {"Quality Laplace",     "Laplace de qualité"},
        {"Spawn Search",        "Force brute"},
        {"Nelder-Mead",         "Nelder-Mead"},
        {"Multi Elem NM",       "NM multiélément"},
        {"Gradient Descent",    "Descente du gradient"},
        {"Multi Elem GD",       "DG multiélément"},
        {"Multi Pos GD",        "DG multiposition"},
        {"Patch GD",            "DG multiaxe"},
        {"GETMe",               "GETMe"}
    };

    _meshNames = {
        {MESH_NODE_ORDER,  "Sphere"},
        {MESH_TETCUBE_10K,  "TetCube (N=10K)"},
        {MESH_TETCUBE_175K, "TetCube (N=175K)"},
        {MESH_TETCUBE_500K, "TetCube (N=500K)"},
        {MESH_HEXCUBE_10K,  "HexCube (N=10K)"},
        {MESH_HEXCUBE_175K, "HexCube (N=175K)"},
        {MESH_HEXCUBE_500K, "HexCube (N=500K)"},
        {MESH_TURBINE_500K, "Turbine (N=500K)"}
    };


    // Build directory structure
    if(!QDir(RESULTS_PATH.c_str()).exists())
        QDir().mkdir(RESULTS_PATH.c_str());

    if(!QDir(RESULT_LATEX_PATH.c_str()).exists())
        QDir().mkdir(RESULT_LATEX_PATH.c_str());

    if(!QDir(RESULT_MESH_PATH.c_str()).exists())
        QDir().mkdir(RESULT_MESH_PATH.c_str());

    if(!QDir(RESULT_CSV_PATH.c_str()).exists())
        QDir().mkdir(RESULT_CSV_PATH.c_str());
}

MastersTestSuite::~MastersTestSuite()
{

}

OptionMapDetails MastersTestSuite::availableTests() const
{
    return _availableMastersTests.details();
}

void MastersTestSuite::runTests(
        QTextDocument& reportDocument,
        const vector<string>& tests)
{
#ifdef COMPUTE_CUBE_SIZE_FUNC
    double cubeSmallK = 10.0;
    setupAdaptedCube(cubeSmallK, ADAPTATION_METRIC_A);
    double cubeSmallSize = _character.getNodeCount();

    double cubeBigK = 20.0;
    setupAdaptedCube(cubeBigK, ADAPTATION_METRIC_A);
    double cubeBigSize = _character.getNodeCount();

    _character.clearMesh();

    double cubePower = glm::log(cubeSmallK / cubeBigK) /
            glm::log(cubeSmallSize/cubeBigSize);
    double cubeCoeff = cubeSmallK / glm::pow(cubeSmallSize, cubePower);

    getLog().postMessage(new Message('D', false, "Cube K = N^(1/" +
        to_string(1/cubePower) + ") /" + to_string(1/cubeCoeff),
        "MastersTestSuite"));

    exit(0);
#endif

#ifdef COMPUTE_SPHERE_SIZE_FUNC
    double sphereSmallK = 10.0;
    setupAdaptedSphere(sphereSmallK, ADAPTATION_METRIC_A);
    double sphereSmallSize = _character.getNodeCount();

    double sphereBigK = 20.0;
    setupAdaptedSphere(sphereBigK, ADAPTATION_METRIC_A);
    double sphereBigSize = _character.getNodeCount();

    _character.clearMesh();

    double spherePower = glm::log(sphereSmallK / sphereBigK) /
            glm::log(sphereSmallSize/sphereBigSize);
    double sphereCoeff = sphereSmallK / glm::pow(sphereSmallSize, spherePower);

    getLog().postMessage(new Message('D', false, "Sphere K = N^(1/" +
        to_string(1/spherePower) + ") /" + to_string(1/sphereCoeff),
        "MastersTestSuite"));

    exit(0);
#endif

    // Prepare cases
    if(!QFile(MESH_NODE_ORDER.c_str()).exists())
    {
        _character.generateMesh("Delaunay", "Sphere", 10e3);

        _character.saveMesh(MESH_NODE_ORDER);
    }

    if(!QFile(MESH_TETCUBE_10K.c_str()).exists())
    {
        setupAdaptedCube(
            ADAPTATION_METRIC_K_10K,
            ADAPTATION_METRIC_A);

        _character.saveMesh(MESH_TETCUBE_10K);
    }

    if(!QFile(MESH_TETCUBE_175K.c_str()).exists())
    {
        setupAdaptedCube(
            ADAPTATION_METRIC_K_175K,
            ADAPTATION_METRIC_A);

        _character.saveMesh(MESH_TETCUBE_175K);
    }

    if(!QFile(MESH_TETCUBE_500K.c_str()).exists())
    {
        setupAdaptedCube(
            ADAPTATION_METRIC_K_500K,
            ADAPTATION_METRIC_A);

        _character.saveMesh(MESH_TETCUBE_500K);
    }

    if(!QFile(MESH_HEXCUBE_10K.c_str()).exists())
    {
        _character.useSampler("Analytic");

        _character.generateMesh("Debug", "HexGrid", 10e3);
        QCoreApplication::processEvents();

        _character.saveMesh(MESH_HEXCUBE_10K);
    }

    if(!QFile(MESH_HEXCUBE_175K.c_str()).exists())
    {
        _character.useSampler("Analytic");

        _character.generateMesh("Debug", "HexGrid", 175e3);
        QCoreApplication::processEvents();

        _character.saveMesh(MESH_HEXCUBE_175K);
    }

    if(!QFile(MESH_HEXCUBE_500K.c_str()).exists())
    {
        _character.useSampler("Analytic");

        _character.generateMesh("Debug", "HexGrid", 500e3);
        QCoreApplication::processEvents();

        _character.saveMesh(MESH_HEXCUBE_500K);
    }

    if(!QFile(QString(MESH_PRECISION_BASE.c_str()).arg(PRECISION_METRIC_As.back())).exists())
    {
        double metricA = PRECISION_METRIC_As.front();
        QString name = QString(MESH_PRECISION_BASE.c_str()).arg(metricA);

        if(!QFile(name).exists())
        {
            setupAdaptedCube(PRECISION_METRIC_K, PRECISION_METRIC_As.front());
            _character.saveMesh(name.toStdString());
        }
        else
        {
            _character.loadMesh(name.toStdString());
        }


        Schedule schedule;
        schedule.topoOperationEnabled = true;
        schedule.topoOperationPassCount = ADAPTATION_TOPO_PASS;
        schedule.refinementSweepCount = ADAPTATION_REFINEMENT_SWEEPS;
        _character.setMetricScaling(PRECISION_METRIC_K);

        for(int a=1; a < PRECISION_METRIC_As.size(); ++a)
        {
            metricA = PRECISION_METRIC_As[a];
            name = QString(MESH_PRECISION_BASE.c_str()).arg(metricA);

            if(QFile(name).exists())
            {
                _character.loadMesh(name.toStdString());
            }
            else
            {
                _character.useSampler("Analytic");
                _character.setMetricAspectRatio(metricA);
                _character.restructureMesh(schedule);

                _character.saveMesh(name.toStdString());
            }
        }
    }

    for(int s=0; s < SPHERE_TARGET_SIZES.size(); ++s)
    {
        size_t size = SPHERE_TARGET_SIZES[s];
        QString name = QString(MESH_SCALING_BASE.c_str()).arg(s);

        if(!QFile(name).exists())
        {
            setupAdaptedSphere(sphereSizeToScale(size), ADAPTATION_METRIC_A);
            _character.saveMesh(name.toStdString());
        }
    }

    _turbineIsMissing = false;
    if(!QFile(MESH_TURBINE_500K.c_str()).exists())
    {
        _turbineIsMissing = true;
        getLog().postMessage(new Message('W', true,
            "Missing 500K nodes turbine mesh",
            "MastersTestSuite"));
        getLog().postMessage(new Message('W', true,
            "Tests using this mesh won't execute",
            "MastersTestSuite"));
    }

    _cavityIsMissing = false;
    if(!QFile(MESH_CAVITY_32K.c_str()).exists())
    {
        _cavityIsMissing = true;
        getLog().postMessage(new Message('W', true,
            "Missing 32K Cavity mesh",
            "MastersTestSuite"));
        getLog().postMessage(new Message('W', true,
            "Tests using this mesh won't execute",
            "MastersTestSuite"));
    }


    _character.clearMesh();
    QCoreApplication::processEvents();


    reportDocument.clear();
    _reportDocument = &reportDocument;
    QTextCursor cursor(_reportDocument);


    cellar::Time duration;
    auto allStart = chrono::high_resolution_clock::now();

    for(const string& test : tests)
    {
        getLog().postMessage(new Message('I', false,
            "Running master's test: " + test,
            "MastersTestSuite"));

        MastersTestFunc func;
        if(_availableMastersTests.select(test, func))
        {
            cursor.movePosition(QTextCursor::End);
            cursor.insertBlock();
            cursor.insertHtml(("<h2>" + test + "</h2>").c_str());

            auto testStart = chrono::high_resolution_clock::now();

            func(test);

            auto testEnd = chrono::high_resolution_clock::now();
            duration.fromSeconds((testEnd - testStart).count() / (1.0E9));

            cursor.movePosition(QTextCursor::End);
            cursor.insertBlock();
            cursor.insertHtml(("[duration: " +
                duration.toString() + "]").c_str());

            _character.clearMesh();
            QCoreApplication::processEvents();

        }
        else
        {
            cursor.movePosition(QTextCursor::End);
            cursor.insertBlock();
            cursor.insertHtml("UNDEFINED TEST");
        }
    }

    auto allEnd = chrono::high_resolution_clock::now();
    duration.fromSeconds((allEnd - allStart).count() / (1.0E9));

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    cursor.insertHtml(("[Total duration: " +
            duration.toString() + "]").c_str());

    //QCoreApplication::quit();
}

void MastersTestSuite::setupAdaptedCube(
        double metricK,
        double metricA)
{
    _character.useSampler("Analytic");
    _character.setMetricAspectRatio(metricA);
    _character.useEvaluator("Metric Conformity");
    _character.generateMesh("Debug", "Cube", 8);

    QCoreApplication::processEvents();


    Schedule schedule;
    schedule.topoOperationEnabled = false;
    schedule.topoOperationPassCount = ADAPTATION_TOPO_PASS;
    schedule.refinementSweepCount = ADAPTATION_REFINEMENT_SWEEPS;
    schedule.relocationPassCount = ADAPTATION_RELOC_PASS;

    for(double k = 1.0;k < metricK; k *= glm::pow(2.0,1/3.0))
    {
        _character.setMetricScaling(k);
        _character.restructureMesh(schedule);

        if(k <= 2.0)
        {
            _character.smoothMesh(
                "Nelder-Mead",
                "Thread",
                schedule);
        }

        QCoreApplication::processEvents();
    }


    _character.setMetricScaling(metricK);
    _character.restructureMesh(schedule);

    QCoreApplication::processEvents();
}

void MastersTestSuite::setupAdaptedSphere(
        double metricK,
        double metricA)
{
    _character.useSampler("Analytic");
    _character.setMetricScaling(metricK);
    _character.setMetricAspectRatio(metricA);

    int approxSize = glm::pow(metricK * sphereCoeff, spherePower);
    _character.generateMesh("Delaunay", "Sphere", approxSize);

    Schedule schedule;
    schedule.topoOperationEnabled = true;
    schedule.topoOperationPassCount = 10;
    schedule.refinementSweepCount = ADAPTATION_REFINEMENT_SWEEPS;

    _character.restructureMesh(schedule);
}

void MastersTestSuite::saveToFile(
        const std::string& results,
        const std::string& fileName) const
{
    ofstream file;
    file.open(fileName, std::ios_base::trunc);

    if(file.is_open())
    {
        file << results;
        file.close();
    }
}

void MastersTestSuite::output(
        const std::string& title,
        const std::vector<std::string>& header,
        const std::vector<QualityHistogram>& histograms)
{
    saveCsvTable(title, header, histograms);
    saveLatexTable(title, header, histograms);
    saveReportTable(title, header, histograms);
}

void MastersTestSuite::saveCsvTable(
        const std::string& title,
        const std::vector<std::string>& header,
        const std::vector<QualityHistogram>& histograms)
{
    stringstream ss;

    ss << "Qualités" << ", ";

    for(int h=0; h < header.size(); ++h)
    {
        if(h != 0)
        {
            ss << ", ";
        }

        ss << header[h];
    }
    ss << "\n";

    size_t bc = histograms.front().bucketCount();

    for(int b=0; b < bc; ++b)
    {
        double bottom = b / double(bc);
        double top = (b+1) / double(bc);

        ss << bottom << " - " << top << ", ";

        for(int h=0; h < histograms.size(); ++h)
        {
            const QualityHistogram& hist =
                    histograms[h];

            if(h != 0)
            {
                ss << ", ";
            }

            ss << hist.buckets()[b];
        }

        ss << "\n";
    }
    ss << "\n";

    saveToFile(ss.str(), RESULT_CSV_PATH + title + ".csv");
}

void MastersTestSuite::saveLatexTable(
        const std::string& title,
        const std::vector<std::string>& header,
        const std::vector<QualityHistogram>& histograms)
{
    stringstream ss;

    ss << "\\hline\n";

    ss << "Qualités" << "\t& ";

    for(int h=0; h < header.size(); ++h)
    {
        if(h != 0)
        {
            ss << " \t& ";
        }

        ss << header[h];
    }
    ss << "\\\\\n";

    ss << "\\hline\n";

    size_t bc = histograms.front().bucketCount();

    for(int b=0; b < bc; ++b)
    {
        double bottom = b / double(bc);
        double top = (b+1) / double(bc);

        ss << bottom << " - " << top << "\t& ";

        for(int h=0; h < histograms.size(); ++h)
        {
            const QualityHistogram& hist =
                    histograms[h];

            if(h != 0)
            {
                ss << " \t& ";
            }

            ss << hist.buckets()[b];
        }

        ss << "\\\\\n";
    }
    ss << "\\hline\n";
    ss << "\n";

    saveToFile(ss.str(), RESULT_LATEX_PATH + title + ".txt");
}

void MastersTestSuite::saveReportTable(
        const std::string& title,
        const std::vector<std::string>& header,
        const std::vector<QualityHistogram>& histograms)
{
    QTextCursor cursor(_reportDocument);
    QTextBlockFormat blockFormat;
    QTextTableCell cell;
    QTextTable* table;

    QTextCharFormat tableHeaderFormat;
    tableHeaderFormat.setFontWeight(QFont::Bold);

    QTextTableFormat propertyTableFormat;
    propertyTableFormat.setCellPadding(5.0);
    propertyTableFormat.setBorderStyle(
        QTextFrameFormat::BorderStyle_Solid);

    size_t hc = histograms.size();
    size_t bc = histograms.front().bucketCount();

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock(blockFormat);
    cursor.insertHtml(("<h3>" + title + "</h3>").c_str());
    table = cursor.insertTable(
        bc + 1, hc+1, propertyTableFormat);


    cell = table->cellAt(0, 0);
    QTextCursor cellCursor = cell.firstCursorPosition();
    cellCursor.insertText("Qualités", tableHeaderFormat);


    for(int h=0; h < header.size(); ++h)
    {
        cell = table->cellAt(0, h+1);
        QTextCursor cellCursor =
            cell.firstCursorPosition();

        cellCursor.insertText(header[h].c_str(), tableHeaderFormat);
    }

    for(int b=0; b < bc; ++b)
    {
        double bottom = b / double(bc);
        double top = (b+1) / double(bc);

        cell = table->cellAt(b+1, 0);
        QTextCursor cellCursor =
            cell.firstCursorPosition();

        cellCursor.insertText(QString("%1 - %2")
                .arg(bottom).arg(top));


        for(int h=0; h < histograms.size(); ++h)
        {
            const QualityHistogram& hist =
                    histograms[h];

            cell = table->cellAt(b+1, h+1);
            QTextCursor cellCursor =
                cell.firstCursorPosition();

            cellCursor.insertText(
                QString::number(hist.buckets()[b]));
        }
    }

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock(blockFormat);
}

void MastersTestSuite::output(
        const std::string& title,
        const std::vector<std::pair<std::string, int>>& header,
        const std::vector<std::pair<std::string, int>>& subHeader,
        const std::vector<std::string>& lineNames,
        const std::vector<int>& columnPrecision,
        const cellar::Grid2D<double>& data)
{
    saveCsvTable(title, header, subHeader, lineNames, columnPrecision, data);
    saveLatexTable(title, header, subHeader, lineNames, columnPrecision, data);
    saveReportTable(title, header, subHeader, lineNames, columnPrecision, data);
}

void MastersTestSuite::saveCsvTable(
        const std::string& title,
        const std::vector<std::pair<std::string, int>>& header,
        const std::vector<std::pair<std::string, int>>& subHeader,
        const std::vector<std::string>& lineNames,
        const std::vector<int>& columnPrecision,
        const cellar::Grid2D<double>& data)
{
    stringstream ss;

    for(int h=0; h < header.size(); ++h)
    {
        int width = header[h].second;
        const string& name = header[h].first;

        if(h != 0)
        {
            ss << ", ";
        }

        ss << name;

        for(int w=1; w < width; ++w)
            ss << ", ";
    }
    ss << "\n";

    if(!subHeader.empty())
    {
        for(int h=0; h < subHeader.size(); ++h)
        {
            int width = subHeader[h].second;
            const string& name = subHeader[h].first;

            if(h != 0)
            {
                ss << ", ";
            }

            ss << name;

            for(int w=1; w < width; ++w)
                ss << ", ";
        }
        ss << "\n";
    }

    for(int l=0; l < lineNames.size(); ++l)
    {
        ss << lineNames[l];

        for(int c=0; c < data.getWidth(); ++c)
        {
            ss << ", " << data[l][c];
        }

        ss << "\n";
    }
    ss << "\n";

    saveToFile(ss.str(), RESULT_CSV_PATH + title + ".csv");
}

void MastersTestSuite::saveLatexTable(
        const std::string& title,
        const std::vector<std::pair<std::string, int>>& header,
        const std::vector<std::pair<std::string, int>>& subHeader,
        const std::vector<std::string>& lineNames,
        const std::vector<int>& columnPrecision,
        const cellar::Grid2D<double>& data)
{
    stringstream ss;

    ss << "\\hline\n";

    for(int h=0; h < header.size(); ++h)
    {
        int width = header[h].second;
        const string& name = header[h].first;

        if(h != 0)
        {
            ss << " \t& ";
        }

        if(width > 1)
        {
            ss << "\\multicolumn{" << width << "}{c|}{" << name << "}";
        }
        else
        {
            ss << name;
        }
    }
    ss << "\\\\\n";

    if(!subHeader.empty())
    {
        for(int h=0; h < subHeader.size(); ++h)
        {
            int width = subHeader[h].second;
            const string& name = subHeader[h].first;

            if(h != 0)
            {
                ss << " \t& ";
            }

            if(width > 1)
            {
                ss << "\\multicolumn{" << width << "}{c|}{" << name << "}";
            }
            else
            {
                ss << name;
            }
        }
        ss << "\\\\\n";
    }
    ss << "\\hline\n";

    ss << std::fixed;

    for(int l=0; l < lineNames.size(); ++l)
    {
        ss << lineNames[l];

        for(int c=0; c < data.getWidth(); ++c)
        {
            ss << " \t& " <<
               std::setprecision(columnPrecision[c])
               << data[l][c];
        }

        ss << "\\\\\n";
    }

    ss << "\\hline\n";
    ss << "\n";

    saveToFile(ss.str(), RESULT_LATEX_PATH + title + ".txt");
}

void MastersTestSuite::saveReportTable(
        const std::string& title,
        const std::vector<std::pair<std::string, int>>& header,
        const std::vector<std::pair<std::string, int>>& subHeader,
        const std::vector<std::string>& lineNames,
        const std::vector<int>& columnPrecision,
        const cellar::Grid2D<double>& data)
{
    QTextCursor cursor(_reportDocument);
    QTextBlockFormat blockFormat;
    QTextTableCell cell;
    QTextTable* table;

    QTextTableFormat propertyTableFormat;
    propertyTableFormat.setCellPadding(5.0);
    propertyTableFormat.setBorderStyle(
        QTextFrameFormat::BorderStyle_Solid);

    QTextCharFormat tableHeaderFormat;
    tableHeaderFormat.setFontWeight(QFont::Bold);


    size_t lc = data.getHeight() + 1;
    if(!subHeader.empty()) ++lc;
    size_t cc = data.getWidth() + 1;

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock(blockFormat);
    cursor.insertHtml(("<h3>" + title + "</h3>").c_str());
    table = cursor.insertTable(
        lc, cc, propertyTableFormat);

    int headpos = 0;
    int linepos = 0;
    for(int h=0; h < header.size(); ++h)
    {
        int width = header[h].second;
        const string& name = header[h].first;

        table->mergeCells(0, headpos, 1, width);
        cell = table->cellAt(0, headpos);
        QTextCursor cellCursor =
            cell.firstCursorPosition();

        cellCursor.insertText(name.c_str(), tableHeaderFormat);

        headpos += width;
    }
    ++linepos;

    if(!subHeader.empty())
    {
        int headpos = 0;
        for(int h=0; h < subHeader.size(); ++h)
        {
            int width = subHeader[h].second;
            const string& name = subHeader[h].first;

            table->mergeCells(1, headpos, 1, width);
            cell = table->cellAt(1, headpos);
            QTextCursor cellCursor =
                cell.firstCursorPosition();

            cellCursor.insertText(name.c_str());

            headpos += width;
        }
        ++linepos;
    }

    for(int l=0; l < lineNames.size(); ++l)
    {
        cell = table->cellAt(linepos+l, 0);
        QTextCursor cellCursor =
            cell.firstCursorPosition();

        cellCursor.insertText(lineNames[l].c_str());

        for(int c=0; c < data.getWidth(); ++c)
        {
            cell = table->cellAt(linepos+l, c+1);
            cellCursor = cell.firstCursorPosition();

                cellCursor.insertText(
                    QString::number(data[l][c],
                        'f', columnPrecision[c]));
        }
    }

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock(blockFormat);
}

void MastersTestSuite::metricPrecision(
        const string& testName)
{
    // Test case description
    string implementation = "Thread";
    string smoother = "Gradient Descent";
    string evaluator = "Metric Conformity";

    vector<string> samplings = {
        "Analytic",
        "Local",
        "Texture",
        //"Kd-Tree"
    };

    Schedule schedule;
    schedule.autoPilotEnabled = false;
    schedule.topoOperationEnabled = false;
    schedule.relocationPassCount = ADAPTATION_RELOC_PASS;


    // Setup test
    _character.setMetricScaling(PRECISION_METRIC_K);

    _character.useEvaluator(evaluator);


    // Run test
    vector<QualityHistogram> histograms;
    Grid2D<double> minData(samplings.size() + 1, PRECISION_METRIC_As.size(), 0.0);
    Grid2D<double> meanData(samplings.size() + 1, PRECISION_METRIC_As.size(), 0.0);

    for(int a=0; a < PRECISION_METRIC_As.size(); ++a)
    {
        double metricA = PRECISION_METRIC_As[a];

        _character.setMetricAspectRatio(metricA);
        _character.loadMesh(QString(MESH_PRECISION_BASE.c_str())
            .arg(metricA).toStdString());

        OptimizationPlot plot;
        vector<Configuration> configs;
        for(const string& samp : samplings)
            configs.push_back(Configuration{
                samp, smoother, implementation});

        _character.benchmarkSmoothers(
            plot, schedule, configs);

        const QualityHistogram& initHist = plot.initialHistogram();

        minData[a][0] = initHist.minimumQuality();
        meanData[a][0] = initHist.harmonicMean();
        for(int s=0; s < samplings.size(); ++s)
        {
            const QualityHistogram& hist =
                plot.implementations()[s].finalHistogram;
            minData[a][s+1] = hist.minimumQuality();
            meanData[a][s+1] = hist.harmonicMean();
        }

        if(a == PRECISION_METRIC_As.size()-1)
        {
            histograms.push_back(initHist);

            for(int s=0; s < samplings.size(); ++s)
                histograms.push_back(
                    plot.implementations()[s].finalHistogram);
        }
    }


    // Print results
    vector<pair<string, int>> header = {{"A", 1}};
    header.push_back({"Initial", 1});
    for(const string& s : samplings)
        header.push_back({_translateSamplingTechniques[s], 1});

    vector<pair<string, int>> subheader = {};

    vector<string> lineNames = {};
    for(double r : PRECISION_METRIC_As)
        lineNames.push_back(QString::number(r).toStdString());

    vector<string> histHeader = {"Initial"};
    for(const string& s : samplings)
        histHeader.push_back(_translateSamplingTechniques[s]);

    vector<int> minPrec(samplings.size()+1, QUAL_MIN_PREC);
    vector<int> meanPrec(samplings.size()+1, QUAL_MEAN_PREC);

    output(testName + "(Minimums)",
           header, subheader, lineNames, minPrec, minData);
    output(testName + "(Harmonic means)",
           header, subheader, lineNames, meanPrec, meanData);
    output(testName + "(Histograms)",
           histHeader, histograms);
}

void MastersTestSuite::texturePrecision(
        const string& testName)
{
    // Test case description
    double metricK = PRECISION_METRIC_K;
    double metricA = PRECISION_METRIC_As.back();
    string mesh = QString(MESH_PRECISION_BASE.c_str())
            .arg(metricA).toStdString();

    string implementation = "Thread";
    string smoother = "Gradient Descent";
    string evaluator = "Metric Conformity";
    string sampling = "Texture";

    vector<double> depths;
    const int resolutionCount = 10;
    const double baseResolution = 16;
    const double cbRtOfTwo = glm::pow(2.0, 1/3.0);
    for(int r = 0; r < resolutionCount; ++r)
        depths.push_back(glm::round(
            baseResolution * glm::pow(cbRtOfTwo, r)));


    Schedule schedule;
    schedule.autoPilotEnabled = false;
    schedule.topoOperationEnabled = false;
    schedule.relocationPassCount = ADAPTATION_RELOC_PASS;


    // Setup test
    _character.useEvaluator(evaluator);

    _character.setMetricScaling(metricK);
    _character.setMetricAspectRatio(metricA);


    // Run test
    Grid2D<double> data(2, depths.size() + 1, 0.0);

    for(int d=0; d < depths.size(); ++d)
    {
        _character.loadMesh(mesh);
        _character.setMetricDiscretizationDepth(depths[d]);

        OptimizationPlot plot;
        vector<Configuration> configs;
        configs.push_back(Configuration{
            sampling, smoother, implementation});

        _character.benchmarkSmoothers(
            plot, schedule, configs);


        if(d == 0)
        {
            const QualityHistogram& initHist =
                    plot.initialHistogram();

            data[0][0] = initHist.minimumQuality();
            data[0][1] = initHist.harmonicMean();
        }


        const QualityHistogram& hist =
            plot.implementations()[0].finalHistogram;

        data[d+1][0] = hist.minimumQuality();
        data[d+1][1] = hist.harmonicMean();
    }


    // Print results
    vector<pair<string, int>> header = {
        {"Résolutions", 1}, {"Minimums", 1}, {"Moyennes", 1}};
    vector<pair<string, int>> subheader = {};

    vector<string> lineNames = {"Initial"};
    for(int d : depths)
        lineNames.push_back(to_string(d));

    vector<int> precision = {QUAL_MIN_PREC, QUAL_MEAN_PREC};

    output(testName, header, subheader, lineNames, precision, data);
}

void MastersTestSuite::evaluatorBlockSize(
        const string& testName)
{
    // Test case description
    const vector<string> meshes = {
        MESH_TETCUBE_500K,
        MESH_HEXCUBE_500K
    };

    const string sampler = "Texture";

    const string evaluator = "Metric Conformity";

    vector<string> implementations = {
        "GLSL", "CUDA"
    };

    map<string, int> cycleCounts;
    for(const string& impl : implementations)
        cycleCounts[impl] = 5;

    vector<uint> threadCounts = {
        1, 2, 4, 8, 16, 32,
        64, 128, 192, 256, //512, 1024
    };


    // Setup test
    _character.useSampler(sampler);
    _character.setMetricScaling(ADAPTATION_METRIC_K_500K);
    _character.setMetricAspectRatio(ADAPTATION_METRIC_A);

    _character.useEvaluator(evaluator);


    // Run test
    Grid2D<double> data(meshes.size() * implementations.size(), threadCounts.size(), 0.0);

    for(int m=0; m < meshes.size(); ++m)
    {
        _character.loadMesh(meshes[m]);

        //QCoreApplication::processEvents();


        for(int t=0; t < threadCounts.size(); ++t)
        {
            uint tc = threadCounts[t];
            _character.setGlslEvaluatorThreadCount(tc);
            _character.setCudaEvaluatorThreadCount(tc);

            map<string, double> avrgTimes;
            _character.benchmarkEvaluator(
                avrgTimes, evaluator, cycleCounts);

            data[t][m*2 + 0] = avrgTimes[implementations[0]];
            data[t][m*2 + 1] = avrgTimes[implementations[1]];
        }
    }


    // Print results
    vector<pair<string, int>> header = {{"Tailles", 1}};
    vector<pair<string, int>> subheader = {{"", 1}};
    for(const string& m : meshes)
    {
        header.push_back({_meshNames[m], 2});

        for(const string& i : implementations)
            subheader.push_back({
                _translateImplementations[i], 1});
    }

    vector<string> lineNames;
    for(int tc : threadCounts)
        lineNames.push_back(to_string(tc));

    vector<int> precision(meshes.size()*implementations.size(), TIME_MS_PREC);

    output(testName, header, subheader, lineNames, precision, data);
}

void MastersTestSuite::metricCost(
        const string& testName,
        const string& mesh)
{
    // Test case description
    size_t evalCycleCount = 5;
    string evaluator = "Metric Conformity";

    vector<string> samplings = {
        "Analytic",
        "Local",
        "Texture",
        //"Kd-Tree"
    };

    vector<string> implementations = {
        "Serial",
        "Thread",
        "GLSL",
        "CUDA"
    };

    map<string, int> implCycle;
    for(const auto& impl : implementations)
        implCycle[impl] = evalCycleCount;


    // Setup test
    _character.loadMesh(mesh);

    //QCoreApplication::processEvents();

    _character.setMetricScaling(ADAPTATION_METRIC_K_500K);
    _character.setMetricAspectRatio(ADAPTATION_METRIC_A);
    _character.setGlslEvaluatorThreadCount(EVALUATION_THREAD_COUNT_GLSL);
    _character.setCudaEvaluatorThreadCount(EVALUATION_THREAD_COUNT_CUDA);


    // Run test
    int implCount = implementations.size();
    Grid2D<double> data((implCount-1)*2+1, samplings.size(), 0.0);

    for(int s=0; s < samplings.size(); ++s)
    {
        const string& samp = samplings[s];
        _character.useSampler(samp);

        // Process exceptions
        map<string, int> currCycle = implCycle;
        if(mesh == MESH_HEXCUBE_500K && samp == "Local")
        {
            currCycle["GLSL"] = 0;
        }

        map<string, double> avgTimes;
        _character.benchmarkEvaluator(avgTimes, evaluator, currCycle);

        for(int i=0; i < implCount; ++i)
            data[s][i] = avgTimes[implementations[i]];

        for(int i=1; i < implCount; ++i)
            data[s][implCount+i-1] = avgTimes[implementations[0]]
                    / avgTimes[implementations[i]];
    }


    // Print results
    vector<pair<string, int>> header = {
        {"Métriques", 1},
        {"Temps (ms)", implementations.size()},
        {"Accélérations", implementations.size()-1}};

    vector<pair<string, int>> subheader = {{"", 1}};
    for(const string& i : implementations)
        subheader.push_back({
            _translateImplementations[i], 1});

    for(int i=1; i < implementations.size(); ++i)
        subheader.push_back({
            _translateImplementations[implementations[i]], 1});


    vector<string> lineNames;
    for(const string& s : samplings)
        lineNames.push_back(_translateSamplingTechniques[s]);

    vector<int> precisions;
    for(int i=0; i < implementations.size(); ++i)
        precisions.push_back(TIME_MS_PREC);
    for(int i=1; i < implementations.size(); ++i)
        precisions.push_back(TIME_ACC_PREC);

    output(testName, header, subheader, lineNames, precisions, data);
}

void MastersTestSuite::metricCostTetCube(
        const string& testName)
{
    return metricCost(testName, MESH_TETCUBE_500K);
}

void MastersTestSuite::metricCostHexCube(
        const string& testName)
{
    return metricCost(testName, MESH_HEXCUBE_500K);
}

void MastersTestSuite::nodeOrder(
        const string& testName)
{
    // Test case description
    int fullRelocationPassCount = 100;
    int displayRelocationPassCount = 10;
    string mesh = MESH_NODE_ORDER;

    const string sampler = "Analytic";

    const string evaluator = "Metric Conformity";

    const string smoother = "Gradient Descent";

    const vector<string> implementations = {
        "Serial",
        "Thread",
        "GLSL",
        "CUDA"
    };

    OptimizationPlot plot;
    vector<Configuration> configs;
    for(const auto& impl : implementations)
        configs.push_back(Configuration{
            sampler, smoother, impl});

    Schedule schedule;
    schedule.autoPilotEnabled = false;
    schedule.topoOperationEnabled = false;
    schedule.relocationPassCount = fullRelocationPassCount;


    // Setup test
    _character.loadMesh(mesh);

    _character.useSampler(sampler);
    _character.setMetricScaling(NODE_ORDER_METRIC_K);
    _character.setMetricAspectRatio(ADAPTATION_METRIC_A);

    _character.useEvaluator(evaluator);


    // Run test
    _character.benchmarkSmoothers(
        plot, schedule, configs);

    Grid2D<double> dataCPU(2 * 2, displayRelocationPassCount+2, 0.0);
    Grid2D<double> dataPara((implementations.size()-1) * 2, displayRelocationPassCount+2, 0.0);

    for(int r=0; r <= displayRelocationPassCount; ++r)
    {
        for(int i=0; i < 2; ++i)
        {
            const QualityHistogram& hist =
                plot.implementations()[i].passes[r].histogram;
            dataCPU[r][0 + i] = hist.minimumQuality();
            dataCPU[r][2 + i] = hist.harmonicMean();
        }

        for(int i=1; i < implementations.size(); ++i)
        {
            const QualityHistogram& hist =
                plot.implementations()[i].passes[r].histogram;
            dataPara[r][0 + i-1] = hist.minimumQuality();
            dataPara[r][3 + i-1] = hist.harmonicMean();
        }
    }

    for(int i=0; i < 2; ++i)
    {
        const QualityHistogram& hist =
            plot.implementations()[i].passes[fullRelocationPassCount].histogram;
        dataCPU[displayRelocationPassCount+1][0 + i] = hist.minimumQuality();
        dataCPU[displayRelocationPassCount+1][2 + i] = hist.harmonicMean();
    }

    for(int i=1; i < implementations.size(); ++i)
    {
        const QualityHistogram& hist =
            plot.implementations()[i].passes[fullRelocationPassCount].histogram;
        dataPara[displayRelocationPassCount+1][0 + i-1] = hist.minimumQuality();
        dataPara[displayRelocationPassCount+1][3 + i-1] = hist.harmonicMean();
    }

    vector<QualityHistogram> histograms;
    histograms.push_back(plot.initialHistogram());
    for(int i=0; i < implementations.size(); ++i)
        histograms.push_back(plot.implementations()[i].finalHistogram);


    // Print results
    vector<pair<string, int>> headerCPU = {
        {"Itérations", 1}, {"Minimums", 2}, {"Moyennes", 2}};
    vector<pair<string, int>> headerPara = {
        {"Itérations", 1}, {"Minimums", 3}, {"Moyennes", 3}};

    vector<pair<string, int>> subheaderCPU = {{"", 1}};
    for(int m=0; m < 2; ++m)
    {
        for(int i=0; i < 2; ++i)
        {
            subheaderCPU.push_back(
                {_translateImplementations[implementations[i]], 1});
        }
    }

    vector<pair<string, int>> subheaderPara = {{"", 1}};
    for(int m=0; m < 2; ++m)
    {
        for(int i=1; i < implementations.size(); ++i)
        {
            subheaderPara.push_back(
                {_translateImplementations[implementations[i]], 1});
        }
    }

    vector<string> lineNames;
    for(int p=0; p <= displayRelocationPassCount; ++p)
        lineNames.push_back(to_string(p));
    lineNames.push_back(to_string(fullRelocationPassCount));

    vector<string> histHeader = {"Initial"};
    for(int i=0; i < implementations.size(); ++i)
    {
        histHeader.push_back(_translateImplementations[implementations[i]]);
    }

    vector<int> cpuPrecisions = {
        QUAL_MIN_PREC, QUAL_MIN_PREC,
        QUAL_MEAN_PREC, QUAL_MEAN_PREC};
    vector<int> paraPrecisions = {
        QUAL_MIN_PREC, QUAL_MIN_PREC, QUAL_MIN_PREC,
        QUAL_MEAN_PREC, QUAL_MEAN_PREC, QUAL_MEAN_PREC};

    output(testName + "(CPU)",      headerCPU,  subheaderCPU,  lineNames, cpuPrecisions, dataCPU);
    output(testName + "(Parallel)", headerPara, subheaderPara, lineNames, paraPrecisions, dataPara);
    output(testName + "(Histograms)", histHeader, histograms);
}

void MastersTestSuite::smootherEfficacity(
        const string& testName)
{
    // Test case description
    vector<string> meshes = {
        MESH_TETCUBE_10K,
        MESH_HEXCUBE_10K,
    };

    string sampler = "Analytic";

    string evaluator = "Metric Conformity";

    vector<string> smoothers = {
        "Spring Laplace",
        "Quality Laplace",
        "Spawn Search",
        "Nelder-Mead",
        "Gradient Descent"
    };

    string implementation = "Thread";

    vector<Configuration> configs;
    for(const auto& s : smoothers)
            configs.push_back(Configuration{
                sampler, s, implementation});


    Schedule schedule;
    schedule.autoPilotEnabled = false;
    schedule.topoOperationEnabled = false;
    schedule.relocationPassCount = ADAPTATION_RELOC_PASS;


    // Setup test
    _character.useSampler(sampler);
    _character.setMetricScaling(ADAPTATION_METRIC_K_10K);
    _character.setMetricAspectRatio(ADAPTATION_METRIC_A);

    _character.useEvaluator(evaluator);


    // Run test
    Grid2D<double> data(2*meshes.size(), smoothers.size()+1, 0.0);

    for(int m=0; m < meshes.size(); ++m)
    {
        _character.loadMesh(meshes[m]);

        OptimizationPlot plot;
        _character.benchmarkSmoothers(
            plot, schedule, configs);

        const QualityHistogram& initHist = plot.initialHistogram();
        data[0][0+m*2] = initHist.minimumQuality();
        data[0][1+m*2] = initHist.harmonicMean();

        for(int s=0; s < smoothers.size(); ++s)
        {
            const QualityHistogram& hist =
                plot.implementations()[s].finalHistogram;

            data[s+1][0+m*2] = hist.minimumQuality();
            data[s+1][1+m*2] = hist.harmonicMean();
        }
    }


    // Print results
    vector<pair<string, int>> header = {{"Métriques", 1}};
    vector<pair<string, int>> subheader = {{"", 1}};
    for(int m=0; m < meshes.size(); ++m)
    {
        header.push_back({_meshNames[meshes[m]], 2});
        subheader.push_back({"Minimums", 1});
        subheader.push_back({"Moyennes", 1});
    }

    vector<string> lineNames = {"Initial"};
    for(const string& s : smoothers)
        lineNames.push_back(_translateSmoothers[s]);

    vector<int> precisions;
    for(const auto& m : meshes)
    {
        precisions.push_back(QUAL_MIN_PREC);
        precisions.push_back(QUAL_MEAN_PREC);
    }

    output(testName, header, subheader, lineNames, precisions, data);
}

void MastersTestSuite::smootherBlockSize(
        const string& testName,
        const string& mesh)
{
    // Test case description
    string sampler = "Texture";

    string evaluator = "Metric Conformity";

    vector<string> smoothers = {
        "Gradient Descent",
        "Nelder-Mead"
    };

    vector<string> implementations = {
        "GLSL", "CUDA"
    };

    vector<uint> threadCounts = {
        1, 2, 4, 8, 16, 32,
        64, 128, 192, 256//, 512, 1024
    };

    vector<Configuration> configs;
    for(const auto& smooth : smoothers)
        for(const auto& impl : implementations)
            configs.push_back(Configuration{
                sampler, smooth, impl});

    Schedule schedule;
    schedule.autoPilotEnabled = false;
    schedule.topoOperationEnabled = false;
    schedule.relocationPassCount = ADAPTATION_RELOC_PASS;


    // Setup test
    _character.useSampler(sampler);
    _character.setMetricScaling(ADAPTATION_METRIC_K_175K);
    _character.setMetricAspectRatio(ADAPTATION_METRIC_A);

    _character.useEvaluator(evaluator);
    _character.setGlslEvaluatorThreadCount(EVALUATION_THREAD_COUNT_GLSL);
    _character.setCudaEvaluatorThreadCount(EVALUATION_THREAD_COUNT_CUDA);


    // Run test
    Grid2D<double> data(smoothers.size()*implementations.size(), threadCounts.size(), 0.0);

    for(int t=0; t < threadCounts.size(); ++t)
    {
        _character.loadMesh(mesh);

        uint tc = threadCounts[t];

        _character.setGlslSmootherThreadCount(tc);
        _character.setCudaSmootherThreadCount(tc);

        OptimizationPlot plot;
        _character.benchmarkSmoothers(
            plot, schedule, configs);

        for(int s=0; s < smoothers.size(); ++s)
        {
            for(int i=0; i < implementations.size(); ++i)
            {
                int id = s*implementations.size() + i;
                data[t][id] = plot.implementations()[id]
                        .passes.back().timeStamp;
            }
        }
    }


    // Print results
    vector<pair<string, int>> header = {{"Tailles", 1}};
    vector<pair<string, int>> subheader = {{"", 1}};
    for(const string& smooth : smoothers)
    {
        header.push_back({_translateSmoothers[smooth], implementations.size()});
        for(const string& impl : implementations)
        {
            subheader.push_back({_translateImplementations[impl], 1});
        }
    }

    vector<string> lineNames;
    for(int tc : threadCounts)
        lineNames.push_back(to_string(tc));

    vector<int> precisions;
    for(const auto& s : smoothers)
        for(const auto& i : implementations)
            precisions.push_back(TIME_MS_PREC);

    output(testName, header, subheader, lineNames, precisions, data);
}

void MastersTestSuite::smootherBlockSizeTetCube(
        const string& testName)
{
    string mesh = MESH_TETCUBE_175K;

    smootherBlockSize(testName, mesh);
}

void MastersTestSuite::smootherBlockSizeHexCube(
        const string& testName)
{
    string mesh = MESH_HEXCUBE_175K;

    smootherBlockSize(testName, mesh);
}

void MastersTestSuite::smootherSpeed(
        const std::string& testName,
        const std::string& mesh)
{
    // Test case description
    string evaluator = "Metric Conformity";

    string cpuSampler = "Local";
    string gpuSampler = "Texture";

    vector<string> smoothers = {
        "Spring Laplace",
        "Quality Laplace",
        "Spawn Search",
        "Nelder-Mead",
        "Multi Elem NM",
        "Gradient Descent",
        "Multi Elem GD",
        "Multi Pos GD",
        "Patch GD",
    };

    int nelderMeadPos = 3;
    int gradDescPos = 5;

    vector<string> implementations = {
        "Serial",
        "Thread",
        "GLSL",
        "CUDA"
    };

    vector<int> cpuLocs = {
        0, 1, 2, 3, 5
    };

    vector<int> refImpls = {
        0,1,    // Laplace
        2,      // Spawn

        nelderMeadPos, nelderMeadPos,

        gradDescPos, gradDescPos,
        gradDescPos, gradDescPos
    };

    std::vector<Configuration> configs;

    for(int s=0; s <= nelderMeadPos; ++s)
        configs.push_back({cpuSampler, smoothers[s], "Serial"});
    configs.push_back({cpuSampler, smoothers[gradDescPos], "Serial"});
    int serialConfigEnd = configs.size();

    for(int s=0; s <= nelderMeadPos; ++s)
        configs.push_back({cpuSampler, smoothers[s], "Thread"});
    configs.push_back({cpuSampler, smoothers[gradDescPos], "Thread"});
    int threadConfigEnd = configs.size();

    for(int s=0; s < smoothers.size(); ++s)
        configs.push_back({gpuSampler, smoothers[s], "GLSL"});
    int glslConfigEnd = configs.size();

    for(int s=0; s < smoothers.size(); ++s)
        configs.push_back({gpuSampler, smoothers[s], "CUDA"});
    int cudaConfigEnd = configs.size();


    Schedule schedule;
    schedule.autoPilotEnabled = false;
    schedule.topoOperationEnabled = false;
    schedule.relocationPassCount = ADAPTATION_RELOC_PASS;


    // Setup test
    _character.loadMesh(mesh);

    _character.useSampler("Analytic");
    _character.setMetricScaling(ADAPTATION_METRIC_K_175K);
    _character.setMetricAspectRatio(ADAPTATION_METRIC_A);

    _character.useEvaluator(evaluator);
    _character.setGlslEvaluatorThreadCount(EVALUATION_THREAD_COUNT_GLSL);
    _character.setCudaEvaluatorThreadCount(EVALUATION_THREAD_COUNT_CUDA);

    _character.setGlslSmootherThreadCount(SMOOTHING_THREAD_COUNT_GLSL);
    _character.setCudaSmootherThreadCount(SMOOTHING_THREAD_COUNT_CUDA);


    // Run test
    Grid2D<double> data(4 + 3, smoothers.size(), 0.0);

    OptimizationPlot plot;
    _character.benchmarkSmoothers(
        plot, schedule, configs);

    for(int s=0; s < serialConfigEnd; ++s)
    {
        int i = cpuLocs[s - 0];
        data[i][0] = plot.implementations()[s].passes.back().timeStamp;
    }

    for(int s=serialConfigEnd; s < threadConfigEnd; ++s)
    {
        int i = cpuLocs[s - serialConfigEnd];
        data[i][1] = plot.implementations()[s].passes.back().timeStamp;
        data[i][4] = data[refImpls[i]][0] / data[i][1];
    }

    for(int s=threadConfigEnd; s < glslConfigEnd; ++s)
    {
        int i = s - threadConfigEnd;
        data[i][2] = plot.implementations()[s].passes.back().timeStamp;
        data[i][5] = data[refImpls[i]][0] / data[i][2];
    }

    for(int s=glslConfigEnd; s < cudaConfigEnd; ++s)
    {
        int i = s - glslConfigEnd;
        data[i][3] = plot.implementations()[s].passes.back().timeStamp;
        data[i][6] = data[refImpls[i]][0] / data[i][3];
    }


    // Print results
    vector<pair<string, int>> header = {
        {"Algorithmes", 1},
        {"Temps (s)", implementations.size()},
        {"Accélérations", implementations.size()-1}
    };

    vector<pair<string, int>> subheader = {{"", 1}};
    for(const string& i : implementations)
        subheader.push_back({
            _translateImplementations[i], 1});

    for(int i=1; i < implementations.size(); ++i)
        subheader.push_back({
            _translateImplementations[implementations[i]], 1});

    vector<string> lineNames;
    for(const string& smooth : smoothers)
        lineNames.push_back(_translateSmoothers[smooth]);

    vector<int> precisions;
    for(int i=0; i < implementations.size(); ++i)
        precisions.push_back(TIME_SEC_PREC);
    for(int i=1; i < implementations.size(); ++i)
        precisions.push_back(TIME_ACC_PREC);

    output(testName, header, subheader, lineNames, precisions, data);
}

void MastersTestSuite::smootherSpeedTetCube(
        const string& testName)
{
    string mesh = MESH_TETCUBE_175K;

    smootherSpeed(testName, mesh);
}

void MastersTestSuite::smootherSpeedHexCube(
        const string& testName)
{
    string mesh = MESH_HEXCUBE_175K;

    smootherSpeed(testName, mesh);
}

void MastersTestSuite::relocationScaling(
        const string& testName)
{
    // Test case description
    string evaluator = "Metric Conformity";

    vector<Configuration> configs;
    configs.push_back(Configuration{"Local", "Nelder-Mead", "Serial"});
    configs.push_back(Configuration{"Local", "Nelder-Mead", "Thread"});
    configs.push_back(Configuration{"Texture", "Multi Elem NM", "GLSL"});
    configs.push_back(Configuration{"Texture", "Multi Elem NM", "CUDA"});

    Schedule schedule;
    schedule.autoPilotEnabled = false;
    schedule.topoOperationEnabled = false;
    schedule.relocationPassCount = ADAPTATION_RELOC_PASS;
    schedule.topoOperationPassCount = ADAPTATION_TOPO_PASS;


    // Setup test
    _character.setMetricAspectRatio(ADAPTATION_METRIC_A);

    _character.useEvaluator(evaluator);
    _character.setGlslEvaluatorThreadCount(EVALUATION_THREAD_COUNT_GLSL);
    _character.setCudaEvaluatorThreadCount(EVALUATION_THREAD_COUNT_CUDA);

    _character.setGlslSmootherThreadCount(SMOOTHING_THREAD_COUNT_GLSL);
    _character.setCudaSmootherThreadCount(SMOOTHING_THREAD_COUNT_CUDA);


    // Run test
    Grid2D<double> data(4, SPHERE_TARGET_SIZES.size(), 0.0);

    for(int s=0; s < SPHERE_TARGET_SIZES.size(); ++s)
    {
        int size = SPHERE_TARGET_SIZES[s];
        _character.setMetricScaling(sphereSizeToScale(size));

        _character.loadMesh(QString(
            MESH_SCALING_BASE.c_str()).arg(s).toStdString());


        OptimizationPlot plot;
        _character.benchmarkSmoothers(
            plot, schedule, configs);

        for(int i=0; i < plot.implementations().size(); ++i)
        {
            data[s][i] = plot.implementations()[i]
                    .passes.back().timeStamp;
        }
    }


    // Print results
    vector<pair<string, int>> header = {
        {"Tailles", 1},
        {_translateImplementations["Serial"], 1},
        {_translateImplementations["Thread"], 1},
        {_translateImplementations["GLSL"], 1},
        {_translateImplementations["CUDA"], 1}};
    vector<pair<string, int>> subheader = {};

    vector<string> lineNames;
    for(int size : SPHERE_TARGET_SIZES)
        lineNames.push_back(to_string(size));

    vector<int> precisions;
    for(int i=0; i < configs.size(); ++i)
        precisions.push_back(TIME_SEC_PREC);

    output(testName, header, subheader, lineNames, precisions, data);
}

void MastersTestSuite::smoothingGainSpeed(
        const std::string& testName)
{
    string mesh = QString(MESH_SCALING_BASE.c_str()).arg(2).toStdString();

    string sampler = "Analytic";
    string evaluator = "Metric Conformity";
    string smoother = "Nelder-Mead";

    int size = SPHERE_TARGET_SIZES[2];
    double metricK = sphereSizeToScale(size);
    double metricA = ADAPTATION_METRIC_A;

    _character.useEvaluator(evaluator);

    _character.useSampler(sampler);
    _character.setMetricScaling(metricK);
    _character.setMetricAspectRatio(metricA);

    _character.loadMesh(mesh);


    Schedule schedule;
    schedule.autoPilotEnabled = false;
    schedule.topoOperationEnabled = false;
    schedule.relocationPassCount = 5;

    Configuration config;
    config.samplerName = sampler;
    config.smootherName = smoother;
    config.implementationName = "Thread";

    OptimizationPlot plot;

    _character.benchmarkSmoothers(plot, schedule, {config});

    const OptimizationImpl& impl = plot.implementations().front();
    Grid2D<double> data(2, impl.passes.size(), 0.0);

    for(int p=0; p < impl.passes.size(); ++p)
    {
        const OptimizationPass& pass = impl.passes[p];
        data[p][0] = pass.histogram.minimumQuality();
        data[p][1] = pass.histogram.harmonicMean();
    }


    vector<pair<string, int>> header = {
        {"Temps", 1},
        {"Minimum", 1},
        {"Moyenne", 1}};
    vector<pair<string, int>> subheader = {};

    vector<string> lineNames;
    for(int p=0; p < impl.passes.size(); ++p)
    {
        const OptimizationPass& pass = impl.passes[p];
        lineNames.push_back(to_string(pass.timeStamp));
    }

    vector<int> precisions;
    precisions.push_back(QUAL_MIN_PREC);
    precisions.push_back(QUAL_MEAN_PREC);

    output(testName, header, subheader, lineNames, precisions, data);
}

void MastersTestSuite::cavityTestCase(
        const std::string& testName)
{
    if(_cavityIsMissing)
    {
        getLog().postMessage(new Message('W', true,
            "Missing 32K Cavity mesh",
            "MastersTestSuite"));
        getLog().postMessage(new Message('W', true,
            "Cannot execute this test",
            "MastersTestSuite"));
        return;
    }

    string mesh = MESH_CAVITY_32K;

    string evaluator = "Metric Conformity";
    string smoother = "Multi Elem NM";

    double metricK = 1.0;
    double metricA = 1.0;
    int discretization = 40;

    int relocPassCount = 300;

    _character.useEvaluator(evaluator);

    // We need to set sampler to one of Computed samplers
    // So the initial and final histograms can be computed
    // in the computed metric
    _character.useSampler("Computed Loc");

    _character.setMetricScaling(metricK);
    _character.setMetricAspectRatio(metricA);
    _character.setMetricDiscretizationDepth(discretization);

    _character.loadMesh(mesh);


    Schedule schedule;
    schedule.autoPilotEnabled = false;
    schedule.topoOperationEnabled = false;
    schedule.relocationPassCount = relocPassCount;

    vector<string> implementations = {
        "Serial", "Thread", "GLSL", "CUDA"
    };

    map<string, string> impToSamp;
    impToSamp["Serial"] = "Computed Loc";
    impToSamp["Thread"] = "Computed Loc";
    impToSamp["GLSL"] = "Computed Tex";
    impToSamp["CUDA"] = "Computed Tex";

    vector<Configuration> configs;
    configs.push_back(Configuration{impToSamp["Serial"], smoother, "Serial"});
    for(const string& impl : implementations)
        configs.push_back(Configuration{impToSamp[impl], smoother, impl});

    OptimizationPlot plot;
    _character.benchmarkSmoothers(plot, schedule, configs);

    Grid2D<double> timeData(implementations.size()*2-1, 1, 0.0);
    for(int i=0; i < implementations.size(); ++i)
    {
        const OptimizationImpl& impl = plot.implementations()[i+1];
        timeData[0][i] = impl.passes.back().timeStamp;

        if(i > 0)
            timeData[0][i+implementations.size()-1] = timeData[0][0]/ timeData[0][i];
    }


    Grid2D<double> qualData(2, implementations.size()+1, 0.0);
    qualData[0][0] = plot.initialHistogram().minimumQuality();
    qualData[0][1] = plot.initialHistogram().harmonicMean();
    for(int i=0; i <implementations.size(); ++i)
    {
        const QualityHistogram hist = plot.implementations()[i+1].finalHistogram;
        qualData[i+1][0] = hist.minimumQuality();
        qualData[i+1][1] = hist.harmonicMean();
    }


    vector<pair<string, int>> timeHeader = {{"Cas", 1}};
    for(int i=0; i <implementations.size(); ++i)
        timeHeader.push_back({_translateImplementations[implementations[i]], 1});
    for(int i=1; i <implementations.size(); ++i)
        timeHeader.push_back({_translateImplementations[implementations[i]], 1});

    vector<pair<string, int>> subheader = {};

    vector<string> timeLineNames;
    timeLineNames.push_back("Cavitation");

    vector<int> timePrecisions;
    for(int i=0; i <implementations.size(); ++i)
        timePrecisions.push_back(TIME_SEC_PREC);
    for(int i=0; i <implementations.size(); ++i)
        timePrecisions.push_back(TIME_ACC_PREC);


    vector<pair<string, int>> qualHeader = {
        {"Implémentations", 1},
        {"Minimums", 1},
        {"Moyennes", 1}};

    vector<string> qualLineNames;
    qualLineNames.push_back("Initiale");
    for(int i=0; i <implementations.size(); ++i)
        qualLineNames.push_back(_translateImplementations[implementations[i]]);

    vector<int> qualPrecisions;
    qualPrecisions.push_back(QUAL_MIN_PREC);
    qualPrecisions.push_back(QUAL_MEAN_PREC);

    output(testName + "(Times)", timeHeader, subheader, timeLineNames, timePrecisions, timeData);
    output(testName + "(Quality)", qualHeader, subheader, qualLineNames, qualPrecisions, qualData);
}
