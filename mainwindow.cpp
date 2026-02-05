#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

// Fortran subroutines declarations
extern "C" {
    void lsq_get_parameters(int* refinement_type, double* damping_factor,
                           int* reflections_cutoff, int* num_cycles,
                           int* weighting_scheme, double* weight_params,
                           int* refine_weight, int* num_observations,
                           double* percent_observations, int* num_parameters,
                           double* ratio, int* num_weight_params, int* ier);
    
    void lsq_execute(int refinement_type, double damping_factor,
                    int reflections_cutoff, int num_cycles,
                    int weighting_scheme, double* weight_params,
                    int refine_weight);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , lsqDialog(nullptr)
{
    ui->setupUi(this);
    
    // Create the LSQDialog
    lsqDialog = new LSQDialog(this);
    
    // Connect the New Project action
    connect(ui->actionNewProject, &QAction::triggered, this, &MainWindow::onNewProject);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onNewProject()
{
    if (lsqDialog) {
        // 1. Call Fortran to get initial parameters
        int refinementType;
        double dampingFactor;
        int reflectionsCutoff;
        int numCycles;
        int weightingScheme;
        double weightParams[10];
        int refineWeight;
        int numObservations;
        double percentObservations;
        int numParameters;
        double ratio;
        int numWeightParams[18];
        int ier;
        
        lsq_get_parameters(&refinementType, &dampingFactor, &reflectionsCutoff,
                          &numCycles, &weightingScheme, weightParams, &refineWeight,
                          &numObservations, &percentObservations, &numParameters,
                          &ratio, numWeightParams, &ier);
        
        // Check for errors from Fortran
        if (ier != 0) {
            QMessageBox::warning(this, "Error", 
                               QString("Error getting LSQ parameters from Fortran (ier=%1)").arg(ier));
            return;
        }
        
        // 2. Set the dialog parameters with values from Fortran
        LSQParameters params;
        params.refinementType = static_cast<LSQParameters::RefinementType>(refinementType);
        params.dampingFactor = dampingFactor;
        params.reflectionsCutoff = reflectionsCutoff;
        params.numCycles = numCycles;
        params.weightingSchemeIndex = weightingScheme;
        for (int i = 0; i < 10; ++i) {
            params.weightParameters[i] = weightParams[i];
        }
        params.refineWeightParams = (refineWeight != 0);
        params.numObservations = numObservations;
        params.percentObservations = percentObservations;
        params.numParameters = numParameters;
        params.ratio = ratio;
        for (int i = 0; i < 18; ++i) {
            params.numWeightParamsPerScheme[i] = numWeightParams[i];
        }
        
        lsqDialog->setParameters(params);
        
        // 3. Execute the dialog (modal)
        if (lsqDialog->exec() == QDialog::Accepted) {
            // 4. Get parameters from dialog (only if Apply was pressed)
            LSQParameters resultParams = lsqDialog->getParameters();
            
            // 5. Pass parameters to Fortran for calculation
            int refType = static_cast<int>(resultParams.refinementType);
            double wParams[10];
            for (int i = 0; i < 10; ++i) {
                wParams[i] = resultParams.weightParameters[i];
            }
            int refWeight = resultParams.refineWeightParams ? 1 : 0;
            
            lsq_execute(refType, resultParams.dampingFactor,
                       resultParams.reflectionsCutoff, resultParams.numCycles,
                       resultParams.weightingSchemeIndex, wParams, refWeight);
            
            QMessageBox::information(this, "LSQ Complete", 
                                   "Least Squares Refinement calculation completed!\n"
                                   "Check terminal for Fortran output.");
        }
    }
}
