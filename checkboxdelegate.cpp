#include "checkboxdelegate.h"

#include <QKeyEvent>
#include <QMouseEvent>

CheckBoxDelegate::CheckBoxDelegate(QObject *parent )
    : QItemDelegate(parent),
      checkBoxColumnIx(0)
{    
}

//void CheckBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
//{
//    if (index.column() == checkBoxColumnIx)
//    {
//        QWidget *w = dynamic_cast<QWidget *>(painter->device());
//        if (w)
//        {
//            QItemDelegate::drawBackground( painter, option, index );
//
//            drawCheck(painter, option, QRect(option.rect.left()+(option.rect.width()-15)/2,option.rect.top()+(option.rect.height()-15)/2,15,15),
//                      index.data(Qt::EditRole).toBool() ? Qt::Checked : Qt::Unchecked );
//            drawFocus(painter, option, option.rect);
//        }
//        else
//        {
//            QItemDelegate::paint(painter, option, index);
//        }
//    }
//}

void CheckBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == checkBoxColumnIx)
    {
        QWidget *w = dynamic_cast<QWidget *>(painter->device());
        if (w)
        {
            // Save current painter state
            painter->save();

            // Create a copy of the option that we can modify
            QStyleOptionViewItem opt = option;
            opt.initFrom(w);

            // Draw the background using the correct style
            QStyle *style = w->style();
            style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, w);

            // Get the size of a checkbox from the style
            QStyleOptionButton checkBoxStyleOption;
            QRect checkBoxRect = style->subElementRect(QStyle::SE_CheckBoxIndicator, &checkBoxStyleOption);

            // Calculate the centered position for the checkbox in the cell
            QRect cellRect = option.rect;
            int x = cellRect.x() + (cellRect.width() - checkBoxRect.width()) / 2;
            int y = cellRect.y() + (cellRect.height() - checkBoxRect.height()) / 2;

            // Set up the checkbox style option
            checkBoxStyleOption.rect = QRect(x, y, checkBoxRect.width(), checkBoxRect.height());
            checkBoxStyleOption.state = QStyle::State_Enabled;

            if (index.data(Qt::EditRole).toBool())
                checkBoxStyleOption.state |= QStyle::State_On;
            else
                checkBoxStyleOption.state |= QStyle::State_Off;

            if (option.state & QStyle::State_Selected)
                checkBoxStyleOption.state |= QStyle::State_Selected;

            if (option.state & QStyle::State_HasFocus)
                checkBoxStyleOption.state |= QStyle::State_HasFocus;

            // Draw the checkbox
            style->drawControl(QStyle::CE_CheckBox, &checkBoxStyleOption, painter, w);

            // Restore painter state
            painter->restore();
        }
        else
        {
            QItemDelegate::paint(painter, option, index);
        }
    }
    else
    {
        QItemDelegate::paint(painter, option, index);
    }
}

QWidget *CheckBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == checkBoxColumnIx)
    {
        QCheckBox* chb = new QCheckBox(parent);
        return chb;
    }

    return QItemDelegate::createEditor(parent, option, index);
}

void CheckBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.column() == checkBoxColumnIx)
    {
        int checked = index.model()->data(index, Qt::DisplayRole).toInt();
        QCheckBox* chb = qobject_cast<QCheckBox*>(editor);
        chb->setChecked(checked == 1);
    } else
        QItemDelegate::setEditorData(editor, index);
}

void CheckBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (index.column() == checkBoxColumnIx)
    {
        QCheckBox* chb = qobject_cast<QCheckBox*>(editor);
        int value = chb->isChecked() ? 1 : 0;
        model->setData(index, value);
    }
    else
    {
        QItemDelegate::setModelData(editor, model, index);
    }
}

void CheckBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(QRect(option.rect.left()+(option.rect.width()-15)/2,option.rect.top()+(option.rect.height()-15)/2,15,15));
}

void CheckBoxDelegate::commitAndCloseEditor()
{
    QCheckBox* editor = qobject_cast<QCheckBox*>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}

void CheckBoxDelegate::setColumn(int value)
{    
    checkBoxColumnIx = value;
}

bool CheckBoxDelegate::editorEvent(QEvent * event, QAbstractItemModel * model,
                                   const QStyleOptionViewItem & option, const QModelIndex & index)
{
    Q_UNUSED(option);

    // This makes not checkable when disabled
    if (!index.flags().testFlags(Qt::ItemIsEnabled)) return false;

    if ((event->type() == QEvent::MouseButtonRelease) ||
            (event->type() == QEvent::MouseButtonDblClick))
    {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() != Qt::LeftButton)
        {
            return false;
        }
        if (event->type() == QEvent::MouseButtonDblClick)
        {
            return true;
        }

        // Check if the click is within the checkbox area
        //QRect checkBoxRect = QRect(option.rect.left() + (option.rect.width() - 15) / 2,
        //                           option.rect.top() + (option.rect.height() - 15) / 2,
        //                           15, 15);

        //if (!checkBoxRect.contains(mouseEvent->pos()))
        //{
        //    return false;
        //}

        // Get checkbox rect from style
        QStyleOptionButton checkBoxStyleOption;
        QRect checkBoxRect = option.widget->style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkBoxStyleOption);

        // Calculate checkbox position in cell
        QRect cellRect = option.rect;
        int x = cellRect.x() + (cellRect.width() - checkBoxRect.width()) / 2;
        int y = cellRect.y() + (cellRect.height() - checkBoxRect.height()) / 2;
        QRect checkRect(x, y, checkBoxRect.width(), checkBoxRect.height());

        if (!checkRect.contains(mouseEvent->pos()))
            return false;

    }
    else if (event->type() == QEvent::KeyPress)
    {
        if (static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space &&
            static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select)
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    //old code
    //    if (index.column()==checkBoxColumnIx && event->type()==QEvent::MouseButtonPress){

    //        if(index.data(Qt::DisplayRole).toInt()==0)
    //        {
    //            model->setData(index,1);
    //            emit rowChecked(index, true);
    //        }  else {
    //            model->setData(index,0);
    //            emit rowChecked(index, false);
    //        }

    //        return true;
    //    }
    //    return false;

    // Now process here QEvent::MouseButtonPress
    bool checked = index.model()->data(index, Qt::DisplayRole).toBool();
    model->setData(index, !checked, Qt::EditRole);
    emit rowChecked(index, !checked);
    return true;
}
