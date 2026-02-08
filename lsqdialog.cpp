#include "lsqdialog.h"
#include "ui_lsqdialog.h"
#include "weightparamsdialog.h"
#include "checkboxdelegate.h"
#include "checkboxheader.h"
#include "mainwindow.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QTableWidgetItem>

// Fortran subroutines declarations
extern "C" {
    void lsq_get_parameters(int* refinement_type, double* damping_factor,
                           int* reflections_cutoff, int* num_cycles,
                           int* weighting_scheme, double* weight_params,
                           int* refine_weight, int* num_observations,
                           double* percent_observations, int* num_weight_params,
                           double* all_weight_params, int* num_atoms, int* ier);
    
    void lsq_get_atoms(int num_atoms, char** atom_names, int* fix_xyz,
                      int* fix_b, int* fix_occ, int* set_isotropic, int* ier);
    
    void lsq_execute(int refinement_type, double damping_factor,
                    int reflections_cutoff, int num_cycles,
                    int weighting_scheme, double* weight_params,
                    int refine_weight, int num_atoms, int* fix_xyz,
                    int* fix_b, int* fix_occ, int* set_isotropic);
}

void LSQDialog::openLSQDialog()
{
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
    int numWeightParams[18];
    double allWeightParams[18 * 10];  // 18 schemes x 10 parameters (column-major for Fortran)
    int numAtoms;
    int ier;
    
    lsq_get_parameters(&refinementType, &dampingFactor, &reflectionsCutoff,
                      &numCycles, &weightingScheme, weightParams, &refineWeight,
                      &numObservations, &percentObservations, numWeightParams,
                      allWeightParams, &numAtoms, &ier);
    
    // Check for errors from Fortran
    if (ier != 0) {
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
        QVector<char*> atomNames(numAtoms);
        QVector<int> fixXYZ(numAtoms);
        QVector<int> fixB(numAtoms);
        QVector<int> fixOcc(numAtoms);
        QVector<int> setIsotropic(numAtoms);
        int ierAtoms;
        
        lsq_get_atoms(numAtoms, atomNames.data(), fixXYZ.data(), fixB.data(), fixOcc.data(), setIsotropic.data(), &ierAtoms);
        
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
    
    setParameters(params);
    
    // 3. Execute the dialog (modal)
    if (exec() == QDialog::Accepted) {
        // 4. Get parameters from dialog (only if Apply was pressed)
        LSQParameters resultParams = getParameters();
        
        // 5. Pass parameters to Fortran for calculation
        int refType = static_cast<int>(resultParams.refinementType);
        double wParams[10];
        for (int i = 0; i < 10; ++i) {
            wParams[i] = resultParams.weightParameters[i];
        }
        int refWeight = resultParams.refineWeightParams ? 1 : 0;
        
        // Prepare atoms data
        int numAtomsResult = resultParams.numAtoms;
        QVector<int> fixXYZ(numAtomsResult);
        QVector<int> fixB(numAtomsResult);
        QVector<int> fixOcc(numAtomsResult);
        QVector<int> setIsotropic(numAtomsResult);
        
        for (int i = 0; i < numAtomsResult; ++i) {
            fixXYZ[i] = (i < resultParams.fixXYZ.size() && resultParams.fixXYZ[i]) ? 1 : 0;
            fixB[i] = (i < resultParams.fixB.size() && resultParams.fixB[i]) ? 1 : 0;
            fixOcc[i] = (i < resultParams.fixOcc.size() && resultParams.fixOcc[i]) ? 1 : 0;
            setIsotropic[i] = (i < resultParams.setIsotropic.size() && resultParams.setIsotropic[i]) ? 1 : 0;
        }
        
        lsq_execute(refType, resultParams.dampingFactor,
                   resultParams.reflectionsCutoff, resultParams.numCycles,
                   resultParams.weightingSchemeIndex, wParams, refWeight,
                   numAtomsResult, fixXYZ.data(), fixB.data(), fixOcc.data(), setIsotropic.data());
    }
}

LSQDialog::LSQDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LSQDialog)
    , weightParameters(10, 0.0)
    , numWeightParamsPerScheme(18, 0)
    , allWeightParams(18, QVector<double>(10, 0.0))
    , applyPressed(false)
    , numObservations(0)
{
    ui->setupUi(this);
    
    // Populate weighting scheme combo
    ui->weightingSchemeCombo->addItem("#  1 : W=Fo/P(1) if Fo >= P(1); W=1/W otherwise");
    ui->weightingSchemeCombo->addItem("#  2 : W=1/(P(1)+P(2)*Fo+P(3)*Fo^2)");
    ui->weightingSchemeCombo->addItem("#  3 : W=(Sin(Theta)/Lambda)^P(1)");
    ui->weightingSchemeCombo->addItem("#  4 : W=(Lambda/Sin(Theta))^P(1)");
    ui->weightingSchemeCombo->addItem("#  5 : W=1/(Sigma(Fo))^2");
    ui->weightingSchemeCombo->addItem("#  6 : W=1.0");
    ui->weightingSchemeCombo->addItem("#  7 : W=1/(1.+((Fo-P(2))/P(1))^2");
    ui->weightingSchemeCombo->addItem("#  8 : W=1.0 if Fo<=P(1); W=P(1)/Fo if Fo>P(1)");
    ui->weightingSchemeCombo->addItem("#  9 : W=Sigma(Fo)");
    ui->weightingSchemeCombo->addItem("# 10 : W=1.0/Sigma(Fo)");
    ui->weightingSchemeCombo->addItem("# 11 : W=(Sin(theta)/Lambda)^P(1)/(P(2)+P(3)*Fo+P(4)*Fo^2)");
    ui->weightingSchemeCombo->addItem("# 12 : W=1/(Sigma(Fo))^2+P(1)*Fo+P(2)*Fo^2)");
    ui->weightingSchemeCombo->addItem("# 13 : W=Sc/(P(1)+P(2)*Fo+P(3)*Fo^2+P(4)*(Sin(Th)/L))");
    ui->weightingSchemeCombo->addItem("# 14 : W=Sc/(P(1)+P(2)*Fo+P(3)*Fo^2)");
    ui->weightingSchemeCombo->addItem("# 15 : W=Sc/(P(1)+P(2)*(Sin(Th)/L)+P(3)*(Sin(Th)/L)^2)");
    ui->weightingSchemeCombo->addItem("# 16 : W=Sc/(SUM(aj*Tj(F))j=1,n");
    ui->weightingSchemeCombo->addItem("# 17 : W=Sc*(Sin(Th)/L)^P(3)/(P(1)*Sigma(Fo))^2+P(2)*Fo^2)");
    ui->weightingSchemeCombo->addItem("# 18 : W=Sc/((P(1)+P(2)*Fo+P(3)*Fo^2+P(4)*(Sin(Th)/L)^P(5))*(Sigma(Fo))^2)");
    
    // Connect the LSQ Run button
    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, &LSQDialog::onLSQRun);
    
    // Connect Help button
    connect(ui->buttonBox->button(QDialogButtonBox::Help), &QPushButton::clicked,
            this, [parent]() {
                MainWindow *mainWindow = qobject_cast<MainWindow*>(parent);
                if (mainWindow) {
                    mainWindow->openHelp("refine/#lsq");
                }
            });
    
    // Connect Modify weight parameters button
    connect(ui->modifyWeightButton, &QPushButton::clicked,
            this, &LSQDialog::onModifyWeightParameters);
    
    // Connect weighting scheme combo change to update button/checkbox state
    connect(ui->weightingSchemeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LSQDialog::updateWeightParametersState);
    
    // Setup atoms table
    setupAtomsTable();
}

LSQDialog::~LSQDialog()
{
    delete ui;
}

void LSQDialog::onLSQRun()
{
    // Set the apply pressed flag and accept the dialog
    applyPressed = true;
    accept();
}

void LSQDialog::onModifyWeightParameters()
{
    // Get the current selected formula index
    int currentIndex = ui->weightingSchemeCombo->currentIndex();
    
    // Get current scheme parameters
    if (currentIndex >= 0 && currentIndex < numWeightParamsPerScheme.size()) {
        int numParams = numWeightParamsPerScheme[currentIndex];
        
        // Get the current selected formula
        QString currentFormula = ui->weightingSchemeCombo->currentText();
        
        // Load the parameters for the selected scheme from allWeightParams
        QVector<double> schemeParams = allWeightParams[currentIndex];
        
        // Create and show the weight parameters dialog with the correct number of params
        WeightParamsDialog dialog(currentFormula, numParams, this);
        dialog.setParameters(schemeParams);
        
        if (dialog.exec() == QDialog::Accepted) {
            // Save the modified parameters back to allWeightParams
            allWeightParams[currentIndex] = dialog.getParameters();
            // Also update weightParameters for backward compatibility
            weightParameters = dialog.getParameters();
        }
    }
}

void LSQDialog::setParameters(const LSQParameters &params)
{
    // Reset apply pressed flag
    applyPressed = false;
    
    // Set Refinement Request
    switch (params.refinementType) {
        case LSQParameters::Diagonal:
            ui->diagonalRadio->setChecked(true);
            break;
        case LSQParameters::FullMatrix:
            ui->fullMatrixRadio->setChecked(true);
            break;
        case LSQParameters::SFCOnly:
            ui->sfcOnlyRadio->setChecked(true);
            break;
    }
    
    // Set Refinement Conditions
    ui->dampingFactorSpin->setValue(params.dampingFactor);
    ui->reflectionsCutoffSpin->setValue(params.reflectionsCutoff);
    ui->cyclesSpin->setValue(params.numCycles);
    
    // Set Weighting Scheme
    ui->weightingSchemeCombo->setCurrentIndex(params.weightingSchemeIndex);
    weightParameters = params.weightParameters;
    ui->refineWeightCheck->setChecked(params.refineWeightParams);
    numWeightParamsPerScheme = params.numWeightParamsPerScheme;
    allWeightParams = params.allWeightParams;
    
    // Update weight parameters button/checkbox state
    updateWeightParametersState();
    
    // Store observations for ratio calculation
    numObservations = params.numObservations;
    
    // Set Observations & Parameters labels
    ui->observationsLabel->setText(QString("Observations: %1 (%2%)")
                                   .arg(params.numObservations)
                                   .arg(params.percentObservations, 0, 'f', 1));
    
    // Populate atoms table (this will trigger updateObservationsParameters)
    populateAtomsTable(params);
}

LSQParameters LSQDialog::getParameters() const
{
    LSQParameters params;
    
    // Only return parameters if Apply was pressed
    if (!applyPressed) {
        return params; // Returns default parameters
    }
    
    // Get Refinement Request
    if (ui->diagonalRadio->isChecked()) {
        params.refinementType = LSQParameters::Diagonal;
    } else if (ui->fullMatrixRadio->isChecked()) {
        params.refinementType = LSQParameters::FullMatrix;
    } else if (ui->sfcOnlyRadio->isChecked()) {
        params.refinementType = LSQParameters::SFCOnly;
    }
    
    // Get Refinement Conditions
    params.dampingFactor = ui->dampingFactorSpin->value();
    params.reflectionsCutoff = ui->reflectionsCutoffSpin->value();
    params.numCycles = ui->cyclesSpin->value();
    
    // Get Weighting Scheme
    params.weightingSchemeIndex = ui->weightingSchemeCombo->currentIndex();
    params.weightParameters = weightParameters;
    params.refineWeightParams = ui->refineWeightCheck->isChecked();
    
    // Get Atoms data from table
    int rowCount = ui->atomsTable->rowCount();
    params.numAtoms = rowCount;
    params.atomNames.clear();
    params.fixXYZ.clear();
    params.fixB.clear();
    params.fixOcc.clear();
    params.setIsotropic.clear();
    
    // Resize vectors to hold all atoms
    params.atomNames.resize(rowCount);
    params.fixXYZ.resize(rowCount);
    params.fixB.resize(rowCount);
    params.fixOcc.resize(rowCount);
    params.setIsotropic.resize(rowCount);
    
    // Read data and place it in the original index position
    for (int row = 0; row < rowCount; ++row) {
        // Get original index from UserData
        QTableWidgetItem *atomItem = ui->atomsTable->item(row, 0);
        int originalIndex = row;  // Default to current row if no UserData
        if (atomItem) {
            QVariant userData = atomItem->data(Qt::UserRole);
            if (userData.isValid()) {
                originalIndex = userData.toInt();
            }
            params.atomNames[originalIndex] = atomItem->text();
        } else {
            params.atomNames[originalIndex] = QString("Atom_%1").arg(originalIndex + 1);
        }
        
        // Get Fix XYZ
        QTableWidgetItem *fixXYZItem = ui->atomsTable->item(row, 1);
        params.fixXYZ[originalIndex] = fixXYZItem ? fixXYZItem->data(Qt::EditRole).toBool() : false;
        
        // Get Fix B
        QTableWidgetItem *fixBItem = ui->atomsTable->item(row, 2);
        params.fixB[originalIndex] = fixBItem ? fixBItem->data(Qt::EditRole).toBool() : false;
        
        // Get Fix Occ.
        QTableWidgetItem *fixOccItem = ui->atomsTable->item(row, 3);
        params.fixOcc[originalIndex] = fixOccItem ? fixOccItem->data(Qt::EditRole).toBool() : false;
        
        // Get Set Isotropic
        QTableWidgetItem *setIsotropicItem = ui->atomsTable->item(row, 4);
        params.setIsotropic[originalIndex] = setIsotropicItem ? setIsotropicItem->data(Qt::EditRole).toBool() : false;
    }
    
    return params;
}

void LSQDialog::setupAtomsTable()
{
    // Configure atoms table
    ui->atomsTable->setColumnCount(5);
    ui->atomsTable->setHorizontalHeaderLabels({"Atom", "Fix XYZ", "Fix B", "Fix Occ.", "Set Isotropic"});
    
    // Create and set CheckBoxHeader for columns 1-4
    QVector<int> checkboxColumns = {1, 2, 3, 4};
    checkboxHeader = new CheckBoxHeader(Qt::Horizontal, checkboxColumns, true, this);
    ui->atomsTable->setHorizontalHeader(checkboxHeader);
    
    // Connect header checkbox signal
    connect(checkboxHeader, &CheckBoxHeader::checkBoxClicked,
            this, &LSQDialog::onHeaderCheckBoxClicked);
    
    // Set up column behavior
    checkboxHeader->setStretchLastSection(true);
    checkboxHeader->setSectionResizeMode(0, QHeaderView::Stretch);
    for (int i = 1; i < 5; ++i) {
        checkboxHeader->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
    
    // Enable sorting
    ui->atomsTable->setSortingEnabled(true);
    
    // Set selection behavior
    ui->atomsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->atomsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // Create and set CheckBoxDelegate for checkbox columns (1-4)
    for (int col = 1; col < 5; ++col) {
        CheckBoxDelegate *delegate = new CheckBoxDelegate(this);
        delegate->setColumn(col);
        ui->atomsTable->setItemDelegateForColumn(col, delegate);
    }
    
    // Connect to cell changes to update header checkboxes
    connect(ui->atomsTable, &QTableWidget::itemChanged,
            this, [this](QTableWidgetItem *item) {
                if (item && item->column() >= 1 && item->column() <= 4) {
                    updateHeaderCheckBox(item->column());
                    // Update Parameters and Ratio when Fix B or Set Isotropic change
                    if (item->column() == 2 || item->column() == 4) {
                        updateObservationsParameters();
                    } else if (item->column() == 1 || item->column() == 3) {
                        updateObservationsParameters();
                    }
                }
            });
}

void LSQDialog::populateAtomsTable(const LSQParameters &params)
{
    int numAtoms = params.numAtoms;
    
    // Temporarily disable sorting while populating
    ui->atomsTable->setSortingEnabled(false);
    
    // Block signals to avoid triggering itemChanged during population
    ui->atomsTable->blockSignals(true);
    
    // Clear existing rows
    ui->atomsTable->setRowCount(0);
    
    // Populate table with atoms
    ui->atomsTable->setRowCount(numAtoms);
    
    for (int row = 0; row < numAtoms; ++row) {
        // Column 0: Atom name
        QString atomName = (row < params.atomNames.size()) ? params.atomNames[row] : QString("Atom_%1").arg(row + 1);
        QTableWidgetItem *atomItem = new QTableWidgetItem(atomName);
        atomItem->setFlags(atomItem->flags() & ~Qt::ItemIsEditable);  // Make read-only
        atomItem->setData(Qt::UserRole, row);  // Store original index
        ui->atomsTable->setItem(row, 0, atomItem);
        
        // Column 1: Fix XYZ
        QTableWidgetItem *fixXYZItem = new QTableWidgetItem();
        bool fixXYZValue = (row < params.fixXYZ.size()) ? params.fixXYZ[row] : false;
        fixXYZItem->setData(Qt::EditRole, fixXYZValue);
        fixXYZItem->setFlags(fixXYZItem->flags() | Qt::ItemIsEditable);
        ui->atomsTable->setItem(row, 1, fixXYZItem);
        
        // Column 2: Fix B
        QTableWidgetItem *fixBItem = new QTableWidgetItem();
        bool fixBValue = (row < params.fixB.size()) ? params.fixB[row] : false;
        fixBItem->setData(Qt::EditRole, fixBValue);
        fixBItem->setFlags(fixBItem->flags() | Qt::ItemIsEditable);
        ui->atomsTable->setItem(row, 2, fixBItem);
        
        // Column 3: Fix Occ.
        QTableWidgetItem *fixOccItem = new QTableWidgetItem();
        bool fixOccValue = (row < params.fixOcc.size()) ? params.fixOcc[row] : false;
        fixOccItem->setData(Qt::EditRole, fixOccValue);
        fixOccItem->setFlags(fixOccItem->flags() | Qt::ItemIsEditable);
        ui->atomsTable->setItem(row, 3, fixOccItem);
        
        // Column 4: Set Isotropic
        QTableWidgetItem *setIsotropicItem = new QTableWidgetItem();
        bool setIsotropicValue = (row < params.setIsotropic.size()) ? params.setIsotropic[row] : false;
        setIsotropicItem->setData(Qt::EditRole, setIsotropicValue);
        setIsotropicItem->setFlags(setIsotropicItem->flags() | Qt::ItemIsEditable);
        ui->atomsTable->setItem(row, 4, setIsotropicItem);
    }
    
    // Re-enable sorting
    ui->atomsTable->setSortingEnabled(true);
    
    // Unblock signals
    ui->atomsTable->blockSignals(false);
    
    // Update header checkboxes state based on actual data
    for (int col = 1; col < 5; ++col) {
        updateHeaderCheckBox(col);
    }
    
    // Update Parameters and Ratio labels
    updateObservationsParameters();
}

void LSQDialog::onHeaderCheckBoxClicked(bool state, int column)
{
    // When header checkbox is clicked, set all cells in that column to the same state
    ui->atomsTable->blockSignals(true);  // Block signals to avoid multiple updates
    
    for (int row = 0; row < ui->atomsTable->rowCount(); ++row) {
        QTableWidgetItem *item = ui->atomsTable->item(row, column);
        if (item) {
            item->setData(Qt::EditRole, state);
        }
    }
    
    ui->atomsTable->blockSignals(false);
}

void LSQDialog::updateHeaderCheckBox(int column)
{
    // Check if all items in the column are checked
    bool allChecked = true;
    int rowCount = ui->atomsTable->rowCount();
    
    if (rowCount == 0) {
        checkboxHeader->setChecked(column, false);
        return;
    }
    
    for (int row = 0; row < rowCount; ++row) {
        QTableWidgetItem *item = ui->atomsTable->item(row, column);
        if (item && !item->data(Qt::EditRole).toBool()) {
            allChecked = false;
            break;
        }
    }
    
    checkboxHeader->setChecked(column, allChecked);
}

void LSQDialog::updateWeightParametersState()
{
    int currentIndex = ui->weightingSchemeCombo->currentIndex();
    bool hasParams = false;
    
    if (currentIndex >= 0 && currentIndex < numWeightParamsPerScheme.size()) {
        hasParams = (numWeightParamsPerScheme[currentIndex] > 0);
    }
    
    // Enable/disable the modify button and refine checkbox based on whether scheme has parameters
    ui->modifyWeightButton->setEnabled(hasParams);
    ui->refineWeightCheck->setEnabled(hasParams);
    
    // If disabled, uncheck the refine checkbox
    if (!hasParams) {
        ui->refineWeightCheck->setChecked(false);
    }
}

int LSQDialog::calculateParameters() const
{
    int parameters = 0;
    int rowCount = ui->atomsTable->rowCount();
    
    for (int row = 0; row < rowCount; ++row) {
        // Count parameters for each atom based on checkboxes
        
        // XYZ coordinates: 3 parameters if NOT fixed
        QTableWidgetItem *fixXYZItem = ui->atomsTable->item(row, 1);
        if (!fixXYZItem || !fixXYZItem->data(Qt::EditRole).toBool()) {
            parameters += 3;
        }
        
        // B factor: depends on isotropic/anisotropic setting
        // First check if B is fixed
        QTableWidgetItem *fixBItem = ui->atomsTable->item(row, 2);
        bool bIsFixed = fixBItem && fixBItem->data(Qt::EditRole).toBool();
        
        if (!bIsFixed) {
            // B is not fixed, count depends on isotropic setting
            QTableWidgetItem *setIsotropicItem = ui->atomsTable->item(row, 4);
            bool isIsotropic = setIsotropicItem && setIsotropicItem->data(Qt::EditRole).toBool();
            
            if (isIsotropic) {
                // Isotropic: 1 parameter (single B value)
                parameters += 1;
            } else {
                // Anisotropic: 6 parameters (B11, B22, B33, B12, B13, B23)
                parameters += 6;
            }
        }
        
        // Occupancy: 1 parameter if NOT fixed
        QTableWidgetItem *fixOccItem = ui->atomsTable->item(row, 3);
        if (!fixOccItem || !fixOccItem->data(Qt::EditRole).toBool()) {
            parameters += 1;
        }
    }
    
    return parameters;
}

void LSQDialog::updateObservationsParameters()
{
    int parameters = calculateParameters();
    
    // Update Parameters label
    ui->parametersLabel->setText(QString("Parameters: %1").arg(parameters));
    
    // Update Ratio label
    if (parameters > 0 && numObservations > 0) {
        double ratio = static_cast<double>(numObservations) / parameters;
        ui->ratioLabel->setText(QString("Ratio: %1").arg(ratio, 0, 'f', 2));
    } else {
        ui->ratioLabel->setText(QString("Ratio: "));
    }
}
