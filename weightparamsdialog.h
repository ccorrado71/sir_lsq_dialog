#ifndef WEIGHTPARAMSDIALOG_H
#define WEIGHTPARAMSDIALOG_H

#include <QDialog>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class WeightParamsDialog; }
QT_END_NAMESPACE

class WeightParamsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WeightParamsDialog(const QString &formula, int numParams, QWidget *parent = nullptr);
    ~WeightParamsDialog();

    QVector<double> getParameters() const;
    void setParameters(const QVector<double> &params);

private:
    Ui::WeightParamsDialog *ui;
};

#endif // WEIGHTPARAMSDIALOG_H
