#ifndef CHECKBOXHEADER_H
#define CHECKBOXHEADER_H

#include <QHeaderView>
#include <QPainter>
#include <QMouseEvent>

class CheckBoxHeader : public QHeaderView
{
    Q_OBJECT

public:
    CheckBoxHeader(Qt::Orientation orientation, QVector<int> col, bool lAdjusted, QWidget* parent = 0);    
    bool isChecked(int index) const;
    void setChecked(int index, bool val);

signals:
    void checkBoxClicked(bool state, int index);

protected:
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const;
    void mousePressEvent(QMouseEvent* event);

private:    
    QMap<int,bool> isChecked_;
    QVector<int>  mColumn;
    bool leftAdjusted;
    QRect getRect(int col) const;
    void redrawCheckBox();
    int  minWidth();
};
#endif
