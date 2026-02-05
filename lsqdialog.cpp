#include "lsqdialog.h"
#include "ui_lsqdialog.h"
#include "weightparamsdialog.h"
#include <QMessageBox>

LSQDialog::LSQDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LSQDialog)
    , weightParameters(10, 0.0)
    , applyPressed(false)
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
            this, []() { 
                QMessageBox::information(nullptr, "Help", "Least Squares Refinement Help");
            });
    
    // Connect Modify weight parameters button
    connect(ui->modifyWeightButton, &QPushButton::clicked,
            this, &LSQDialog::onModifyWeightParameters);
}

LSQDialog::~LSQDialog()
{
    delete ui;
}

void LSQDialog::onLSQRun()
{
    // Set the apply pressed flag
    applyPressed = true;
    
    // Implementation for LSQ refinement procedure
    QMessageBox::information(this, "LSQ Run", "Running Least Squares Refinement...");
}

void LSQDialog::onModifyWeightParameters()
{
    // Get the current selected formula
    QString currentFormula = ui->weightingSchemeCombo->currentText();
    
    // Create and show the weight parameters dialog
    WeightParamsDialog dialog(currentFormula, this);
    dialog.setParameters(weightParameters);
    
    if (dialog.exec() == QDialog::Accepted) {
        // Save the modified parameters
        weightParameters = dialog.getParameters();
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
    
    return params;
}
