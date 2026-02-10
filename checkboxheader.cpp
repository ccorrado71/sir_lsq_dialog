#include "checkboxheader.h"
#include <QPainter>
#include <QStyleOption>

#include <QDebug>

namespace {
static constexpr int margin = 4;
}

CheckBoxHeader::CheckBoxHeader(Qt::Orientation orientation, QVector<int> col, bool lAdjusted, QWidget* parent /*= 0*/)
    : QHeaderView(orientation, parent),      
      mColumn(col),
      leftAdjusted(lAdjusted)
{    
    setSectionsClickable(true);
    setMinimumSectionSize(minWidth());
    foreach(auto index, col) {
        //isChecked_[index] = true;
        isChecked_[index] = false;
    }
}

bool CheckBoxHeader::isChecked(int index) const
{
    return isChecked_.value(index);
}

QRect CheckBoxHeader::getRect(int col) const
{
    QStyleOptionButton option;

    option.rect = QRect(sectionPosition(col), 0, sectionSize(col), sizeHint().height());
    QRect checkBoxRect = style()->subElementRect(QStyle::SE_CheckBoxIndicator, &option);
    QPoint checkBoxPoint;
    if (leftAdjusted) {
        checkBoxPoint = QPoint(option.rect.x() + margin,
                      option.rect.y() + option.rect.height() / 2 - checkBoxRect.height() / 2);
    } else {
        checkBoxPoint = QPoint(option.rect.x() + option.rect.width() / 2 - checkBoxRect.width() / 2,
                      option.rect.y() + option.rect.height() / 2 - checkBoxRect.height() / 2);
    }
    option.rect = QRect(checkBoxPoint, checkBoxRect.size());
    return option.rect;
}

int CheckBoxHeader::minWidth()
{
    QStyleOptionButton option;

    QRect checkBoxRect = style()->subElementRect(QStyle::SE_CheckBoxIndicator, &option);
    int k = 3*style()->pixelMetric(QStyle::PM_HeaderMargin) + style()->pixelMetric(QStyle::PM_HeaderMarkSize) + checkBoxRect.width();
    return k;
}

void CheckBoxHeader::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{    
    if (mColumn.contains(logicalIndex))
    {
        QStyleOptionButton option;
        option.rect = getRect(logicalIndex);
        if (leftAdjusted) {
            // First paint the background as we later narrow the area the QHeaderView itself paints.
            const QVariant backgroundBrush =
                model()->headerData(logicalIndex, orientation(), Qt::BackgroundRole);

            painter->save();
            QStyleOptionHeader opt;
            initStyleOption(&opt);
            opt.rect = rect;
            if (!backgroundBrush.isNull()) {
                opt.palette.setBrush(QPalette::Button, backgroundBrush.value<QBrush>());
                opt.palette.setBrush(QPalette::Window, backgroundBrush.value<QBrush>());
            }
            style()->drawControl(QStyle::CE_Header, &opt, painter, this);
            painter->restore();

            painter->save();
            int checkBoxAreaWidth = option.rect.width() + 2 * margin;
            QHeaderView::paintSection(painter, rect.adjusted(checkBoxAreaWidth, 0, 0, 0), logicalIndex);
            painter->restore();
        } else {
            painter->save();
            QHeaderView::paintSection(painter, rect, logicalIndex);
            painter->restore();
        }

        // paint the check box
        option.state = QStyle::State_Enabled | QStyle::State_Active;

        if (isChecked_.value(logicalIndex))
        //if (model()->headerData(logicalIndex, orientation(), Qt::CheckStateRole) == Qt::Checked)
            option.state |= QStyle::State_On;
        else
            option.state |= QStyle::State_Off;
        option.state |= QStyle::State_Off;
        style()->drawControl(QStyle::CE_CheckBox, &option, painter);
    } else {
        painter->save();
        QHeaderView::paintSection(painter, rect, logicalIndex);
        painter->restore();
    }
}

void CheckBoxHeader::mousePressEvent(QMouseEvent* event)
{
    int index = logicalIndexAt(event->pos());
    QRect rect = getRect(index);
    if((mColumn.contains(index)) && rect.contains(event->pos()))
    {
        setChecked(index,!isChecked(index));
        emit checkBoxClicked(isChecked(index), index);
        return; // Corrado: this avoid to activate sort
    }
    QHeaderView::mousePressEvent(event);
}

void CheckBoxHeader::redrawCheckBox()
{
    viewport()->update();
}

void CheckBoxHeader::setChecked(int index, bool val)
{
    if (isChecked_.value(index) != val)
    {
        isChecked_[index] = val;
        redrawCheckBox();
    }
}
