#include "weightparamsdialog.h"
#include "ui_weightparamsdialog.h"
#include <QLineEdit>

WeightParamsDialog::WeightParamsDialog(const QString &formula, int numParams, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WeightParamsDialog)
{
    ui->setupUi(this);
    
    // Set the formula text
    ui->formulaLabel->setText(formula);
    
    // Initialize line edits with default values and show/hide based on numParams
    for (int i = 0; i < 10; ++i) {
        QLineEdit *lineEdit = findChild<QLineEdit*>(QString("param%1LineEdit").arg(i + 1));
        QLabel *label = findChild<QLabel*>(QString("param%1Label").arg(i + 1));
        
        if (lineEdit && label) {
            lineEdit->setText("0.000000");
            
            // Show or hide based on numParams
            if (i < numParams) {
                lineEdit->setVisible(true);
                label->setVisible(true);
            } else {
                lineEdit->setVisible(false);
                label->setVisible(false);
            }
        }
    }
}

WeightParamsDialog::~WeightParamsDialog()
{
    delete ui;
}

QVector<double> WeightParamsDialog::getParameters() const
{
    QVector<double> params;
    for (int i = 0; i < 10; ++i) {
        QLineEdit *lineEdit = findChild<QLineEdit*>(QString("param%1LineEdit").arg(i + 1));
        if (lineEdit) {
            params.append(lineEdit->text().toDouble());
        }
    }
    return params;
}

void WeightParamsDialog::setParameters(const QVector<double> &params)
{
    for (int i = 0; i < qMin(params.size(), 10); ++i) {
        QLineEdit *lineEdit = findChild<QLineEdit*>(QString("param%1LineEdit").arg(i + 1));
        if (lineEdit) {
            lineEdit->setText(QString::number(params[i], 'f', 6));
        }
    }
}
