#ifndef LSQDIALOG_H
#define LSQDIALOG_H

#include <QDialog>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class LSQDialog; }
QT_END_NAMESPACE

struct LSQParameters {
    // Refinement Request
    enum RefinementType { Diagonal, FullMatrix, SFCOnly } refinementType;
    
    // Refinement Conditions
    double dampingFactor;
    int reflectionsCutoff;
    int numCycles;
    
    // Weighting Scheme
    int weightingSchemeIndex;
    QVector<double> weightParameters;
    bool refineWeightParams;
    
    LSQParameters()
        : refinementType(Diagonal)
        , dampingFactor(0.5)
        , reflectionsCutoff(0)
        , numCycles(5)
        , weightingSchemeIndex(0)
        , weightParameters(10, 0.0)
        , refineWeightParams(false)
    {}
};

class LSQDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LSQDialog(QWidget *parent = nullptr);
    ~LSQDialog();
    
    void setParameters(const LSQParameters &params);
    LSQParameters getParameters() const;

private slots:
    void onLSQRun();
    void onModifyWeightParameters();

private:
    Ui::LSQDialog *ui;
    QVector<double> weightParameters;
    bool applyPressed;
};

#endif // LSQDIALOG_H
