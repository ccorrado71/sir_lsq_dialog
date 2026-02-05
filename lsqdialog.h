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
    QVector<int> numWeightParamsPerScheme;  // Number of parameters for each scheme (size 18)
    
    // Observations & Parameters
    int numObservations;
    double percentObservations;
    int numParameters;
    double ratio;
    
    LSQParameters()
        : refinementType(Diagonal)
        , dampingFactor(0.5)
        , reflectionsCutoff(0)
        , numCycles(5)
        , weightingSchemeIndex(0)
        , weightParameters(10, 0.0)
        , refineWeightParams(false)
        , numWeightParamsPerScheme(18, 0)
        , numObservations(0)
        , percentObservations(0.0)
        , numParameters(0)
        , ratio(0.0)
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
    QVector<int> numWeightParamsPerScheme;
    bool applyPressed;
};

#endif // LSQDIALOG_H
