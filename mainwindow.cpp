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
                           double* ratio, int* num_weight_params,
                           double* all_weight_params, int* num_atoms, int* ier);
    
    void lsq_get_atoms(int num_atoms, char** atom_names, int* fix_xyz,
                      int* fix_b, int* fix_occ, int* set_isotropic, int* ier);
    
    void lsq_execute(int refinement_type, double damping_factor,
                    int reflections_cutoff, int num_cycles,
                    int weighting_scheme, double* weight_params,
                    int refine_weight, int num_atoms, int* fix_xyz,
                    int* fix_b, int* fix_occ, int* set_isotropic);
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

void MainWindow::openHelp(const QString &page)
{
    // To be implemented
    Q_UNUSED(page);
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
        double allWeightParams[18 * 10];  // 18 schemes x 10 parameters (column-major for Fortran)
        int numAtoms;
        int ier;
        
        lsq_get_parameters(&refinementType, &dampingFactor, &reflectionsCutoff,
                          &numCycles, &weightingScheme, weightParams, &refineWeight,
                          &numObservations, &percentObservations, &numParameters,
                          &ratio, numWeightParams, allWeightParams, &numAtoms, &ier);
        
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
        
        // Copy all weight parameters from Fortran (column-major) to QVector
        for (int scheme = 0; scheme < 18; ++scheme) {
            for (int param = 0; param < 10; ++param) {
                // Fortran stores as column-major: (param, scheme)
                params.allWeightParams[scheme][param] = allWeightParams[param + scheme * 10];
            }
        }
        
        params.numAtoms = numAtoms;
        
        // Get atom data from Fortran
        if (numAtoms > 0) {
            char* atomNames[numAtoms];
            int fixXYZ[numAtoms];
            int fixB[numAtoms];
            int fixOcc[numAtoms];
            int setIsotropic[numAtoms];
            int ierAtoms;
            
            lsq_get_atoms(numAtoms, atomNames, fixXYZ, fixB, fixOcc, setIsotropic, &ierAtoms);
            
            if (ierAtoms == 0) {
                // Convert atom data to QVectors
                for (int i = 0; i < numAtoms; ++i) {
                    // Extract atom name (null-terminated string pointer)
                    QString atomName = QString::fromUtf8(atomNames[i]);
                    params.atomNames.append(atomName);
                    params.fixXYZ.append(fixXYZ[i] != 0);
                    params.fixB.append(fixB[i] != 0);
                    params.fixOcc.append(fixOcc[i] != 0);
                    params.setIsotropic.append(setIsotropic[i] != 0);
                }
            }
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
            
            // Prepare atoms data
            int numAtomsResult = resultParams.numAtoms;
            int fixXYZ[numAtomsResult];
            int fixB[numAtomsResult];
            int fixOcc[numAtomsResult];
            int setIsotropic[numAtomsResult];
            
            for (int i = 0; i < numAtomsResult; ++i) {
                fixXYZ[i] = (i < resultParams.fixXYZ.size() && resultParams.fixXYZ[i]) ? 1 : 0;
                fixB[i] = (i < resultParams.fixB.size() && resultParams.fixB[i]) ? 1 : 0;
                fixOcc[i] = (i < resultParams.fixOcc.size() && resultParams.fixOcc[i]) ? 1 : 0;
                setIsotropic[i] = (i < resultParams.setIsotropic.size() && resultParams.setIsotropic[i]) ? 1 : 0;
            }
            
            lsq_execute(refType, resultParams.dampingFactor,
                       resultParams.reflectionsCutoff, resultParams.numCycles,
                       resultParams.weightingSchemeIndex, wParams, refWeight,
                       numAtomsResult, fixXYZ, fixB, fixOcc, setIsotropic);
            
            QMessageBox::information(this, "LSQ Complete", 
                                   "Least Squares Refinement calculation completed!\n"
                                   "Check terminal for Fortran output.");
        }
    }
}
